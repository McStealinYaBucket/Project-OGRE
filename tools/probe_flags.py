"""Flag probes per engine library (Phase 3 steps 2-3): compile tools/flagprobes/<lib>.cpp with VC9 under a
base flag set and under one-flag flips, and byte-compare each probed function with the target.

Run:
    python3 -P tools/probe_flags.py                 # every probe, base flags + every flip
    python3 -P tools/probe_flags.py LIB [LIB ...]   # selected probes (first column of probes.txt)
    python3 -P tools/probe_flags.py LIB -v          # masked diffs for the base flags
    python3 -P tools/probe_flags.py LIB --flags "/O2 ..."   # one explicit flag set

Each probe source (tools/flagprobes/<Lib>.cpp) is a small TU with minimal local declarations (layouts
taken from the target's own stores); tools/flagprobes/probes.txt lists, per probe, the target, the winning
flag set and the probed symbol@address pairs ("Lib.STUDIO" reuses Lib.cpp against STUDIO addresses).
Symbols match exactly or by the start of their decorated name ("name@Class@RBX" matches "?name@Class@RBX@@..").
The G3D 8.00 headers are prepared in the scratch dir as tools/lib/g3d_match.py does (prior tree + staged
patches); boost 1.53 comes from 3rdparty/_downloads. A flip replaces one token of the base set.
Objects go to a scratch dir (mktemp under /tmp/claude-1000); nothing is written in the repo.
The comparison is tools/probe_compare.py's (relocations masked on both sides).
"""
import argparse
import csv
import os
import glob
import shlex
import shutil
import subprocess
import sys
import tempfile

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import probe_compare as pc  # noqa: E402

CL = os.path.join(ROOT, "tools/msvc9/cl")
COMMON = "/nologo /c /TP /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /W0"
INCLUDES = ["/I" + os.path.join(ROOT, "3rdparty/_downloads/boost_1_53_0/boost_1_53_0")]


def prepare_g3d(tmp):
    """G3D 8.00 headers as tools/lib/g3d_match.py builds them (prior tree's copy + staged patches), in tmp."""
    dst = os.path.join(tmp, "g3d")
    for sub in ("G3Dcpp", "include"):
        shutil.copytree(os.path.join(ROOT, "2013 55% App/Rendering/g3d", sub), os.path.join(dst, sub),
                        copy_function=shutil.copyfile)
    for p in sorted(glob.glob(os.path.join(ROOT, "staging/g3d-8.00/G3D/patches/*.patch"))):
        subprocess.run(["patch", "-p1", "-s", "-d", dst, "-i", p], check=True)
    os.makedirs(os.path.join(tmp, "compat"))
    open(os.path.join(tmp, "compat/stdint.h"), "w").write(
        "#pragma once\ntypedef signed char int8_t; typedef unsigned char uint8_t; typedef short int16_t;\n"
        "typedef unsigned short uint16_t; typedef int int32_t; typedef unsigned int uint32_t;\n"
        "typedef __int64 int64_t; typedef unsigned __int64 uint64_t;\n")
    INCLUDES.extend(["/I" + os.path.join(dst, "include"), "/I" + os.path.join(tmp, "compat")])
APP = "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast"
TARGETS = {"PLAYER": ("bin/player_1023/RobloxPlayerBeta.exe", "ida/exports/PLAYER-functions.csv"),
           "STUDIO": ("bin/studio/RobloxStudioBeta.exe", "ida/exports/STUDIO-functions.csv"),
           "LOG": ("bin/player/Log.dll", "ida/exports/LOG_DLL-functions.csv")}
FLIPS = [("/D_SECURE_SCL=0", "/D_SECURE_SCL=1"), ("/Ob2", "/Ob0"), ("/Oy", "/Oy-"), ("/Oy-", "/Oy"), ("/GS-", "/GS"), ("/GS", "/GS-"), ("/arch:SSE", "/arch:SSE2"),
         ("/arch:SSE", ""), ("/fp:fast", "/fp:precise"), ("/fp:precise", "/fp:fast"), ("/O2", "/arch:SSE /O2"), ("/Ob2", "/Ob1"), ("/O2", "/O1"), ("/Od", "/O2"),
         ("/Oi", "/Oi-")]


def probes():
    """PROBES live in tools/flagprobes/probes.txt: 'name target "flags" sym@addr ...' per line."""
    out = {}
    for line in open(os.path.join(ROOT, "tools/flagprobes/probes.txt")):
        if line.strip() and not line.startswith("#"):
            p = shlex.split(line)
            out[p[0]] = (p[1], p[2], [tuple(x.rsplit("@", 1)) for x in p[3:]])
    return out


def find(secs, syms, want):
    for s in syms.values():
        if s[2] > 0 and s[3] & 0x20 and (s[0] == want or s[0].startswith("?" + want + "@")):
            return pc.find_func(secs, syms, s[0])
    sys.exit(f"{want}: no symbol")


def compare(obj, pairs, target, verbose):
    exe, fcsv = TARGETS[target]
    sizes = {int(r["address"], 16): int(r["size"]) for r in csv.DictReader(open(os.path.join(ROOT, fcsv)))}
    pe = pefile.PE(os.path.join(ROOT, exe), fast_load=True)
    img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    secs, syms = pc.read_obj(obj)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    res = []
    for want, addr_s in pairs:
        addr = int(addr_s, 16)
        nm, mine, relocs = find(secs, syms, want)
        mine = pc.trim(mine)
        theirs = pc.trim(img[addr - base:addr - base + max(sizes.get(addr, 0), len(mine))])
        mask = {r[0] + k for r in relocs for k in range(4)}
        ok = len(mine) == len(theirs) and all(i in mask or mine[i] == theirs[i] for i in range(len(mine)))
        res.append((want, addr, ok, len(mine), len(theirs)))
        if verbose and not ok:
            print(f"  -- {want} 0x{addr:08x}")
            import difflib
            for line in difflib.unified_diff(pc.masked_text(md, mine, mask, 0),
                                             pc.masked_text(md, theirs, mask if len(mine) == len(theirs) else set(), 0),
                                             "obj", "target", lineterm="", n=2):
                print("    " + line)
    return res


def run(name, spec, flags, tmp, verbose=False):
    target, _, pairs = spec
    src = os.path.join(ROOT, "tools/flagprobes", name.split(".")[0] + ".cpp")  # "Lib.STUDIO" reuses Lib.cpp
    obj = os.path.join(tmp, f"{name}.obj")
    if os.path.exists(obj):
        os.remove(obj)
    r = subprocess.run([CL, *COMMON.split(), *INCLUDES, *flags.split(), src, "/Fo" + obj], capture_output=True, text=True)
    if not os.path.exists(obj):
        print(r.stdout[-2000:], r.stderr[-2000:])
        return None
    return compare(obj, pairs, target, verbose)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("libs", nargs="*")
    ap.add_argument("--flags")
    ap.add_argument("-v", action="store_true")
    a = ap.parse_args()
    allp = probes()
    tmp = tempfile.mkdtemp(prefix="ogre-flags-", dir="/tmp/claude-1000")
    prepare_g3d(tmp)
    for name in a.libs or allp:
        spec = allp[name]
        base = a.flags or spec[1]
        res = run(name, spec, base, tmp, a.v)
        if res is None:
            continue
        n = sum(r[2] for r in res)
        print(f"== {name} ({spec[0]}) base [{base}]: {n}/{len(res)} identical")
        for want, addr, ok, l1, l2 in res:
            print(f"   {'IDENTICAL' if ok else 'different'}  0x{addr:08x}  {l1:4}/{l2:<4} B  {want}")
        if a.flags:
            continue
        for old, new in FLIPS:
            toks = base.split()
            if old not in toks or (old == "/O2" and "/arch" in new and any(t.startswith("/arch") for t in toks)):
                continue
            fl = " ".join(new if t == old else t for t in toks if not (new == "" and t == old))
            r2 = run(name, spec, fl, tmp)
            if r2 is not None:
                print(f"   flip {old} -> {new or '(none)'}: {sum(r[2] for r in r2)}/{len(r2)}"
                      + "".join(f"  lost 0x{x[1]:08x}" for x, y in zip(res, r2) if x[2] and not y[2])
                      + "".join(f"  gained 0x{y[1]:08x}" for x, y in zip(res, r2) if y[2] and not x[2]))


if __name__ == "__main__":
    main()

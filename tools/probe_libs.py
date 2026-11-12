"""Per-library codegen census over PLAYER / STUDIO / Log.dll (the cheap half of the flag proof, Phase 3 step 2).

Run:
    python3 -P tools/probe_libs.py [--target PLAYER|STUDIO|LOG|both] [--range LO-HI ...] [--list LIB]

For every library range in RANGES (bounded by the .cpp __FILE__ assert strings each library's code refers
to), it counts over the Roblox functions in the range (third-party bodies dropped: names tagged
match:/lib:/flirt: and every address in ida/exports/<T>-lib-resolved.csv; functions < 8 bytes dropped):
  frame    starts push ebp / mov ebp,esp                    /Oy- gives ~all (and /Od), /Oy a few
  alnfr    ebp frame followed by and esp,-N in the prologue (double or __m128 locals, or /GL)
  cookie   reads ___security_cookie                         /GS: under /GS every C++ EH frame reads it too,
                                                            so cookie >= eh means /GS and cookie = 0 means /GS-
  eh       reads fs:[0] or calls __EH_prolog*               C++ EH frames present
  al16     share of 16-byte aligned starts
  sse1 / sse2 / p6x87 / fnstsw   instruction counts         /arch:SSE vs SSE2 vs none (as tools/arch_census.py)
  plainfr  frame share among 'plain' functions (no EH frame, no and-esp alignment, no SEH): the clean /Oy tell
  thisless / members   non-virtual thiscall members (iOS/Mac _ZNK const members, C1/C2/D1/D2 ctors/dtors, MSVC
           non-virtual __thiscall names), not in a vftable, whose entry code never reads ecx. Measured: does
           not separate /GL from non-/GL (too few named members in the shells); kept for the record.
  ltcg8    and esp,-8 frames in functions that touch no qword float data. App (no /GL) has 22 in 19,302;
           the /GL client shells 166/2,596 (PLAYER), 401/5,244 (STUDIO), Log.dll 22/290.
  regarg   entry code (up to the first branch) reads eax/ebx/esi/edi before writing them: a custom register
           convention. Without /GL VC9 gives one only to file-static functions (Lua: pushstr, index2adr);
           under /GL also to template/member/extern functions. Engine libraries 0-4%, /GL shells 10-28%.
--list LIB prints the per-function rows of one library (address, size, flags, name).
"""
import argparse
import bisect
import collections
import csv
import json
import os
import re

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe",
       "LOG": "bin/player/Log.dll"}
# Log.dll has no names export: functions from ida/exports/LOG_DLL-functions.csv, cookie address read from the
# /GS prologue of 0x100010d0 (mov eax,[0x1001101c]; xor eax,esp).
LOG_COOKIE = 0x1001101C
# (library, lo, hi): lo/hi are function starts bracketing the library's own .cpp assert anchors
# (first/last anchor named in the comment). Gaps between libraries hold third-party code.
RANGES = {
    "PLAYER": [
        ("WindowsClient+Win+ClientBase", 0x401000, 0x46c000),  # Win\DSVideoCaptureEngine 0x430f90 .. WindowsClient\WebBrowserAxDialog 0x46b0b0
        ("WindowsClient-/GL-tail", 0x46c000, 0x480700),         # program_options (/GL, 0x00478330) + its STL/date_time COMDATs
        ("RbxOgre", 0x480700, 0x51f6b0),                       # ViewRbxGfx 0x4815b0 .. istreamDataStream 0x51f290
        ("Network", 0x51f6b0, 0x6091f8),                       # Network\Player 0x5276b0 .. Replicator.GCJob 0x6063b0
        ("GfxAdapters", 0x609330, 0x612550),                   # SceneUpdater 0x60d020 .. PartIdentifier 0x612170
        ("App", 0x613530, 0x9ad000),                           # Name::setOrderIndex 0x613530 .. AdvLuaDragTool 0x9aa4d0
        ("DeviceCaps", 0x9ad000, 0x9b1000),                    # devicecaps\dxut\DXUTmisc.h 0x9ad3a0 .. 0x9b08d0
        ("Base", 0x9b1000, 0x9bd680),                          # TaskScheduler 0x9b37c0 .. MathUtil 0x9bc640, SimpleJSON 0x9bd200
        ("AppDraw", 0x9c1000, 0x9c5e10),                       # DrawAdorn 0x9c24b0 .. 0x9c3a80
        ("GfxBase", 0x9c5e10, 0x9d5b70),                       # RenderSettings 0x9c5e10 .. AdornBillboarder 0x9d5a80
        ("RbxG3D", 0x9ee000, 0x9eed6e),                        # Frustum 0x9ee010 (after libzip, before FMOD thunks)
    ],
    "STUDIO": [
        ("RobloxStudio+ClientShared+Win+ClientBase", 0x401000, 0x514000),  # AuthoringSettings 0x405a10 .. moc_WebDialog 0x50ed50
        ("RbxOgre", 0x514000, 0x5b7700),                       # ViewRbxGfx 0x514ab0 .. istreamDataStream 0x5b7590
        ("Network", 0x5b7700, 0x69f000),                       # Players 0x5ccb00 .. DS_HuffmanEncodingTree 0x69e530
        ("GfxAdapters", 0x6a3000, 0x6a9000),                   # SceneUpdater 0x6a37a0 .. PartIdentifier 0x6a8750
        ("RbxG3D", 0x6a9000, 0x6aa800),                        # Frustum 0x6a9d50
        ("GfxBase", 0x6c2000, 0x6c8000),                       # RenderSettings 0x6c21e0 .. IAdornableCollector 0x6c72e0
        ("App", 0x6cb000, 0xa62000),                           # 0x6cb2c0 .. 0xa61a20
        ("Base", 0xa6a000, 0xa77000),                          # CEvent 0xa6aef0 .. RegistryUtil 0xa769d0
        ("AppDraw", 0xa79000, 0xa7c000),                       # DrawAdorn 0xa79c90 .. 0xa7b260
        ("GfxBase-FileMeshData", 0xa99000, 0xa9a800),          # FileMeshData 0xa99b20
    ],
    "LOG": [("Log.dll", 0x10001000, 0x10010000)],
}
FAM = {**dict.fromkeys("addsd subsd mulsd divsd sqrtsd cvtss2sd cvtsd2ss cvttsd2si cvtsi2sd ucomisd comisd movsd".split(), "sse2"),
       **dict.fromkeys("addss subss mulss divss sqrtss comiss ucomiss cvttss2si cvtsi2ss".split(), "sse1"),
       **dict.fromkeys("fucomip fcomip fucomi fcomi fcmovb fcmove fcmovbe fcmovu fcmovnb fcmovne".split(), "p6x87"),
       "fnstsw": "fnstsw"}
THIRD = ("match:", "lib:", "flirt:")
REG32 = {"al": "eax", "ah": "eax", "ax": "eax", "bl": "ebx", "bh": "ebx", "bx": "ebx", "si": "esi", "di": "edi"}


def load(t):
    if t == "LOG":
        funcs = sorted((int(r["address"], 16), int(r["size"])) for r in csv.DictReader(open(f"{ROOT}/ida/exports/LOG_DLL-functions.csv")))
        return {}, funcs, set(), LOG_COOKIE, set(), set()
    names = {int(r["address"], 16): r for r in csv.DictReader(open(f"{ROOT}/ida/exports/{t}-names.csv")) if r["kind"] == "func"}
    funcs = sorted((int(r["address"], 16), int(r["size"])) for r in csv.DictReader(open(f"{ROOT}/ida/exports/{t}-functions.csv")))
    lib = {int(r["address"], 16) for r in csv.DictReader(open(f"{ROOT}/ida/exports/{t}-lib-resolved.csv"))}
    cookie = next(a for a, r in ((int(r["address"], 16), r) for r in csv.DictReader(open(f"{ROOT}/ida/exports/{t}-names.csv")))
                  if r["name"] == "___security_cookie")
    ehp = {a for a, r in names.items() if r["name"].startswith("__EH_prolog")}
    slots = {sl["ea"] for line in open(f"{ROOT}/ida/exports/{t}-evidence/vtables.jsonl")
             for g in json.loads(line).get("groups", []) for sl in g.get("slots", [])}
    members = set()
    for ad, r in names.items():
        nm, dm = r["name"], r["demangled"] or ""
        if ad in slots:
            continue
        if r["source"].startswith(("ios:", "mac:")) and nm.startswith("_ZN") and (
                nm.startswith("_ZNK") or re.search(r"[^0-9A-Za-z_](C[12]|D[12])E", nm)):
            members.add(ad)
        elif "__thiscall" in dm and "virtual" not in dm and "`" not in dm:
            members.add(ad)
    return names, funcs, lib, cookie, ehp, members


def uses_ecx_this(md, code, addr):
    """True if entry code (to the first branch) reads ecx before writing it ('push ecx' is a stack slot)."""
    for ins in md.disasm(code[:96], addr):
        if ins.mnemonic == "push" and ins.op_str == "ecx":
            continue
        rd, wr = ins.regs_access()
        rd = {ins.reg_name(x) for x in rd}
        wr = {ins.reg_name(x) for x in wr}
        if ins.mnemonic in ("xor", "sub") and ins.op_str in ("ecx, ecx",):
            return False
        if "ecx" in rd or "cx" in rd or "cl" in rd:
            return True
        if "ecx" in wr:
            return False
        if ins.group(capstone.CS_GRP_JUMP) or ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_RET):
            return False
    return False


def features(md, img, base, addr, size, cookie, ehp):
    f = collections.Counter()
    code = img[addr - base:addr - base + size]
    f["frame"] = code[:3] == b"\x55\x8b\xec"
    written = set()
    f["seh"] = code[3:5] == b"\x6a\xfe" or (code[3:5] == b"\x6a\xff" and code[10:11] == b"\x68")
    for k, ins in enumerate(md.disasm(code, addr)):
        m = ins.mnemonic
        if k < 8 and f["frame"] and m == "and" and ins.op_str.startswith("esp, 0xfffffff"):
            f["alnfr"] = 1
            f["aln8"] = ins.op_str == "esp, 0xfffffff8"
        if "qword ptr" in ins.op_str and (m.startswith("f") or m in ("movsd", "movq")):
            f["dbl"] = 1
        if f"0x{cookie:x}" in ins.op_str:
            f["cookie"] = 1
        if m == "mov" and "fs:[0]" in ins.op_str:
            f["eh"] = 1
        fam = FAM.get(m)
        if fam:
            f[fam] += 1
        if m == "call" and ins.op_str.startswith("0x") and int(ins.op_str, 16) in ehp:
            f["eh"] = 1
        if not f["regdone"]:
            if ins.group(capstone.CS_GRP_JUMP) or ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_RET):
                f["regdone"] = 1
            else:
                rd, wr = ins.regs_access()
                rd = {REG32.get(ins.reg_name(x), ins.reg_name(x)) for x in rd}
                if m == "push" or (m in ("xor", "sub") and len(set(ins.op_str.split(", "))) == 1):
                    rd = set()
                if any(r in rd and r not in written for r in ("eax", "ebx", "esi", "edi")):
                    f["regarg"], f["regdone"] = 1, 1
                written |= {REG32.get(ins.reg_name(x), ins.reg_name(x)) for x in wr}
    return f


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--target", default="both")
    ap.add_argument("--range", nargs="*", default=[])
    ap.add_argument("--list")
    a = ap.parse_args()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    for t in (["PLAYER", "STUDIO", "LOG"] if a.target == "both" else [a.target]):
        pe = pefile.PE(os.path.join(ROOT, EXE[t]), fast_load=True)
        img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
        names, funcs, lib, cookie, ehp, members = load(t)
        ranges = RANGES[t] + [(r, int(r.split("-")[0], 16), int(r.split("-")[1], 16)) for r in a.range]
        starts = [x[0] for x in funcs]
        print(f"== {t}  (cookie at 0x{cookie:08x})")
        print(f"{'library':42} {'n':>5} {'frame':>6} {'alnfr':>5} {'cookie':>6} {'eh':>5} {'al16':>5} {'sse1':>6} {'sse2':>5} {'p6x87':>5} {'fnstsw':>6} {'plainfr':>9} {'thisless':>12} {'ltcg8':>6} {'regarg':>7}")
        for libname, lo, hi in ranges:
            tot, n, al16 = collections.Counter(), 0, 0
            for addr, size in funcs[bisect.bisect_left(starts, lo):bisect.bisect_left(starts, hi)]:
                r = names.get(addr)
                if size < 8 or addr in lib or (r and r["source"].startswith(THIRD)):
                    continue
                f = features(md, img, base, addr, size, cookie, ehp)
                n += 1
                al16 += addr % 16 == 0
                for k in ("frame", "alnfr", "cookie", "eh"):
                    tot[k] += bool(f[k])
                for k in ("sse1", "sse2", "p6x87", "fnstsw"):
                    tot[k] += f[k]
                tot["ltcg8"] += bool(f["aln8"] and not f["dbl"])
                tot["regarg"] += bool(f["regarg"])
                if not (f["eh"] or f["alnfr"] or f["seh"]):
                    tot["plain"] += 1
                    tot["plainfr"] += bool(f["frame"])
                if addr in members:
                    tot["members"] += 1
                    f["thisless"] = not uses_ecx_this(md, img[addr - base:addr - base + size], addr)
                    tot["thisless"] += f["thisless"]
                if a.list == libname:
                    flags = ",".join(k for k in ("frame", "alnfr", "cookie", "eh", "thisless") if f[k])
                    print(f"   0x{addr:08x} {size:6} {flags:24} {(r['demangled'] or r['name'])[:90] if r else ''}")
            pct = lambda k: f"{100 * tot[k] / max(n, 1):5.1f}%"
            print(f"{libname:42} {n:5} {pct('frame'):>6} {tot['alnfr']:5} {tot['cookie']:6} {tot['eh']:5} {100*al16/max(n,1):4.0f}% "
                  f"{tot['sse1']:6} {tot['sse2']:5} {tot['p6x87']:5} {tot['fnstsw']:6} {tot['plainfr']:4}/{tot['plain']:<4} {tot['thisless']:5}/{tot['members']:<5} {tot['ltcg8']:6} {100 * tot['regarg'] / max(n, 1):6.1f}%")


if __name__ == "__main__":
    main()

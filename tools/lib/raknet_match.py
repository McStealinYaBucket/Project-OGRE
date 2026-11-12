"""Compile RakNet (closest pristine upstream release + the 2013 Roblox patches) with VC9 SP1 and
locate/compare every function it emits against PLAYER and STUDIO.

Run (from anywhere, about two minutes; compiles in parallel):
    python3 -P tools/lib/raknet_match.py                    # build, match, write ida/exports/PLAYER-lib-raknet.csv
    python3 -P tools/lib/raknet_match.py --no-patches       # pristine RakNet 4.034 only (baseline)
    python3 -P tools/lib/raknet_match.py --flags "/O2 ..."  # probe another flag set (default FLAGS below)
    python3 -P tools/lib/raknet_match.py --no-write --targets PLAYER -v RakNet::RakPeer::Receive
                                                            # also print masked diffs for these qualified names
    python3 -P tools/lib/raknet_match.py --keep DIR         # build in DIR and keep it (default: mktemp dir)
    python3 -P tools/lib/raknet_match.py --keep DIR --no-compile   # re-match the .obj files already in DIR

Source: staging/raknet/RakNet-4.034/Source, the Source/ folder of RakNet_PC-4.034.zip unmodified
(jenkinssoftware.com/raknet/downloads, fetched from web.archive.org snapshot 20240406103304; its sha1
(base32 IT6UDU2IUL7PCY2LMKHBMHQKM2DWNPZW) equals the digest the archive recorded, sha256
64e00e71025bd5eada890c28f9fda81aacaf702d4a5722e6792d75e022c3edac). 4.034 is the newest PC source
release that can be fetched: the archived 4.035 and 4.037 zips are console (PS3/Vita) builds and are
password protected, and the next archived PC release is 4.067. Then every staging/raknet/patches/*.patch
is applied in name order (patch -p1 inside a scratch copy); each patch says which PLAYER bytes justify it.

Every Source/*.cpp is compiled with tools/msvc9/cl (a TU that fails to compile is reported and skipped).
Flags: FLAGS + COMMON below (the App set proven by the Lua probe; the --flags probes are in the run report).

Per target:
  0. TU presence. Templates and tiny helpers fold under /OPT:ICF and are emitted by many objects, so a
     TU counts as linked only when a non-template, non-compiler-generated function that only it
     defines is identical at an address no other compiled function lands on (a folded body proves
     nothing about which object supplied it), located by name or by an identical caller's
     relocation, or by a unique masked-body hit if it is >= 32 bytes. Functions of other TUs are dropped. A COMDAT emitted by several linked
     objects is kept once; its obj column is the defining TU whose located span holds its address.
  1. search  (4) masked .obj body (relocated bytes masked) occurs exactly once, 16-aligned, in .text.
  2. name    (3) qualified name and normalized parameter list equal an ios:/mac: row of <T>-names.csv.
  3. reloc   (5) a located identical function's relocation names the address; (2) the same from a
                 non-identical located function after instruction alignment, accepted only when the
                 callee is at least 50% similar there.
  4. order   (1) with /Gy the linker keeps a TU's COMDATs in .obj order, so unlocated functions between
                 two located neighbours pair, in order, with the unclaimed target functions between
                 their addresses (best masked-instruction similarity, floor 0.45).
  A stronger step overrides a weaker one. Target functions inside a linked TU's span (the run, without
  0x1000-byte holes, of its own non-template identical functions) that nothing claims get their own rows
  (identical=no, masked_diff_bytes empty).
A non-identical location where more than half of the .obj function's bytes differ is rejected (printed
as a note): such a function is not linked, or its address came from a folded or generic caller.
identical = yes only if the masked bytes are equal over the whole .obj function length and IDA's size
is not longer. masked_diff_bytes = unmasked differing bytes + any extra target length.
"""
import argparse
import bisect
import concurrent.futures
import csv
import difflib
import glob
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from collections import defaultdict

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "tools/lib"))
from probe_compare import read_obj  # noqa: E402
import lua_match as LM  # noqa: E402  (const_diff: referenced constants compared by value)

STAGE = os.path.join(ROOT, "staging/raknet")
SRC = os.path.join(STAGE, "RakNet-4.034/Source")
CL = os.path.join(ROOT, "tools/msvc9/cl")
VERSION = "4.036+roblox-2013"  # built as 4.034 + staging/raknet/patches (0001 = the 4.035/4.036 changes)
FLAGS = "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast"
COMMON = ("/nologo /c /TP /W0 /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_MBCS "
          "/D_CRT_SECURE_NO_WARNINGS").split()
LOG_INC = os.path.join(ROOT, "2013 55% App/Log/include")  # FastLog.h, needed by the Roblox patch (0002)
TARGETS = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
REL32 = 0x14
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def build(d, patches, flags, src=SRC, inc=(), jobs=8):
    shutil.copytree(src, d, dirs_exist_ok=True)
    for p in patches:
        r = subprocess.run(["patch", "-p1", "-s", "-d", d, "-i", p], capture_output=True, text=True)
        if r.returncode:
            sys.exit(f"{p}: {r.stdout}{r.stderr}")
    srcs = sorted(f[:-4] for f in os.listdir(d) if f.endswith(".cpp"))

    def one(tu):
        obj = os.path.join(d, tu + ".obj")
        if os.path.exists(obj):
            os.remove(obj)
        r = subprocess.run([CL] + COMMON + flags.split() + ["/I" + d, "/I" + LOG_INC] + ["/I" + i for i in inc] + ["/Fo" + obj, os.path.join(d, tu + ".cpp")],
                           capture_output=True, text=True, timeout=900)
        ok = r.returncode == 0 and os.path.exists(obj)
        return tu, ok, "" if ok else "\n".join(l for l in r.stdout.splitlines() if "error" in l)[:400]

    good = []
    with concurrent.futures.ThreadPoolExecutor(jobs) as ex:
        for tu, ok, err in ex.map(one, srcs):
            if ok:
                good.append(tu)
            else:
                print(f"compile failed, skipped: {tu}.cpp {err}")
    return good


def obj_funcs(path, tu):
    """Functions of one /Gy object in section order: dicts with name, code, relocs, mask."""
    secs, syms = read_obj(path)
    out = []
    for nm, value, secno, typ, cls in sorted(syms.values(), key=lambda s: (s[2], s[1])):
        if secno <= 0 or not typ & 0x20 or nm.startswith(("__unwindfunclet$", "__ehhandler$", "__catch$")):
            continue  # EH funclets are not functions in the reccmp sense
        sec = secs[secno - 1]
        ends = [s[1] for s in syms.values() if s[2] == secno and s[1] > value and s[3] & 0x20]
        end = min(ends) if ends else len(sec["data"])
        code = sec["data"][value:end].rstrip(b"\xcc\x90") or sec["data"][value:end]
        relocs = [(o - value, syms[si][0], t) for o, si, t in sec["relocs"] if value <= o < value + len(code)]
        out.append(dict(tu=tu, name=nm, static=cls == 3, code=code, relocs=relocs,
                        mask={r[0] + k for r in relocs for k in range(4)}))
    return out


def undname(names):
    """llvm-undname prints each input line, its demangling (or an error) and a blank line."""
    r = subprocess.run(["llvm-undname"], input="\n".join(names) + "\n", capture_output=True, text=True)
    dem = {}
    for block in r.stdout.split("\n\n"):
        ls = block.strip("\n").splitlines()
        if len(ls) >= 2 and not ls[1].startswith("error"):
            dem[ls[0]] = ls[1]
    return dem


def qualname(dem):
    """'public: virtual void __thiscall RakNet::BitStream::Write(...)' -> 'RakNet::BitStream::Write'
    (the token before the parameter list, which is the first '(' outside template brackets)."""
    depth, cut = 0, len(dem)
    for i, ch in enumerate(dem):
        depth += {"<": 1, ">": -1}.get(ch, 0)
        if ch == "(" and depth == 0 and not dem[:i].endswith("operator"):
            cut = i
            break
    head = re.sub(r"`[^']*'", lambda m: m.group(0).replace(" ", "\x01"), dem[:cut]).replace("operator ", "operator\x01")
    depth, toks, cur = 0, [], ""
    for ch in head:
        depth += {"<": 1, ">": -1}.get(ch, 0)
        if ch == " " and depth == 0:
            toks.append(cur)
            cur = ""
        else:
            cur += ch
    return (cur or (toks[-1] if toks else dem)).replace("\x01", " ")


def params(sig):
    """Normalized parameter list of an MSVC or Itanium demangled signature, for overload matching."""
    i = sig.find("(")
    if i < 0:
        return None
    depth, j = 0, i
    for j in range(i, len(sig)):
        depth += {"(": 1, ")": -1}.get(sig[j], 0)
        if depth == 0:
            break
    p = sig[i + 1:j]
    p = re.sub(r"\b(?:struct|class|enum|union|__cdecl|__thiscall|__stdcall|__ptr32)\b", "", p)
    p = p.replace("unsigned __int64", "unsigned long long").replace("__int64", "long long")
    p = re.sub(r"\s+", "", p)
    return "" if p == "void" else p


def tokens(code, mask):
    out = []
    for ins in MD.disasm(code, 0):
        masked = any(ins.address + k in mask for k in range(ins.size))
        out.append((ins.address, ins.size, ins.mnemonic + ("*" if masked else "")))
    return out


class Target:
    def __init__(self, name, cmap=None):
        self.name = name
        self.cmap = cmap or {}  # LM.consts() of our objects: referenced strings/floats compared by value
        pe = pefile.PE(os.path.join(ROOT, TARGETS[name]), fast_load=True)
        self.img, self.base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
        t = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
        self.t0, self.t1 = self.base + t.VirtualAddress, self.base + t.VirtualAddress + t.Misc_VirtualSize
        self.text = self.img[t.VirtualAddress:t.VirtualAddress + t.Misc_VirtualSize]
        rows = csv.DictReader(open(os.path.join(ROOT, f"ida/exports/{name}-functions.csv")))
        self.size = {int(r["address"], 16): int(r["size"]) for r in rows}
        self.starts = sorted(self.size)
        self.names = {}
        self.byqual = defaultdict(list)
        for r in csv.DictReader(open(os.path.join(ROOT, f"ida/exports/{name}-names.csv"))):
            a = int(r["address"], 16)
            self.names[a] = (r["name"], r["demangled"], r["source"])
            if r["source"] in ("ios:", "mac:") and r["kind"] == "func" and "(" in (r["demangled"] or ""):
                q = qualname(r["demangled"])
                if "[D0" in r["demangled"]:  # Itanium deleting dtor = MSVC scalar deleting destructor
                    q = q.rsplit("::", 1)[0] + "::`scalar deleting dtor'"
                    self.byqual[q].append((a, "unsignedint", r["demangled"]))
                    continue
                self.byqual[q].append((a, params(r["demangled"]), r["demangled"]))

    def bytes(self, a, n):
        return self.img[a - self.base:a - self.base + n] if a >= self.base else b""

    def search(self, f):
        code, mask = f["code"], f["mask"]
        pat = b"".join(b"." if i in mask else re.escape(code[i:i + 1]) for i in range(len(code)))
        return [self.t0 + m.start() for m in re.finditer(pat, self.text, re.S) if (self.t0 + m.start()) % 16 == 0]

    def compare(self, f, a):
        code, mask = f["code"], f["mask"]
        theirs = self.bytes(a, len(code))
        diffs = sum(1 for i in range(len(code)) if i not in mask and code[i] != theirs[i])
        if diffs == 0:
            diffs = LM.const_diff(f, theirs, self.bytes, self.cmap)
        extra = max(0, self.size.get(a, len(code)) - len(code))
        return diffs + extra, diffs == 0 and extra == 0

    def similarity(self, f, a):
        n = self.size.get(a, 0)
        if not n:
            return 0.0
        m = [t[2].rstrip("*") for t in tokens(f["code"], f["mask"])]
        t = [x[2] for x in tokens(self.bytes(a, n), set())]
        return difflib.SequenceMatcher(None, m, t, autojunk=False).ratio()

    def reloc_targets(self, f, a, ident):
        """(symbol, target address) for each rel32 relocation, after aligning our instructions with theirs."""
        code = f["code"]
        mine = tokens(code, f["mask"])
        if ident:
            at = {o: o for o, *_ in mine}
        else:
            theirs = tokens(self.bytes(a, max(self.size.get(a, len(code)), len(code))), set())
            sm = difflib.SequenceMatcher(None, [t[2].rstrip("*") for t in mine], [t[2] for t in theirs],
                                         autojunk=False)
            at = {}
            for i, j, k in sm.get_matching_blocks():
                if k >= 4:  # short blocks align unrelated calls
                    for x in range(k):
                        if mine[i + x][1] == theirs[j + x][1]:
                            at[mine[i + x][0]] = theirs[j + x][0]
        out = []
        starts = [t[0] for t in mine]
        for off, sym, typ in f["relocs"]:
            if typ != REL32:
                continue
            i = bisect.bisect_right(starts, off) - 1
            if i < 0 or starts[i] not in at:
                continue
            toff = at[starts[i]] + (off - starts[i])
            v = struct.unpack_from("<I", self.bytes(a + toff, 4))[0]
            out.append((sym, (a + toff + 4 + v) & 0xFFFFFFFF))
        return out


def show_diff(f, tgt, a):
    """Masked unified diff of the .obj function against the target (probe_compare-style listing)."""
    def listing(code, mask):
        out = []
        for ins in MD.disasm(code, 0):
            m = any(ins.address + k in mask for k in range(ins.size))
            raw = "".join("??" if ins.address + k in mask else "%02x" % code[ins.address + k] for k in range(ins.size))
            out.append(f"{raw:<24} {ins.mnemonic + ' <masked>' if m else ins.mnemonic + ' ' + ins.op_str}")
        return out
    code = f["code"]
    theirs = tgt.bytes(a, max(tgt.size.get(a, len(code)), len(code)))
    print(f"== {tgt.name} 0x{a:08x} {f['dem']}")
    for line in difflib.unified_diff(listing(code, f["mask"]), listing(theirs, f["mask"]), "obj", "target",
                                     lineterm="", n=2):
        print("   " + line)


def align(gap, cand, tgt, floor=0.45):
    """Monotone pairing of obj functions with target functions maximizing summed similarity."""
    if not gap or not cand or len(gap) * len(cand) > 4000:
        return []
    S = [[tgt.similarity(f, a) for a in cand] for f in gap]
    n, m = len(gap), len(cand)
    best = [[0.0] * (m + 1) for _ in range(n + 1)]
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            take = S[i][j] + best[i + 1][j + 1] if S[i][j] >= floor else -1
            best[i][j] = max(take, best[i + 1][j], best[i][j + 1])
    out, i, j = [], 0, 0
    while i < n and j < m:
        if S[i][j] >= floor and best[i][j] == S[i][j] + best[i + 1][j + 1]:
            out.append((gap[i], cand[j]))
            i, j = i + 1, j + 1
        elif best[i][j] == best[i + 1][j]:
            i += 1
        else:
            j += 1
    return out


def cluster(addrs, gap=0x4000):
    """Largest run of addresses with no hole over `gap` (drops folded or far-away outliers)."""
    runs, cur = [], []
    for x in addrs:
        if cur and x - cur[-1] > gap:
            runs.append(cur)
            cur = []
        cur.append(x)
    runs.append(cur)
    return max(runs, key=len)


def locate(funcs, tgt, log):
    """{(tu, name): (address, strength, evidence)}; strengths as in the module docstring."""
    key = lambda f: (f["tu"], f["name"])  # noqa: E731
    fk = {key(f): f for f in funcs}
    byname = defaultdict(list)
    for f in funcs:
        byname[f["name"]].append(f)
    loc, hits = {}, {}

    def put(k, a, st, e):
        cur = loc.get(k)
        if cur and cur[0] != a and st >= cur[1] - 1:
            log.append(f"{tgt.name} {fk[k]['cname']}: 0x{a:08x} ({e}) vs 0x{cur[0]:08x} ({cur[2]})"
                       + (" -> replaced" if st > cur[1] else " -> kept"))
        if cur is None or st > cur[1]:
            loc[k] = (a, st, e)

    for f in funcs:
        if len(f["code"]) - len(f["mask"]) >= 6:
            hits[key(f)] = tgt.search(f)
            if len(hits[key(f)]) == 1 and len(f["code"]) >= 16:
                put(key(f), hits[key(f)][0], 4, "search: unique masked body in .text")
    quals = defaultdict(list)
    for f in funcs:
        quals[f["cname"]].append(f)
    for q, fl in quals.items():
        cands = tgt.byqual.get(q, [])
        for f in fl:
            p = params(f["dem"])
            m = [c for c in cands if c[1] == p] if len(fl) > 1 or len(cands) > 1 else cands
            if len(m) == 1:
                put(key(f), m[0][0], 3, f"{tgt.names[m[0][0]][2]} {m[0][2]}")

    def resolve(f, sym):
        same = [g for g in byname.get(sym, []) if g["tu"] == f["tu"]]
        if same:
            return key(same[0])
        ext = [g for g in byname.get(sym, []) if not g["static"]]
        return key(ext[0]) if len(ext) == 1 else None

    tus = defaultdict(list)
    for f in funcs:
        tus[f["tu"]].append(f)
    for _ in range(12):
        snap = dict(loc)
        votes = defaultdict(lambda: defaultdict(list))
        for k, (a, st, _) in list(loc.items()):
            f = fk[k]
            ident = tgt.compare(f, a)[1]
            for sym, t in tgt.reloc_targets(f, a, ident):
                c = resolve(f, sym)
                if c and c != k and tgt.t0 <= t < tgt.t1 and t in tgt.size:
                    votes[c][t].append((5 if ident else 2, f"{f['cname']}@0x{a:08x}"))
        for c, d in votes.items():
            best = max(v[0] for vs in d.values() for v in vs)
            top = {t: vs for t, vs in d.items() if any(v[0] == best for v in vs)}
            if len(top) != 1:
                log.append(f"{tgt.name} {fk[c]['cname']}: callers disagree " + ", ".join(f"0x{t:08x}" for t in top))
                continue
            t, vs = next(iter(top.items()))
            if best == 2 and not (tgt.compare(fk[c], t)[1] or tgt.similarity(fk[c], t) >= 0.5):
                continue
            put(c, t, best, f"reloc: referenced by {vs[0][1]}" + (f" (+{len(vs) - 1})" if len(vs) > 1 else ""))
        for tu, fl in tus.items():
            taken = {v[0] for v in loc.values()}
            idx = [i for i, f in enumerate(fl) if key(f) in loc]
            for x, y in zip(idx, idx[1:]):
                a0, a1 = loc[key(fl[x])][0], loc[key(fl[y])][0]
                if y - x < 2 or a1 <= a0 or a1 - a0 > 0x8000:
                    continue
                cand = [s for s in tgt.starts if a0 < s < a1 and s not in taken]
                for f, a in align(fl[x + 1:y], cand, tgt):
                    put(key(f), a, 1, f"order: between {fl[x]['cname']}@0x{a0:08x} and {fl[y]['cname']}@0x{a1:08x}")
        if loc == snap:
            break
    return loc


def load(d, tus):
    """All functions of the compiled objects, demangled; defs: decorated name -> defining TUs."""
    allf, defs = [], defaultdict(list)
    for tu in tus:
        for f in obj_funcs(os.path.join(d, tu + ".obj"), tu):
            allf.append(f)
            defs[f["name"]].append(tu)
    dem = undname(sorted(defs))
    for f in allf:
        f["dem"] = dem.get(f["name"], f["name"])
        f["cname"] = qualname(f["dem"]) if f["name"].startswith("?") else f["name"].lstrip("_")
        f["own"] = f["static"] or len(defs[f["name"]]) == 1
    return allf, defs


def analyse(allf, defs, tus, tgt, log):
    """TU presence (step 0), then location. Returns (funcs, loc, span)."""
    def dedupe(keep):
        out, seen = [], set()
        for f in allf:
            if f["tu"] in keep and (f["static"] or f["name"] not in seen):
                seen.add(f["name"])
                out.append(f)
        return out

    loc = locate(dedupe(set(tus)), tgt, [])
    at = defaultdict(int)
    for v in loc.values():
        at[v[0]] += 1

    def proves(f):
        k = (f["tu"], f["name"])
        if not f["own"] or re.search(r"[<`]", f["cname"]) or k not in loc:
            return False
        addr, st, _ = loc[k]
        return (at[addr] == 1 and tgt.compare(f, addr)[1]
                and (st in (3, 5) or (st == 4 and len(f["code"]) >= 32 and len(tgt.search(f)) == 1)))
    present = sorted({f["tu"] for f in allf if proves(f)})
    print(f"{tgt.name}: linked RakNet TUs ({len(present)}): {' '.join(present)}")
    funcs = dedupe(set(present))
    loc = locate(funcs, tgt, log)
    span = {}
    for tu in present:
        # TU span: its own, non-template functions that are identical (RakNet and Roblox objects interleave)
        own = sorted(loc[(f["tu"], f["name"])][0] for f in funcs
                     if f["tu"] == tu and f["own"] and "<" not in f["cname"] and (f["tu"], f["name"]) in loc
                     and tgt.compare(f, loc[(f["tu"], f["name"])][0])[1])
        if own:
            own = cluster(own, 0x1000)
            span[tu] = (own[0], own[-1])
    return funcs, loc, span


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--no-patches", action="store_true")
    ap.add_argument("--flags", default=FLAGS)
    ap.add_argument("--keep")
    ap.add_argument("--src", default=SRC, help="probe another RakNet Source dir (copied to the build dir)")
    ap.add_argument("--inc", action="append", default=[], help="extra include dir (probes)")
    ap.add_argument("--no-write", action="store_true")
    ap.add_argument("--no-compile", action="store_true", help="reuse the .obj files already in --keep DIR")
    ap.add_argument("--out", default=os.path.join(ROOT, "ida/exports/PLAYER-lib-raknet.csv"))
    ap.add_argument("--targets", default="PLAYER,STUDIO")
    ap.add_argument("-v", nargs="*", default=[], help="qualified names to print masked diffs for (PLAYER)")
    a = ap.parse_args()
    os.makedirs("/tmp/claude-1000", exist_ok=True)
    d = a.keep or tempfile.mkdtemp(prefix="ogre-raknet-", dir="/tmp/claude-1000")
    os.makedirs(d, exist_ok=True)
    patches = [] if a.no_patches else sorted(glob.glob(os.path.join(STAGE, "patches/*.patch")))
    tus = sorted(f[:-4] for f in os.listdir(d) if f.endswith(".obj")) if a.no_compile else build(d, patches, a.flags, a.src, a.inc)
    allf, defs = load(d, tus)
    cmap = LM.consts([x for tu in tus for x in LM.obj_data(os.path.join(d, tu + ".obj"), tu)])
    rows, log, summary = [], [], []
    for tname in a.targets.split(","):
        tgt = Target(tname, cmap)
        funcs, loc, span = analyse(allf, defs, tus, tgt, log)
        claimed = defaultdict(list)
        n_id = 0
        for f in funcs:
            k = (f["tu"], f["name"])
            if k not in loc:
                continue
            addr, _, evid = loc[k]
            claimed[addr].append(f["cname"])
            diff, same = tgt.compare(f, addr)
            obj = f["tu"]
            if not f["own"]:  # the copy the linker kept: the defining TU whose span holds the address
                obj = next((t for t in defs[f["name"]] if t in span and span[t][0] <= addr <= span[t][1]), obj)
                if not same:  # another object's copy of the same COMDAT (inlining depends on the TU)
                    alts = sorted((g for g in allf if g["name"] == f["name"] and g is not f), key=lambda g: g["tu"] not in span)
                    alt = next((g for g in alts if tgt.compare(g, addr)[1]), None)
                    if alt:
                        diff, same, obj = 0, True, alt["tu"]
                        evid += f"; identical to the {alt['tu']}.obj copy"
            if not same and diff > len(f["code"]) / 2:  # not the same function: a weak or folded location
                log.append(f"{tname} {f['cname']}: rejected 0x{addr:08x} ({evid}): {diff} of {len(f['code'])} bytes differ")
                claimed[addr].remove(f["cname"])
                continue
            n_id += same
            rows.append(dict(target=tname, address=f"0x{addr:08x}", size=len(f["code"]), name=f["name"],
                             demangled=f["dem"], library="raknet", version=VERSION,
                             identical="yes" if same else "no", masked_diff_bytes=diff, obj=obj + ".obj",
                             evidence=evid))
            if f["cname"] in a.v or f["name"] in a.v:
                show_diff(f, tgt, addr)
        for addr, who in claimed.items():
            if len(who) > 1:
                log.append(f"{tname} 0x{addr:08x} claimed by {who} (folded by /OPT:ICF or a mislocation)")
        extra = 0
        for tu, (lo, hi) in sorted(span.items()):
            for s in tgt.starts:
                if lo < s < hi and s not in claimed and tgt.size[s] > 0:
                    nm = tgt.names.get(s, ("sub_%X" % s, "", ""))
                    extra += 1
                    claimed[s].append(nm[0])
                    rows.append(dict(target=tname, address=f"0x{s:08x}", size=tgt.size[s], name=nm[0],
                                     demangled=nm[1], library="raknet", version=VERSION,
                                     identical="no", masked_diff_bytes="", obj=tu + ".obj",
                                     evidence=f"inside {tu}.obj span 0x{lo:08x}-0x{hi:08x}, no compiled "
                                              f"counterpart" + (f"; {nm[2]} name" if nm[2] else "")))
        n_loc = sum(r["target"] == tname and r["masked_diff_bytes"] != "" for r in rows)
        summary.append(f"{tname}: {len(funcs)} functions in linked TUs, {n_loc} located, {n_id} identical, "
                       f"{extra} unclaimed target functions inside RakNet TU spans")
    rows.sort(key=lambda r: (r["target"] != "PLAYER", r["target"], r["address"]))
    if not a.no_write and rows:
        with open(a.out, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=list(rows[0]))
            w.writeheader()
            w.writerows(rows)
    for line in dict.fromkeys(log):
        print("note:", line)
    for tname in a.targets.split(","):
        by_tu = defaultdict(lambda: [0, 0])
        for r in rows:
            if r["target"] == tname and r["masked_diff_bytes"] != "":
                by_tu[r["obj"]][0] += 1
                by_tu[r["obj"]][1] += r["identical"] == "yes"
        print(tname + " identical/located: " + "  ".join(f"{o[:-4]} {v[1]}/{v[0]}" for o, v in sorted(by_tu.items())))
    print("\n".join(summary))
    print(("" if a.no_write else f"wrote {a.out}  ") + f"build dir {d}")


if __name__ == "__main__":
    main()

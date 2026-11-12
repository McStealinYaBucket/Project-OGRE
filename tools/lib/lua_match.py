"""Compile the 2013 Lua 5.1.4 reconstruction with VC9 SP1 and locate/match every function in PLAYER and STUDIO.

Run (from anywhere, about 30 s; needs the VC9 wrappers in tools/msvc9 and wine):
    python3 -P tools/lib/lua_match.py                 # upstream + patches -> ida/exports/PLAYER-lib-lua.csv
    python3 -P tools/lib/lua_match.py --no-patches    # pristine lua.org 5.1.4 only (baseline)
    python3 -P tools/lib/lua_match.py -v luaH_new     # also print a masked diff for these C names (PLAYER)
    python3 -P tools/lib/lua_match.py --src DIR --targets PLAYER --out X.csv   # iterate on an edited tree
    python3 -P tools/lib/lua_match.py --keep DIR      # build in DIR and keep the .obj files
    python3 -P tools/lib/lua_match.py --selftest      # check const_diff (1.0f vs 0.6f behind a masked operand)

Measured 2026-11-12: pristine 5.1.4 358 of 501 located PLAYER functions identical; with patches 01-15
PLAYER 514 located, 514 identical; STUDIO 514 located, 514 identical (same Lua source in both lines).

Source: staging/lua-5.1.4-2013/upstream (lua.org lua-5.1.4.tar.gz, sha256 b038e225...af3a as published
on lua.org/ftp, unmodified) plus every staging/lua-5.1.4-2013/patches/*.patch in name order (patch -p1
inside a scratch copy of upstream/src; each patch's header says which PLAYER bytes justify it). Every .c
except lua.c, luac.c and print.c is compiled as C++ with the proven App flags (FLAGS below) through
tools/msvc9/cl, with the boost 1.53.0 headers (BOOST below) on the include path for patch 14.

Location, per target (PLAYER = bin/player_1023, STUDIO = bin/studio), each step recorded in `evidence`:
  search - the masked .obj body (every relocated byte masked) occurs exactly once, 16-aligned, in .text,
           counting only hits whose referenced literals (strings, floats) hold the same values.
  reloc  - an identical (or >= 0.75 similar, aligned with difflib) located caller calls/references it there.
  data   - a luaL_Reg-style table found in the image by its known function pointers holds it.
  xref   - the only target function referencing its unique string literals / located callees.
  order  - with /Gy every function is a COMDAT kept in .obj order: paired with the unclaimed target
           functions between two located neighbours (best masked-instruction similarity).
  ios:   - the names CSV (ida/exports/<T>-names.csv) names it (ios:/mac:, or match: = an earlier byte match
           applied to the database); weakest after order, and overridden by the byte evidence above (10 such PLAYER names were off by one function; printed as notes).
A candidate whose callees disagree with stronger locations, or whose pooled literals (strings, __real@
floats/doubles) hold other values, is dropped. Target functions inside a Lua .obj's located span that no reconstruction function claims
get rows of their own (library rbx-in-lua-tu: RBX inline code compiled into lapi/lstate, e.g.
RobloxExtraSpace and RBX::Intrusive::Set members). Functions the run cannot locate are printed per TU,
split by whether anything in our objects references them.
identical = yes only if the masked bytes are equal over the whole .obj function length, the target
function is not longer (IDA's size stops before an in-function jump table, so a shorter IDA size is
fine), and every referenced pooled literal (string literal, __real@ float/double; see const_diff, shared
by every tools/lib matcher) holds the same bytes; a __FILE__ literal counts as equal
when only the checkout path differs. masked_diff_bytes = unmasked differing bytes over the .obj length +
any extra target length (+ differing constant bytes).
"""
import argparse
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
from probe_compare import read_obj  # noqa: E402

STAGE = os.path.join(ROOT, "staging/lua-5.1.4-2013")
CL = os.path.join(ROOT, "tools/msvc9/cl")
FLAGS = ("/nologo /c /TP /W0 /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD "
         "/D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_RELEASE /D_CRT_SECURE_NO_WARNINGS").split()
# boost 1.53.0 headers (sourceforge boost_1_53_0.tar.bz2, already downloaded by the boost task); only
# boost/scoped_ptr.hpp is used, by the ROBLOX luaL_loadprotected addition.
BOOST = os.path.join(ROOT, "3rdparty/_downloads/boost_1_53_0/boost_1_53_0")
SKIP = {"lua.c", "luac.c", "print.c"}
TARGETS = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
REL32, DIR32 = 0x14, 0x06
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def build(d, patches, src=None):
    shutil.copytree(src or os.path.join(STAGE, "upstream/src"), d, dirs_exist_ok=True)
    for p in patches:
        r = subprocess.run(["patch", "-p1", "-s", "-d", d, "-i", p], capture_output=True, text=True)
        if r.returncode:
            sys.exit(f"{p}: {r.stdout}{r.stderr}")
    srcs = sorted(f for f in os.listdir(d) if f.endswith(".c") and f not in SKIP)
    r = subprocess.run([CL] + FLAGS + ["/I" + BOOST, "/Fo" + d + "/"] + [os.path.join(d, s) for s in srcs],
                       capture_output=True, text=True, timeout=600)
    errs = [l for l in r.stdout.splitlines() if " error " in l]
    if r.returncode or errs:
        sys.exit(r.stdout)
    return [s[:-2] for s in srcs]


def obj_funcs(path, tu):
    """Functions of one /Gy object in section order: dicts with name, code, relocs, mask."""
    secs, syms = read_obj(path)
    out = []
    funcsyms = [s for s in syms.values() if s[2] > 0 and s[3] & 0x20 and not re.match(r"__\w+\$", s[0])]
    for nm, value, secno, typ, cls in sorted(funcsyms, key=lambda s: (s[2], s[1])):
        sec = secs[secno - 1]  # catch$/unwind$ funclets stay inside their parent, as in the linked image
        ends = [s[1] for s in funcsyms if s[2] == secno and s[1] > value]
        end = min(ends) if ends else len(sec["data"])
        relocs = [(o - value, syms[si][0], t) for o, si, t in sec["relocs"] if value <= o < end]
        out.append(dict(tu=tu, name=nm, static=cls == 3, code=sec["data"][value:end], relocs=relocs,
                        mask={r[0] + k for r in relocs for k in range(4)}))
    return out


def obj_data(path, tu):
    """Initialized data symbols (string literals, luaL_Reg arrays, ...): dicts with name, data, relocs."""
    secs, syms = read_obj(path)
    out = []
    for nm, value, secno, typ, cls in syms.values():
        if secno <= 0 or typ & 0x20 or nm.startswith(".") or cls not in (2, 3):
            continue
        sec = secs[secno - 1]
        if sec["flags"] & 0x20 or not sec["data"]:
            continue
        ends = [x[1] for x in syms.values() if x[2] == secno and x[1] > value and not x[0].startswith(".")]
        end = min(ends) if ends else len(sec["data"])
        out.append(dict(tu=tu, name=nm, static=cls == 3, data=sec["data"][value:end],
                        ro=not sec["flags"] & 0x80000000,
                        relocs=[(o - value, syms[si][0], t) for o, si, t in sec["relocs"] if value <= o < end]))
    return out


POOLED = ("??_C@", "__real@", "__xmm@")  # compiler-pooled literals: string literals, float/double constants


def consts(data):
    """The pooled literals of obj_data (string literals, __real@ float/double constants), keyed by name
    (statics by (tu, name)): what const_diff compares by value. Named const tables are left out: a
    different table (Lua's optsnum) is changed data, which reccmp-datacmp reports, not changed code."""
    return {(x["tu"], x["name"]) if x["static"] else x["name"]: x["data"]
            for x in data if x["name"].startswith(POOLED) and x.get("ro", True) and not x["relocs"] and x["data"]}


def const_diff(f, theirs, read, cmap):
    """Differing bytes between the constants f references (DIR32 relocations to cmap entries) and the
    target data its aligned operands point at. theirs: the target's bytes at the function (same offsets
    as f["code"]); read(addr, n): target bytes. Masking hides which constant an operand addresses, so
    without this 1.0f and 0.6f loads compare equal."""
    n = 0
    for o, sym, t, *_ in f["relocs"]:
        lit = cmap.get((f.get("tu", f.get("obj")), sym), cmap.get(sym))
        if t != DIR32 or lit is None or o + 4 > len(theirs):
            continue
        v = (struct.unpack_from("<I", theirs, o)[0] - struct.unpack_from("<I", f["code"], o)[0]) & 0xFFFFFFFF
        got = read(v, len(lit))
        d = sum(x != y for x, y in zip(lit, got)) + len(lit) - len(got)
        if d and sym.startswith("??_C@") and SRCPATH.search(lit):  # __FILE__: only the build checkout differs
            theirs_s = read(v, 260).split(b"\0", 1)[0]
            d = 0 if re.split(rb"[\\/]", theirs_s)[-1].lower() == re.split(rb"[\\/]", lit[:-1])[-1].lower() else d
        n += d
    return n


SRCPATH = re.compile(rb"[\\/][\w.+-]+\.(?:c|cc|cpp|cxx|h|hpp|inl)\0$", re.I)


def undname(names):
    """llvm-undname prints each input line, its demangling (or an error) and a blank line."""
    r = subprocess.run(["llvm-undname"], input="\n".join(names) + "\n", capture_output=True, text=True)
    dem = {}
    for block in r.stdout.split("\n\n"):
        ls = block.strip("\n").splitlines()
        if len(ls) >= 2 and not ls[1].startswith("error"):
            dem[ls[0]] = ls[1]
    return dem


def cname(n, dem):
    d = dem.get(n, n)
    m = re.search(r"(\w+)\(", d)
    return m.group(1) if m and n.startswith("?") else n.lstrip("_")


def tokens(code, mask, n=None):
    out = []
    for ins in MD.disasm(code[:n] if n else code, 0):
        masked = any(ins.address + k in mask for k in range(ins.size))
        raw = bytes(0 if ins.address + k in mask else code[ins.address + k] for k in range(ins.size))
        out.append((ins.address, ins.size, ins.mnemonic + ("*" if masked else ""), raw))
    return out


class Target:
    def __init__(self, name):
        self.name = name
        self.strlit = {}  # read-only data our objects reference, from consts(); compare() checks it by value
        pe = pefile.PE(os.path.join(ROOT, TARGETS[name]), fast_load=True)
        self.img, self.base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
        t = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
        self.t0, self.t1 = self.base + t.VirtualAddress, self.base + t.VirtualAddress + t.Misc_VirtualSize
        self.text = self.img[t.VirtualAddress:t.VirtualAddress + t.Misc_VirtualSize]
        rows = list(csv.DictReader(open(os.path.join(ROOT, f"ida/exports/{name}-functions.csv"))))
        self.size = {int(r["address"], 16): int(r["size"]) for r in rows}
        self.starts = sorted(self.size)
        self.names = {}
        p = os.path.join(ROOT, f"ida/exports/{name}-names.csv")
        for r in csv.DictReader(open(p)):
            self.names[int(r["address"], 16)] = (r["name"], r["demangled"], r["source"])

    def containing(self, a):
        import bisect
        i = bisect.bisect_right(self.starts, a) - 1
        return self.starts[i] if i >= 0 and a < self.starts[i] + max(self.size[self.starts[i]], 1) else None

    def calls_to(self, t):
        if not hasattr(self, "_calls"):
            self._calls = defaultdict(list)
            for m in re.finditer(b"[\xe8\xe9]", self.text):
                o = m.start()
                if o + 5 <= len(self.text):
                    self._calls[(self.t0 + o + 5 + struct.unpack_from("<i", self.text, o + 1)[0]) & 0xFFFFFFFF].append(self.t0 + o)
        return self._calls.get(t, [])

    def dword_refs(self, v):
        return [self.t0 + m.start() for m in re.finditer(re.escape(struct.pack("<I", v)), self.text)]

    def find_all(self, pat):
        return [self.base + m.start() for m in re.finditer(pat, self.img, re.S)]

    def bytes(self, a, n):
        return self.img[a - self.base:a - self.base + n] if a >= self.base else b""

    def search(self, f):
        code, mask = f["code"], f["mask"]
        pat = b"".join(b"." if i in mask else re.escape(code[i:i + 1]) for i in range(len(code)))
        return [self.t0 + m.start() for m in re.finditer(pat, self.text, re.S) if (self.t0 + m.start()) % 16 == 0]

    def compare(self, f, a, strings=True):
        code, mask = f["code"], f["mask"]
        theirs = self.bytes(a, len(code))
        diffs = sum(1 for i in range(len(code)) if i not in mask and code[i] != theirs[i])
        extra = max(0, self.size.get(a, len(code)) - len(code))
        if diffs == 0 and strings:  # masked bytes equal: the referenced constants must hold the same values too
            diffs += const_diff(f, theirs, self.bytes, self.strlit)
        return diffs + extra, diffs == 0 and extra == 0

    def reloc_targets(self, f, a):
        """(symbol, target address) for each relocation, after aligning our instructions with theirs."""
        code = f["code"]
        if not f["relocs"]:
            return []
        n = max(self.size.get(a, len(code)), len(code))
        mine, theirs = tokens(code, f["mask"]), tokens(self.bytes(a, n), set())
        if self.compare(f, a)[1]:
            pairs = {(o, o) for o, *_ in mine}
        else:
            sm = difflib.SequenceMatcher(None, [t[2].rstrip("*") for t in mine], [t[2] for t in theirs],
                                         autojunk=False)
            pairs = set()
            for i, j, k in sm.get_matching_blocks():
                if k < 4:  # short blocks align unrelated calls
                    continue
                for x in range(k):
                    if mine[i + x][1] == theirs[j + x][1]:
                        pairs.add((mine[i + x][0], theirs[j + x][0]))
        at = {o: s for o, s in pairs}
        out = []
        for off, sym, typ in f["relocs"]:
            ins = next((t for t in mine if t[0] <= off < t[0] + t[1]), None)
            if ins is None or ins[0] not in at:
                continue
            toff = at[ins[0]] + (off - ins[0])
            v = struct.unpack_from("<I", self.bytes(a + toff, 4))[0]
            out.append((sym, (a + toff + 4 + v) & 0xFFFFFFFF if typ == REL32 else v))
        return out


def similarity(f, tgt, a):
    n = tgt.size.get(a, 0)
    if not n:
        return 0.0
    m = [t[2].rstrip("*") for t in tokens(f["code"], f["mask"])]
    t = [x[2] for x in tokens(tgt.bytes(a, n), set())]
    return difflib.SequenceMatcher(None, m, t, autojunk=False).ratio()


def locate(funcs, data, tgt, log):
    """Return {(tu, name): (address, strength, evidence)}. A stronger source overrides a weaker one:
    5 reloc from an identical caller, 4 unique search hit, 3 names CSV, 2 reloc from an aligned
    non-identical caller or the only search hit between neighbours, 1 .obj order."""
    key = lambda f: (f["tu"], f["name"])  # noqa: E731
    byname = defaultdict(list)
    for f in funcs:
        byname[f["name"]].append(f)
    fk = {key(f): f for f in funcs}
    loc, banned, hits = {}, set(), {}

    def put(k, a, st, e):
        if (k, a) in banned:
            return
        cur = loc.get(k)
        if cur and cur[0] != a and st >= cur[1] - 1:
            log.append(f"{tgt.name} {fk[k]['cname']}: 0x{a:08x} ({e}) vs 0x{cur[0]:08x} ({cur[2]})"
                       + (" -> replaced" if st > cur[1] else " -> kept"))
        if cur is None or st > cur[1]:
            loc[k] = (a, st, e)

    strlit = consts(data)  # a different literal under equal masked bytes means a lookalike function
    for f in funcs:
        if len(f["code"]) - len(f["mask"]) >= 6:
            hits[key(f)] = [h for h in tgt.search(f)
                            if not const_diff(f, tgt.bytes(h, len(f["code"])), tgt.bytes, strlit)]
            if len(hits[key(f)]) == 1 and len(f["code"]) >= 16:
                put(key(f), hits[key(f)][0], 4, "search: unique masked body in .text")
    for f in funcs:  # iOS/Mac names already transferred into the target's names CSV
        if sum(g["cname"] == f["cname"] for g in funcs) > 1:
            continue
        for a, (n, d, s) in tgt.names.items():
            if s in ("ios:", "mac:", "match:") and re.match(r"(?:\S+ )?%s\(" % re.escape(f["cname"]), d or ""):
                put(key(f), a, 3, f"{s} {d}")
    tus = defaultdict(list)
    for f in funcs:
        tus[f["tu"]].append(f)

    def resolve(f, sym):
        same = [g for g in byname.get(sym, []) if g["tu"] == f["tu"]]
        if same:
            return key(same[0])
        ext = [g for g in byname.get(sym, []) if not g["static"]]
        return key(ext[0]) if len(ext) == 1 else None

    daddr = {}  # data symbol (tu, name) or name -> target address
    for x in data:  # unique string literals
        if x["name"].startswith("??_C@") and len(x["data"]) >= 5:
            h = tgt.find_all(re.escape(x["data"]))
            if len(h) == 1:
                daddr[x["name"]] = h[0]

    def fresolve(x, sym):
        same = [g for g in byname.get(sym, []) if g["tu"] == x["tu"]]
        ext = [g for g in byname.get(sym, []) if not g["static"]]
        return key(same[0]) if same else key(ext[0]) if len(ext) == 1 else None

    for _ in range(12):
        snap = dict(loc)
        for x in data:  # arrays of function pointers (luaL_Reg tables): locate the array, then its entries
            if (x["tu"], x["name"]) in daddr or not x["relocs"]:
                continue
            known, pat, pos = 0, bytearray(), 0
            rel = {o: (sym, t) for o, sym, t in x["relocs"]}
            parts = []
            i = 0
            while i < len(x["data"]):
                if i in rel:
                    sym, t = rel[i]
                    k = fresolve(x, sym)
                    v = loc[k][0] if k in loc and loc[k][1] >= 3 else daddr.get(sym)
                    if v is not None and t == DIR32:
                        parts.append(re.escape(struct.pack("<I", v)))
                        known += k is not None
                    else:
                        parts.append(b"....")
                    i += 4
                else:
                    parts.append(re.escape(x["data"][i:i + 1]))
                    i += 1
            if known < 2:
                continue
            h = tgt.find_all(b"".join(parts))
            if len(h) != 1:
                continue
            daddr[(x["tu"], x["name"])] = h[0]
            for o, sym, t in x["relocs"]:
                k = fresolve(x, sym)
                if k and t == DIR32:
                    v = struct.unpack_from("<I", tgt.bytes(h[0] + o, 4))[0]
                    if tgt.t0 <= v < tgt.t1:
                        put(k, v, 3, f"data: entry of {cname(x['name'], {}) if not x['name'].startswith('?') else x['name'].split('@')[0][1:]} at 0x{h[0] + o:08x}")
        taken = {v[0] for v in loc.values()}
        for f in funcs:  # xref: the only target function referencing all known data / calling all known callees
            if key(f) in loc:
                continue
            cands, why = None, []
            for o, sym, t in f["relocs"]:
                c = resolve(f, sym)
                if t == DIR32:
                    v = daddr.get(sym, daddr.get((f["tu"], sym)))
                    if v is None and c in loc and loc[c][1] >= 3:
                        v = loc[c][0]
                    if v is None:
                        continue
                    refs = {tgt.containing(r) for r in tgt.dword_refs(v)} - {None}
                elif t == REL32 and c in loc and loc[c][1] >= 3:
                    refs = {tgt.containing(r) for r in tgt.calls_to(loc[c][0])} - {None}
                else:
                    continue
                why += [sym] * (2 if sym.startswith("??_C@") else 1)  # a unique string literal counts double
                cands = refs if cands is None else cands & refs
            if cands is not None and len(why) >= 2 and len(cands - taken) == 1:
                a = next(iter(cands - taken))
                sim = similarity(f, tgt, a)
                if sim >= 0.5 and 0.5 <= tgt.size[a] / len(f["code"]) <= 2:
                    put(key(f), a, 2, f"xref: only function referencing {len(why)} of its located callees/data "
                                      f"(similarity {sim:.2f})")
                    taken.add(a)

        votes = defaultdict(lambda: defaultdict(list))
        for k, (a, st, _) in list(loc.items()):
            f = fk[k]
            ident = tgt.compare(f, a)[1]
            agree = bad = 0
            for sym, t in tgt.reloc_targets(f, a):
                c = resolve(f, sym)
                if not c or c == k or not tgt.t0 <= t < tgt.t1:
                    continue
                if not ident and similarity(f, tgt, a) < 0.75:
                    continue  # a loosely aligned caller is no evidence
                votes[c][t].append((5 if ident and st >= 3 else 2, f"{f['cname']}@0x{a:08x}"))
                if c in loc and loc[c][1] >= 4:
                    agree += loc[c][0] == t
                    bad += loc[c][0] != t
            if tgt.compare(f, a, strings=False)[1] and const_diff(f, tgt.bytes(a, len(f["code"])), tgt.bytes, strlit):
                bad += 100  # a different literal or float constant is decisive
            if bad > agree and st <= 4:
                log.append(f"{tgt.name} {f['cname']}: dropped 0x{a:08x} ({loc[k][2]}): {bad} callee(s) disagree")
                banned.add((k, a))
                del loc[k]
        for c, d in votes.items():
            best = max(v[0] for vs in d.values() for v in vs)
            top = {t: vs for t, vs in d.items() if any(v[0] == best for v in vs)}
            if len(top) == 1:
                t, vs = next(iter(top.items()))
                put(c, t, best, f"reloc: referenced by {vs[0][1]}" + (f" (+{len(vs) - 1})" if len(vs) > 1 else ""))
            else:
                log.append(f"{tgt.name} {fk[c]['cname']}: callers disagree " + ", ".join(f"0x{t:08x}" for t in top))
        for tu, fl in tus.items():
            # neighbours: the TU's own functions inside its main run of addresses (an inline/template COMDAT
            # or an /OPT:ICF copy kept elsewhere can sit far away and would close the window)
            own = [i for i, f in enumerate(fl) if key(f) in loc and not f.get("inline")]
            run = set(cluster(sorted(loc[key(fl[i])][0] for i in own))) if own else set()
            idx = [i for i in own if loc[key(fl[i])][0] in run]
            taken = {v[0] for v in loc.values()}
            for i, f in enumerate(fl):
                if key(f) in loc or len(hits.get(key(f), [])) < 2 or not idx:
                    continue
                lo = max((loc[key(fl[j])][0] for j in idx if j < i), default=None)
                hi = min((loc[key(fl[j])][0] for j in idx if j > i), default=None)
                c = [h for h in hits[key(f)] if (lo is None or h > lo) and (hi is None or h < hi)
                     and (lo is not None or hi - h < 0x2000) and (hi is not None or h - lo < 0x2000)]
                if len(c) == 1 and c[0] not in taken:
                    put(key(f), c[0], 2, "search: only masked-body hit between located neighbours")
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


def align(gap, cand, tgt, floor=0.45):
    """Monotone pairing of obj functions with target functions maximizing summed similarity."""
    if not gap or not cand:
        return []
    S = [[similarity(f, tgt, a) for a in cand] for f in gap]
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


def selftest():
    code = b"\xd9\x05\0\0\0\0\xc3"  # fld dword ptr [__real@3f800000]; ret
    f = dict(tu="t", code=code, relocs=[(2, "__real@3f800000", DIR32)])
    mem = {0x1000: struct.pack("<f", 0.6), 0x2000: struct.pack("<f", 1.0)}
    cmap = consts([dict(tu="t", name="__real@3f800000", static=False, ro=True, relocs=[], data=mem[0x2000])])
    at = lambda a: code[:2] + struct.pack("<I", a) + code[6:]  # noqa: E731
    assert const_diff(f, at(0x1000), lambda a, n: mem.get(a, b"")[:n], cmap) == 3  # 9a 99 19 vs 00 00 80
    assert const_diff(f, at(0x2000), lambda a, n: mem.get(a, b"")[:n], cmap) == 0
    lit = b"Z:\\tmp\\x\\RakPeer.cpp\0"  # __FILE__ of our build vs the original checkout: equal
    g = dict(tu="t", code=code, relocs=[(2, "??_C@_0BA@x", DIR32)])
    mem[0x3000] = b"C:\\BuildAgent\\work\\407e\\Client\\Network\\RakPeer.cpp\0"
    mem[0x4000] = b"C:\\BuildAgent\\work\\407e\\Client\\Network\\RakString.cpp\0"
    assert const_diff(g, at(0x3000), lambda a, n: mem.get(a, b"")[:n], {"??_C@_0BA@x": lit}) == 0
    assert const_diff(g, at(0x4000), lambda a, n: mem.get(a, b"")[:n], {"??_C@_0BA@x": lit}) > 0
    print("selftest ok")


def main():
    if "--selftest" in sys.argv:
        return selftest()
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--no-patches", action="store_true")
    ap.add_argument("--keep")
    ap.add_argument("--src", help="compile this tree as is instead of upstream + patches (for iterating)")
    ap.add_argument("--targets", default="PLAYER,STUDIO")
    ap.add_argument("--out", default=os.path.join(ROOT, "ida/exports/PLAYER-lib-lua.csv"))
    ap.add_argument("-v", nargs="*", default=[], help="C names to print masked diffs for (PLAYER)")
    a = ap.parse_args()
    os.makedirs("/tmp/claude-1000", exist_ok=True)
    d = a.keep or tempfile.mkdtemp(prefix="ogre-lua-", dir="/tmp/claude-1000")
    os.makedirs(d, exist_ok=True)
    patches = [] if a.no_patches else sorted(glob.glob(os.path.join(STAGE, "patches/*.patch")))
    tus = build(d, [] if a.src else patches, a.src)
    funcs = [f for tu in tus for f in obj_funcs(os.path.join(d, tu + ".obj"), tu)]
    data = [x for tu in tus for x in obj_data(os.path.join(d, tu + ".obj"), tu)]
    dem = undname([f["name"] for f in funcs])
    for f in funcs:
        f["dem"], f["cname"] = dem.get(f["name"], f["name"]), cname(f["name"], dem)
    rows, log = [], []
    summary = []
    for tname in a.targets.split(","):
        tgt = Target(tname)
        tgt.strlit = consts(data)
        loc = locate(funcs, data, tgt, log)
        claimed = defaultdict(list)
        n_id = 0
        for f in funcs:
            k = (f["tu"], f["name"])
            if k not in loc:
                continue
            addr, _, evid = loc[k]
            claimed[addr].append(f["cname"])
            diff, same = tgt.compare(f, addr)
            n_id += same
            rows.append(dict(target=tname, address=f"0x{addr:08x}", size=len(f["code"]), name=f["name"],
                             demangled=f["dem"], library="lua", version="5.1.4-roblox-2013",
                             identical="yes" if same else "no", masked_diff_bytes=diff, obj=f["tu"] + ".obj",
                             evidence=evid))
            if tname == "PLAYER" and f["cname"] in a.v:
                subprocess.run([sys.executable, "-P", os.path.join(ROOT, "tools/probe_compare.py"),
                                os.path.join(d, f["tu"] + ".obj"), f"{f['name']}@0x{addr:x}"])
        for addr, who in claimed.items():
            if len(who) > 1:
                log.append(f"{tname} 0x{addr:08x} claimed by {who} (folded by /OPT:ICF or a mislocation)")
        # target functions inside each TU's located span that nothing claims
        extra = 0
        for tu in tus:
            addrs = cluster(sorted(loc[(f["tu"], f["name"])][0] for f in funcs
                                if f["tu"] == tu and (f["tu"], f["name"]) in loc))
            if len(addrs) < 2:
                continue
            for s in tgt.starts:
                if addrs[0] < s < addrs[-1] and s not in claimed and tgt.size[s] > 0:
                    nm = tgt.names.get(s, ("sub_%X" % s, "", ""))
                    extra += 1
                    rows.append(dict(target=tname, address=f"0x{s:08x}", size=tgt.size[s], name=nm[0],
                                     demangled=nm[1], library="rbx-in-lua-tu", version="5.1.4-roblox-2013",
                                     identical="no", masked_diff_bytes="", obj=tu + ".obj",
                                     evidence=f"inside {tu}.obj span 0x{addrs[0]:08x}-0x{addrs[-1]:08x}, no "
                                              f"reconstruction counterpart" + (f"; {nm[2]} name" if nm[2] else "")))
        for tu in tus:
            refd = {r[1] for g in funcs + data for r in g["relocs"] if g["name"] != r[1]}
            miss = [f for f in funcs if f["tu"] == tu and (f["tu"], f["name"]) not in loc]
            for kind, sel in (("referenced", lambda f: f["name"] in refd),
                              ("unreferenced", lambda f: f["name"] not in refd)):
                m = [f["cname"] for f in miss if sel(f)]
                if m:
                    summary.append(f"  {tname} {tu}: not located, {kind} in our objs: {' '.join(m)}")
        summary.append(f"{tname}: {len(funcs)} reconstruction functions, {len(loc)} located, {n_id} identical, "
                       f"{extra} unclaimed functions inside Lua TU spans")
    rows.sort(key=lambda r: (r["target"] != "PLAYER", r["address"]))
    with open(a.out, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    for l in dict.fromkeys(log):
        print("note:", l)
    by_tu = defaultdict(lambda: [0, 0, 0])
    for r in rows:
        if r["target"] == "PLAYER" and r["masked_diff_bytes"] != "":
            t = by_tu[r["obj"]]
            t[0] += 1
            t[1] += r["identical"] == "yes"
    for tu in tus:
        n = sum(f["tu"] == tu for f in funcs)
        t = by_tu[tu + ".obj"]
        print(f"  {tu:<10} {n:3d} functions  PLAYER located {t[0]:3d}  identical {t[1]:3d}")
    print("\n".join(summary))
    print(f"wrote {a.out}  build dir {d}")


if __name__ == "__main__":
    main()

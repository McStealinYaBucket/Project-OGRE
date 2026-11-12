"""One-build sweep over the imported App decomp's markers (DECOMP_PLAN Phase 3 step 0).

Scope: every `// STUB:` / `// FUNCTION:` marker for PLAYER or STUDIO in Client/{App,Base,Network,Log,
Rendering}. Client/_generated (link stubs) is never touched. A marker whose next declaration ends in
`;` before any `{` is not on a definition and is left alone.

    python3 -P tools/app_sweep.py set FUNCTION          # flip every in-scope marker to FUNCTION
    python3 -P tools/app_sweep.py set STUB              # revert every in-scope marker to STUB
    python3 -P tools/app_sweep.py apply P.json S.json   # FUNCTION iff effective 100% in that target
    python3 -P tools/app_sweep.py report P.json S.json [--out notes/app-sweep.csv]
    python3 -P tools/app_sweep.py xrefs P.json S.json [--out notes/app-sweep-xrefs.csv] [--min 90]
    python3 -P tools/app_sweep.py order P.json S.json     # per-file descents, PLAYER vs STUDIO order

P.json / S.json come from `reccmp-reccmp --target PLAYER --json f` run in build/ and
`reccmp-reccmp --target STUDIO --json f` run in build-studio/. `apply` and
`set` are idempotent and only ever rewrite the marker word, so line numbers (and PDB line records)
never change. An address at 100% that carries several markers becomes `FUNCTION: ... FOLDED` on every
one of them when our linker folded all their definitions onto the recompiled body (by the link map's
names at that address, as tools/check_folded.py requires); otherwise only the marker whose definition
the recompiled function came from (the only one the map puts there, else by PDB name,
<build>/<exe>.pdbview.tsv) may become FUNCTION.

`report` writes one row per marker: file, function, target, address, score, effective, shape, tags,
orig/recomp instruction counts of the changed lines. Shapes (first that applies): stack-cookie,
ebp-frame, x87-vs-sse, inlining-size, other, assert-file-line, regalloc-sched, unpaired-callee, data-ref.
pairing_only=1 means every changed line is an unpaired reference (it scores 100% once those pair).

`xrefs` harvests reference pairings from the scored functions (score >= --min): in each aligned
orig/recomp line pair where the recomp side names a symbol the orig side does not, it decodes the
original instruction (capstone) to get the original address. One row per (target, recomp symbol,
original address) with the number of functions that agree. Evidence class: xref chain from a function
paired by marker; a row with conflicts or one vote is weak.
"""
import argparse
import collections
import csv
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCOPE = ("App", "Base", "Network", "Log", "Rendering")
EXTS = (".cpp", ".c", ".h", ".hpp", ".inl", ".cc")
MARK = re.compile(r"^(\s*// )(STUB|FUNCTION)(: (PLAYER|STUDIO) 0x([0-9a-f]{8}))( FOLDED\b)?(.*)$")
EXE = {"PLAYER": "RobloxPlayerBeta", "STUDIO": "RobloxStudioBeta"}
# Each target is scored from its own build: STUDIO's __FILE__ strings carry its own checkout path
# (cmake -DOGRE_TARGET=STUDIO, tools/msvc9/winewrap.py).
BUILD = {"PLAYER": "build", "STUDIO": "build-studio"}


def files():
    for d in SCOPE:
        for dp, _, fns in os.walk(os.path.join(ROOT, "Client", d)):
            for fn in sorted(fns):
                if fn.endswith(EXTS):
                    yield os.path.join(dp, fn)


def read(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8", "surrogateescape").split("\n")


def write(path, lines):
    with open(path, "wb") as f:
        f.write("\n".join(lines).encode("utf-8", "surrogateescape"))


def decl_after(lines, i):
    """Text of the declaration after the marker block at line i (up to the first { or ;)."""
    out = []
    for j in range(i + 1, min(i + 40, len(lines))):
        s = lines[j].split("//")[0].strip()
        if not out and not s:
            continue
        out.append(s)
        if "{" in s or ";" in s:
            break
    txt = " ".join(out)
    cut = min([p for p in (txt.find("{"), txt.find(";")) if p >= 0] or [len(txt)])
    return txt[:cut].strip(), (txt[cut:cut + 1] if cut < len(txt) else "")


def markers():
    """Yield (path, line_index, target, addr, kind, decl, is_definition)."""
    for p in files():
        lines = read(p)
        for i, ln in enumerate(lines):
            m = MARK.match(ln)
            if m:
                decl, end = decl_after(lines, i)
                # a ctor init list `X::X() : a(0)` has no ; before {; a declaration ends in ;
                kind = "FOLDED" if m.group(2) == "FUNCTION" and m.group(6) else m.group(2)
                yield p, i, m.group(4), "0x" + m.group(5), kind, decl, end != ";"


def load_json(path):
    with open(path) as f:
        return {int(e["address"], 16): e for e in json.load(f)["data"]}


def is_100(e):
    return e is not None and (e["matching"] >= 1.0 or bool(e.get("effective")))


def pdb_names(target):
    names = {}
    path = os.path.join(ROOT, BUILD[target], EXE[target] + ".pdbview.tsv")
    with open(path, encoding="utf-8", errors="replace") as f:
        for row in f:
            c = row.rstrip("\n").split("\t")
            if len(c) >= 4 and c[1] == "FUNCTION":
                names.setdefault(int(c[0], 16), c[3])
    return names


def ident(decl):
    """Qualified function name from a definition head, e.g. `RBX::Instance::Instance`."""
    head = decl.split("(")[0]
    head = re.sub(r"<[^<>]*>", "", re.sub(r"<[^<>]*>", "", head))
    toks = head.replace("*", " ").replace("&", " ").split()
    return toks[-1] if toks else ""


def owner_ok(decl, recomp_name):
    """Does the recompiled function's PDB name end with this definition's (unqualified-safe) name?"""
    if not recomp_name:
        return False
    a = re.sub(r"<[^<>]*>", "", ident(decl)).split("::")
    b = re.sub(r"<[^<>]*>", "", re.sub(r"<[^<>]*>", "", recomp_name.split("(")[0])).split("::")
    n = min(len(a), len(b), 2)
    return n > 0 and a[-n:] == b[-n:]


def map_names(target, addrs):
    """{recompiled address: [qualified names of every symbol the link map puts there]} for addrs: the
    names /OPT:ICF folded onto one body (the PDB view keeps only one of them)."""
    sys.path.insert(0, os.path.join(ROOT, "tools", "ida"))
    from gen_reccmp_csv import norm_sig, qual, undname
    syms = {}
    with open(os.path.join(ROOT, BUILD[target], EXE[target] + ".map"), errors="replace") as f:
        for line in f:
            parts = line.split()
            if len(parts) >= 4 and re.fullmatch(r"[0-9a-f]{8}", parts[2]) and int(parts[2], 16) in addrs:
                syms.setdefault(int(parts[2], 16), []).append(parts[1])
    und = undname([d for v in syms.values() for d in v if d.startswith("?")])
    return {a: [qual(und[d]) for d in v if und.get(d)] for a, v in syms.items()}


def wanted(jsons, names):
    """Map (path, line) -> desired kind for `apply`: STUB, FUNCTION or FOLDED (FUNCTION ... FOLDED).
    An address at 100% that carries several markers is FOLDED on all of them when every marker's
    definition is among the symbols our linker folded onto the recompiled body (tools/check_folded.py's
    rule); otherwise only the marker whose definition the PDB names keeps FUNCTION."""
    by_addr = collections.defaultdict(list)
    ms = [m for m in markers() if m[6]]
    for m in ms:
        by_addr[(m[2], m[3])].append(m)
    multi = collections.defaultdict(set)
    for (tgt, addr), group in by_addr.items():
        e = jsons[tgt].get(int(addr, 16))
        if len(group) > 1 and is_100(e) and e.get("recomp"):
            multi[tgt].add(int(e["recomp"], 16))
    folds = {t: map_names(t, multi[t]) if multi[t] else {} for t in jsons}
    want = {}
    for (tgt, addr), group in by_addr.items():
        e = jsons[tgt].get(int(addr, 16))
        ok = is_100(e)
        r = int(e["recomp"], 16) if ok and e.get("recomp") else None
        landed = [m for m in group if any(owner_ok(m[5], n) for n in folds[tgt].get(r, ()))]
        folded = ok and len(group) > 1 and len(landed) == len(group)
        for m in group:
            k = "STUB"
            if folded:
                k = "FOLDED"
            elif ok:
                if len(group) == 1:
                    k = "FUNCTION"
                elif len(landed) == 1:  # the only definition our linker put there (others not emitted)
                    k = "FUNCTION" if m is landed[0] else "STUB"
                else:
                    k = "FUNCTION" if owner_ok(m[5], names[tgt].get(r)) else "STUB"
            want[(m[0], m[1])] = k
    return want


def rewrite(want_fn):
    changed = collections.Counter()
    cache = {}
    for p, i, tgt, addr, kind, decl, isdef in list(markers()):
        if not isdef:
            continue
        k = want_fn(p, i, tgt)
        if k != kind:
            lines = cache.setdefault(p, read(p))
            word, fold = ("FUNCTION", " FOLDED") if k == "FOLDED" else (k, "")
            lines[i] = MARK.sub(lambda m: m.group(1) + word + m.group(3) + fold + m.group(7), lines[i])
            changed[(tgt, k)] += 1
    for p, lines in cache.items():
        write(p, lines)
    return changed


# ---------- diff shape ----------
X87 = re.compile(r"^f[a-z0-9]+ ")
SSE = re.compile(r"xmm|^(movss|movsd|cvt\w+|comiss|ucomiss|addss|subss|mulss|divss|sqrtss|maxss|minss|andps|xorps)\b")
REG = re.compile(r"\b(e?[abcd]x|[abcd][lh]|e?[sd]i|e?[sb]p|xmm\d|st\(\d\))\b")
NAMED = re.compile(r"\((FUNCTION|DATA|UNK|STRING|FLOAT|IMPORT|IMPORT_THUNK|VTABLE|LABEL|POINTER)\)")
ORDER = ["stack-cookie", "ebp-frame", "x87-vs-sse", "inlining-size", "other", "assert-file-line", "regalloc-sched",
         "unpaired-callee", "data-ref"]
PAIRING = {"unpaired-callee", "data-ref"}


def mnem(t):
    return t.split(" ")[0]


def ref_kind(a, b):
    """Shape of an aligned orig/recomp line pair with the same mnemonic, or None if not a reference diff."""
    unnamed = "<OFFSET" in a or re.search(r"\b0x[0-9a-f]{5,}\b", a) or NAMED.search(a)
    if not (unnamed and NAMED.search(b) or NAMED.search(a) and NAMED.search(b)):
        return None
    if mnem(a) in ("call", "jmp") or "(FUNCTION)" in b and mnem(a) == "push":
        return "unpaired-callee"
    return "data-ref"


def pair_tag(a, b):
    if mnem(a) != mnem(b):
        return "other"
    k = ref_kind(a, b)
    if k:
        return k
    if REG.sub("R", a) == REG.sub("R", b):
        return "regalloc-sched"
    return "other"   # same opcode, different displacement/immediate: layout or constant


def classify(e):
    """Return (shape, tags, n_orig_changed, n_recomp_changed, pairing_only)."""
    o, r, tags = [], [], set()
    for _, blocks in e.get("diff") or []:
        for b in blocks:
            bo = [x[1].split("\t")[0].strip() for x in b.get("orig", [])]
            br = [x[1].split("\t")[0].strip() for x in b.get("recomp", [])]
            o += bo
            r += br
            if not bo and not br:
                continue
            if bo and br and len(bo) == len(br):
                pt = [pair_tag(x, y) for x, y in zip(bo, br)]
                # an assert's __FILE__ (build path) and __LINE__ (source layout) strings and line immediate
                if any("(STRING)" in x and ("file:" in x or "BuildAgent" in x) for x in bo):
                    pt = ["assert-file-line" if t == "other" and (mnem(x) == "push" or "(STRING)" in x) or
                          t == "data-ref" and "(STRING)" in x else t for t, x in zip(pt, bo)]
                tags.update(pt)
            elif sorted(bo) != sorted(br) or not bo:
                # unequal blocks: reordering if the other side carries the same lines, else other
                tags.add("regalloc-sched" if sorted(map(mnem, bo)) == sorted(map(mnem, br)) and bo else "other")
    ch = o + r
    if any("__security_cookie" in t or "__security_check_cookie" in t for t in ch):
        tags.add("stack-cookie")
    if any(t in ("push ebp", "mov ebp, esp", "pop ebp", "mov esp, ebp", "leave") for t in ch):
        tags.add("ebp-frame")
    fo, fr = sum(bool(X87.match(t)) for t in o), sum(bool(X87.match(t)) for t in r)
    so, sr = sum(bool(SSE.search(t)) for t in o), sum(bool(SSE.search(t)) for t in r)
    if (fo and sr) or (so and fr):
        tags.add("x87-vs-sse")
    if abs(len(o) - len(r)) >= max(4, 0.1 * max(len(o), len(r))):
        tags.add("inlining-size")
    if (o or r) and not tags:
        tags.add("other")
    # an unequal block of whole moved lines (sorted equal) is scheduling, already handled above
    shape = next((s for s in ORDER if s in tags), "other")
    return shape, sorted(tags, key=ORDER.index), len(o), len(r), bool(tags) and tags <= PAIRING


def report(jsons, out):
    try:
        placed = {}
        with open(os.path.join(ROOT, "notes", "app-marker-placements.csv")) as f:
            for row in csv.DictReader(f):
                for t in ("PLAYER", "STUDIO"):
                    if row[t.lower()]:
                        placed[(row["file"], t, row[t.lower()])] = row["function"]
    except FileNotFoundError:
        placed = {}
    rows = []
    for p, i, tgt, addr, kind, decl, isdef in markers():
        rel = os.path.relpath(p, ROOT)
        e = jsons[tgt].get(int(addr, 16))
        fn = placed.get((rel, tgt, addr)) or ident(decl)
        if e is None:
            rows.append([rel, i + 1, fn, tgt, addr, kind, "", "", "unpaired", "", "", "", ""])
            continue
        if is_100(e):
            shape, tags, no, nr, po = ("match", [], 0, 0, False)
        else:
            shape, tags, no, nr, po = classify(e)
        rows.append([rel, i + 1, fn, tgt, addr, kind, f"{e['matching'] * 100:.2f}", int(bool(e.get("effective"))),
                     shape, ";".join(tags), int(po), no, nr])
    with open(out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["file", "line", "function", "target", "address", "marker", "score", "effective", "shape",
                    "tags", "pairing_only", "orig_changed", "recomp_changed"])
        w.writerows(rows)
    return rows


BIN = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
SYM = re.compile(r"(?:\[|, |^\w+ )([^\[\]]+?) \((FUNCTION|DATA|UNK|VTABLE)\)")


class Image:
    def __init__(self, path):
        import struct
        self.d = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.d, 0x3c)[0]
        nsec, opt = struct.unpack_from("<H", self.d, pe + 6)[0], struct.unpack_from("<H", self.d, pe + 20)[0]
        self.base = struct.unpack_from("<I", self.d, pe + 24 + 28)[0]
        self.secs = []
        for k in range(nsec):
            o = pe + 24 + opt + 40 * k
            vs, va, rs, ro = struct.unpack_from("<IIII", self.d, o + 8)
            self.secs.append((self.base + va, rs, ro))  # raw bytes only; bss reads as empty

    def read(self, va, n):
        for s, size, ro in self.secs:
            if s <= va < s + size:
                return self.d[ro + va - s: ro + va - s + n]
        return b""


def operand_addr(md, img, va):
    """Absolute address an original instruction refers to (branch target, memory disp or immediate)."""
    from capstone import x86
    ins = next(md.disasm(img.read(va, 16), va), None)
    if ins is None:
        return None
    for op in ins.operands:
        if op.type == x86.X86_OP_IMM and op.imm >= 0x400000:
            return op.imm
        if op.type == x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0 and op.mem.disp >= 0x400000:
            return op.mem.disp
    return None


def xrefs(jsons, out, min_score):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    rows = []
    for tgt, J in jsons.items():
        img = Image(os.path.join(ROOT, BIN[tgt]))
        names = {}
        try:
            with open(os.path.join(ROOT, "ida", "exports", tgt + "-names.csv"), encoding="utf-8") as f:
                for row in csv.DictReader(f):
                    names[int(row["address"], 16)] = (row["demangled"] or row["name"], row["source"])
        except FileNotFoundError:
            pass
        votes = collections.defaultdict(set)
        for addr, e in J.items():
            if e["matching"] * 100 < min_score or not e.get("diff"):
                continue
            for _, blocks in e["diff"]:
                for b in blocks:
                    bo, br = b.get("orig", []), b.get("recomp", [])
                    if not bo or len(bo) != len(br):
                        continue
                    for x, y in zip(bo, br):
                        a, c = x[1].split("\t")[0].strip(), y[1].split("\t")[0].strip()
                        if mnem(a) != mnem(c) or not ref_kind(a, c):
                            continue
                        m = SYM.search(c)
                        if not m or m.group(1) in a:
                            continue
                        oa = operand_addr(md, img, int(x[0], 16))
                        if oa is not None:
                            votes[(m.group(1), m.group(2))].add((oa, addr))
        for (name, kind), vs in votes.items():
            by = collections.Counter(oa for oa, _ in vs)
            for oa, n in by.most_common():
                on, src = names.get(oa, ("", ""))
                rows.append([tgt, name, kind, f"0x{oa:08x}", n, len(by) - 1, on, src])
    rows.sort(key=lambda r: (r[0], -r[4], r[1]))
    with open(out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["target", "recomp_symbol", "kind", "orig_address", "votes", "other_addresses", "orig_name",
                    "orig_name_source"])
        w.writerows(rows)
    return rows


def inversions(seq):
    return sum(1 for i in range(len(seq)) for j in range(i + 1, len(seq)) if seq[i] > seq[j])


def order(jsons):
    """Per file with markers: descents in source order, and whether PLAYER-ascending is STUDIO-ascending."""
    sizes = {}
    for t, fn in (("PLAYER", "player"), ("STUDIO", "studio")):
        with open(os.path.join(ROOT, "reccmp", fn + "-function-size.csv")) as f:
            for row in csv.reader(f):
                if len(row) == 3 and row[2].isdigit():
                    sizes[(t, int(row[0], 16))] = int(row[2])
    byfile = collections.defaultdict(list)
    pend = {}
    for p, i, tgt, addr, kind, decl, isdef in markers():
        # pair the PLAYER and STUDIO markers of one definition (adjacent lines)
        key = (p, decl)
        pend.setdefault(key, {})[tgt] = (int(addr, 16), kind)
        if tgt == "STUDIO" or True:
            byfile[p].append((i, tgt, int(addr, 16), kind))
    out = []
    for p, ms in byfile.items():
        defs = collections.OrderedDict()
        for i, tgt, addr, kind in ms:
            # markers of one definition sit on consecutive lines; key by the first line of the block
            k = next((d for d in defs if 0 < i - d <= 2 and tgt not in defs[d]), i)
            defs.setdefault(k, {})[tgt] = (addr, kind)
        rows = list(defs.values())
        P = [r["PLAYER"][0] for r in rows if "PLAYER" in r]
        both = [(r["PLAYER"][0], r["STUDIO"][0]) for r in rows if "PLAYER" in r and "STUDIO" in r]
        desc = sum(1 for a, b in zip(P, P[1:]) if b < a)
        s_after_p = [s for _, s in sorted(both)]
        n100 = sum(1 for r in rows if "PLAYER" in r and is_100(jsons["PLAYER"].get(r["PLAYER"][0])))
        big100 = sum(1 for r in rows if "PLAYER" in r and is_100(jsons["PLAYER"].get(r["PLAYER"][0]))
                     and sizes.get(("PLAYER", r["PLAYER"][0]), 0) > 16)
        out.append((os.path.relpath(p, ROOT), len(P), desc, n100, big100, len(both), inversions(s_after_p)))
    return out


def selftest():
    e = {"matching": 0.95, "diff": [["@@", [{"orig": [["0x1", "mov eax, dword ptr [__security_cookie (DATA)]"]],
                                               "recomp": []}]]]}
    assert classify(e)[0] == "stack-cookie"
    e = {"matching": 0.95, "diff": [["@@", [{"orig": [["0x1", "call <OFFSET1>"]],
                                               "recomp": [["0x2", "call Foo (FUNCTION)"]]}]]]}
    assert classify(e)[0] == "unpaired-callee"
    e = {"matching": 0.95, "diff": [["@@", [{"orig": [["0x1", "mov eax, ecx"], ["0x2", "mov edx, 1"]],
                                               "recomp": [["0x3", "mov ebx, ecx"], ["0x4", "mov edx, 1"]]}]]]}
    assert classify(e)[0] == "regalloc-sched"
    e = {"matching": 0.95, "diff": [["@@", [{"orig": [["0x1", "mov eax, dword ptr [<OFFSET1>]"]],
                                               "recomp": [["0x3", "mov eax, dword ptr [X::y (DATA)]"]]}]]]}
    assert classify(e)[0] == "data-ref" and classify(e)[4]
    e = {"matching": 0.95, "diff": [["@@", [{"orig": [["0x1", "mov edx, dword ptr [eax + 0x10]"]],
                                               "recomp": [["0x3", "mov edx, dword ptr [eax + 0xc]"]]}]]]}
    assert classify(e)[0] == "other"
    assert owner_ok("bool Instance::setParentInternal(Instance* p, bool b)", "RBX::Instance::setParentInternal")
    assert not owner_ok("Foo::bar()", "RBX::Baz::bar")
    m = MARK.match("\t// FUNCTION: PLAYER 0x00575d00 FOLDED")
    assert m and m.group(6) and not m.group(7)
    m = MARK.match("// STUB: STUDIO 0x00575d00 // note")
    assert m and not m.group(6) and m.group(7) == " // note"
    print("selftest ok")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("set")
    s.add_argument("kind", choices=["STUB", "FUNCTION"])
    for name in ("apply", "report", "xrefs", "order"):
        s = sub.add_parser(name)
        s.add_argument("player_json")
        s.add_argument("studio_json")
        if name == "report":
            s.add_argument("--out", default=os.path.join(ROOT, "notes", "app-sweep.csv"))
        if name == "xrefs":
            s.add_argument("--out", default=os.path.join(ROOT, "notes", "app-sweep-xrefs.csv"))
            s.add_argument("--min", type=float, default=90.0)
    sub.add_parser("selftest")
    a = ap.parse_args()
    if a.cmd == "selftest":
        return selftest()
    if a.cmd == "set":
        print(dict(rewrite(lambda p, i, t: a.kind)))
        return
    jsons = {"PLAYER": load_json(a.player_json), "STUDIO": load_json(a.studio_json)}
    if a.cmd == "order":
        print("file,player_markers,player_descents,player_100,player_100_over16B,paired_defs,studio_inversions_"
              "after_player_sort")
        for r in sorted(order(jsons), key=lambda r: -r[2]):
            if r[2] or r[3]:
                print(",".join(map(str, r)))
        return
    if a.cmd == "xrefs":
        rows = xrefs(jsons, a.out, a.min)
        print(len(rows), "rows;", collections.Counter((r[0], r[2]) for r in rows))
        return
    if a.cmd == "apply":
        want = wanted(jsons, {t: pdb_names(t) for t in EXE})
        print(dict(rewrite(lambda p, i, t: want.get((p, i), "STUB"))))
        c = collections.Counter((m[2], m[4]) for m in markers())
        print(dict(c))
    else:
        rows = report(jsons, a.out)
        c = collections.Counter((r[3], r[8]) for r in rows)
        for k in sorted(c):
            print(*k, c[k])


if __name__ == "__main__":
    sys.exit(main())

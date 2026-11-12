"""Arity check of proposed PLAYER names: compare each name's Itanium parameter list with the
`ret N` that the PLAYER body actually executes. Virtuals are __thiscall on MSVC, so the callee pops
N = 4 * (stack words), plus 4 when the return value goes through a hidden pointer (a class
returned by value; Itanium names do not encode return types, so that +4 is always allowed).

Parameter words: pointer/reference/int-like/float = 1, double/long long = 2, any other named type
by value (enum = 1, class = 1+) = "at least 1". A row is flagged when N is below the lower bound,
or, with every parameter type known, above 4*words + 4. Functions with no `ret`, or with two
different `ret N` values, are skipped.

Run from the repo root:
  python3 -P tools/rtti_slot_arity_check.py [--class rtti-slot] [--csv ida/exports/PLAYER-names-proposed.csv]
Prints a summary and one TSV line per flagged row. `--selftest` checks the parameter parser.
"""
import argparse
import csv
import json
import sys

import capstone
import pefile

INTLIKE = {"bool", "char", "signed char", "unsigned char", "short", "unsigned short", "int", "unsigned int",
           "long", "unsigned long", "float", "wchar_t", "unsigned", "char16_t", "char32_t"}
WIDE = {"double", "long long", "unsigned long long", "long double", "__int64"}


def params(dem):
    """'A::f(int, G3D::Vector3 const&, double) const' -> list of param strings, or None (varargs)."""
    s = dem.split(" [")[0]
    if "(" not in s:
        return None
    # the parameter list is the last top-level (...) group
    depth, end = 0, None
    for i in range(len(s) - 1, -1, -1):
        c = s[i]
        if c == ")":
            if depth == 0 and end is None:
                end = i
            depth += 1
        elif c == "(":
            depth -= 1
            if depth == 0 and end is not None:
                start = i
                break
    else:
        return None
    body = s[start + 1:end]
    out, depth, cur = [], 0, ""
    for c in body:
        if c in "<(":
            depth += 1
        elif c in ">)":
            depth -= 1
        if c == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += c
    if cur.strip():
        out.append(cur.strip())
    if out == ["void"]:
        out = []
    if "..." in out:
        return None
    return out


def words(p):
    """-> (min_words, exact) for one parameter."""
    q = p.replace("const", "").replace("volatile", "").strip()
    if q.endswith("*") or q.endswith("&") or "(*)" in q or "(&)" in q:
        return 1, True
    q = " ".join(q.split())
    if q in WIDE:
        return 2, True
    if q in INTLIKE:
        return 1, True
    return 1, False


def selftest():
    assert params("A::f()") == []
    assert params("A::f(int, G3D::Vector3 const&, double) const") == ["int", "G3D::Vector3 const&", "double"]
    assert params("A<B, C>::f(std::map<int, int>, bool (*)(int, int))") == ["std::map<int, int>", "bool (*)(int, int)"]
    assert words("double") == (2, True) and words("RBX::Name const&") == (1, True)
    assert words("RBX::Canvas") == (1, False)
    print("selftest ok")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default="ida/exports/PLAYER-names-proposed.csv")
    ap.add_argument("--cls", default="rtti-slot")
    ap.add_argument("--pe", default="bin/player_1023/RobloxPlayerBeta.exe")
    ap.add_argument("--functions", default="ida/exports/PLAYER-evidence/functions.jsonl")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        selftest()
        return
    pe = pefile.PE(a.pe, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    size = {}
    for l in open(a.functions):
        d = json.loads(l)
        size[d["address"]] = d["size"]
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    cache = {}

    def rets(ea):
        if ea not in cache:
            data = pe.get_data(ea - base, size.get(ea, 0))
            r = set()
            for ins in md.disasm(data, ea):
                if ins.mnemonic == "ret":
                    r.add(int(ins.op_str, 16) if ins.op_str else 0)
            cache[ea] = r
        return cache[ea]

    n = checked = 0
    flagged = []
    for r in csv.DictReader(open(a.csv)):
        if r["evidence_class"] != a.cls:
            continue
        n += 1
        ea = int(r["address"], 16)
        rs = rets(ea)
        ps = params(r["demangled"])
        if len(rs) != 1 or ps is None:
            continue
        N = rs.pop()
        rs.add(N)
        ws = [words(p) for p in ps]
        lo = 4 * sum(w for w, _ in ws)
        exact = all(e for _, e in ws)
        checked += 1
        if N < lo or (exact and N > lo + 4) or N % 4:
            flagged.append((r["address"], N, lo, "exact" if exact else ">=", r["demangled"], r["oracle"], r["evidence"][:160]))
    print("rows %d, checked %d (one ret N and a parsable parameter list), flagged %d" % (n, checked, len(flagged)))
    w = csv.writer(sys.stdout, delimiter="\t")
    for f in flagged:
        w.writerow(f)


if __name__ == "__main__":
    main()

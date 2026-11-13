"""Name coverage of the App block and where the unnamed functions sit, grouped by the class of their callers.
Offline (no IDA): reads ida/exports/<T>-functions.csv, <T>-names.csv (or --names), <T>-evidence/functions.jsonl
(callees, func_refs) and <T>-evidence/vtables.jsonl.

    python3 -P tools/ida/app_name_coverage.py PLAYER [--names old-names.csv] [--csv ida/exports/PLAYER-app-unnamed-by-class.csv]

"Named" means an ios:/mac:/match: name (the oracle or a byte match), as the Phase 5 status counts it; other
sources (rtti:, prior:, lib:, flirt:, ida:, str:) are counted apart. Each unnamed App function gets a class:
  vtable   it is a slot of a vftable (the RTTI class of that vftable; several classes: the most frequent)
  callers  the most frequent class among its named callers (a caller's class: the qualifier of its demangled
           name, template arguments collapsed to <>; a free function counts as its namespace + "::(free)")
  prop     no named caller: the most frequent class already given to its callers (three rounds)
  -        no caller with a class
App block (CLAUDE.md "Existing App decomp"): 0x00606a00-0x009ad3a0 in PLAYER; --lo/--hi for another range.
"""
import argparse
import csv
import json
import os
import re
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EXP = os.path.join(ROOT, "ida", "exports")
NAMED = ("ios:", "mac:", "match:")


def qualname(dem):
    """'public: void __thiscall RBX::A<int>::f(int)' or 'RBX::A<int>::f(int) const' -> 'RBX::A<>::f'."""
    depth, cut = 0, len(dem)
    for k, ch in enumerate(dem):
        depth += ch in "<"
        depth -= ch in ">"
        if ch == "(" and depth == 0:
            cut = k
            break
    head = dem[:cut]
    out, depth = [], 0
    for ch in head:     # collapse template arguments
        if ch == "<":
            depth += 1
            if depth == 1:
                out.append("<>")
            continue
        if ch == ">":
            depth -= 1
            continue
        if depth == 0:
            out.append(ch)
    return "".join(out).split(" ")[-1].lstrip("*&")


def cls_of(dem):
    q = qualname(dem or "")
    if "::" not in q:
        return None
    c = q.rsplit("::", 1)[0]
    if c in ("RBX", "G3D", "std", "boost") or c.count("::") == 0 and c[:1].islower():
        return c + "::(free)"
    return c


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("target")
    ap.add_argument("--names")
    ap.add_argument("--csv")
    ap.add_argument("--lo", type=lambda s: int(s, 16), default=0x00606a00)
    ap.add_argument("--hi", type=lambda s: int(s, 16), default=0x009ad3a0)
    ap.add_argument("--top", type=int, default=40)
    o = ap.parse_args()
    T = o.target
    funcs = {int(r["address"], 16): int(r["size"]) for r in csv.DictReader(open(os.path.join(EXP, f"{T}-functions.csv")))}
    names = {int(r["address"], 16): r for r in csv.DictReader(open(o.names or os.path.join(EXP, f"{T}-names.csv")))
             if r["kind"] == "func"}
    app = [a for a in funcs if o.lo <= a < o.hi]
    src = Counter(names[a]["source"] if a in names else "unnamed" for a in app)
    unnamed = {a for a in app if a not in names or names[a]["source"] not in NAMED}

    callers = defaultdict(set)
    for line in open(os.path.join(EXP, f"{T}-evidence", "functions.jsonl")):
        f = json.loads(line)
        for c in f.get("callees", []) + f.get("func_refs", []):
            callers[c].add(f["address"])
    vt = defaultdict(Counter)
    for line in open(os.path.join(EXP, f"{T}-evidence", "vtables.jsonl")):
        v = json.loads(line)
        for g in v.get("groups", []):
            for s in g.get("slots", []):
                vt[s["ea"]][v["class"]] += 1

    def named_cls(a):
        r = names.get(a)
        return cls_of(r["demangled"] or r["name"]) if r and r["source"] in NAMED + ("rtti:", "prior:", "lib:", "str:") else None

    got = {}
    for a in unnamed:
        if vt.get(a):
            got[a] = ("vtable", vt[a].most_common(1)[0][0])
            continue
        c = Counter(x for x in (named_cls(b) for b in callers.get(a, ())) if x)
        if c:
            got[a] = ("callers", c.most_common(1)[0][0])
    for _ in range(3):
        add = {}
        for a in unnamed - set(got):
            c = Counter(got[b][1] for b in callers.get(a, ()) if b in got)
            if c:
                add[a] = ("prop", c.most_common(1)[0][0])
        if not add:
            break
        got.update(add)

    by = defaultdict(lambda: [0, 0, Counter()])
    for a in unnamed:
        how, c = got.get(a, ("-", "(no caller with a class)"))
        by[c][0] += 1
        by[c][1] += funcs[a]
        by[c][2][how] += 1
    print(f"{T} App block 0x{o.lo:08x}-0x{o.hi:08x}: {len(app)} functions; ios:/mac:/match: {len(app) - len(unnamed)}, "
          f"without one {len(unnamed)} ({100.0 * len(unnamed) / len(app):.1f}%)")
    print("  by source: " + ", ".join(f"{k} {v}" for k, v in src.most_common()))
    print("  grouping: " + ", ".join(f"{k} {v}" for k, v in Counter(g[0] for g in got.values()).most_common())
          + f", none {len(unnamed) - len(got)}")
    rows = sorted(by.items(), key=lambda kv: -kv[1][0])
    print(f"  {'class (of callers / vftable)':70s} {'funcs':>6s} {'bytes':>8s}  how")
    for c, (n, b, how) in rows[:o.top]:
        print(f"  {c[:70]:70s} {n:6d} {b:8d}  " + " ".join(f"{k}:{v}" for k, v in how.most_common()))
    if o.csv:
        with open(o.csv, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["class", "functions", "bytes", "vtable", "callers", "prop", "none"])
            for c, (n, b, how) in rows:
                w.writerow([c, n, b, how["vtable"], how["callers"], how["prop"], how["-"]])


if __name__ == "__main__":
    assert qualname("public: void __thiscall RBX::A<int,B<C> >::f(int)") == "RBX::A<>::f"
    assert qualname("RBX::DataModel::onChildAdded(RBX::Instance*)") == "RBX::DataModel::onChildAdded"
    assert cls_of("RBX::foo(int)") == "RBX::(free)"
    main()

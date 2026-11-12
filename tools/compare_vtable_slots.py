"""Compare vftable slot counts of the same classes between an MSVC evidence export (PLAYER,
PLAYER_1015, STUDIO) and an Itanium one (IOS_1029, MACSTUDIO), as a check of the MSVC scanner.

Classes are matched by demangled name (non-template names only: the two demanglers spell template
arguments differently). Groups are compared in order (MSVC by COL offset, Itanium by
offset-to-top), and only when both sides have the same number of groups; the expected MSVC
count of each group is worked out from the Itanium names (see expected()).

Run:  python3 -P tools/compare_vtable_slots.py MSVC_DIR ITANIUM_DIR [N_SHOWN] [SEED]
  e.g. python3 -P tools/compare_vtable_slots.py ida/exports/PLAYER-evidence ida/exports/IOS_1029-evidence 20
"""
import json
import random
import re
import sys
from collections import Counter


def load(path, kind="vtable"):
    out = {}
    for l in open(path + "/vtables.jsonl"):
        d = json.loads(l)
        if d["kind"] == kind and "<" not in d["class"]:
            out.setdefault(d["class"], d)
    return out


def dtors(slots):
    return sum(1 for s in slots if re.search(r"D[01]Ev$|D[01]Ev\$|~", s["name"] or ""))


def base_name(n):
    """`__ZThn24_N3RBX8Instance...` (a this-adjusting thunk) -> `ZN3RBX8Instance...`."""
    n = (n or "").lstrip("_")
    return re.sub(r"^ZT[hv](?:n?\d+_)+", "Z", n)


def expected(ig):
    """MSVC slot count expected for each Itanium group, as a list per group of the counts allowed.
    Itanium has one slot more per group with a virtual destructor (D1 + D0), and its primary group
    also holds a slot for every function that overrides a function of a non-primary base (the
    secondary groups reach it through a `_ZThn` thunk); MSVC puts those overriders only in the
    secondary vftable. That includes the destructor when the primary base has no virtual
    destructor of its own (RBX::Instance: RBX::Debugable has only dump(); its destructor
    overrides secondary bases'), which the names cannot tell from the case where it has one, so
    a thunked destructor allows both counts."""
    thunked = {base_name(x["name"]) for g in ig[1:] for x in g["slots"] if x["name"] and x["name"].lstrip("_").startswith("ZT")}
    out = []
    for k, g in enumerate(ig):
        n = len(g["slots"]) - (1 if dtors(g["slots"]) >= 2 else 0)
        alt = n
        if k == 0:
            n -= sum(1 for x in g["slots"] if base_name(x["name"]) in thunked and not re.search(r"D[01]Ev$", x["name"] or ""))
            alt = n - (1 if any(re.search(r"D1Ev$", x["name"] or "") and base_name(x["name"]) in thunked for x in g["slots"]) else 0)
        out.append({n, alt})
    return out


def main():
    ms, it = load(sys.argv[1]), load(sys.argv[2])
    shown = int(sys.argv[3]) if len(sys.argv) > 3 else 20
    rnd = random.Random(int(sys.argv[4]) if len(sys.argv) > 4 else 1)
    common = sorted(set(ms) & set(it))
    verdict, rows = Counter(), {}
    for c in common:
        mg = sorted(ms[c]["groups"], key=lambda g: g["col_offset"])
        ig = sorted(it[c]["groups"], key=lambda g: -g["offset_to_top"])
        if len(mg) != len(ig):
            verdict["group count differs"] += 1
            rows[c] = ("group count differs", [len(g["slots"]) for g in mg], [len(g["slots"]) for g in ig], None)
            continue
        exp = expected(ig)
        got = [len(g["slots"]) for g in mg]
        ok = all(a in e for a, e in zip(got, exp))
        verdict["slot counts as expected" if ok else "slot count differs"] += 1
        rows[c] = ("ok" if ok else "DIFFERS", got, [len(g["slots"]) for g in ig], [sorted(e) for e in exp])
    print("classes in both (non-template names): %d of %d MSVC / %d Itanium" % (len(common), len(ms), len(it)))
    for k, v in verdict.most_common():
        print("  %s: %d" % (k, v))
    pick = sorted(rnd.sample([c for c in common if c.startswith("RBX::")], min(shown, len(common))))
    print("\nspot checks: MSVC slots per vftable | Itanium slots per group | expected MSVC from Itanium")
    for c in pick:
        print("  %-50s %-8s %s | %s | %s" % ((c,) + rows[c]))
    bad = [c for c in common if rows[c][0] != "ok"]
    print("\nfirst differences:")
    for c in bad[:15]:
        print("  %-50s %s %s | %s | %s" % ((c,) + rows[c]))


if __name__ == "__main__":
    main()

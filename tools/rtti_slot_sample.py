"""Draw a fixed-seed, stratified sample of rtti-slot rows from <TARGET>-names-proposed.csv for an
adversarial review (decompile both sides by hand). One representative row per address.

Strata (sampled without replacement, in this order, each address used once):
  overload  the class's iOS vtable repeats an unqualified method name (MSVC regroups overloads)
  mi        the slot is in a secondary MSVC vftable (vftable k >= 1)
  tail      the vftable has >= 40 slots and the slot is among its last 8
  folded    the address carries several oracle names (ICF folding was assumed)
  random    anything left

Run from the repo root:
  python3 -P tools/rtti_slot_sample.py [--seed 20261112] [--n overload=15,mi=15,tail=15,folded=5,random=10]
Writes TSV to stdout: stratum, address, oracle, mangled, demangled, class, vftable, slot, nslots.
"""
import argparse
import csv
import json
import random
import re
import sys
from collections import defaultdict

sys.path.insert(0, "tools")
from pair_names import norm_class, unqualified  # noqa: E402

EV = re.compile(r"(?:ios1029|mac): (?:folded \(\d+ oracle functions\): )?(.+?) vftable (\d+) slot (\d+)")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--seed", type=int, default=20261112)
    ap.add_argument("--n", default="overload=15,mi=15,tail=15,folded=5,random=10")
    a = ap.parse_args()
    want = {k: int(v) for k, v in (x.split("=") for x in a.n.split(","))}
    pv = {}
    for l in open("ida/exports/PLAYER-evidence/vtables.jsonl"):
        d = json.loads(l)
        if d.get("class"):
            pv[norm_class(d["class"])] = d
    ov = {}
    for l in open("ida/exports/IOS_1029-evidence/vtables.jsonl"):
        d = json.loads(l)
        if d.get("class"):
            names = [unqualified(s.get("dem") or s["name"] or "") for g in d["groups"] for s in g["slots"]]
            names = [n for n in names if n and not n.startswith("sub_") and not n.startswith("~")]
            ov[norm_class(d["class"])] = len(names) != len(set(names))
    rows = defaultdict(list)
    for r in csv.DictReader(open("ida/exports/PLAYER-names-proposed.csv")):
        if r["evidence_class"] == "rtti-slot":
            rows[r["address"]].append(r)
    info = {}
    for ad, rs in sorted(rows.items()):
        r = sorted(rs, key=lambda r: r["oracle"] != "both")[0]
        m = EV.search(r["evidence"])
        cls, k, i = m.group(1), int(m.group(2)), int(m.group(3))
        c = norm_class(cls)
        groups = sorted(pv[c]["groups"], key=lambda g: g["col_offset"]) if c in pv else []
        n = len(groups[k]["slots"]) if k < len(groups) else -1
        info[ad] = dict(r=r, cls=cls, k=k, i=i, n=n, folded=len(rs) > 1, over=ov.get(c, False))
    rnd = random.Random(a.seed)
    used = set()
    tests = {"overload": lambda x: x["over"], "mi": lambda x: x["k"] >= 1,
             "tail": lambda x: x["n"] >= 40 and x["i"] >= x["n"] - 8, "folded": lambda x: x["folded"],
             "random": lambda x: True}
    w = csv.writer(sys.stdout, delimiter="\t")
    for st, cnt in want.items():
        pool = [ad for ad in sorted(info) if ad not in used and tests[st](info[ad])]
        pick = rnd.sample(pool, min(cnt, len(pool)))
        print("# %s: %d of %d" % (st, len(pick), len(pool)), file=sys.stderr)
        for ad in sorted(pick):
            used.add(ad)
            x = info[ad]
            w.writerow([st, ad, x["r"]["oracle"], x["r"]["mangled"], x["r"]["demangled"], x["cls"], x["k"], x["i"], x["n"]])


if __name__ == "__main__":
    main()

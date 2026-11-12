"""Draw a fixed-seed sample of rows from ida/exports/<TARGET>-names-proposed.csv for an
adversarial body check, and resolve each row's oracle function addresses by name.

Run from the repo root:
  python3 -P tools/sample_proposed_names.py --seed 20261112 \
      --take callgraph:ios:=30 --take callgraph-guess:guess:=20 --take import-set:=10 \
      --out /tmp/.../sample.json

Each --take is evidence_class:source_tag_prefix=N (an empty prefix matches any tag). Rows are
sorted by address before sampling, so the same seed always draws the same rows. The output
lists, per row, the target address and the oracle addresses (by default ios1029 from IOS_1029,
mac from MACSTUDIO; --oracle tag=TARGET replaces the set, e.g. --oracle ios1021=IOS_1021 for
STUDIO) whose symbol equals the row's mangled name; tools/ida/dump_pseudocode.py reads the
address lists.
"""
import argparse
import csv
import json
import random

ORACLES = {"ios1029": "IOS_1029", "mac": "MACSTUDIO"}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", default="PLAYER")
    ap.add_argument("--seed", type=int, required=True)
    ap.add_argument("--take", action="append", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--oracle", action="append")
    a = ap.parse_args()
    oracles = dict(o.split("=") for o in a.oracle) if a.oracle else ORACLES
    rows = sorted(csv.DictReader(open("ida/exports/%s-names-proposed.csv" % a.target)),
                  key=lambda r: (int(r["address"], 16), r["mangled"]))
    rng = random.Random(a.seed)
    picked = []
    for t in a.take:
        spec, n = t.rsplit("=", 1)
        cls, tag = spec.split(":", 1)
        pool = [r for r in rows if r["evidence_class"] == cls and r["source_tag"].startswith(tag)]
        picked += rng.sample(pool, min(int(n), len(pool)))
    want = {r["mangled"].lstrip("_") for r in picked}
    where = {k: {} for k in oracles}
    for k, t in oracles.items():
        for line in open("ida/exports/%s-evidence/functions.jsonl" % t):
            f = json.loads(line)
            n = (f.get("name") or "").lstrip("_")
            if n in want:
                where[k].setdefault(n, []).append(f["address"])
    for r in picked:
        n = r["mangled"].lstrip("_")
        r["oracle_ea"] = {k: where[k].get(n, []) for k in oracles}
    json.dump(picked, open(a.out, "w"), indent=1)
    print("%d rows -> %s" % (len(picked), a.out))


if __name__ == "__main__":
    main()

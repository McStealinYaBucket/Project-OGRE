"""Draw a fixed-seed, weighted sample of string-unique / string-set rows from
ida/exports/<TARGET>-names-proposed.csv and print a side-by-side evidence card for each
(PLAYER function vs the oracle function of the proposed name): size, blocks, strings,
callees with their proposed/oracle names, callers.

At most --per-family rows per template family. Weights over-sample the rows most likely to be wrong: short or generic strings, asserts from
headers (file: *.h), FILE: keys and string-set rows. The sample is reproducible from --seed.

  python3 -P tools/adversarial_string_sample.py --seed 20261112 --n 60 > /tmp/cards.txt
  python3 -P tools/adversarial_string_sample.py --rows 0x0062b470,0x00559450   # cards for given rows

Prints `ROW <addr> <oracle-tag> <oracle-addr>` lines that the IDA dump script can consume.
Reads only the evidence exports; opens no database.
"""
import argparse
import csv
import json
import random
import re
from collections import defaultdict

EXP = "ida/exports"


def load(target):
    F, byname = {}, defaultdict(list)
    for l in open("%s/%s-evidence/functions.jsonl" % (EXP, target)):
        d = json.loads(l)
        F[d["address"]] = d
        byname[d["name"].lstrip("_")].append(d["address"])
    return F, byname


def weight(r):
    ev = r["evidence"]
    m = re.search(r"'(.*?)'", ev)
    s = m.group(1) if m else ""
    w = 1.0
    if r["evidence_class"] == "string-set":
        w *= 2
    if len(s) < 14:
        w *= 3
    if re.fullmatch(r"[A-Za-z_][\w.:]*", s):  # identifier-like, generic
        w *= 2
    if re.search(r"file: \S+\.(h|hpp|inl) ", s) or "FILE:" in s:
        w *= 3
    if "<" in r["demangled"]:  # template instantiation
        w *= 1.5
    return w


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", default="PLAYER")
    ap.add_argument("--seed", type=int, default=20261112)
    ap.add_argument("--n", type=int, default=60)
    ap.add_argument("--per-family", type=int, default=3, help="cap per template family")
    ap.add_argument("--rows")
    a = ap.parse_args()
    rows = [r for r in csv.DictReader(open("%s/%s-names-proposed.csv" % (EXP, a.target)))
            if r["evidence_class"] in ("string-unique", "string-set")]
    if a.rows:
        want = {int(x, 16) for x in a.rows.split(",")}
        sample = [r for r in rows if int(r["address"], 16) in want]
    else:
        rng = random.Random(a.seed)
        pool = list(rows)
        sample, fam = [], defaultdict(int)
        while len(sample) < a.n and pool:  # weighted sampling without replacement
            ws = [weight(r) for r in pool]
            r = pool.pop(rng.choices(range(len(pool)), ws)[0])
            k = re.sub(r"<.*>", "<>", r["demangled"].split("(")[0])  # template family
            if fam[k] < a.per_family:
                fam[k] += 1
                sample.append(r)
    T, _ = load(a.target)
    prop = defaultdict(list)
    for r in csv.DictReader(open("%s/%s-names-proposed.csv" % (EXP, a.target))):
        prop[int(r["address"], 16)].append(r["demangled"])
    O = {"ios1029": load("IOS_1029"), "mac": load("MACSTUDIO")}
    for r in sample:
        p = int(r["address"], 16)
        tags = ["ios1029", "mac"] if r["oracle"] == "both" else [r["oracle"]]
        t = T[p]
        print("=" * 100)
        print("%s  %s  [%s/%s]" % (r["address"], r["demangled"], r["evidence_class"], r["oracle"]))
        print("  evidence:", r["evidence"][:300])
        print("  PLAYER size=%d blocks=%d callers=%d strings=%s" % (t["size"], t["blocks"], t["callers"], t["strings"][:12]))
        for c in t["callees"][:25]:
            print("    -> 0x%08x %s %s" % (c, T.get(c, {}).get("name", "?"), prop.get(c, [""])[0][:90]))
        for tag in tags:
            F, by = O[tag]
            for oa in by.get(r["mangled"].lstrip("_"), []):
                d = F[oa]
                print("  ROW %s %s 0x%x" % (r["address"], tag, oa))
                print("  %s size=%d blocks=%d callers=%d strings=%s" % (tag, d["size"], d["blocks"], d["callers"], d["strings"][:12]))
                for c in d["callees"][:25]:
                    cd = F.get(c, {})
                    print("    -> 0x%x %s" % (c, (cd.get("demangled") or cd.get("name", "?"))[:110]))


if __name__ == "__main__":
    main()

"""Per-library reccmp score: how many reccmp/<t>-<lib>.csv rows reccmp paired, and how many of those are 100%.

Run from the build directory after a build (reccmp writes the JSON; this script only reads it):
    PYTHONIOENCODING=utf-8 ../toolchain/venv/bin/reccmp-reccmp --target PLAYER --json player.json
    python3 -P ../tools/lib/lib_score.py PLAYER player.json            # table
    python3 -P ../tools/lib/lib_score.py PLAYER player.json --list     # plus every unpaired / <100% row
    python3 -P ../tools/lib/lib_score.py PLAYER player.json --classify # why each <100% row differs

--classify reads the diff reccmp puts in the JSON (so do not pass --json-diet) and labels every differing
instruction pair; a function's cause is the set of labels (see CAUSES).

"100%" counts reccmp's exact ratio 1.0 and its effective matches (JSON key "effective": true).
"""
import collections
import csv
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
# libdata: only its function rows (callees paired by position from library callers; tools/ida/gen_reccmp_csv.py).
LIBS = ("lua", "raknet", "g3d", "zlib", "libzip", "boost", "libdata")


def rows(target, lib):
    path = os.path.join(ROOT, "reccmp", f"{target.lower()}-{lib}.csv")
    with open(path) as f:
        rs = list(csv.DictReader(l for l in f if not l.startswith("#")))
    return [r for r in rs if r["type"] in ("library", "")] if lib == "libdata" else rs


CAUSES = {
    "file": "__FILE__ string: original build-agent path vs our O:\\ path",
    "unresolved": "call/ref to a symbol our link leaves unresolved (/FORCE:UNRESOLVED: Log.dll, RBX code)",
    "crt-thunk": "original calls a static CRT stub where ours calls the MSVCR90 import thunk",
    "reloc-heuristic": "immediate in the original taken for an address (no .reloc in the original)",
    "ehhandler": "push of the function's __ehhandler$ stub: no PDB symbol for it, so no recompiled entity",
    "const": "a float constant with a different value (the original copy came from another TU)",
    "data": "data reference not paired (no libdata row, or COMDAT/static copy from another TU)",
    "icf-roblox": "the original's callee is a library body /OPT:ICF folded with a Roblox function (oracle-named, left unpaired)",
    "callee": "call/ref to a different function (ICF folding in either image, or callee not paired)",
    "code": "instructions differ",
}


def classify_pair(o, r):
    """Label one differing orig/recomp instruction text pair."""
    if o is None or r is None:
        return "code"
    o, r = o.split("\t")[0].strip(), r.split("\t")[0].strip()  # drop reccmp's "(file.c:line)" note
    if "(STRING)" in o and "(STRING)" in r and ("BuildAgent" in o or "buildagent" in o):
        return "file"
    if "(IMPORT_THUNK)" in r and "<OFFSET" in o:
        return "crt-thunk"
    if "<OFFSET" in r and not o.startswith("push 0x") or "(IMPORT_THUNK)" in o and "Log.dll" in o:
        return "unresolved"
    if o.startswith("push 0x") and r.startswith("push <OFFSET"):
        return "ehhandler"
    if o.count("(FLOAT)") == 1 and r.count("(FLOAT)") == 1:
        return "const"
    om, rm = o.split(" ", 1)[0], r.split(" ", 1)[0]
    if om != rm:
        return "code"
    if "<OFFSET" in o and "(" not in r and re.search(r"0x[0-9a-f]+", r):
        return "reloc-heuristic"
    if re.search(r"RBX::.*\(FUNCTION\)", o) and "(FUNCTION)" in r:
        return "icf-roblox"
    if "(DATA)" in r or "(OFFSET)" in r or "(UNK)" in r or "(FLOAT)" in r or "(VTABLE)" in r:
        return "data"
    if "(FUNCTION)" in r or "(FUNCTION)" in o or "<OFFSET" in o:
        return "callee"
    return "code"


def classify(entry):
    labels = set()
    for _, chunks in entry.get("diff") or []:
        for c in chunks:
            if "both" in c:
                continue
            os_, rs = c.get("orig", []), c.get("recomp", [])
            for i in range(max(len(os_), len(rs))):
                o = os_[i][1] if i < len(os_) else None
                r = rs[i][1] if i < len(rs) else None
                labels.add(classify_pair(o, r))
    return "+".join(sorted(labels)) or "no diff in JSON"


def main(target, path, listing, classifying=False):
    got = {int(e["address"], 16): e for e in json.load(open(path))["data"]}
    print(f"{'library':8} {'rows':>6} {'paired':>7} {'100%':>6} {'<100%':>6} {'unpaired':>8}")
    tot = [0] * 5
    for lib in LIBS:
        rs = rows(target, lib)
        paired = [r for r in rs if int(r["address"], 16) in got]
        full = [r for r in paired if got[int(r["address"], 16)]["matching"] == 1.0
                or got[int(r["address"], 16)].get("effective")]
        n = (len(rs), len(paired), len(full), len(paired) - len(full), len(rs) - len(paired))
        tot = [a + b for a, b in zip(tot, n)]
        print(f"{lib:8} {n[0]:6} {n[1]:7} {n[2]:6} {n[3]:6} {n[4]:8}")
        if classifying:
            causes = collections.Counter(classify(got[int(r["address"], 16)]) for r in paired if r not in full)
            for k, v in causes.most_common():
                print(f"   {v:5} {k}")
        if listing:
            for r in rs:
                e = got.get(int(r["address"], 16))
                if e is None:
                    print(f"   unpaired 0x{r['address']} {r['symbol']}")
                elif not (e["matching"] == 1.0 or e.get("effective")):
                    print(f"   {e['matching']*100:6.2f}% 0x{r['address']} recomp {e['recomp']} {r['symbol']}")
    print(f"{'total':8} {tot[0]:6} {tot[1]:7} {tot[2]:6} {tot[3]:6} {tot[4]:8}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], "--list" in sys.argv, "--classify" in sys.argv)
    if "--classify" in sys.argv:
        print("\n".join(f"{k}: {v}" for k, v in CAUSES.items()))

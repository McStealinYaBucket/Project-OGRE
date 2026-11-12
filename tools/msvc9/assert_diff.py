"""Classify reccmp diffs that come from assert __FILE__ / __LINE__ strings.

    toolchain/venv/bin/python -P tools/msvc9/assert_diff.py P.json [--before OLD.json] [--lines N]

P.json is `reccmp-reccmp --target PLAYER|STUDIO --json P.json` (run without --nolib to include library
rows). Each function below effective 100% is put in one class:
  file-only   every changed line pair differs only in the path inside an assert string (or a bare
              __FILE__ string): the build-machine path spelling is the whole diff.
  line-only   only the "line: N" inside an assert string, or a pushed immediate in a function that
              pushes a __FILE__ string (the __LINE__ argument), differs.
  file+line   both, and nothing else.
  other       anything else (including functions where assert strings differ among other changes).
Prints the counts (library and non-library), and with --lines N, N examples of line misses as
(file, 2013 line, our line). With --before, also lists functions at effective 100% in OLD that are
not in P (drops), and how many OLD file-only/file+line functions are now at 100%.
"""
import argparse
import collections
import json
import re

STR = re.compile(r'"((?:[^"\\]|\\.)*)" \(STRING\)')
ASSERT = re.compile(r"^(.*) file: (.*) line: (\d+)$", re.S)
PATH = re.compile(r"^[A-Za-z]:\\\\")
IMM = re.compile(r"push (0x[0-9a-f]+|\d+)")


def load(path):
    return {int(e["address"], 16): e for e in json.load(open(path))["data"]}


def is_100(e):
    return bool(e.get("matching") == 1.0 or e.get("effective"))


def split_str(line):
    """(prefix-without-string, message, file, line) for a line with an assert/__FILE__ string."""
    m = STR.search(line)
    if not m:
        return None
    s = m.group(1)
    a = ASSERT.match(s)
    rest = line[:m.start()] + line[m.end():]
    if a:
        return rest, a.group(1), a.group(2), a.group(3)
    if PATH.match(s):
        return rest, None, s, None
    return None


def pair_kind(o, r):
    """set of 'file'/'line' for a differing line pair, or None if it is some other difference."""
    so, sr = split_str(o), split_str(r)
    if so and sr:
        if so[0] != sr[0] or so[1] != sr[1]:
            return None
        k = set()
        if so[2] != sr[2]:
            k.add("file")
        if so[3] != sr[3]:
            k.add("line")
        return k
    return None


def classify(e):
    kinds, lines = set(), []
    imms, has_file = 0, False
    for _, blocks in e.get("diff") or []:
        for b in blocks:
            has_file |= any(split_str(x[1]) for side in b.values() for x in side)
            if "both" in b:
                continue
            o = [x[1] for x in b.get("orig", [])]
            r = [x[1] for x in b.get("recomp", [])]
            if len(o) != len(r) or not o:
                return "other", []
            for x, y in zip(o, r):
                if x == y:
                    continue
                k = pair_kind(x, y)
                if k is None:
                    # an immediate __LINE__ argument (assert handlers taking file, line); counted as a
                    # line difference only if the function pushes a __FILE__ string somewhere
                    if IMM.fullmatch(x) and IMM.fullmatch(y):
                        imms += 1
                        continue
                    return "other", []
                kinds |= k
                if "line" in k:
                    so, sr = split_str(x), split_str(y)
                    lines.append((so[2], int(so[3]), int(sr[3])))
    if imms:
        if not has_file:
            return "other", []
        kinds.add("line")
    if not kinds:
        return "other", []
    return {"file": "file-only", "line": "line-only"}.get("+".join(sorted(kinds)), "file+line"), lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("json")
    ap.add_argument("--before")
    ap.add_argument("--lines", type=int, default=0)
    a = ap.parse_args()
    cur = load(a.json)
    counts = collections.Counter()
    examples = []
    for e in cur.values():
        if is_100(e) or not e.get("diff"):
            continue
        c, ls = classify(e)
        counts[(c, bool(e.get("library")))] += 1
        if ls and c in ("line-only", "file+line"):
            examples.append((e["name"], ls[0]))
    print(f"{a.json}: {sum(map(is_100, cur.values()))} at effective 100% of {len(cur)}")
    for c in ("file-only", "file+line", "line-only"):
        print(f"  {c:10} library {counts[(c, True)]:5}  other {counts[(c, False)]:5}")
    for name, (f, o, r) in examples[:a.lines]:
        print(f"    {name}: {f} 2013 line {o}, ours {r}")
    if a.before:
        old = load(a.before)
        drops = [e for k, e in old.items() if is_100(e) and not (k in cur and is_100(cur[k]))]
        print(f"  drops vs {a.before}: {len(drops)}")
        for e in drops:
            print(f"    {e['address']} {e['name']} -> {cur.get(int(e['address'], 16), {}).get('matching')}")
        gained = collections.Counter()
        for k, e in old.items():
            if not is_100(e) and e.get("diff") and k in cur and is_100(cur[k]):
                gained[(classify(e)[0], bool(e.get("library")))] += 1
        print("  gained (by class in before):", dict(gained))


if __name__ == "__main__":
    main()

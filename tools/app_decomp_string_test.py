"""String-reference test: which binary do the prior App decomp's `// 0xADDRESS` comments address?

Step 1, pull (address comment, string literal in the function it annotates) pairs from the tree:
    python3 -P tools/app_decomp_string_test.py extract "2013 55% App" work/pairs.json work/strings.json
Step 2, run tools/ida/string_refs.py with work/strings.json on a scratch copy of each database,
    writing work/<TARGET>.json (see that script's docstring).
Step 3, score:
    python3 -P tools/app_decomp_string_test.py score work/pairs.json work/MACSTUDIO.json work/PLAYER.json ...

A pair passes for a target under offset D when address+D lies inside a function that references
the string. Besides D=0 the scorer votes over every 64K-aligned D (a PE rebase is 64K-aligned)
and every 4K-aligned D, and prints the best one per target: a real constant base shows up as one
D collecting most pairs.
"""
import json, os, re, sys
from collections import Counter, defaultdict

ADDR = re.compile(r"\b0x([0-9A-Fa-f]{5,8})\b")
LIT = re.compile(r'"((?:[^"\\\n]|\\.)*)"')
ESC = {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"', "'": "'", "0": "\0"}
WINDOW = 40
# ponytail: the "function a comment annotates" is a line window (comment line to the next
# closing brace at indent <= 1, the next different address, or 40 lines). A real C++ parse
# would pair more comments; this already yields far more than the 60 the test needs.
FUNC_END = re.compile(r"^\t?}\s*;?\s*$")


def extract(root, pairs_out, strings_out):
    pairs = []
    for d, _, files in os.walk(root):
        for fn in files:
            if not fn.endswith(".cpp"):
                continue
            path = os.path.join(d, fn)
            lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
            for i, ln in enumerate(lines):
                p = ln.find("//")
                if p < 0:
                    continue
                addrs = [int(m.group(1), 16) for m in ADDR.finditer(ln[p:])]
                if len(addrs) != 1 or addrs[0] < 0x1000:  # one real address on the line: unambiguous pairing
                    continue
                for j in range(i, min(i + WINDOW, len(lines))):
                    code = lines[j]
                    q = code.find("//")
                    if j > i and q >= 0 and any(int(m.group(1), 16) != addrs[0] for m in ADDR.finditer(code[q:])):
                        break
                    if code.lstrip().startswith("#"):
                        continue
                    for m in LIT.finditer(code if q < 0 else code[:q]):
                        s = re.sub(r"\\(.)", lambda e: ESC.get(e.group(1), e.group(1)), m.group(1))
                        if len(s) >= 10:
                            pairs.append({"file": os.path.relpath(path, root), "line": i + 1, "addr": addrs[0], "str": s})
                    if j > i and FUNC_END.match(code):
                        break
    # distinctive = the literal is tied to exactly one address in the whole tree
    owners = defaultdict(set)
    for p in pairs:
        owners[p["str"]].add(p["addr"])
    seen, keep = set(), []
    for p in pairs:
        k = (p["addr"], p["str"])
        if len(owners[p["str"]]) == 1 and k not in seen:
            seen.add(k)
            keep.append(p)
    json.dump(keep, open(pairs_out, "w"), indent=0)
    json.dump(sorted({p["str"] for p in keep}), open(strings_out, "w"))
    print("pairs: %d   distinct addresses: %d   distinct strings: %d" % (len(keep), len({p["addr"] for p in keep}), len({p["str"] for p in keep})))


def score(pairs_path, target_paths):
    pairs = json.load(open(pairs_path))
    print("pairs %d, distinct addresses %d" % (len(pairs), len({p["addr"] for p in pairs})))
    for tp in target_paths:
        t = os.path.splitext(os.path.basename(tp))[0]
        refs = json.load(open(tp))
        found = referenced = hit0 = start0 = 0
        fhit = set()
        votes = {0x10000: Counter(), 0x1000: Counter()}
        fails = []
        a_ref, a_hit, a_start, voted = set(), set(), set(), set()
        for p in pairs:
            hits = refs.get(p["str"])
            if not hits:
                continue
            found += 1
            funcs = {(r[1], r[2], r[3]) for h in hits for r in h["refs"] if r[1] is not None}
            if not funcs:
                continue
            referenced += 1
            a_ref.add(p["addr"])
            inside = [f for f in funcs if f[0] <= p["addr"] < f[1]]
            if inside:
                hit0 += 1
                fhit.add(inside[0][0])
                a_hit.add(p["addr"])
                if any(f[0] == p["addr"] for f in inside):
                    start0 += 1
                    a_start.add(p["addr"])
            else:
                fails.append((p, sorted(funcs)[:2]))
            for align, c in votes.items():
                ds = set()
                for s, e, _ in funcs:
                    lo = -(-(s - p["addr"]) // align) * align  # smallest aligned D with addr+D >= s
                    if p["addr"] + lo < e:
                        ds.add(lo)
                ds = {dv for dv in ds if (align, p["addr"], dv) not in voted}  # one vote per address
                voted.update((align, p["addr"], dv) for dv in ds)
                c.update(ds)
        print("\n== %s: string present %d, referenced from a function %d" % (t, found, referenced))
        print("   offset 0: address inside a referencing function %d/%d (%.1f%%), at its start %d, distinct functions %d"
              % (hit0, referenced, 100.0 * hit0 / max(1, referenced), start0, len(fhit)))
        print("   per distinct address: testable %d, inside a referencing function at offset 0: %d (%.1f%%), at its start %d"
              % (len(a_ref), len(a_hit), 100.0 * len(a_hit) / max(1, len(a_ref)), len(a_start)))
        for align, c in votes.items():
            print("   best 0x%X-aligned offsets (votes = distinct addresses): %s" % (align, ", ".join("%+#x: %d" % dv for dv in c.most_common(4))))
        for p, fs in fails[:int(os.environ.get("FAILS", "8"))]:
            print("   offset-0 fail %s:%d 0x%X %r -> refs %s" % (p["file"], p["line"], p["addr"], p["str"][:40],
                  ["0x%X-0x%X %s" % (s, e, n[:50]) for s, e, n in fs]))


if __name__ == "__main__":
    if sys.argv[1] == "extract":
        extract(*sys.argv[2:5])
    else:
        score(sys.argv[2], sys.argv[3:])

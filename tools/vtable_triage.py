"""Triage a reccmp-vtable report: which mismatching vftables differ only in destructor slots
(scalar deleting destructor pairing) and which have real slot differences.

    toolchain/venv/bin/reccmp-vtable --target PLAYER > vt.txt   (from a build dir)
    python3 -P tools/vtable_triage.py vt.txt [--real] [--diff other-vt.txt]
--real prints only the vftables with non-destructor differences, with their differing slots.
--diff compares two reports by vftable name: newly matching / newly mismatching.
"""
import re
import sys

ANSI = re.compile(r"\x1b\[[0-9;]*m")
HEAD = re.compile(r"^(.*) : orig (0x[0-9a-f]+), recomp (0x[0-9a-f]+)\s*$")


def is_dtor(text):
    return any(k in text for k in ("destructor", "~", "??_E", "??_G", "??1"))


def parse(path):
    blocks, cur = {}, None
    for line in open(path, encoding="utf-8", errors="replace"):
        line = ANSI.sub("", line.rstrip("\n"))
        m = HEAD.match(line)
        if m:
            cur = {"orig": m.group(2), "slots": [], "lines": []}
            blocks[m.group(1)] = cur
            continue
        if cur is None:
            continue
        if line.startswith("vtable0x") and (" : -" in line):
            cur["slots"].append([line])
        elif line.startswith("           : +") and cur["slots"]:
            cur["slots"][-1].append(line)
        cur["lines"].append(line)
    return blocks


def real(b):
    return [s for s in b["slots"] if not all(is_dtor(x) for x in s)]


def main():
    args = sys.argv[1:]
    blocks = parse(args[0])
    if "--diff" in args:
        other = parse(args[args.index("--diff") + 1])
        for n in sorted(set(blocks) - set(other)):
            print("now mismatching:", n)
        for n in sorted(set(other) - set(blocks)):
            print("now matching:", n)
        return
    rl = {n: real(b) for n, b in blocks.items()}
    print(f"mismatching {len(blocks)}, destructor-only {sum(1 for r in rl.values() if not r)}, "
          f"real {sum(1 for r in rl.values() if r)}")
    if "--real" in args:
        for n, r in sorted(rl.items(), key=lambda kv: -len(kv[1])):
            if r:
                print(f"\n{n} orig {blocks[n]['orig']} ({len(r)} slots)")
                for s in r:
                    print("   " + "\n   ".join(s))


if __name__ == "__main__":
    main()

"""List near misses whose only diff is an unpaired EH-handler operand.

Reads a reccmp --json (with diffs, i.e. a run without --nolib's filtering still has them) and prints
every entry below effective 100% in [LO, HI) whose orig/recomp differences are all operand-only
lines of the form `push 0xADDR` / `mov eax, 0xADDR` on the original side against `<OFFSETn>` (or a
different address) on ours, where the original address is an `__ehhandler$` stub or an EH funclet
return target: i.e. the body is identical and only a code pointer the CSV rows do not pair differs.
Those are generator/libdata work (pair the `__ehhandler$` stubs), not source work.

    python3 -P tools/ehhandler_near_misses.py build/sweep17/P-final.json [--lo 0x401000 --hi 0x480418]

Prints: address, score, name, and the unpaired original operand addresses.
"""
import argparse
import json
import re

OPND = re.compile(r"^(push|mov \w+,) (0x[0-9a-f]+|<OFFSET\d+>)$")


def only_code_pointer_diffs(diff):
    """True and the original operands when every changed line pair differs only in a code pointer."""
    opnds = []
    for _hdr, blocks in diff:
        for b in blocks:
            if "both" in b:
                continue
            o, r = b.get("orig", []), b.get("recomp", [])
            if len(o) != len(r):
                return False, []
            for (_, oi, *_x), (_, ri, *_y) in zip(o, r):
                mo, mr = OPND.match(oi), OPND.match(ri)
                if not (mo and mr and mo.group(1) == mr.group(1)):
                    return False, []
                opnds.append(mo.group(2))
    return bool(opnds), opnds


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("json")
    ap.add_argument("--lo", default="0x401000")
    ap.add_argument("--hi", default="0x480418")
    a = ap.parse_args()
    lo, hi = int(a.lo, 16), int(a.hi, 16)
    n = 0
    for e in json.load(open(a.json))["data"]:
        ad = int(e["address"], 16)
        if not lo <= ad < hi or e.get("effective") or e["matching"] >= 1.0 or not e.get("diff"):
            continue
        ok, ops = only_code_pointer_diffs(e["diff"])
        if ok:
            n += 1
            print(f"{ad:#010x} {e['matching'] * 100:6.2f} {e['name'][:90]}  orig: {' '.join(sorted(set(ops)))}")
    print(f"# {n} entries")


if __name__ == "__main__":
    assert only_code_pointer_diffs([["h", [{"orig": [["0x1", "push 0xa4efe9"]], "recomp": [["0x2", "push <OFFSET1>"]]}]]]) == (True, ["0xa4efe9"])
    assert not only_code_pointer_diffs([["h", [{"orig": [["0x1", "push eax"]], "recomp": [["0x2", "push ebx"]]}]]])[0]
    main()

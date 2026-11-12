"""Pair the functions of two Windows builds whose bodies are byte-identical once addresses are masked.

No IDA: reads both PE files and both evidence exports (ida/exports/<T>-evidence/functions.jsonl,
the entry chunk [address, address+size) of every function). Each body is disassembled with
capstone; every operand that holds an absolute address inside the image (imm32, disp32) and every
relative call/jmp/jcc whose target lies outside the function is masked. Neither target has a
.reloc, so this operand test is the only mask there is.

A pair is accepted when the masked body occurs exactly once on each side (identical bytes, so
identical size), or when it occurs several times on each side and an accepted pair calls one copy
on each side at the same call position (every such caller agreeing; repeated to a fixpoint), and
the positional operands do not refute it:
  - the k-th outside call/jmp target on each side: when both targets are themselves unique
    body pairs, they must be paired with each other;
  - the k-th data operand: when either side points at a C string (4+ printable bytes and a NUL),
    both must point at the same string (build-agent checkout and assert line numbers masked).
A body under MINSIZE bytes is accepted only with support: at least one caller or callee pair
(positional, as above) that is itself an accepted pair.

Run from the repo root (about 3 minutes):
  python3 -P tools/body_match.py STUDIO bin/studio/RobloxStudioBeta.exe PLAYER bin/player_1023/RobloxPlayerBeta.exe
Writes ida/exports/<A>-<B>-bodymatch.csv (a_address, b_address, size, support, status) for every
A function whose masked body occurs on side B at all; status is `pair` for accepted pairs, else
the reason (`ambiguous a:N b:M`, `callee disagrees`, `string disagrees`, `small, no support`);
`pair-callpos` marks a pair resolved through a caller's call position.
Measured 2026-11-12, STUDIO vs PLAYER: 40,902 of 44,599 STUDIO bodies occur in PLAYER at all (most
in large groups of tiny thunks and template copies); 20,115 accepted pairs (14,718 unique +
5,397 call-position), 4,413 of them on a PLAYER function that names-proposed.csv names.
`--selftest` checks the masking on a few hand-made bodies.
"""
import csv
import hashlib
import json
import re
import sys
from collections import Counter, defaultdict

import capstone
import pefile

EXP = "ida/exports"
MINSIZE = 16
PRINT = set(range(32, 127)) | {9, 10, 13}
AGENT = re.compile(rb"BuildAgent[\\/]work[\\/][0-9a-f]+[\\/]")
LINE = re.compile(rb"line: \d+")


class Image:
    def __init__(self, target, path):
        pe = pefile.PE(path, fast_load=True)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.end = self.base + pe.OPTIONAL_HEADER.SizeOfImage
        self.img = pe.get_memory_mapped_image()
        self.F = {}
        for l in open("%s/%s-evidence/functions.jsonl" % (EXP, target)):
            d = json.loads(l)
            self.F[d["address"]] = d["size"]
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.md.detail = True

    def cstr(self, va):
        o = va - self.base
        if not (0 <= o < len(self.img)):
            return None
        s = self.img[o:o + 256]
        n = s.find(b"\0")
        if n >= 4 and all(c in PRINT for c in s[:n]):
            # the build-agent checkout differs per build, an assert's line number per revision
            return LINE.sub(b"line: N", AGENT.sub(b"BuildAgent/", s[:n]))
        return None

    def body(self, a):
        """-> (masked bytes, [outside branch targets], [data operand values]) or None."""
        n = self.F[a]
        o = a - self.base
        raw = bytearray(self.img[o:o + n])
        if len(raw) != n or n == 0:
            return None
        calls, data = [], []
        for i in self.md.disasm(bytes(raw), a):
            k = i.address - a
            if i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL):
                if i.imm_size and i.operands and i.operands[0].type == capstone.CS_OP_IMM:
                    t = i.operands[0].imm & 0xffffffff
                    if not (a <= t < a + n):
                        raw[k + i.imm_offset:k + i.imm_offset + i.imm_size] = b"\0" * i.imm_size
                        calls.append(t)
                    continue
            if i.disp_size == 4:
                v = int.from_bytes(raw[k + i.disp_offset:k + i.disp_offset + 4], "little")
                if self.base <= v < self.end:
                    raw[k + i.disp_offset:k + i.disp_offset + 4] = b"\0\0\0\0"
                    data.append(v)
            if i.imm_size == 4:
                v = int.from_bytes(raw[k + i.imm_offset:k + i.imm_offset + 4], "little")
                if self.base <= v < self.end:
                    raw[k + i.imm_offset:k + i.imm_offset + 4] = b"\0\0\0\0"
                    data.append(v)
        return bytes(raw), calls, data


def match(A, B):
    bodies = {}
    for side, I in (("a", A), ("b", B)):
        bodies[side] = {}
        for a in I.F:
            b = I.body(a)
            if b:
                bodies[side][a] = b
    key = {s: {a: hashlib.sha1(b[0]).digest() for a, b in bodies[s].items()} for s in bodies}
    groups = {s: defaultdict(list) for s in key}
    for s in key:
        for a, h in key[s].items():
            groups[s][h].append(a)
    rows, uniq = {}, {}
    for a, h in key["a"].items():
        if h not in groups["b"]:
            continue
        na, nb = len(groups["a"][h]), len(groups["b"][h])
        if na == 1 and nb == 1:
            uniq[a] = groups["b"][h][0]
            rows[a] = (groups["b"][h][0], "pair")
        else:
            rows[a] = (groups["b"][h][0] if nb == 1 else 0, "ambiguous a:%d b:%d" % (na, nb))
    # positional refutation, repeated until stable (a dropped pair can no longer refute others)
    ok = dict(uniq)
    # same body twice or more on each side (template instantiations): an accepted pair whose k-th
    # call targets are both in one such group pairs them, when every caller agrees
    while True:
        cand = defaultdict(set)
        taken = set(ok.values())
        for a, b in ok.items():
            for x, y in zip(bodies["a"][a][1], bodies["b"][b][1]):
                if x not in ok and y not in taken and x in key["a"] and key["a"][x] == key["b"].get(y):
                    cand[x].add(y)
        back = Counter(y for ys in cand.values() for y in ys)
        new = {x: next(iter(ys)) for x, ys in cand.items() if len(ys) == 1 and back[next(iter(ys))] == 1}
        if not new:
            break
        for x, y in new.items():
            ok[x] = y
            rows[x] = (y, "pair-callpos")
    while True:
        bad = {}
        for a, b in ok.items():
            _, ca, da = bodies["a"][a]
            _, cb, db = bodies["b"][b]
            if any(x in ok and ok[x] != y for x, y in zip(ca, cb)):
                bad[a] = "callee disagrees"
            elif any((A.cstr(x) or B.cstr(y)) and A.cstr(x) != B.cstr(y) for x, y in zip(da, db)):
                bad[a] = "string disagrees"
        if not bad:
            break
        for a, why in bad.items():
            del ok[a]
            rows[a] = (rows[a][0], why)
    # support: a positional callee pair, or a caller whose positional callee is this pair
    support = Counter()
    for a, b in ok.items():
        for x, y in zip(bodies["a"][a][1], bodies["b"][b][1]):
            if x in ok and ok[x] == y and x != a:
                support[a] += 1
                support[x] += 1
    for a in list(ok):
        if A.F[a] < MINSIZE and not support[a]:
            del ok[a]
            rows[a] = (rows[a][0], "small, no support")
    return rows, support, ok


def selftest():
    class Fake(Image):
        def __init__(self, code, base=0x400000):
            self.base, self.end = base, base + 0x1000000
            self.img = b"\0" * 0x1000 + code
            self.F = {base + 0x1000: len(code)}
            self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            self.md.detail = True
    # mov eax,[ecx+0x104]; push 0x00b01234; call rel32 (outside); jz short (inside); ret
    x = Fake(bytes.fromhex("8b8104010000" "683412b000" "e800100000" "7400" "c3"))
    y = Fake(bytes.fromhex("8b8104010000" "687856c000" "e8f0200000" "7400" "c3"))
    bx, by = x.body(0x401000), y.body(0x401000)
    assert bx[0] == by[0] and bx[2] == [0xb01234] and by[2] == [0xc05678], (bx, by)
    assert bx[1] == [0x401000 + 16 + 0x1000]
    z = Fake(bytes.fromhex("8b8108010000" "683412b000" "e800100000" "7400" "c3"))  # other offset
    assert z.body(0x401000)[0] != bx[0]
    print("selftest ok")


def main():
    if sys.argv[1:] == ["--selftest"]:
        return selftest()
    ta, pa, tb, pb = sys.argv[1:5]
    A, B = Image(ta, pa), Image(tb, pb)
    rows, support, ok = match(A, B)
    out = "%s/%s-%s-bodymatch.csv" % (EXP, ta, tb)
    with open(out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["a_address", "b_address", "size", "support", "status"])
        for a in sorted(rows):
            b, st = rows[a]
            w.writerow(["0x%08x" % a, "0x%08x" % b if b else "", A.F[a], support[a], st])
    c = Counter(st.split(" ")[0] for _, st in rows.values())
    print("%s: %d %s functions, %d with a masked twin in %s; %s" % (out, len(A.F), ta, len(rows), tb, dict(c)))
    print("  accepted pairs %d (bytes %d); under %d bytes %d" % (len(ok), sum(A.F[a] for a in ok), MINSIZE,
                                                               sum(1 for a in ok if A.F[a] < MINSIZE)))


if __name__ == "__main__":
    main()

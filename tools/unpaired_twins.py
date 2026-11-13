"""Find our compiled body for original functions that reccmp left unpaired, by identical masked bytes.

    python3 -P tools/unpaired_twins.py TARGET JSON EXE PDBVIEW LO-HI [LO-HI ...] [--max 64]

TARGET   PLAYER or STUDIO (original exe and ida/exports/<T>-functions.csv sizes).
JSON     reccmp --json of the build EXE (what is already paired: original `address`, our `recomp`).
EXE      our linked exe; PDBVIEW its tools/lib/pdb_view.py dump (our symbol per address).
LO-HI    original address ranges (hex) to scan, e.g. the DataModel.obj span 0x0062d1e0-0x00650660.

For every original function in a range with no JSON entry and size <= --max, the body is masked
(rel32 branch targets leaving the function, absolute operands inside the image) and compared with
every symbol of ours that is not already paired (no JSON entry has it as `recomp`) and whose bytes
up to the next symbol are the same masked body plus int3/nop padding. One line per original:
`ADDR SIZE unique|N candidates|none` then our address and PUBLICS name(s). A unique hit is a
marker candidate (a header inline getter, a template copy); it is a pairing, not yet a score.
Tiny bodies (`ret 4`, `mov eax,[ecx+0x70]; ret`) have many twins: only `unique` lines are usable.
Candidates are then refuted operand by operand where both sides can be read: a C string (direct,
or through a pointer such as `sPlayers`), a vftable's RTTI name (COL at [-4]), and a call or jump
target that the JSON already pairs. That separates the per-class template copies (doDeclare<sX>,
shared_count<X*, Deleter>, static_getCreator) that masking alone leaves ambiguous.
"""
import csv
import re
import json
import sys

import capstone
import pefile


def loadimg(path):
    pe = pefile.PE(path, fast_load=True)
    return pe.OPTIONAL_HEADER.ImageBase, pe.OPTIONAL_HEADER.SizeOfImage, pe.get_memory_mapped_image()


MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


PRINT = set(range(32, 127)) | {9, 10, 13}
AGENT = re.compile(rb"BuildAgent[\\/]work[\\/][0-9a-f]+[\\/]")
LINE = re.compile(rb"line: \d+")


def u32(img, base, va):
    o = va - base
    return int.from_bytes(img[o:o + 4], "little") if 0 <= o <= len(img) - 4 else None


def cstr(img, base, va):
    o = va - base
    if not (0 <= o < len(img)):
        return None
    s = img[o:o + 256]
    n = s.find(b"\0")
    if n >= 4 and all(c in PRINT for c in s[:n]):
        return LINE.sub(b"line: N", AGENT.sub(b"BuildAgent/", s[:n]))
    return None


def operand_sig(img, base, v):
    """What a data operand points at, when that is comparable across images, else None."""
    s = cstr(img, base, v)
    if s is not None:
        return ("s", s)
    p = u32(img, base, v)
    if p is not None:
        s = cstr(img, base, p)
        if s is not None:
            return ("S", s)
    col = u32(img, base, v - 4)
    if col is not None and u32(img, base, col) == 0:
        td = u32(img, base, col + 12)
        if td is not None:
            s = cstr(img, base, td + 8)
            if s is not None and s.startswith(b".?A"):
                return ("V", s)
    return None


def masked(img, base, size, a, n, ops=None):
    raw = bytearray(img[a - base:a - base + n])
    if len(raw) != n:
        return None
    end = 0
    for i in MD.disasm(bytes(raw), a):
        k = i.address - a
        end = k + i.size
        if (i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL)) and i.imm_size and \
                i.operands and i.operands[0].type == capstone.CS_OP_IMM:
            t = i.operands[0].imm & 0xffffffff
            if not (a <= t < a + n):
                raw[k + i.imm_offset:k + i.imm_offset + i.imm_size] = b"\0" * i.imm_size
                if ops is not None:
                    ops.append(("C", t))
            continue
        for off, sz in ((i.disp_offset, i.disp_size), (i.imm_offset, i.imm_size)):
            if sz == 4:
                v = int.from_bytes(raw[k + off:k + off + 4], "little")
                if base <= v < base + size:
                    raw[k + off:k + off + 4] = b"\0\0\0\0"
                    if ops is not None:
                        ops.append(operand_sig(img, base, v))
    if end != n:
        return None
    return bytes(raw)


def refutes(o, r, pair):
    if o is None or r is None:
        return False
    if o[0] == "C" or r[0] == "C":
        return o[0] == r[0] == "C" and o[1] in pair and pair[o[1]] != r[1]
    return o != r


def main(argv):
    mx = 64
    if "--max" in argv:
        i = argv.index("--max")
        mx = int(argv[i + 1])
        del argv[i:i + 2]
    T, J, EXE, VIEW = argv[:4]
    ranges = [tuple(int(x, 16) for x in r.split("-")) for r in argv[4:]]
    oexe = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}[T]
    ob, osz, oimg = loadimg(oexe)
    rb, rsz, rimg = loadimg(EXE)
    data = json.load(open(J))["data"]
    paired = {int(e["address"], 16) for e in data}
    taken = {int(e["recomp"], 16) for e in data if e.get("recomp")}
    pair = {int(e["address"], 16): int(e["recomp"], 16) for e in data if e.get("recomp")}
    syms = {}
    for line in open(VIEW, encoding="utf-8", errors="replace"):
        p = line.rstrip("\n").split("\t")
        if len(p) >= 3 and p[1] in ("FUNCTION", "-") and p[2] != "-":
            syms.setdefault(int(p[0], 16), []).append(p[2])
    addrs = sorted(syms)
    text_end = rb + rsz
    bylen = {}
    for k, a in enumerate(addrs):
        nxt = addrs[k + 1] if k + 1 < len(addrs) else text_end
        bylen.setdefault(min(nxt - a, mx + 16), []).append(a)
    sizes = {int(r["address"], 16): int(r["size"]) for r in csv.DictReader(open("ida/exports/%s-functions.csv" % T))}
    cache = {}
    for a in sorted(sizes):
        n = sizes[a]
        if a in paired or n > mx or not any(lo <= a < hi for lo, hi in ranges):
            continue
        oops = []
        body = masked(oimg, ob, osz, a, n, oops)
        if body is None:
            continue
        hits = []
        for L in range(n, n + 16):
            for b in bylen.get(L, ()):
                if b in taken:
                    continue
                key = (b, n)
                if key not in cache:
                    cache[key] = masked(rimg, rb, rsz, b, n)
                pad = rimg[b - rb + n:b - rb + L]
                if cache[key] == body and all(c in (0xcc, 0x90) for c in pad):
                    rops = []
                    masked(rimg, rb, rsz, b, n, rops)
                    if not any(refutes(x, y, pair) for x, y in zip(oops, rops)):
                        hits.append(b)
        tag = "unique" if len(hits) == 1 else ("none" if not hits else "%d candidates" % len(hits))
        print("0x%08x %3d %s" % (a, n, tag))
        for b in hits[:6]:
            print("    %08x %s" % (b, " | ".join(syms[b])[:200]))


if __name__ == "__main__":
    main(sys.argv[1:])

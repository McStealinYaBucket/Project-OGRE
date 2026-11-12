"""Compare one function from an MSVC COFF .obj against the same function in a target PE.

Run:
    python3 -P tools/probe_compare.py OBJ NAME@0xADDR [NAME@0xADDR ...] [--exe PE] [--csv FUNCS] [-q]

    OBJ        COFF object from tools/msvc9/cl (x86).
    NAME       symbol: exact COFF name, a C++ name (matches "?NAME@@..."), or a C name ("_NAME").
    0xADDR     function start in the target image. Its size comes from --csv (default
               ida/exports/PLAYER-functions.csv), or give NAME@0xADDR:SIZE.
    --exe      target image (default bin/player_1023/RobloxPlayerBeta.exe).

Masking: every byte an .obj relocation covers is masked on both sides (rel32 call/jmp targets,
absolute addresses, jump-table entries). The target's value at each masked spot is printed
(resolved to an address) so the symbol -> target-address mapping can be checked by hand: the
same symbol must land on the same address everywhere. Trailing int3/nop padding is trimmed on
both sides. Exit code is 0 only if every pair is identical after masking.
"""
import argparse
import csv
import difflib
import os
import struct
import sys

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REL32, DIR32 = 0x14, 0x06


def read_obj(path):
    d = open(path, "rb").read()
    machine, nsec, _, symptr, nsym, opt, _ = struct.unpack_from("<HHIIIHH", d, 0)
    assert machine == 0x14C, "not an x86 COFF object"
    strtab = symptr + nsym * 18
    def name(raw):
        if raw[:4] == b"\0\0\0\0":
            off = struct.unpack_from("<I", raw, 4)[0] + strtab
            return d[off:d.index(b"\0", off)].decode("latin1")
        return raw.rstrip(b"\0").decode("latin1")
    secs = []
    for i in range(nsec):
        o = 20 + opt + i * 40
        _, _, _, size, rawptr, relptr, _, nrel, _, flags = struct.unpack_from("<8sIIIIIIHHI", d, o)
        relocs = [struct.unpack_from("<IIH", d, relptr + k * 10) for k in range(nrel)]
        secs.append(dict(data=d[rawptr:rawptr + size] if rawptr else b"", relocs=relocs, flags=flags))
    syms, i = {}, 0
    while i < nsym:
        o = symptr + i * 18
        nm = name(d[o:o + 8])
        value, secno, typ, cls, naux = struct.unpack_from("<IhHBB", d, o + 8)
        syms[i] = (nm, value, secno, typ, cls)
        i += 1 + naux
    return secs, syms


def find_func(secs, syms, want):
    cands = [s for s in syms.values() if s[2] > 0 and (s[3] & 0x20 or s[4] == 3) and
             (s[0] == want or s[0] == "_" + want or s[0].startswith("?" + want + "@@"))]
    cands = [s for s in cands if s[3] & 0x20] or cands
    if len(cands) != 1:
        sys.exit(f"{want}: {len(cands)} candidate symbols: {[c[0] for c in cands]}")
    nm, start, secno, _, _ = cands[0]
    sec = secs[secno - 1]
    # Without /Gy several functions share a section: end at the next symbol in it.
    ends = [s[1] for s in syms.values() if s[2] == secno and s[1] > start and s[3] & 0x20]
    end = min(ends) if ends else len(sec["data"])
    code = sec["data"][start:end]
    relocs = [(off - start, syms[si][0], t) for off, si, t in sec["relocs"] if start <= off < end]
    return nm, code, relocs


def trim(b):
    return b.rstrip(b"\xcc\x90")


def masked_text(md, code, mask, base):
    out = []
    for ins in md.disasm(code, base):
        rel = ins.address - base
        raw = "".join("??" if (rel + k) in mask else "%02x" % code[rel + k] for k in range(ins.size))
        txt = f"{ins.mnemonic} {ins.op_str}"
        if any((rel + k) in mask for k in range(ins.size)):
            txt = ins.mnemonic + " <masked>"
        out.append(f"{rel:4x}: {raw:<24} {txt}")
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("obj")
    ap.add_argument("pairs", nargs="+")
    ap.add_argument("--exe", default=os.path.join(ROOT, "bin/player_1023/RobloxPlayerBeta.exe"))
    ap.add_argument("--csv", default=os.path.join(ROOT, "ida/exports/PLAYER-functions.csv"))
    ap.add_argument("-q", action="store_true", help="one summary line per pair, no listings")
    a = ap.parse_args()

    sizes = {int(r["address"], 16): int(r["size"]) for r in csv.DictReader(open(a.csv))}
    pe = pefile.PE(a.exe, fast_load=True)
    img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    secs, syms = read_obj(a.obj)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    ok_all = True
    for pair in a.pairs:
        want, _, where = pair.partition("@")
        addr_s, _, size_s = where.partition(":")
        addr = int(addr_s, 16)
        size = int(size_s, 0) if size_s else sizes[addr]
        nm, mine, relocs = find_func(secs, syms, want)
        mine = trim(mine)
        # IDA's size stops before an in-function jump table; the .obj section includes it.
        theirs = trim(img[addr - base:addr - base + max(size, len(mine))])
        mask = {r[0] + k for r in relocs for k in range(4)}
        same_len = len(mine) == len(theirs)
        diffs = [i for i in range(min(len(mine), len(theirs))) if i not in mask and mine[i] != theirs[i]]
        ok = same_len and not diffs
        ok_all &= ok
        print(f"{want:<20} 0x{addr:08x} obj {len(mine):4d} B  target {len(theirs):4d} B  "
              f"relocs {len(relocs):2d}  {'IDENTICAL' if ok else 'DIFFERENT'}"
              + ("" if ok else f"  ({len(diffs)} unmasked byte diffs{'' if same_len else ', length differs'})"))
        if a.q:
            continue
        for off, sym, t in (relocs if same_len else []):
            if off + 4 > len(theirs):
                continue
            v = struct.unpack_from("<I", theirs, off)[0]
            tgt = addr + off + 4 + v if t == REL32 else v
            if t == REL32:
                tgt &= 0xFFFFFFFF
            print(f"    +{off:03x} {'rel32' if t == REL32 else 'dir32' if t == DIR32 else hex(t)} "
                  f"{sym:<40} -> 0x{tgt:08x}")
        if not ok:
            l1 = masked_text(md, mine, mask, 0)
            l2 = masked_text(md, theirs, mask if same_len else set(), 0)
            for line in difflib.unified_diff(l1, l2, "obj", "target", lineterm="", n=2):
                print("    " + line)
    return 0 if ok_all else 1


if __name__ == "__main__":
    sys.exit(main())

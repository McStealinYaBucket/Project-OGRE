"""Independent check of the `strings` lists in a PE evidence export (tools/ida/export_evidence.py),
the x86 PE counterpart of tools/verify_evidence_strings.py. It does not use IDA at all.

It loads the PE file with pefile, disassembles every function range in functions.jsonl with
capstone, and takes every 32-bit immediate and every memory displacement (`push imm32`,
`mov reg, imm32`, `lea reg, [disp]`, `mov reg, [reg*4 + disp]`, ...) that lands in a non-code
section. A value is a string reference when the bytes there are a NUL-terminated printable ASCII
string of 4+ characters (or UTF-16LE of 4+ ASCII characters), or when the dword there holds the
address of such a string (the export's one pointer hop). The per-function sets are compared with
the export's, restricted to the same population (ASCII / UTF-16 strings of 4+ characters).

Run:  python3 -P tools/verify_evidence_strings_pe.py BINARY EVIDENCE_DIR [N_SHOWN] [SEED]
  e.g. python3 -P tools/verify_evidence_strings_pe.py bin/player_1023/RobloxPlayerBeta.exe ida/exports/PLAYER-evidence 30
N_SHOWN functions with strings, picked at random (SEED, default 1), are printed with every
instruction that references a string, for reading against IDA's listing.
"""
import json
import random
import struct
import sys

import capstone
import pefile
from capstone import x86_const as X

PRINT = set(range(32, 127)) | {9, 10, 13}


class PE:
    def __init__(self, path):
        pe = pefile.PE(path, fast_load=True)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.img = pe.get_memory_mapped_image()
        self.data, self.code = [], []
        for s in pe.sections:
            a = self.base + s.VirtualAddress
            b = a + max(s.Misc_VirtualSize, s.SizeOfRawData)
            (self.code if s.Characteristics & 0x20000000 else self.data).append((a, b))

    def in_data(self, v):
        return any(a <= v < b for a, b in self.data)

    def u32(self, v):
        o = v - self.base
        return struct.unpack_from("<I", self.img, o)[0] if 0 <= o <= len(self.img) - 4 else None

    def string(self, v):
        """Text of the string starting at v, or None."""
        if not self.in_data(v):
            return None
        o = v - self.base
        e = self.img.find(b"\0", o, o + 4096)
        if e - o >= 4 and all(c in PRINT for c in self.img[o:e]):
            return self.img[o:e].decode("ascii")
        w = []
        for k in range(o, min(o + 8192, len(self.img) - 1), 2):
            lo, hi = self.img[k], self.img[k + 1]
            if lo == 0 and hi == 0:
                break
            if hi or lo not in PRINT:
                return None
            w.append(chr(lo))
        return "".join(w) if len(w) >= 4 else None


def eligible(t):
    return len(t) >= 4 and all(ord(c) in PRINT for c in t)


def main():
    pe = PE(sys.argv[1])
    shown = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    rnd = random.Random(int(sys.argv[4]) if len(sys.argv) > 4 else 1)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    rows = [json.loads(l) for l in open(sys.argv[2] + "/functions.jsonl")]
    same = only_export = only_mine = with_any = 0
    diffs, lines = [], {}
    for r in rows:
        ranges = r.get("ranges") or [[r["address"], r["address"] + r["size"]]]
        mine, where = {}, []
        for a, b in ranges:
            o = a - pe.base
            for ins in md.disasm(bytes(pe.img[o:o + (b - a)]), a):
                vals = [op.imm & 0xFFFFFFFF for op in ins.operands if op.type == X.X86_OP_IMM]
                vals += [op.mem.disp & 0xFFFFFFFF for op in ins.operands if op.type == X.X86_OP_MEM]
                for v in vals:
                    t = pe.string(v)
                    if t is None and pe.in_data(v):
                        p = pe.u32(v)
                        t = pe.string(p) if p else None
                    if t is not None:
                        mine[t] = None
                        where.append("  0x%08x  %-6s %-40s ; %r" % (ins.address, ins.mnemonic, ins.op_str[:40], t[:60]))
        theirs = {t for t in r["strings"] if eligible(t)}
        mine = set(mine)
        if mine or theirs:
            with_any += 1
            if mine == theirs:
                same += 1
            else:
                only_export += bool(theirs - mine)
                only_mine += bool(mine - theirs)
                diffs.append((r["address"], r["name"], sorted(theirs - mine)[:3], sorted(mine - theirs)[:3]))
            lines[r["address"]] = (r["name"], where, theirs == mine)
    print("functions with an eligible string (either side): %d; identical sets: %d (%.2f%%); "
          "export has one the verifier lacks: %d; verifier has one the export lacks: %d"
          % (with_any, same, 100.0 * same / max(with_any, 1), only_export, only_mine))
    for d in diffs[:25]:
        print("  differs: 0x%08x %s  export-only %r  verifier-only %r" % d)
    for a in sorted(rnd.sample(sorted(lines), min(shown, len(lines)))):
        name, where, ok = lines[a]
        print("\n0x%08x %s  (%s)" % (a, name, "same" if ok else "DIFFERS"))
        print("\n".join(where[:12]) + ("\n  ..." if len(where) > 12 else ""))


if __name__ == "__main__":
    main()

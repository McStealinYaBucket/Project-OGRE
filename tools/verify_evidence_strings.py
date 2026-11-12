"""Independent check of the `strings` lists in an evidence export (tools/ida/export_evidence.py).

It does not use IDA's cross references. It reads the Mach-O file itself (thin, 32-bit: i386 or
armv7), disassembles every function range from functions.jsonl with capstone, and resolves string
references with its own register tracking:
  armv7  movw/movt pairs, literal-pool loads (ldr rX, [pc, #n]) and the PC-relative add that follows
  i386   the PIC base (call $+5 / pop reg, or a get_pc_thunk) plus every [reg + disp] operand with a
         displacement of 0x1000 or more, and absolute addresses in non-PIC code
then follows non-lazy pointers (resolved through the indirect symbol table) and takes one hop
through __cfstring, literal-pointer sections (ObjC selrefs) and data words that point at the start
of a C string, the same reach the export claims. Then it compares the two sets per function. Only
strings in C-string-literal sections are compared (the export's `typenames` are out of scope).

Run:  python3 -P tools/verify_evidence_strings.py BINARY EVIDENCE_DIR [EXAMPLES]
  e.g. python3 -P tools/verify_evidence_strings.py ref/ios_1029/robloxmobile ida/exports/IOS_1029-evidence 10

The ARM tracker is linear and never resets at labels, so it can over- and under-report a little; a
string only one side finds is a candidate to look at by hand, not a proven error.
"""
import json
import struct
import sys

import capstone
from capstone import arm_const as A, x86_const as X


class MachO:
    def __init__(self, path):
        self.data = d = open(path, "rb").read()
        magic, self.cpu, _, _, ncmds, _, flags = struct.unpack_from("<7I", d, 0)
        self.pie = bool(flags & 0x200000)
        assert magic == 0xFEEDFACE, "thin 32-bit little-endian Mach-O only"
        self.sects, off = [], 28  # (addr, end, fileoff, type, sectname, reserved1)
        self.symoff = self.indoff = 0
        for _ in range(ncmds):
            cmd, size = struct.unpack_from("<2I", d, off)
            if cmd == 1:  # LC_SEGMENT
                nsects = struct.unpack_from("<I", d, off + 48)[0]
                for i in range(nsects):
                    o = off + 56 + 68 * i
                    name = d[o:o + 16].rstrip(b"\0").decode()
                    addr, sz, fo = struct.unpack_from("<3I", d, o + 32)
                    flags = struct.unpack_from("<I", d, o + 56)[0]
                    self.sects.append((addr, addr + sz, fo, flags & 0xFF, name, struct.unpack_from("<I", d, o + 60)[0]))
            elif cmd == 2:  # LC_SYMTAB
                self.symoff = struct.unpack_from("<I", d, off + 8)[0]
            elif cmd == 0xB:  # LC_DYSYMTAB
                self.indoff = struct.unpack_from("<I", d, off + 56)[0]
            off += size

    def sect(self, ea):
        for s in self.sects:
            if s[0] <= ea < s[1]:
                return s
        return None

    def u32(self, ea):
        s = self.sect(ea)
        if not s or s[3] in (1, 12) or ea + 4 > s[1]:  # zerofill has no file bytes
            return None
        return struct.unpack_from("<I", self.data, s[2] + ea - s[0])[0]

    def cstr(self, ea, exact=False):
        """The non-empty C string containing ea if ea is in a cstring-literal section, else None."""
        s = self.sect(ea)
        if not s or s[3] != 2:
            return None
        lo, hi = s[2], s[2] + s[1] - s[0]
        o = lo + ea - s[0]
        a = self.data.rfind(b"\0", lo, o) + 1
        a = max(a, lo)
        if exact and a != o:
            return None
        b = self.data.find(b"\0", o, hi)
        b = hi if b < 0 else b
        return self.data[a:b].decode("utf-8", "replace") or None

    def nlptr(self, ea, s):
        """Target of the non-lazy pointer at ea, or None for an import."""
        idx = struct.unpack_from("<I", self.data, self.indoff + 4 * (s[5] + (ea - s[0]) // 4))[0]
        if idx & 0xC0000000:  # INDIRECT_SYMBOL_LOCAL / ABS: the file holds the address
            return self.u32(ea)
        _, typ, _, _, val = struct.unpack_from("<IBBHI", self.data, self.symoff + 12 * idx)
        return val if typ & 0x0E == 0x0E else None

    def strings_at(self, ea, depth=0):
        """Strings a reference to ea denotes: the string itself, or one pointer hop."""
        s = self.sect(ea)
        if not s:
            return []
        if s[3] == 6:
            p = self.nlptr(ea, s) if depth < 2 else None
            return self.strings_at(p, depth + 1) if p else []
        if s[3] == 2:
            t = self.cstr(ea)
        elif s[4] == "__cfstring":
            p = self.u32(ea + 8)
            t = self.cstr(p) if p else None
        elif s[4] == "__text" or s[3] in (7, 8):
            t = None
        else:
            p, exact = self.u32(ea), s[3] != 5
            ps = self.sect(p) if p else None
            if ps and ps[4] == "__cfstring":  # NSString* global -> CFString -> chars
                p, exact = self.u32(p + 8), False
            t = self.cstr(p, exact=exact) if p else None
        return [t] if t else []


def scan_arm(m, md_thumb, md_arm, f, found):
    regs = {}
    md = md_thumb if f.get("thumb", True) else md_arm
    pcoff = 4 if f.get("thumb", True) else 8
    for a, b in f.get("ranges") or [[f["address"], f["address"] + f["size"]]]:
        s = m.sect(a)
        code = m.data[s[2] + a - s[0]: s[2] + b - s[0]]
        for i in md.disasm(code, a):
            if i.id == 0:
                continue
            ops = i.operands
            dst = ops[0].reg if ops and ops[0].type == A.ARM_OP_REG else None
            if i.id in (A.ARM_INS_MOVW, A.ARM_INS_MOV, A.ARM_INS_MOVS) and len(ops) == 2 and ops[1].type == A.ARM_OP_IMM:
                regs[dst] = ops[1].imm & 0xFFFFFFFF
            elif i.id == A.ARM_INS_MOVT and dst in regs:
                regs[dst] = (regs[dst] & 0xFFFF) | (ops[1].imm & 0xFFFF) << 16
            elif i.id == A.ARM_INS_LDR and len(ops) == 2 and ops[1].type == A.ARM_OP_MEM and ops[1].mem.base == A.ARM_REG_PC:
                if ops[1].mem.index:  # ARM mode: ldr rd, [pc, rm] loads through a PC-relative pointer
                    if ops[1].mem.index in regs:
                        found.extend(m.strings_at((regs[ops[1].mem.index] + i.address + pcoff) & 0xFFFFFFFF))
                    regs.pop(dst, None)
                else:
                    v = m.u32(((i.address + pcoff) & ~3) + ops[1].mem.disp)
                    if v is None:
                        regs.pop(dst, None)
                    else:
                        regs[dst] = v
                        if not m.pie:  # an absolute address in a literal pool; in PIE code it is a delta
                            found.extend(m.strings_at(v))
            elif (i.id == A.ARM_INS_ADD and all(o.type == A.ARM_OP_REG for o in ops)
                  and any(o.reg == A.ARM_REG_PC for o in ops[1:])):
                src = [o.reg for o in ops[1:] if o.reg != A.ARM_REG_PC] or [dst]
                if len(ops) <= 3 and src[0] in regs:
                    t = (regs[src[0]] + i.address + pcoff) & 0xFFFFFFFF
                    found.extend(m.strings_at(t))
                regs.pop(dst, None)
            elif regs:
                if i.group(capstone.CS_GRP_CALL) or i.id in (A.ARM_INS_BL, A.ARM_INS_BLX):
                    for r in (A.ARM_REG_R0, A.ARM_REG_R1, A.ARM_REG_R2, A.ARM_REG_R3, A.ARM_REG_R12):
                        regs.pop(r, None)
                for r in i.regs_access()[1]:
                    regs.pop(r, None)


def scan_x86(m, md, f, found):
    """Flow-insensitive: [any reg + disp] is taken relative to the PIC base set up most recently
    before it. Only displacements of 0x1000 and more count (a data section is megabytes away)."""
    bases, refs = [], []
    for a, b in f.get("ranges") or [[f["address"], f["address"] + f["size"]]]:
        s = m.sect(a)
        code = m.data[s[2] + a - s[0]: s[2] + b - s[0]]
        for i in md.disasm(code, a):
            if i.id == 0:
                continue
            ops = i.operands
            nxt = i.address + i.size
            if i.id == X.X86_INS_CALL and ops[0].type == X.X86_OP_IMM:
                t = ops[0].imm & 0xFFFFFFFF
                ts = m.sect(t)
                tb = m.data[ts[2] + t - ts[0]: ts[2] + t - ts[0] + 4] if ts else b""
                if t == nxt or (len(tb) == 4 and tb[0] == 0x8B and tb[1] & 0xC7 == 0x04 and tb[2:] == b"\x24\xC3"):
                    bases.append(nxt)
                continue
            for o in ops:
                if o.type == X.X86_OP_MEM:
                    if o.mem.base and abs(o.mem.disp) >= 0x1000:
                        refs.append((i.address, o.mem.disp))
                    elif not o.mem.base and o.mem.disp > 0xFFF:
                        found.extend(m.strings_at(o.mem.disp & 0xFFFFFFFF))
                elif o.type == X.X86_OP_IMM and o.imm > 0xFFF and m.cstr(o.imm & 0xFFFFFFFF, exact=True):
                    found.append(m.cstr(o.imm & 0xFFFFFFFF))
    bases.sort()
    for ea, disp in refs if bases else ():
        base = max([b for b in bases if b <= ea] or bases[:1])
        found.extend(m.strings_at((base + disp) & 0xFFFFFFFF))


def main():
    m = MachO(sys.argv[1])
    evdir = sys.argv[2]
    show = int(sys.argv[3]) if len(sys.argv) > 3 else 5
    arm = m.cpu == 12
    mds = []
    for mode in ((capstone.CS_MODE_THUMB, capstone.CS_MODE_ARM) if arm else (capstone.CS_MODE_32,)):
        md = capstone.Cs(capstone.CS_ARCH_ARM if arm else capstone.CS_ARCH_X86, mode)
        md.detail = True
        md.skipdata = True
        mds.append(md)
    in_cstr = {json.loads(l)["text"] for l in open(evdir + "/strings.jsonl") if '"seg": "__cstring"' in l or '"seg": "__objc_' in l}
    n = both = equal = ida_only = ver_only = with_ida = 0
    ex_ida, ex_ver = [], []
    for line in open(evdir + "/functions.jsonl"):
        f = json.loads(line)
        s = m.sect(f["address"])
        if not s or s[4] != "__text":
            continue  # stubs
        found = []
        if arm:
            scan_arm(m, mds[0], mds[1], f, found)
        else:
            scan_x86(m, mds[0], f, found)
        n += 1
        with_ida += bool(f["strings"])
        ida, ver = set(f["strings"]) & in_cstr, set(found)
        if not ida and not ver:
            continue
        both += 1
        if ida == ver:
            equal += 1
        if ida - ver:
            ida_only += 1
            ex_ida.append((f["address"], f["name"], sorted(ida - ver)))
        if ver - ida:
            ver_only += 1
            ex_ver.append((f["address"], f["name"], sorted(ver - ida)))
    print("functions in __text: %d   with a string in the export: %d (%.1f%%)" % (n, with_ida, 100.0 * with_ida / n))
    print("functions where either side has a string: %d" % both)
    print("  identical string sets:           %d (%.2f%%)" % (equal, 100.0 * equal / both))
    print("  export has strings verifier lacks: %d" % ida_only)
    print("  verifier has strings export lacks: %d" % ver_only)
    step_i, step_v = max(1, len(ex_ida) // max(show, 1)), max(1, len(ex_ver) // max(show, 1))
    for title, ex, step in (("export only", ex_ida, step_i), ("verifier only", ex_ver, step_v)):
        print("--- %s (every %dth of %d)" % (title, step, len(ex)))
        for ea, name, ss in ex[::step][:show]:
            print("  0x%08x %s: %s" % (ea, name[:70], [x[:60] for x in ss[:4]]))


if __name__ == "__main__":
    main()

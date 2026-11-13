"""List every 2013 EnumDesc<T> ctor with its registration calls, read from the original binary.

Run:
    python3 -P tools/enumdesc_ctors.py [--target PLAYER|STUDIO] [--json OUT.json]

A ctor is any function that calls EnumDescriptor::EnumDescriptor (PLAYER 0x006fa1a0, STUDIO 0x006dcae0).
For each one it prints the address, the vftable it stores, the registered type name (the ctor's first
string argument) and the ordered registration ops, which give the C++ ctor body one line per op:
    pair V "Name"            addPair(V, "Name")          (PLAYER copies 0x007097e0 lowercase
                             reflection/enumconverter.h, 0x008217f0 reflection/EnumConverter.h,
                             0x008b5df0 Reflection/EnumConverter.h; the copy names the include spelling)
    legacy OLD "Name" V      addLegacy(OLD, "Name", V)   (0x006c99f0)
    legacyname "Name" V      addLegacyName("Name", V)    (inlined: Name::declare 0x006138c0 + map[] 0x00697570)
Names come from ida/exports/<T>-names.csv (ios:/mac: oracle). Values held in a zeroed register are resolved.
"""
import argparse
import bisect
import csv
import json
import re

import capstone
import pefile

CFG = {
    "PLAYER": dict(exe="bin/player_1023/RobloxPlayerBeta.exe", ed=0x006fa1a0,
                   pairs={0x007097e0: "reflection/enumconverter.h", 0x008217f0: "reflection/EnumConverter.h",
                          0x008b5df0: "Reflection/EnumConverter.h"},
                   legacy={0x006c99f0}, declare=0x006138c0, mapidx=0x00697570),
    "STUDIO": dict(exe="bin/studio/RobloxStudioBeta.exe", ed=0x006dcae0,
                   pairs={0x00775a00: "reflection/enumconverter.h", 0x00914ec0: "reflection/EnumConverter.h",
                          0x009818a0: "Reflection/EnumConverter.h"},
                   legacy={0x007b1c10}, declare=0x00703190, mapidx=0x00943720),
}


class Img:
    def __init__(self, path):
        pe = pefile.PE(path, fast_load=True)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.img = pe.get_memory_mapped_image()
        sec = [s for s in pe.sections if s.Name.startswith(b".text")][0]
        self.text = (self.base + sec.VirtualAddress, sec.Misc_VirtualSize)
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.md.detail = True

    def cstr(self, a):
        o = a - self.base
        if not 0 <= o < len(self.img):
            return None
        b = self.img[o:o + 512]
        s = b[:b.find(0)].decode("latin1") if 0 in b else None
        return s if s and s.isprintable() else None


def ctor_ops(im, start, end, c):
    regs, pushes, ops, last_str, pend, want, tname, vft = {}, [], [], None, None, False, None, None
    for k in im.md.disasm(im.img[start - im.base:end - im.base], start):
        o = k.operands
        val = lambda x: x.imm if x.type == 2 else regs.get(k.reg_name(x.reg)) if x.type == 1 else None
        if k.mnemonic == "xor" and o[0].type == 1 and o[1].type == 1 and o[0].reg == o[1].reg:
            regs[k.reg_name(o[0].reg)] = 0
        elif k.mnemonic == "mov" and o[0].type == 1 and o[1].type == 2:
            regs[k.reg_name(o[0].reg)] = o[1].imm
        elif k.mnemonic == "mov" and o[0].type == 3 and o[1].type == 2:
            if k.reg_name(o[0].mem.base) == "esp":
                last_str = o[1].imm
            elif vft is None and o[0].mem.disp == 0 and tname is not None:
                vft = o[1].imm
        if want and k.mnemonic == "mov" and o[0].type == 3 and k.reg_name(o[0].mem.base) == "eax" and o[0].mem.disp == 0:
            ops.append(("legacyname", pend, val(o[1])))
            want = False
        if k.mnemonic == "push":
            pushes.append(val(o[0]))
        elif k.mnemonic == "call":
            t = o[0].imm if o[0].type == 2 else None
            if t == c["ed"]:
                tname = im.cstr(pushes[-1])
            elif t in c["pairs"]:
                ops.append(("pair", pushes[-1], im.cstr(pushes[-2]), c["pairs"][t]))
            elif t in c["legacy"]:
                ops.append(("legacy", pushes[-1], im.cstr(pushes[-2]), pushes[-3]))
            elif t == c["declare"]:
                pend = im.cstr(last_str)
            elif t == c["mapidx"]:
                want = True
            pushes = []
        if k.mnemonic == "ret":
            break
    return tname, vft, ops


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", default="PLAYER")
    ap.add_argument("--json")
    a = ap.parse_args()
    c = CFG[a.target]
    im = Img(c["exe"])
    funcs = sorted(int(r["address"], 16) for r in csv.DictReader(open(f"ida/exports/{a.target}-functions.csv")))
    names = {int(r["address"], 16): r["demangled"] for r in csv.DictReader(open(f"ida/exports/{a.target}-names.csv"))}
    ts, n = im.text
    data = im.img[ts - im.base:ts - im.base + n]
    out = []
    for i in range(n - 5):
        if data[i] == 0xE8 and (ts + i + 5 + int.from_bytes(data[i + 1:i + 5], "little", signed=True)) & 0xffffffff == c["ed"]:
            j = bisect.bisect_right(funcs, ts + i)
            f, e = funcs[j - 1], funcs[j]
            tname, vft, ops = ctor_ops(im, f, e, c)
            m = re.search(r"EnumDesc<(.+?)>::EnumDesc\(\)", names.get(f, ""))
            out.append(dict(ctor="0x%08x" % f, vftable=vft and "0x%08x" % vft, typename=tname,
                            T=m.group(1) if m else None, ops=ops))
    for r in out:
        print(r["ctor"], r["vftable"], r["typename"], r["T"] or "-")
        for op in r["ops"]:
            print("    ", *op)
    if a.json:
        json.dump(out, open(a.json, "w"), indent=1)


if __name__ == "__main__":
    main()

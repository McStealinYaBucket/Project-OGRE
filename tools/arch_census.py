"""Count floating-point instruction families per 64 KB block of a target's functions (the /arch tell).

Run: python3 -P tools/arch_census.py [--exe PE] [--csv FUNCS] [--lo 0x401000] [--hi 0x9f0000]

sse2  = scalar-double SSE2 (addsd, mulsd, cvtss2sd, cvttsd2si, ucomisd, ...): any hit means /arch:SSE2 code
sse1  = scalar-single SSE (addss, mulss, comiss, cvttss2si, ...): /arch:SSE or /arch:SSE2
p6x87 = fucomip/fcomip/fcmov: x87 compares that need a P6 target, which VC9 only emits under /arch:SSE
fnstsw = pre-P6 x87 compare sequence (no /arch, or hand-written/old library code)
Defaults: PLAYER and ida/exports/PLAYER-functions.csv, .text below the D3DX/asm tail at 0x9f0000.
"""
import argparse
import collections
import csv
import os

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FAM = {**dict.fromkeys("addsd subsd mulsd divsd sqrtsd cvtss2sd cvtsd2ss cvttsd2si cvtsi2sd ucomisd comisd".split(), "sse2"),
       **dict.fromkeys("addss subss mulss divss sqrtss comiss ucomiss cvttss2si cvtsi2ss".split(), "sse1"),
       **dict.fromkeys("fucomip fcomip fucomi fcomi fcmovb fcmove fcmovbe fcmovu fcmovnb fcmovne".split(), "p6x87"),
       "fnstsw": "fnstsw"}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--exe", default=os.path.join(ROOT, "bin/player_1023/RobloxPlayerBeta.exe"))
    ap.add_argument("--csv", default=os.path.join(ROOT, "ida/exports/PLAYER-functions.csv"))
    ap.add_argument("--lo", type=lambda x: int(x, 0), default=0x401000)
    ap.add_argument("--hi", type=lambda x: int(x, 0), default=0x9F0000)
    a = ap.parse_args()
    pe = pefile.PE(a.exe, fast_load=True)
    img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    total, per = collections.Counter(), collections.defaultdict(collections.Counter)
    for r in csv.DictReader(open(a.csv)):
        addr, size = int(r["address"], 16), int(r["size"])
        if not a.lo <= addr < a.hi:
            continue
        for ins in md.disasm(img[addr - base:addr - base + size], addr):
            fam = FAM.get(ins.mnemonic)
            if fam:
                total[fam] += 1
                per[addr >> 16][fam] += 1
    print("total", dict(total))
    for blk in sorted(per):
        print(f"0x{blk << 16:08x}", dict(per[blk]))


if __name__ == "__main__":
    main()

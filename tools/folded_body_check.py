"""Check the ICF assumption behind folded proposals: at every PLAYER address that carries several
names of one evidence class, disassemble each named oracle body (Mac i386 and iOS armv7/Thumb) and
group them by normalized instruction text (call/branch targets and address-like immediates
masked). More than one group means the oracle bodies differ, so at least one name cannot be the
single PLAYER body.

Run from the repo root:
  python3 -P tools/folded_body_check.py [--cls rtti-slot]
Prints, per address with differing bodies, each group's names and its first instructions.
"""
import argparse
import csv
import json
import re
import struct
from collections import defaultdict

import capstone

BIN = {"MACSTUDIO": ("ref/macstudio/version-e19e6d7e1b824d4e/2013.10.23.2 studio/RobloxStudio-2 2013.app/Contents/MacOS/RobloxStudio",
                     capstone.CS_ARCH_X86, capstone.CS_MODE_32),
       "IOS_1029": ("ref/ios_1029/robloxmobile", capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB)}


def segments(path):
    data = open(path, "rb").read()
    magic, _, _, _, ncmds, _, _ = struct.unpack_from("<7I", data, 0)
    assert magic == 0xFEEDFACE, hex(magic)
    off, segs = 28, []
    for _ in range(ncmds):
        cmd, size = struct.unpack_from("<2I", data, off)
        if cmd == 1:  # LC_SEGMENT
            vmaddr, vmsize, fileoff, filesize = struct.unpack_from("<4I", data, off + 24)
            segs.append((vmaddr, vmsize, fileoff, filesize))
        off += size
    return data, segs


def body(data, segs, ea, size):
    for vm, vs, fo, fs in segs:
        if vm <= ea < vm + fs:
            return data[fo + ea - vm: fo + ea - vm + size]
    return b""


def norm(md, code, ea):
    out = []
    for ins in md.disasm(code, ea):
        ops = ins.op_str
        if ins.mnemonic.startswith(("call", "j", "b", "cb")) and not ins.mnemonic.startswith("bic"):
            ops = "X"
        ops = re.sub(r"0x[0-9a-f]{5,}", "A", ops)
        if ins.mnemonic in ("ldr", "add", "adr") and "pc" in ops:
            ops = re.sub(r"#0x[0-9a-f]+|#\d+", "#P", ops)
        out.append(ins.mnemonic + " " + ops)
    return tuple(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cls", default="rtti-slot")
    ap.add_argument("--csv", default="ida/exports/PLAYER-names-proposed.csv")
    a = ap.parse_args()
    by = defaultdict(list)
    for r in csv.DictReader(open(a.csv)):
        if r["evidence_class"] == a.cls:
            by[r["address"]].append(r)
    folded = {k: v for k, v in by.items() if len(v) > 1}
    print("addresses with several names: %d" % len(folded))
    for T, (path, arch, mode) in BIN.items():
        data, segs = segments(path)
        md = capstone.Cs(arch, mode)
        F = {}
        for l in open("ida/exports/%s-evidence/functions.jsonl" % T):
            d = json.loads(l)
            F[d["name"].lstrip("_")] = d
        bad = 0
        for ad, rs in sorted(folded.items()):
            groups = defaultdict(list)
            for r in rs:
                d = F.get(r["mangled"].lstrip("_"))
                if not d:
                    continue
                groups[norm(md, body(data, segs, d["address"], d["size"]), d["address"])].append(r["demangled"])
            if len(groups) > 1:
                bad += 1
                print("\n%s %s: %d different bodies" % (T, ad, len(groups)))
                for g, names in sorted(groups.items(), key=lambda x: -len(x[1])):
                    print("   [%d] %s" % (len(names), " ; ".join(g[:6])))
                    for n in names[:4]:
                        print("        " + n[:150])
        print("%s: %d of %d folded addresses have differing oracle bodies" % (T, bad, len(folded)))


if __name__ == "__main__":
    main()

"""Census of calls from Roblox code into G3D: which G3D helpers the 2013 App code calls out of line.

Run (from anywhere, about a minute):
    python3 -P tools/lib/g3d_calls.py                     # PLAYER, CSV on stdout
    python3 -P tools/lib/g3d_calls.py --target STUDIO --out notes/g3d-calls-STUDIO.csv

For every function of <T>-functions.csv outside the G3D library block, every direct call/jmp rel32 whose
target is a G3D function (a name in G3D::, from <T>-names.csv or the lib CSVs) or any function inside the
block (zlib/libzip rows excepted) is counted. Columns: calls, callers (distinct calling functions),
address, name (names.csv, else the G3D lib row), source, lib_status (<T>-lib-resolved.csv: identical,
located, span = no compiled counterpart). A callee our headers inline shows up here with no call in our
build; tools/probe_compare.py on a caller then shows the inlined body against the original's call.
"""
import argparse
import csv
import os
import re
import sys
from collections import Counter, defaultdict

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EXE = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
BLOCK = {"PLAYER": (0x009d5b70, 0x009edf00), "STUDIO": (0x006aa500, 0x006c2070)}  # lib-g3d.md


def g3d_qualified(dem):
    """True when the function itself (not a parameter type) is in namespace G3D."""
    d = re.sub(r"^(?:public|private|protected): (?:static |virtual )?", "", dem)
    m = re.search(r"__(?:thiscall|cdecl|stdcall|fastcall) ([^(]*)\(", d)
    q = m.group(1) if m else d.split("(")[0].split(" ")[-1]
    return q.startswith("G3D::")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--target", default="PLAYER", choices=sorted(EXE))
    ap.add_argument("--out")
    a = ap.parse_args()
    t = a.target
    pe = pefile.PE(os.path.join(ROOT, EXE[t]), fast_load=True)
    img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    exp = os.path.join(ROOT, "ida/exports")
    size = {int(r["address"], 16): int(r["size"]) for r in csv.DictReader(open(f"{exp}/{t}-functions.csv"))}
    names = {int(r["address"], 16): (r["demangled"] or r["name"], r["source"])
             for r in csv.DictReader(open(f"{exp}/{t}-names.csv"))}
    lib = {int(r["address"], 16): r for r in csv.DictReader(open(f"{exp}/{t}-lib-resolved.csv"))}
    librow = {}
    for r in csv.DictReader(open(f"{exp}/PLAYER-lib-g3d.csv")):
        if r["target"] == t and r["demangled"]:
            librow.setdefault(int(r["address"], 16), r["demangled"])
    b0, b1 = BLOCK[t]

    def is_g3d(x):
        if b0 <= x < b1:
            return lib.get(x, {}).get("library") not in ("zlib", "libzip")
        return g3d_qualified(librow.get(x) or names.get(x, ("", ""))[0])

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    calls = defaultdict(Counter)
    for f, n in size.items():
        if b0 <= f < b1 or n <= 0:
            continue
        for ins in md.disasm(img[f - base:f - base + n], f):
            if ins.mnemonic in ("call", "jmp") and ins.op_str.startswith("0x"):
                x = int(ins.op_str, 16)
                if not f <= x < f + n and is_g3d(x):
                    calls[x][f] += 1
    rows = sorted(((sum(c.values()), len(c), x) for x, c in calls.items()), reverse=True)
    out = open(a.out, "w", newline="") if a.out else sys.stdout
    w = csv.writer(out)
    w.writerow(["calls", "callers", "address", "name", "source", "lib_status"])
    for n, k, x in rows:
        nm, src = names.get(x, ("", ""))
        w.writerow([n, k, f"0x{x:08x}", librow.get(x) or nm or f"sub_{x:x}", src, lib.get(x, {}).get("status", "")])


if __name__ == "__main__":
    main()

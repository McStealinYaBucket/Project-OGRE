"""Find where a string (or any VA) is referenced from code in a PE, without IDA.

Run:  python3 -P tools/pe_strxref.py EXE FUNCTIONS_CSV PATTERN [--near N]
 e.g. python3 -P tools/pe_strxref.py bin/player_1023/RobloxPlayerBeta.exe \
          ida/exports/PLAYER-functions.csv ' deflate 1\\.1\\.4' --near 40

PATTERN is a Python bytes regex matched against the whole file, or 0xVA.
For every match it prints the VA, then every .text location holding that VA as
a little-endian imm32 (absolute addressing, valid because the images are x86
and we read them at their preferred base), with the containing function from
the IDA function export. With --near N it also prints the ASCII strings
referenced by the N functions before and after each referencing function,
which is how a library's neighbours in link order are identified.

Reads the file only. ponytail: imm32 scan, so register-relative or computed
references are missed and a coincidental constant can false-positive; open a
scratch copy of the .i64 when a hit matters.
"""
import bisect
import csv
import re
import sys

import pefile


def main():
    exe, funcs_csv, pat = sys.argv[1:4]
    near = int(sys.argv[5]) if len(sys.argv) > 5 and sys.argv[4] == '--near' else 0
    data = open(exe, 'rb').read()
    pe = pefile.PE(data=data, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    secs = [(base + s.VirtualAddress, s.PointerToRawData, s.SizeOfRawData, s.Name.rstrip(b'\0').decode()) for s in pe.sections]

    def off2va(o):
        for va, raw, sz, _ in secs:
            if raw <= o < raw + sz:
                return va + o - raw

    def va2off(v):
        for va, raw, sz, _ in secs:
            if va <= v < va + sz:
                return raw + v - va

    tva, traw, tsz, _ = next(s for s in secs if s[3] == '.text')
    text = data[traw:traw + tsz]
    funcs = sorted((int(r['address'], 16), r['name']) for r in csv.DictReader(open(funcs_csv)))
    starts = [f[0] for f in funcs]

    def func_index(va):
        return bisect.bisect_right(starts, va) - 1

    def cstr(va):
        o = va2off(va)
        if o is None:
            return None
        e = data.find(b'\0', o, o + 300)
        s = data[o:e]
        if len(s) >= 5 and all(32 <= c < 127 for c in s):
            return s.decode()

    def strings_in(i):
        lo = funcs[i][0]
        hi = funcs[i + 1][0] if i + 1 < len(funcs) else tva + tsz
        out = []
        blob = text[lo - tva:hi - tva]
        rlo, rhi = secs[1][0], secs[2][0] + secs[2][2]  # .rdata .. .data
        for m in re.finditer(rb'(?=(....))', blob, re.S):
            v = int.from_bytes(m.group(1), 'little')
            if rlo <= v < rhi:
                s = cstr(v)
                if s and (v == rlo or data[va2off(v) - 1] == 0):
                    out.append(s)
        return out

    if pat.startswith('0x'):
        targets = [int(pat, 16)]
    else:
        targets = [off2va(m.start()) for m in re.finditer(pat.encode(), data)]
    for t in targets:
        if t is None:
            continue
        print(f'target {t:#010x}  {cstr(t)!r}')
        needle = t.to_bytes(4, 'little')
        p = text.find(needle)
        while p != -1:
            ref = tva + p
            i = func_index(ref)
            print(f'  ref at {ref:#010x} in {funcs[i][1]} ({funcs[i][0]:#010x})')
            for j in range(max(0, i - near), min(len(funcs), i + near + 1)) if near else ():
                for s in dict.fromkeys(strings_in(j)):
                    print(f'      {funcs[j][0]:#010x}{"*" if j == i else " "} {s[:110]}')
            p = text.find(needle, p + 1)


if __name__ == '__main__':
    main()

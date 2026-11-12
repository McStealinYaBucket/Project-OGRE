"""Diff the C function names defined in a source tree against a symbol table.

Run:  python3 -I tools/c_symdiff.py SRC_DIR SYMBOLS_TSV REGEX
 e.g. python3 -I tools/c_symdiff.py /path/to/lua-5.1.4/src \
          ida/exports/IOS_1029-symbols.tsv 0x00823000-0x00836000

SRC_DIR: directory of stock .c files (never executed, only read as text).
SYMBOLS_TSV: ida/exports/<TARGET>-symbols.tsv (address, type, mangled, demangled).
REGEX: selects which symbols of the target belong to the library by base name
(leading underscore stripped; for C++-mangled symbols the demangled name up to
the parameter list, so a C library compiled as C++ still pairs), OR an address
range 0xLO-0xHI when the library is one contiguous block (Lua on iOS is).

Prints (a) symbols in the target matching REGEX that the stock tree does not
define (candidate additions) and (b) external-linkage stock functions that the
target lacks (removed, or dead-stripped). Static stock functions are listed
separately because a compiler may inline them away.

ponytail: function definitions are found with a column-0 regex, which is
right for K&R/ANSI-style C like Lua and zlib where the name starts a line or
follows the return type on one line; use ctags if a library formats otherwise.
"""
import csv
import os
import re
import sys

DEF = re.compile(r'^(?!\s|#|typedef|struct|union|enum|extern|/\*|\*)([A-Za-z_][\w \t\*]*?)\b([A-Za-z_]\w*)\s*\(([^;{]*?)\)\s*\{', re.M | re.S)


def stock_functions(src):
    ext, static = set(), set()
    for f in sorted(os.listdir(src)):
        if not f.endswith('.c'):
            continue
        text = open(os.path.join(src, f), errors='replace').read()
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
        for m in DEF.finditer(text):
            (static if re.search(r'\bstatic\b', m.group(1)) else ext).add(m.group(2))
    return ext, static


def main():
    src, tsv, rx = sys.argv[1:4]
    rng = [int(x, 16) for x in rx.split('-')] if rx.startswith('0x') else None
    rx = None if rng else re.compile(rx)
    ext, static = stock_functions(src)
    syms, target = {}, set()
    for r in csv.reader(open(tsv, errors='replace'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if len(r) < 4 or r[1] not in 'TtSsDdBb':
            continue
        n = r[3].split('(')[0] if r[2].startswith('__Z') else r[2][1:]
        syms.setdefault(n, r[1])
        if (rng[0] <= int(r[0], 16) < rng[1]) if rng else rx.search(n):
            target.add(n)
    print(f'stock: {len(ext)} external + {len(static)} static functions; target symbols matching regex: {len(target)}')
    print(f'stock external present in target: {len(ext & set(syms))}/{len(ext)};  stock static present: {len(static & set(syms))}/{len(static)}')
    print('\n== in target, not defined by stock (candidate additions) ==')
    for n in sorted(target - ext - static):
        print(f'  {syms[n]} {n}')
    print('\n== stock external functions missing from target ==')
    print('  ' + ' '.join(sorted(ext - set(syms))))
    print('\n== stock static functions missing from target (inlined or removed) ==')
    print('  ' + ' '.join(sorted(static - set(syms))))


if __name__ == '__main__':
    main()

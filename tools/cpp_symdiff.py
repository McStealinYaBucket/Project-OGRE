"""Diff C++ Class::method names defined in a source tree against a symbol table.

Run:  python3 -I tools/cpp_symdiff.py SRC_DIR SYMBOLS_TSV NAMESPACE [-v]
 e.g. python3 -I tools/cpp_symdiff.py ref/src2016/.../Network/raknet/Source \
          ida/exports/IOS_1029-symbols.tsv RakNet -v

SRC_DIR is read as text only (non-recursive, *.cpp and *.h). SYMBOLS_TSV is
ida/exports/<TARGET>-symbols.tsv. NAMESPACE selects demangled symbols starting
with "NAMESPACE::" (template arguments are stripped before comparing).

Prints per-class counts, classes present on only one side, and with -v the
individual Class::method names present on only one side. A method only in the
target is a candidate vendor patch or a newer upstream; a method only in the
source may be dead-stripped, inlined, or from a newer release than the target.

ponytail: source definitions are found with a "Class::method(" regex at any
depth, so in-class inline definitions in headers are not seen (they show up as
"only in target") and calls written as Class::method( inside bodies can count
as definitions. Good for a release-fingerprint, not for a function census.
"""
import csv
import os
import re
import sys
from collections import defaultdict


def strip_templates(s):
    out, d = [], 0
    for c in s:
        if c == '<':
            d += 1
        elif c == '>':
            d -= 1
        elif d == 0:
            out.append(c)
    return ''.join(out)


def main():
    src, tsv, ns = sys.argv[1:4]
    verbose = '-v' in sys.argv
    have = defaultdict(set)
    for f in sorted(os.listdir(src)):
        if f.endswith(('.cpp', '.h')):
            t = open(os.path.join(src, f), errors='replace').read()
            t = re.sub(r'/\*.*?\*/|//[^\n]*', '', t, flags=re.S)
            for c, m in re.findall(r'\b([A-Za-z_]\w*)(?:<[^;{}()]*?>)?::(~?[A-Za-z_]\w*|operator[^\s(]{1,3})\s*\(', t):
                have[c].add(m)
    tgt = defaultdict(set)
    for r in csv.reader(open(tsv, errors='replace'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if len(r) >= 4 and r[1] in 'Tt' and r[3].startswith(ns + '::'):
            q = strip_templates(r[3].split('(')[0]).split('::')[1:]
            if len(q) >= 2:
                tgt[q[-2]].add(q[-1])
            else:
                tgt['(free)'].add(q[0])
    only_t = sorted(c for c in tgt if c not in have and c != '(free)')
    print(f'target classes: {len(tgt)}  source classes with out-of-line methods: {len(have)}')
    print('classes only in target:', ' '.join(only_t) or '-')
    print('source classes absent from target:', ' '.join(sorted(c for c in have if c not in tgt)) or '-')
    for c in sorted(tgt):
        a, b = tgt[c], have.get(c, set())
        if c == '(free)':
            b = set().union(*have.values()) | a  # free functions are not matched by the regex
        print(f'{c:40s} target {len(a):3d}  source {len(b):3d}  only-target {len(a - b):3d}  only-source {len(b - a):3d}')
        if verbose and c != '(free)':
            if a - b:
                print('    only in target:', ' '.join(sorted(a - b)))
            if b - a and c in have:
                print('    only in source:', ' '.join(sorted(b - a)))
    if verbose:
        print('free functions in target:', ' '.join(sorted(tgt['(free)'])))


if __name__ == '__main__':
    assert strip_templates('DS::List<int>::Insert') == 'DS::List::Insert'
    main()

"""Namespace census of MSVC RTTI type-descriptor names in a PE file.

Run:  python3 -P tools/rtti_census.py bin/player_1023/RobloxPlayerBeta.exe [--list NAMESPACE]

Scans the raw file for class/struct/union/enum type-descriptor name strings
(".?AV", ".?AU", ".?AT", ".?AW4"), demangles them with llvm-undname (must be on
PATH), and prints counts per top-level namespace and per second-level scope
under RBX. For a template, the namespace is that of the template itself, not of
its arguments. Reads the file only; never modifies it.

--list NS prints every demangled descriptor whose top-level namespace is NS
(use "(global)" for names with no namespace).
"""
import re
import subprocess
import sys
from collections import Counter


def split_scopes(name):
    """'boost::shared_ptr<class RBX::X>' -> ['boost', 'shared_ptr<class RBX::X>']"""
    out, depth, cur, i = [], 0, '', 0
    while i < len(name):
        c = name[i]
        if c in '<(':
            depth += 1
        elif c in '>)':
            depth -= 1
        if depth == 0 and name.startswith('::', i):
            out.append(cur)
            cur = ''
            i += 2
            continue
        cur += c
        i += 1
    return out + [cur]


def main():
    data = open(sys.argv[1], 'rb').read()
    want = sys.argv[3] if len(sys.argv) > 3 and sys.argv[2] == '--list' else None
    descs = sorted(set(re.findall(rb'\.\?A(?:V|U|T|W4)[\x21-\x7e]{1,4000}?@@(?=\x00)', data)))
    inp = ''.join('??_R0' + d.decode()[1:] + '@8\n' for d in descs)
    res = subprocess.run(['llvm-undname'], input=inp, capture_output=True, text=True).stdout
    dem = re.findall(r"^(class|struct|union|enum) (.*) `RTTI Type Descriptor'$", res, re.M)
    top, second, kinds = Counter(), Counter(), Counter()
    for kind, name in dem:
        kinds[kind] += 1
        ch = split_scopes(name)
        ns = ch[0] if len(ch) > 1 else '(global)'
        top[ns] += 1
        if ns == 'RBX':
            second[ch[1].split('<')[0] if len(ch) > 2 else '(directly in RBX)'] += 1
        if want == ns:
            print(kind, name)
    if want:
        return
    print(f'unique type descriptors: {len(descs)}  demangled: {len(dem)}  kinds: {dict(kinds)}')
    print('\n== top-level namespace ==')
    for k, v in top.most_common():
        print(f'{v:6d}  {k}')
    print('\n== second level under RBX (namespace or enclosing class) ==')
    for k, v in second.most_common():
        print(f'{v:6d}  {k}')


assert split_scopes('boost::shared_ptr<class RBX::X>') == ['boost', 'shared_ptr<class RBX::X>']
assert split_scopes('type_info') == ['type_info']

if __name__ == '__main__':
    main()

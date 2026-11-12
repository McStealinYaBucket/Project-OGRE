"""Check FOLDED annotations against the recompiled image.

Run from the build directory (where reccmp-build.yml is), with the reccmp venv:
    ../toolchain/venv/bin/python -P ../tools/check_folded.py --target PLAYER [--list]

/OPT:ICF folds identical function bodies onto one address. In this tree, functions that share an
original address are annotated `// FUNCTION: PLAYER 0x... FOLDED`. The check fails when:
  - some annotations on one address say FOLDED and others do not, or
  - the FOLDED annotations of one address do not all land on one recompiled function (they were not
    folded in our build, so the bodies differ), or none of them could be found in the PDB.
--list also prints every address in the recompiled image where the linker folded several symbols.
Exit status 1 on any failure.
"""
import argparse
import logging
import sys

from reccmp.compare import Compare
from reccmp.parser import DecompCodebase
from reccmp.project.detect import (
    RecCmpProjectException,
    argparse_add_project_target_args,
    argparse_parse_project_target,
)
from reccmp.types import ImageId

logging.getLogger("reccmp").setLevel(logging.ERROR)


def list_folds(compare):
    by_addr = {}
    for sym in compare.cvdump_analysis.parser.symbols:
        by_addr.setdefault(compare.recomp_bin.get_abs_addr(sym.section, sym.offset), []).append(sym.name)
    for addr, names in sorted(by_addr.items()):
        if len(names) > 1:
            ent = compare._db.get(ImageId.RECOMP, addr)
            orig = ent.orig_addr if ent is not None and ent.orig_addr is not None else -1
            print(f"recomp {addr:#010x} orig {orig:#010x}: " + ", ".join(names))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--list", action="store_true", help="print every folded address in the recomp")
    argparse_add_project_target_args(parser)
    args = parser.parse_args()
    try:
        target = argparse_parse_project_target(args=args)
    except RecCmpProjectException as e:
        print(e.args[0])
        return 1

    compare = Compare.from_target(target)
    if args.list:
        list_folds(compare)

    codebase = DecompCodebase(compare.code_files, compare.target_id)
    codebase.prune_invalid_addrs(compare.orig_bin.is_valid_vaddr)
    codebase.prune_reused_addrs()
    by_addr = {}
    for fun in list(codebase.iter_line_functions()) + list(codebase.iter_name_functions()):
        by_addr.setdefault(fun.offset, []).append(fun)

    failed = 0
    for addr, funs in sorted(by_addr.items()):
        folded = [f for f in funs if f.is_folded]
        if folded and len(folded) != len(funs):
            print(f"{addr:#010x}: some annotations are FOLDED, some are not")
            failed += 1
        landed = {
            compare._lines_db.find_function(f.filename, f.line_number, f.end_line, folded=True)
            for f in folded
            if not f.is_nameref() and f.filename is not None
        } - {None}
        if folded and not landed:
            print(f"{addr:#010x}: no FOLDED annotation found in the PDB")
            failed += 1
        elif len(landed) > 1:
            print(f"{addr:#010x}: not folded in the recomp, landed on " + ", ".join(f"{a:#x}" for a in sorted(landed)))
            failed += 1
    print(f"{target.target_id}: {len(by_addr)} annotated addresses, {failed} folding problems")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())

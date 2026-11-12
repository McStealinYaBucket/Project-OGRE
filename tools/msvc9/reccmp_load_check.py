"""Check that reccmp's PE parser and its relocation-free analyses load an image without crashing.

Run with the reccmp venv:
    toolchain/venv/bin/python -P tools/msvc9/reccmp_load_check.py IMAGE.exe [IMAGE2.exe ...]

For each image: header facts, sections, relocation count, imports (and the IAT range they sit in),
import thunks, pointers found (relocation table, or reccmp's heuristic scan when there is none),
string constants, and EH FuncInfo records. Any exception is printed with its traceback and the
script moves on to the next step, so one failing analysis does not hide the others.
"""
import collections
import sys
import time
import traceback

from reccmp.analysis.funcinfo import find_funcinfo
from reccmp.analysis.imports import find_import_thunks
from reccmp.analysis.pointers import find_pointers
from reccmp.analysis.string_const import find_string_consts
from reccmp.formats import detect_image


def step(name, fn):
    t = time.time()
    try:
        print(f"  {name}: {fn()}  ({time.time() - t:.1f}s)")
    except Exception:  # report verbatim and continue
        print(f"  {name}: FAILED")
        traceback.print_exc(file=sys.stdout)


def check(path):
    print(path)
    img = detect_image(path)
    print(f"  type={type(img).__name__} imagebase={img.imagebase:#x} entry={img.entry:#x} "
          f"is_debug={img.is_debug} pdb={img.pdb_filename!r}")
    for s in img.sections:
        print(f"    section {s.name!r:12} {s.virtual_range.start:#010x}-{s.virtual_range.stop:#010x}")

    def imports():
        imps = list(img.imports)
        mods = collections.Counter(i.module for i in imps)
        lo, hi = min(i.addr for i in imps), max(i.addr for i in imps)
        return f"{len(imps)} slots, IAT {lo:#x}-{hi:#x}, modules {dict(mods)}"

    step("relocations", lambda: len(img.relocations))
    step("imports", imports)
    step("import thunks", lambda: sum(1 for _ in find_import_thunks(img)))
    pointers = {}
    step("pointers", lambda: len(pointers.setdefault("p", find_pointers(img))))
    step("string consts", lambda: sum(1 for _ in find_string_consts(img, set(pointers.get("p", {}).values()), "latin1")))
    step("funcinfo", lambda: sum(1 for _ in find_funcinfo(img)))


if __name__ == "__main__":
    for p in sys.argv[1:]:
        check(p)

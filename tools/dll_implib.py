"""Write a module-definition (.def) file listing a DLL's named exports, for `lib /def:` to turn into
an import library. Used by CMakeLists.txt for DLLs the originals import but whose import library we
do not have (tbb.dll: the 2013 TBB 2.1 build has no official download any more).

    python3 -P tools/dll_implib.py bin/player/tbb.dll build/tbb.def
"""
import os
import sys

import pefile


def main(dll, out):
    pe = pefile.PE(dll, fast_load=True)
    pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_EXPORT"]])
    names = [e.name.decode() for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name]
    with open(out, "w") as f:
        f.write(f"LIBRARY {os.path.basename(dll)}\nEXPORTS\n")
        for n in names:
            f.write(f"    {n}\n")
    print(f"{out}: {len(names)} exports")


if __name__ == "__main__":
    main(*sys.argv[1:3])

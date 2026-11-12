"""Write tools/msvc9/paths-<TARGET>.txt: the 2013 spelling of every source file a target's asserts name.

    python3 -P tools/msvc9/gen_paths.py        # PLAYER (bin/player_1023) and STUDIO (bin/studio)

A main file's __FILE__ is the path exactly as it was passed to cl, so these are the spellings
winewrap.py passes for our sources when OGRE_TARGET is set (headers take their spelling from the /I
path and the #include line instead, see winewrap.py). One line per file, relative to the checkout
(Client\\...), .c and .cpp only, taken from C:\\BuildAgent\\work\\<hash>\\ strings in the binary itself
(notes/paths-2013.txt's player column is the Oct 15 build). A file recorded in two spellings is an error.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BINS = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
SRC = re.compile(rb"BuildAgent\\work\\[0-9a-f]{16}\\(Client\\[ -~]*?\.(?:cpp|c))(?![A-Za-z0-9_])")


def main():
    for t, exe in BINS.items():
        data = open(os.path.join(ROOT, exe), "rb").read()
        seen = {}
        for m in SRC.finditer(data):
            p = m.group(1).decode()
            if seen.setdefault(p.lower(), p) != p:
                sys.exit(f"{t}: {p} also recorded as {seen[p.lower()]}")
        out = os.path.join(ROOT, "tools", "msvc9", f"paths-{t}.txt")
        with open(out, "w", newline="\n") as f:
            f.write("".join(p + "\n" for p in sorted(seen.values(), key=str.lower)))
        print(f"{out}: {len(seen)} files")


if __name__ == "__main__":
    main()

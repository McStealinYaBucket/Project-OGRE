"""List files git would commit that are byte-identical to a file in the 2016 source (ref/src2016).

CLAUDE.md: no part of the 2016 source may be committed. Run before every commit; it must print nothing.
    python3 -I tools/check_2016_copies.py            # checks `git ls-files --cached --others --exclude-standard`
Exit status 1 when any copy is found. Only exact copies are caught; edited 2016 text is not.
"""
import hashlib
import os
import subprocess
import sys
from collections import defaultdict

REF = "ref/src2016/extracted"
ALLOWED = {"LICENSE_1_0.txt",  # upstream licence texts that the 2016 tree also vendors
           "lz4_decoder.h"}  # upstream LZ4 r94 (BSD 2-clause), vendored unchanged by 2016 too
# Third-party library trees, committed at the owner's decision (2026-11-13); 2016 vendors the same libraries.
THIRD_PARTY = ("Client/Network/raknet/", "Client/Rendering/g3d/", "Client/fmod/", "Client/App/include/fmod/",
               "staging/raknet/")


def sha1(path):
    with open(path, "rb") as f:
        return hashlib.sha1(f.read()).hexdigest()


def main():
    files = subprocess.run(
        ["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard"],
        check=True, capture_output=True).stdout.decode().split("\0")
    by_size = defaultdict(list)
    for p in filter(None, files):
        if os.path.isfile(p) and os.path.basename(p) not in ALLOWED and not p.startswith(THIRD_PARTY):
            by_size[os.path.getsize(p)].append(p)
    found = {}
    for dp, _, fn in os.walk(REF):
        for f in fn:
            r = os.path.join(dp, f)
            s = os.path.getsize(r)
            if s >= 64 and s in by_size:
                h = sha1(r)
                for p in by_size[s]:
                    if p not in found and sha1(p) == h:
                        found[p] = r
    for p in sorted(found):
        print(f"{p}\t{found[p]}")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())

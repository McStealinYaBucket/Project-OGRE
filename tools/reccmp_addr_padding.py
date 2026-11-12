"""Check that every reccmp marker address is written as 0x plus eight lowercase hex digits.

Run from the repo root with the reccmp venv:
    toolchain/venv/bin/python -P tools/reccmp_addr_padding.py
    toolchain/venv/bin/python -P tools/reccmp_addr_padding.py --selftest

Scans every source file under the source roots of every target in reccmp-project.yml, using reccmp's
own marker regex (so it sees exactly the lines reccmp parses). Exit status 1 if any marker is wrong.
"""
import logging
import re
import sys
from pathlib import Path

from reccmp.dir import source_code_search
from reccmp.parser.marker import markerRegex
from reccmp.project.detect import RecCmpProject

logging.disable(logging.WARNING)  # reccmp warns that the repo root has no reccmp-build.yml

GOOD = re.compile(r"0x[0-9a-f]{8}")


def problems(text):
    for n, line in enumerate(text.splitlines(), start=1):
        m = markerRegex.match(line.strip())
        if m and not GOOD.fullmatch(m.group("offset")):
            yield n, line.strip()


def main():
    root = Path(__file__).resolve().parent.parent
    project = RecCmpProject.from_directory(root)
    files = sorted({p for t in project.targets.values() for p in source_code_search(t.source_paths)})
    count = 0
    for path in files:
        for n, line in problems(path.read_text(encoding="utf-8", errors="replace")):
            print(f"{path.relative_to(root)}:{n}: {line}")
            count += 1
    print(f"{len(files)} files, {count} badly padded markers")
    return 1 if count else 0


def selftest():
    bad = "// FUNCTION: PLAYER 0x8a78c0\n// STUB: STUDIO 0x00401000\n// GLOBAL: PLAYER 0x00C0FFEE\n"
    # markerRegex is case-insensitive, so the uppercase address is caught as a padding problem too.
    assert [n for n, _ in problems(bad)] == [1, 3], list(problems(bad))
    print("selftest ok")
    return 0


if __name__ == "__main__":
    sys.exit(selftest() if "--selftest" in sys.argv else main())

"""Print the VS_FIXEDFILEINFO file version and SHA256 of PE files.

Run: python3 -P tools/msvc9/pever.py FILE...
Prints "<version>\t<sha256>\t<path>" per file; version is "-" when the file
has no VS_FIXEDFILEINFO (e.g. a .lib).
"""
import hashlib
import struct
import sys


def file_version(data: bytes) -> str:
    k = data.find("VS_VERSION_INFO".encode("utf-16-le"))
    i = data.find(struct.pack("<I", 0xFEEF04BD), k) if k >= 0 else -1
    if i < 0:
        return "-"
    ms, ls = struct.unpack_from("<II", data, i + 8)
    return f"{ms >> 16}.{ms & 0xFFFF:02d}.{ls >> 16}.{ls & 0xFFFF:02d}"


def main(paths):
    for p in paths:
        data = open(p, "rb").read()
        print(f"{file_version(data)}\t{hashlib.sha256(data).hexdigest()}\t{p}")


if __name__ == "__main__":
    main(sys.argv[1:])

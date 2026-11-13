"""Save a PE file's first icon group as a PNG (its largest frame), for reccmp's --svg-icon.

    python3 -P tools/exe_icon.py bin/player_1023/RobloxPlayerBeta.exe out.png

Reads the resource directory with the stdlib; PIL picks the largest frame from the rebuilt .ico.
"""
import io
import struct
import sys

from PIL import Image

RT_ICON, RT_GROUP_ICON = 3, 14


def icon_png(path):
    data = open(path, "rb").read()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec, optsize = struct.unpack_from("<H12xH", data, pe + 6)
    opt = pe + 24
    res_rva = struct.unpack_from("<I", data, opt + 96 + 2 * 8)[0]  # PE32 data directory 2
    secs = [struct.unpack_from("<8xIII I", data, opt + optsize + 40 * i) for i in range(nsec)]

    def off(rva):
        for vsize, va, rawsize, raw in secs:
            if va <= rva < va + max(vsize, rawsize):
                return raw + rva - va
        raise ValueError(f"RVA {rva:#x} is in no section")

    base = off(res_rva)

    def entries(dir_off):
        named, ids = struct.unpack_from("<12xHH", data, base + dir_off)
        for i in range(named + ids):
            name, target = struct.unpack_from("<II", data, base + dir_off + 16 + 8 * i)
            yield name, target

    def leaf(target):  # first language's data entry under a name/id directory entry
        while target & 0x80000000:
            target = next(entries(target & 0x7FFFFFFF))[1]
        rva, size = struct.unpack_from("<II", data, base + target)
        return data[off(rva):off(rva) + size]

    types = dict(entries(0))
    icons = {name: leaf(t) for name, t in entries(types[RT_ICON] & 0x7FFFFFFF)}
    group = leaf(next(entries(types[RT_GROUP_ICON] & 0x7FFFFFFF))[1])

    count = struct.unpack_from("<4xH", group)[0]
    head, body, pos = bytearray(group[:6]), bytearray(), 6 + 16 * count
    for i in range(count):
        e = group[6 + 14 * i:20 + 14 * i]
        img = icons[struct.unpack_from("<H", e, 12)[0]]
        head += e[:12] + struct.pack("<I", pos + len(body))
        body += img
    im = Image.open(io.BytesIO(bytes(head + body)))  # ICO: PIL loads the largest frame
    return im


if __name__ == "__main__":
    im = icon_png(sys.argv[1])
    im.save(sys.argv[2])
    print(f"{sys.argv[2]}: {im.size[0]}x{im.size[1]}")

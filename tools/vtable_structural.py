"""Split reccmp-vtable mismatches into structural ones (a slot missing, extra or in the wrong place)
and pairing-only ones (every slot holds the paired function, or an identical-bytes twin of it, e.g.
the original's unfolded `ret 4` / `xor al,al; ret` bodies at 0x004027c0 / 0x00575d00).

    PYTHONIOENCODING=utf-8 toolchain/venv/bin/reccmp-reccmp --target PLAYER --json m.json   (in BUILD)
    toolchain/venv/bin/reccmp-vtable --target PLAYER > vt.txt                            (in BUILD)
    python3 -P tools/vtable_structural.py PLAYER BUILD vt.txt m.json [--all] [--class SUBSTR]

Our slot count stops at the next public in BUILD's .map; the original's comes from
ida/exports/<T>-evidence/vtables.jsonl. A slot is "ok" when our function pairs (json) with the
original's slot function or with one whose first bytes (up to int3 padding) are identical, when our
body's bytes equal the original slot's, or when it is an unpaired deleting destructor (??_E/??_G).
"""
import json
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = {"PLAYER": ("bin/player_1023/RobloxPlayerBeta.exe", "RobloxPlayerBeta"),
       "STUDIO": ("bin/studio/RobloxStudioBeta.exe", "RobloxStudioBeta")}
HEAD = re.compile(r"^(.*) : orig (0x[0-9a-f]+), recomp (0x[0-9a-f]+)\s*$")
ANSI = re.compile(r"\x1b\[[0-9;]*m")


class Img:
    def __init__(self, path):
        self.d = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.d, 0x3c)[0]
        n = struct.unpack_from("<H", self.d, pe + 6)[0]
        opt = struct.unpack_from("<H", self.d, pe + 20)[0]
        self.secs = []
        for i in range(n):
            vs, va, rs, rp = struct.unpack_from("<IIII", self.d, pe + 24 + opt + 40 * i + 8)
            self.secs.append((0x400000 + va, max(vs, rs), rp, rs))

    def read(self, a, k):
        for va, vs, rp, rs in self.secs:
            if va <= a < va + vs:
                o = a - va
                return self.d[rp + o:rp + min(o + k, rs)]
        return b""

    def text(self, a):
        va, vs = self.secs[0][0], self.secs[0][1]
        return va <= a < va + vs

    def dword(self, a):
        b = self.read(a, 4)
        return struct.unpack("<I", b)[0] if len(b) == 4 else None

    def body(self, a):
        b = self.read(a, 64)
        i = b.find(b"\xcc\xcc")
        return b[:i] if i > 0 else b


def main():
    t, bd, vtp, jp = sys.argv[1:5]
    flt = sys.argv[sys.argv.index("--class") + 1] if "--class" in sys.argv else None
    orig = Img(os.path.join(ROOT, EXE[t][0]))
    ours = Img(os.path.join(bd, EXE[t][1] + ".exe"))
    pubs = {}
    for l in open(os.path.join(bd, EXE[t][1] + ".map"), errors="replace"):
        m = re.match(r"\s*[0-9a-f]{4}:[0-9a-f]{8}\s+\S+\s+([0-9a-f]{8})", l)
        if m:
            pubs.setdefault(int(m.group(1), 16), l.split()[1])
    r2o = {}
    for r in json.load(open(jp))["data"]:
        if r.get("recomp"):
            r2o[int(r["recomp"], 16)] = int(r["address"], 16)
    ocount = {}
    for l in open(os.path.join(ROOT, "ida/exports", f"{t}-evidence", "vtables.jsonl")):
        d = json.loads(l)
        for g in d["groups"]:
            ocount[g["address_point"]] = len(g["slots"])
    heads = []
    for l in open(vtp, encoding="utf-8", errors="replace"):
        m = HEAD.match(ANSI.sub("", l.rstrip("\n")))
        if m:
            heads.append((m.group(1), int(m.group(2), 16), int(m.group(3), 16)))
    nstruct = npair = 0
    for name, oa, ra in heads:
        if flt and flt not in name:
            continue
        n = ocount.get(oa)
        rs = []
        for i in range(400):
            if i and ra + 4 * i in pubs:
                break
            v = ours.dword(ra + 4 * i)
            if v is None or not ours.text(v):
                break
            rs.append(v)
        bad = []
        if n is None:
            bad.append("orig slot count unknown")
            n = len(rs)
        for i in range(max(n, len(rs))):
            o = orig.dword(oa + 4 * i) if i < n else None
            r = rs[i] if i < len(rs) else None
            if o is None or r is None:
                bad.append(f"+0x{4*i:02x} {'extra ours' if o is None else 'missing in ours'} orig {o and hex(o)} ours {r and hex(r)}")
                continue
            p = r2o.get(r)
            if p == o or (p is not None and orig.body(p) == orig.body(o)) or ours.body(r) == orig.body(o):
                continue
            if ours.read(r, 2) == b"\xff\x25" and orig.read(o, 2) == b"\xff\x25":
                continue  # both import thunks (_purecall): reccmp pairs no thunk
            if p is None and pubs.get(r, "").startswith(("??_E", "??_G")):
                continue  # scalar/vector deleting destructor reccmp does not pair: pairing, not layout
            bad.append(f"+0x{4*i:02x} orig 0x{o:08x} ours 0x{r:08x} (pairs {p and hex(p)})")
        if bad:
            nstruct += 1
            print(f"STRUCT {name} orig 0x{oa:08x} ({n} slots, ours {len(rs)})")
            for b in bad[:12] if "--all" not in sys.argv else bad:
                print("   ", b)
        else:
            npair += 1
            if "--all" in sys.argv:
                print(f"PAIRING-ONLY {name}")
    print(f"mismatching {len(heads)}: structural {nstruct}, pairing-only {npair}")


if __name__ == "__main__":
    main()

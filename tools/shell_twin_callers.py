"""Which template instantiations the 2013 /GL client shell owns, read from the engine's call sites.

The original links the LTCG shell first, so a COMDAT symbol the shell also instantiates resolves to
the shell's LTCG copy (PLAYER 0x00401000-0x00480430) for every caller, engine callers included;
VC9's /OPT:ICF then folds identical bodies only inside the LTCG set or inside the native set (see
CLAUDE.md, Gotchas). So an engine call into the shell region names a symbol the shell owns, and an
engine call to the engine twin names one it does not. Our link has no such shell code, so all those
symbols fold into one native body and reccmp pairs only one of the two originals.

For every engine function the reccmp JSON pairs, this aligns the original's direct call/jmp targets
with the relocations of our .obj section for the same symbol (in order; only when the counts agree)
and reports, per symbol of ours, how often the original called the shell copy or a native one.

Run:
    python3 -P tools/shell_twin_callers.py --json build/sweep15/P-final.json \
        --map build/sweep15/scored/RobloxPlayerBeta.map --objs build/CMakeFiles [--out f.csv]

Output CSV: symbol, shell_calls, native_calls, example_caller, example_target, our_address,
mixed (1 when our body for it also serves a native-owned symbol: the ones to instantiate). A symbol with both
counts non-zero is a contradiction (wrong alignment, or a different symbol).
"""
import argparse
import collections
import csv
import json
import os
import re
import struct
import sys

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SHELL = (0x00401000, 0x00480430)


def read_obj(path):
    d = open(path, "rb").read()
    machine, nsec, _, symptr, nsym, opt, _ = struct.unpack_from("<HHIIIHH", d, 0)
    if machine != 0x14C:
        return None
    strtab = symptr + nsym * 18

    def name(raw):
        if raw[:4] == b"\0\0\0\0":
            off = struct.unpack_from("<I", raw, 4)[0] + strtab
            return d[off:d.index(b"\0", off)].decode("latin1")
        return raw.rstrip(b"\0").decode("latin1")
    secs = []
    for i in range(nsec):
        o = 20 + opt + i * 40
        _, _, _, size, rawptr, relptr, _, nrel, _, flags = struct.unpack_from("<8sIIIIIIHHI", d, o)
        secs.append((d[rawptr:rawptr + size] if rawptr else b"", relptr, nrel))
    syms, i = {}, 0
    while i < nsym:
        o = symptr + i * 18
        value, secno, typ, cls, naux = struct.unpack_from("<IhHBB", d, o + 8)
        syms[i] = (name(d[o:o + 8]), value, secno, typ)
        i += 1 + naux
    return d, secs, syms


def obj_calls(objinfo, sym):
    """Ordered direct call/jmp relocation targets of SYM's code in the object, or None."""
    d, secs, syms = objinfo
    hit = [s for s in syms.values() if s[0] == sym and s[2] > 0 and s[3] & 0x20]
    if len(hit) != 1:
        return None
    _, start, secno, _ = hit[0]
    data, relptr, nrel = secs[secno - 1]
    ends = [s[1] for s in syms.values() if s[2] == secno and s[1] > start and s[3] & 0x20]
    end = min(ends) if ends else len(data)
    out = []
    for k in range(nrel):
        off, si, typ = struct.unpack_from("<IIH", d, relptr + k * 10)
        if typ != 0x14 or not (start <= off < end) or off < 1:
            continue
        op = data[off - 1]
        if op in (0xE8, 0xE9) or (off >= 2 and data[off - 2] == 0x0F and 0x80 <= op <= 0x8F):
            out.append((off, syms[si][0]))
    return [s for _, s in sorted(out)]


def orig_calls(pe, md, start, size):
    base = pe.OPTIONAL_HEADER.ImageBase
    code = pe.get_data(start - base, size)
    out = []
    for ins in md.disasm(code, start):
        if ins.bytes[0] == 0xE8 or (ins.bytes[0] == 0xE9 and len(ins.bytes) == 5) or \
                (ins.bytes[0] == 0x0F and len(ins.bytes) == 6 and 0x80 <= ins.bytes[1] <= 0x8F):
            t = int(ins.op_str, 16)
            if ins.bytes[0] != 0xE8 and start <= t < start + size:
                continue
            out.append(t)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", required=True)
    ap.add_argument("--map", required=True)
    ap.add_argument("--objs", required=True, help="directory searched for *.obj")
    ap.add_argument("--exe", default=os.path.join(ROOT, "bin/player_1023/RobloxPlayerBeta.exe"))
    ap.add_argument("--funcs", default=os.path.join(ROOT, "ida/exports/PLAYER-functions.csv"))
    ap.add_argument("--shell-end", type=lambda x: int(x, 16), default=SHELL[1],
                    help="end of the LTCG shell region (PLAYER 0x00480430; STUDIO about 0x004f6000)")
    ap.add_argument("--out")
    a = ap.parse_args()
    shell = (SHELL[0], a.shell_end)

    sizes = {int(r["address"], 16): int(r["size"]) for r in csv.DictReader(open(a.funcs))}
    by_addr = collections.defaultdict(list)   # our address -> [(symbol, obj basename)]
    line = re.compile(r"^\s*0001:[0-9a-f]+\s+(\S+)\s+([0-9a-f]{8})\s+f\s+(?:i\s+)?(\S+)$")
    for ln in open(a.map, encoding="latin1"):
        m = line.match(ln)
        if m:
            by_addr[int(m.group(2), 16)].append((m.group(1), m.group(3)))
    objpaths = collections.defaultdict(list)
    for dp, _, fs in os.walk(a.objs):
        for f in fs:
            if f.endswith(".obj"):
                objpaths[f].append(os.path.join(dp, f))
    cache = {}

    def load(p):
        if p not in cache:
            cache[p] = read_obj(p)
        return cache[p]

    pe = pefile.PE(a.exe, fast_load=True)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    shell_ranges = {}
    stats = collections.defaultdict(lambda: [0, 0, "", ""])
    skipped = 0
    for e in json.load(open(a.json))["data"]:
        oa = int(e["address"], 16)
        if e.get("type") not in (None, "function", 1) and "recomp" not in e:
            continue
        if oa < shell[1] or oa not in sizes or not e.get("recomp"):
            continue
        ra = int(e["recomp"], 16)
        mine = None
        for sym, obj in by_addr.get(ra, []):
            for p in objpaths.get(obj, []):
                info = load(p)
                if info:
                    mine = obj_calls(info, sym)
                if mine is not None:
                    break
            if mine is not None:
                break
        if mine is None:
            skipped += 1
            continue
        theirs = orig_calls(pe, md, oa, sizes[oa])
        if len(theirs) != len(mine):
            skipped += 1
            continue
        for t, s in zip(theirs, mine):
            st = stats[s]
            if shell[0] <= t < shell[1]:
                st[0] += 1
            else:
                st[1] += 1
            if not st[2] and shell[0] <= t < shell[1]:
                st[2], st[3] = hex(oa), hex(t)
    ours = {sym: ra for ra, lst in by_addr.items() for sym, _ in lst}
    # A body of ours that serves both shell-owned and native-owned symbols is a twin the link
    # cannot reproduce: its "mixed" column is 1, and those are the symbols to instantiate.
    kinds = collections.defaultdict(set)
    for s, v in stats.items():
        if s in ours:
            kinds[ours[s]].add("S" if v[0] else "N")
    rows = sorted(stats.items(), key=lambda x: (-x[1][0], -x[1][1]))
    out = open(a.out, "w", newline="") if a.out else sys.stdout
    w = csv.writer(out)
    w.writerow(["symbol", "shell_calls", "native_calls", "example_caller", "example_target",
                "our_address", "mixed"])
    for s, v in rows:
        ra = ours.get(s)
        w.writerow([s] + v + [ra and hex(ra), int(ra is not None and len(kinds[ra]) == 2)])
    print(f"# {sum(1 for _, v in rows if v[0])} symbols called in the shell region, "
          f"{sum(1 for _, v in rows if v[0] and ours.get(_) is not None and len(kinds[ours[_]]) == 2)}"
          f" of them sharing a body of ours with a native-owned symbol; {skipped} callers skipped "
          f"(no obj section or call counts differ)", file=sys.stderr)


if __name__ == "__main__":
    main()

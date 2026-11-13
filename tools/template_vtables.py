"""Per reflection/factory class template: how many of its 2013 vftables a reccmp run pairs, and how many at 100%.

Run:
    python3 -P tools/template_vtables.py JSON [--exe bin/player_1023/RobloxPlayerBeta.exe] [--show FAMILY]

JSON is a `reccmp-reccmp --json` run (its type 5 rows are vftables). Each original vftable is named by
its RTTI Complete Object Locator (the type descriptor at [vftable-4]); a vftable belongs to the family
of every class template its class hierarchy descriptor lists (Described, BoundFuncDesc, ...), so the
`{for X}` vftables of a derived class count for the template whose subobject they are only through
the COL's own type. --show prints that family's vftables with their ratio.
"""
import argparse
import collections
import json
import re
import struct

import pefile

FAMILIES = ("Described", "DescribedCreatable", "DescribedNonCreatable", "FactoryProduct", "NonFactoryProduct",
            "Creatable", "AbstractFactoryProduct", "GuidItem", "BoundFuncDesc", "PropDescriptor", "GetSetImpl",
            "EventDesc", "EventDescImpl", "EventDescBase", "EnumDesc", "FuncDesc", "BoundPropGetSet",
            "EnumPropDescriptor", "GetImpl", "RemoteEventDesc", "RemoteEventDescImpl", "TType",
            "BoundYieldFuncDesc", "RefPropDescriptor", "TypedPropertyDescriptor", "YieldFuncDesc", "BoundProp",
            "RefType", "CallbackDesc", "CallbackDescImpl", "BoundCallbackDesc", "SetImpl",
            "MemberDescriptorContainer")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("json")
    ap.add_argument("--exe", default="bin/player_1023/RobloxPlayerBeta.exe")
    ap.add_argument("--show")
    a = ap.parse_args()
    pe = pefile.PE(a.exe, fast_load=True)
    img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    u = lambda x: struct.unpack_from("<I", img, x - base)[0]
    cs = lambda x: img[x - base:img.index(b"\0", x - base)].decode("latin1")
    stats = collections.defaultdict(lambda: [0, 0, []])
    for e in json.load(open(a.json))["data"]:
        if e.get("type") != 5 or not e.get("recomp"):
            continue
        va = int(e["address"], 16)
        try:
            cls = cs(u(u(va - 4) + 12) + 8)
        except Exception:
            continue
        m = re.match(r"\.\?A[VU]\?\$(\w+)@", cls)
        if not m or m.group(1) not in FAMILIES:
            continue
        r = stats[m.group(1)]
        r[0] += 1
        r[1] += e["matching"] >= 1.0
        r[2].append((e["matching"], hex(va), e.get("name", "")))
    for fam in FAMILIES:
        n, full, rows = stats.get(fam, [0, 0, []])
        best = max(rows)[0] if rows else None
        print(f"{fam:28s} paired {n:4d}  at 100% {full:4d}  best {best}")
    if a.show:
        for m_, va, name in sorted(stats[a.show][2]):
            print(f"  {m_:.3f} {va} {name}")


if __name__ == "__main__":
    main()

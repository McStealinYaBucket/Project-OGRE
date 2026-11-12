"""Name the MSVC RTTI structures and vftables of a PE database (PLAYER, PLAYER_1015, STUDIO).

Uses tools/ida/msvc_rtti.py. Every TypeDescriptor, Complete Object Locator, Class Hierarchy
Descriptor, base class array, Base Class Descriptor and vftable gets the decorated name MSVC
itself gives it (??_R0?AVX@@@8, ??_R4X@@6B@, ??_R3X@@8, ??_R2X@@8, ??_R1A@?0A@EA@X@@8,
??_7X@@6B@ / ??_7X@@6BBase@@@ for a class with more than one vftable) and a comment starting
"rtti: ". The naming rule is proven on every vftable the Ogre DLLs export (msvc_rtti._selfcheck).
IDA's own first-pass names are replaced where they differ: it calls every vftable of a class
??_7X@@6B@ and numbers the rest `_0`, `_1`, so its names on secondary vftables are not MSVC's.
Vftable slots and the COL pointer before each table become dword offsets (code xrefs from the
table). Functions are never renamed. Idempotent: a second run changes nothing.

Run on the canonical database, after a snapshot (it saves packed):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-02-pre-rtti.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/name_rtti.py" -Lida/logs/PLAYER-02-rtti.log ida/PLAYER.i64
Pass "nosave" as the script argument to measure on a scratch copy without saving.
Log lines start with `name_rtti:`.
"""
import os
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ida_auto, ida_bytes, ida_loader, ida_name, ida_offset, idaapi, idc
import msvc_rtti as M

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
stats = Counter()


def log(*a):
    print("name_rtti:", *a)


def demangled_class(td_name):
    d = ida_name.demangle_name("??_R0" + td_name[1:] + "@8", 0) or ""
    return d.replace(" `RTTI Type Descriptor'", "").replace("class ", "").replace("struct ", "") or td_name


def main():
    R = M.scan(M.from_ida())
    log("TDs %d, COLs %d, CHDs %d, vftables %d, classes %d" % (
        len(R.tds), len(R.cols), sum(1 for v in R.chds.values() if v), len(R.vfts), len(R.classes)))
    want = {}  # ea -> (name, comment)
    cls = {td: demangled_class(n) for td, n in R.tds.items()}
    for td, n in R.tds.items():
        want[td] = (M.td_name(R, td), "rtti: TypeDescriptor %s" % cls[td])
    chds = {}
    for col in R.cols.values():
        chds.setdefault(col.chd, col.td)
    for chd, td in chds.items():
        c = R.chds[chd]
        want[chd] = (M.chd_name(R, chd), "rtti: ClassHierarchyDescriptor %s, %d bases, attributes %d" % (cls[td], len(c.bases), c.attributes))
        bca = R.u32(chd + 12)
        want[bca] = (M.bca_name(R, chd), "rtti: BaseClassArray %s" % cls[td])
        for b in c.bases:
            d = R.bcds[b]
            want.setdefault(b, (M.bcd_name(R, b), "rtti: BaseClassDescriptor %s, PMD(%d,%d,%d), attributes 0x%x" % (
                cls[d.td], d.mdisp, d.pdisp, d.vdisp, d.attributes)))
    for v in R.vfts.values():
        col = R.cols[v.col]
        mp = M.vfptr_paths(R, col.td).get(col.offset)
        what = cls[col.td] + ("" if not mp else " for " + " / ".join(cls[t] for t in mp))
        vn, cn = M.vftable_name(R, v), M.col_name(R, v)
        if not vn:
            stats["vftable name unresolved"] += 1
            continue
        want[v.ea] = (vn, "rtti: vftable %s, COL 0x%08x (offset %d, cdOffset %d), %d slots" % (
            what, v.col, col.offset, col.cd_offset, len(v.slots)))
        want[v.col] = (cn, "rtti: CompleteObjectLocator %s, offset %d" % (what, col.offset))

    # vftable words: dword offsets (the COL pointer and every slot)
    for v in R.vfts.values():
        for ea in [v.ea - 4] + [v.ea + 4 * k for k in range(len(v.slots))]:
            fl = ida_bytes.get_flags(ea)
            if not (ida_bytes.is_dword(fl) and ida_bytes.get_item_head(ea) == ea):
                ida_bytes.del_items(ea, ida_bytes.DELIT_SIMPLE, 4)
                ida_bytes.create_dword(ea, 4)
                stats["word made dword"] += 1
            if not ida_bytes.is_off0(ida_bytes.get_flags(ea)):
                ida_offset.op_offset(ea, 0, idc.REF_OFF32)
                stats["word made offset"] += 1

    # names: free every wanted name held by a different address first (IDA's _0/_1 numbering)
    for ea, (name, _) in want.items():
        other = ida_name.get_name_ea(idaapi.BADADDR, name)
        if other != idaapi.BADADDR and other != ea:
            if other in want:
                ida_name.set_name(other, "", ida_name.SN_NOWARN)
                stats["name moved between RTTI items"] += 1
            else:
                stats["name held by a non-RTTI address, left"] += 1
                log("conflict: %s wanted at 0x%x, held by 0x%x" % (name, ea, other))
    for ea, (name, cmt) in sorted(want.items()):
        if ida_name.get_name(ea) != name:
            kind = "renamed" if ida_bytes.has_user_name(ida_bytes.get_flags(ea)) else "named"
            if ida_name.set_name(ea, name, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
                stats[kind + " " + name[:5]] += 1
            else:
                stats["set_name failed"] += 1
                log("set_name failed at 0x%x: %s" % (ea, name[:200]))
        old = ida_bytes.get_cmt(ea, False) or ""
        keep = "\n".join(l for l in old.split("\n") if l and not l.startswith("rtti:"))
        new = cmt + ("\n" + keep if keep else "")
        if new != old:
            ida_bytes.set_cmt(ea, new, False)
            stats["comment set"] += 1
    for k in sorted(stats):
        log("%s: %d" % (k, stats[k]))


try:
    main()
    if SAVE:
        ida_loader.save_database(idc.get_idb_path(), ida_loader.DBFL_COMP)
        log("saved packed")
    else:
        idaapi.set_database_flag(idaapi.DBFL_KILL)
except Exception:
    import traceback
    log("FAILED\n" + traceback.format_exc())
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

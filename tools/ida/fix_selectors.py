"""Repair a database whose segment selectors stopped mapping to paragraph 0 (flat 32-bit PE), and the code
xrefs IDA computed while they were wrong.

Found 2026-11-26 (wave19 sync): in ida/STUDIO.i64 selectors 1/2/3 had become paragraphs 1/2/3, so every
segment base was 0x10/0x20/0x30 and each instruction IDA (re)decoded since then got near call/jump targets
0x10 short of the real one (STUDIO Name::declare 0x00703190: `call 0x702f90` recorded as a cref to
0x702f80). It happened during the wave9 sync session (snapshot STUDIO-18 is clean, STUDIO-19 is not);
export_evidence.py then drops those callees (no function start at the target), which is what shrank the
regenerated STUDIO pair_names proposals. PLAYER is unaffected (its bases are 0).

Steps: set every selector of a segment back to paragraph 0; for each code head in a code segment, decode it
and, for each o_near operand, move a code xref that points 0x10/0x20/0x30 short of the operand to the
operand (same xref type); then re-plan the touched heads (AU_USED) and wait. Logs `fix_selectors:` lines.
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/fix_selectors.py" -Lida/logs/STUDIO-28-fix-selectors.log ida/STUDIO.i64
Argument "nosave": measure only (scratch copies).
"""
import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_segment, ida_ua, ida_xref, idaapi, idautils, idc
from collections import Counter

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
STAT = Counter()


def say(*a):
    print("fix_selectors: " + " ".join(str(x) for x in a))


segs = [ida_segment.getseg(s) for s in idautils.Segments()]
for seg in segs:
    STAT["base %x before" % ida_segment.get_segm_base(seg)] += 1
for sel in sorted({seg.sel for seg in segs}):
    ida_segment.set_selector(sel, 0)
for seg in segs:
    STAT["base %x after" % ida_segment.get_segm_base(seg)] += 1
nfunc0 = ida_funcs.get_func_qty()

touched = []
insn = ida_ua.insn_t()
for seg in segs:
    if seg.type != ida_segment.SEG_CODE:
        continue
    for h in idautils.Heads(seg.start_ea, seg.end_ea):
        if not ida_bytes.is_code(ida_bytes.get_flags(h)) or ida_ua.decode_insn(insn, h) <= 0:
            continue
        near = [op.addr for op in insn.ops if op.type == ida_ua.o_near]
        if not near:
            continue
        refs = [(x.to, x.type) for x in idautils.XrefsFrom(h, ida_xref.XREF_FAR) if x.iscode]
        tos = {t for t, _ in refs}
        for t, ty in refs:
            if t in near or ty not in (ida_xref.fl_CN, ida_xref.fl_CF, ida_xref.fl_JN, ida_xref.fl_JF):
                continue
            good = [a for a in near if a - t in (0x10, 0x20, 0x30)]
            if not good:
                continue
            ida_xref.del_cref(h, t, 0)
            if good[0] not in tos:
                ida_xref.add_cref(h, good[0], ty)
            STAT["cref moved (%s)" % ("call" if ty in (ida_xref.fl_CN, ida_xref.fl_CF) else "jump")] += 1
            touched.append(h)
for h in sorted(set(touched)):
    ida_auto.auto_mark_range(h, h + 1, ida_auto.AU_USED)
ida_auto.auto_wait()

# what is left: near operands without their cref
left = 0
for seg in segs:
    if seg.type != ida_segment.SEG_CODE:
        continue
    for f in idautils.Functions(seg.start_ea, seg.end_ea):
        for h in idautils.FuncItems(f):
            if ida_ua.decode_insn(insn, h) > 0 and insn.get_canon_mnem() == "call":
                for op in insn.ops:
                    if op.type == ida_ua.o_near:
                        tos = {x.to for x in idautils.XrefsFrom(h, ida_xref.XREF_FAR) if x.iscode}
                        left += op.addr not in tos
STAT["calls whose near target has no cref, after"] = left
STAT["functions before / after"] = "%d / %d" % (nfunc0, ida_funcs.get_func_qty())
for k, v in sorted(STAT.items(), key=lambda kv: str(kv[0])):
    say("%-50s %s" % (k, v))
if SAVE:
    ida_loader.save_database(idc.get_idb_path(), ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

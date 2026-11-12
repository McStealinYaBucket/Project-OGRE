"""Read-only probe: for each string in a JSON list, find where it sits in the image as a
NUL-terminated C string and which functions reference it. Never saves the database.

Run on a SCRATCH COPY of a database (never the canonical ida/*.i64):
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/string_refs.py strings.json out.json" -Lrun.log copy.i64

out.json: {string: [{"ea": str_ea, "refs": [[xref_from, func_start, func_end, func_name], ...]}, ...]}
A ref outside any function has func_start = null.
"""
import json
import ida_auto, ida_bytes, ida_funcs, ida_name, ida_segment, idaapi, idautils, idc

ida_auto.auto_wait()
src, dst = idc.ARGV[1], idc.ARGV[2]
strings = json.load(open(src))

segs = []
for s in idautils.Segments():
    seg = ida_segment.getseg(s)
    data = ida_bytes.get_bytes(seg.start_ea, seg.end_ea - seg.start_ea)  # unloaded bytes read as 0xFF
    if data:
        segs.append((seg.start_ea, data))

out = {}
for text in strings:
    needle = b"\0" + text.encode("latin-1", "replace") + b"\0"
    hits = []
    for base, data in segs:
        p = data.find(needle)
        while p >= 0:
            ea = base + p + 1
            refs = []
            for x in idautils.XrefsTo(ea):
                fn = ida_funcs.get_func(x.frm)
                refs.append([x.frm, fn.start_ea, fn.end_ea, ida_name.get_name(fn.start_ea)] if fn else [x.frm, None, None, ""])
            hits.append({"ea": ea, "refs": refs})
            p = data.find(needle, p + 1)
    if hits:
        out[text] = hits

with open(dst, "w") as f:
    json.dump(out, f)
    f.flush()
print("string_refs: %d of %d strings found" % (len(out), len(strings)))
idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

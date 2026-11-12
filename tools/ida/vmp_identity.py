"""IDA view of the VMProtect SDK marker functions in the unprotected STUDIO build. Read-only: exits
WITHOUT saving. tools/vmprotect_scope.py finds the same functions from the PE alone; this one adds what
only the database has (IDA's xrefs, names and listing).

For every function that references a VMProtectSDK32.dll import it writes: bounds, name, the marker call
sites, string literals referenced, callees, callers (with the callers' own strings and RTTI slots), vftable
slots holding it (RTTI class + slot index), and the disassembly listing.

Run on a scratch COPY only (never on ida/*.i64), from the repo root (the -S path must not contain spaces):
  TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/vmp_identity.py /abs/out.json" -L/abs/log /abs/copy/STUDIO.i64
"""
import json
import ida_auto, ida_bytes, ida_funcs, ida_nalt, ida_name, ida_segment, idaapi, idautils, idc

ida_auto.auto_wait()
out_path = idc.ARGV[1]


def segname(ea):
    s = ida_segment.getseg(ea)
    return ida_segment.get_segm_name(s) if s else None


def cstr(ea):
    """String literal at ea if it looks like one (printable, >= 4 chars), else None."""
    if segname(ea) not in (".rdata", ".data"):
        return None
    raw = ida_bytes.get_bytes(ea, 200) or b""
    s = raw.split(b"\0")[0]
    if len(s) >= 4 and all(32 <= c < 127 or c in (9, 10, 13) for c in s):
        return s.decode("latin1")
    return None


def rtti_of_slot(ea):
    """ea is an .rdata dword holding a function pointer: walk back to the vftable start and name its class."""
    for i in range(0, 600):
        v = ea - 4 * i
        col = ida_bytes.get_wide_dword(v - 4)
        if segname(col) == ".rdata" and ida_bytes.get_wide_dword(col) == 0:
            td = ida_bytes.get_wide_dword(col + 12)
            if segname(td) == ".data":
                nm = ida_bytes.get_bytes(td + 8, 200) or b""
                if nm.startswith(b".?A"):
                    return {"vftable": v, "slot": i, "class": nm.split(b"\0")[0].decode("latin1"),
                            "offset": ida_bytes.get_wide_dword(col + 4)}
    return None


def strings_in(lo, hi):
    out = []
    for h in idautils.Heads(lo, hi):
        for d in idautils.DataRefsFrom(h):
            s = cstr(d)
            if s and s not in out:
                out.append(s)
    return out


def profile(start, end, deep=True):
    rec = {"start": start, "end": end, "name": ida_name.get_name(start), "strings": strings_in(start, end)}
    callees = []
    for h in idautils.Heads(start, end):
        for x in idautils.XrefsFrom(h, 0):
            if x.type in (idaapi.fl_CN, idaapi.fl_CF, idaapi.fl_JN, idaapi.fl_JF) and not (start <= x.to < end):
                callees.append([h, x.to, ida_name.get_name(x.to)])
            elif x.type == idaapi.dr_R and x.to in IMPORTS:
                callees.append([h, x.to, IMPORTS[x.to]])
    rec["callees"] = callees
    callers, slots = [], []
    for x in idautils.XrefsTo(start, 0):
        f = ida_funcs.get_func(x.frm)
        if f:
            c = {"from": x.frm, "func": f.start_ea, "name": ida_name.get_name(f.start_ea)}
            if deep:
                c["strings"] = strings_in(f.start_ea, f.end_ea)[:12]
                c["slots"] = [r for r in (rtti_of_slot(d) for d in idautils.DataRefsTo(f.start_ea) if segname(d) == ".rdata") if r]
            callers.append(c)
        elif segname(x.frm) == ".rdata":
            r = rtti_of_slot(x.frm)
            slots.append(r or {"vftable": None, "at": x.frm})
        else:
            callers.append({"from": x.frm, "func": None, "seg": segname(x.frm)})
    rec["callers"], rec["slots"] = callers, slots
    rec["listing"] = ["%08x %s" % (h, idc.generate_disasm_line(h, 0)) for h in idautils.Heads(start, end)]
    return rec


IMPORTS = {}
for i in range(ida_nalt.get_import_module_qty()):
    dll = ida_nalt.get_import_module_name(i)

    def cb(ea, name, ordinal, dll=dll):
        IMPORTS[ea] = "%s!%s" % (dll, name if name else "#%d" % ordinal)
        return True

    ida_nalt.enum_import_names(i, cb)

res = {"idb": idc.get_idb_path(), "funcs": []}
sites = {}
for ea, nm in IMPORTS.items():
    if not nm.lower().startswith("vmprotectsdk32"):
        continue
    for frm in sorted({x.frm for x in idautils.XrefsTo(ea, 0)}):  # a call gives a code and a data xref
        f = ida_funcs.get_func(frm)
        # the marker name argument of a Begin call: nearest preceding "push offset string"
        arg, h = None, frm
        for _ in range(6):
            h = idc.prev_head(h)
            if "Begin" in nm and idc.print_insn_mnem(h) == "push" and idc.get_operand_type(h, 0) == idc.o_imm:
                arg = (ida_bytes.get_bytes(idc.get_operand_value(h, 0), 16) or b"").split(b"\0")[0].decode("latin1")
                break
        sites.setdefault(f.start_ea if f else None, []).append({"ea": frm, "import": nm, "arg": arg, "segm": segname(frm)})
res["marker_imports"] = {("%08x" % k): v for k, v in IMPORTS.items() if v.lower().startswith("vmprotectsdk32")}
for fs, calls in sorted(sites.items(), key=lambda kv: kv[0] or 0):
    if fs is None:
        res["orphans"] = calls
        continue
    f = ida_funcs.get_func(fs)
    rec = profile(f.start_ea, f.end_ea)
    rec["markers"] = sorted(calls, key=lambda c: c["ea"])
    res["funcs"].append(rec)

with open(out_path, "w") as f:
    json.dump(res, f)
    f.flush()
print("vmp_identity: funcs=%d" % len(res["funcs"]))
idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

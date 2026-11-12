"""VMProtect scope survey of a VMProtect-wrapped Player database. Read-only: exits WITHOUT saving.

Dumps to a JSON file everything in the image that points from normal sections into .vmp0/.vmp1:
  sites       every defined instruction in .text with an operand (branch target, memory, immediate)
              inside a .vmp section, with its function, the bytes after it and what IDA made of them
  raw         every E8/E9/0F8x rel32 byte pattern in .text landing in .vmp*, flagged by whether IDA
              has an instruction head there (so byte-scan false positives can be told apart)
  ptrs        every 4-aligned dword in .rdata/.data whose value is a .vmp address (vftable slots,
              callbacks) outside the import address table
  back        every rel32 jmp/call pattern in .vmp* landing in .text (return points of mutated regions)
  iat         per IAT slot: name, and the number of .text instructions referencing it by kind
  gaps        maximal runs of .text bytes that are not part of any instruction or data item

  rdata_head_names  IDA names on the first 0x800 dwords of .rdata (the linker's original IAT, see
              tools/vmprotect_scope.py): shows whether the database names the slots .text really uses

Run on a scratch COPY only (never on ida/*.i64), from the repo root (the -S path must not contain spaces):
  TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/vmp_scope.py /abs/out.json" -L/abs/log /abs/copy/PLAYER.i64
"""
import json, struct
import ida_auto, ida_bytes, ida_funcs, ida_idaapi, ida_nalt, ida_name, ida_segment, ida_ua, idaapi, idautils, idc

ida_auto.auto_wait()
out_path = idc.ARGV[1]

segs = {}
for s in idautils.Segments():
    seg = ida_segment.getseg(s)
    segs.setdefault(ida_segment.get_segm_name(seg), []).append((seg.start_ea, seg.end_ea))
allsegs = sorted((a, b, n) for n, l in segs.items() for a, b in l)
vmp = [(a, b, n) for a, b, n in allsegs if n.startswith(".vmp")]
text_lo, text_hi = segs[".text"][0]


def in_vmp(v):
    for a, b, n in vmp:
        if a <= v < b:
            return n
    return None


def segname(v):
    for a, b, n in allsegs:
        if a <= v < b:
            return n
    return None


# import table as IDA loaded it: slot ea -> "dll!name"
imports = {}
for i in range(ida_nalt.get_import_module_qty()):
    dll = ida_nalt.get_import_module_name(i)

    def cb(ea, name, ordinal, dll=dll):
        imports[ea] = "%s!%s" % (dll, name if name else "#%d" % ordinal)
        return True

    ida_nalt.enum_import_names(i, cb)

text = ida_bytes.get_bytes(text_lo, text_hi - text_lo)


def func_of(ea):
    f = ida_funcs.get_func(ea)
    if not f:
        return None
    return {"start": f.start_ea, "end": f.end_ea, "name": ida_name.get_name(f.start_ea)}


sites, iat_refs, other_refs = [], {}, []
insn = ida_ua.insn_t()
ea = text_lo
ncode = 0
gaps = []
gap_start = None
while ea < text_hi and ea != ida_idaapi.BADADDR:
    fl = ida_bytes.get_flags(ea)
    if ida_bytes.is_unknown(fl):
        if gap_start is None:
            gap_start = ea
        ea += 1
        continue
    if gap_start is not None:
        gaps.append((gap_start, ea - gap_start))
        gap_start = None
    sz = ida_bytes.get_item_size(ea)
    if ida_bytes.is_code(fl) and ida_ua.decode_insn(insn, ea) > 0:
        ncode += 1
        for op in insn.ops:
            if op.type == ida_ua.o_void:
                break
            if op.type in (ida_ua.o_near, ida_ua.o_far, ida_ua.o_mem, ida_ua.o_displ):
                v = op.addr & 0xFFFFFFFF
            elif op.type == ida_ua.o_imm:
                v = op.value & 0xFFFFFFFF
            else:
                continue
            sn = in_vmp(v)
            if not sn:
                continue
            kind = {ida_ua.o_near: "near", ida_ua.o_far: "far", ida_ua.o_mem: "mem", ida_ua.o_displ: "displ", ida_ua.o_imm: "imm"}[op.type]
            if kind in ("mem", "displ") and v in imports:
                d = iat_refs.setdefault(v, {})
                m = insn.get_canon_mnem()
                d[m] = d.get(m, 0) + 1
                continue
            nxt = ea + insn.size
            rec = {
                "ea": ea, "size": insn.size, "mnem": insn.get_canon_mnem(), "opkind": kind, "target": v, "tseg": sn,
                "disasm": idc.generate_disasm_line(ea, 0), "func": func_of(ea),
                "bytes": ida_bytes.get_bytes(ea, insn.size).hex(),
                "after": text[nxt - text_lo:nxt - text_lo + 48].hex(),
                "after_is_code": bool(ida_bytes.is_code(ida_bytes.get_flags(nxt))),
                "after_func": func_of(nxt),
            }
            (sites if kind in ("near", "far") else other_refs).append(rec)
    ea += max(sz, 1)
if gap_start is not None:
    gaps.append((gap_start, text_hi - gap_start))

# raw rel32 byte patterns in .text into .vmp*
raw = []
n = len(text)
for i in range(n - 5):
    b = text[i]
    if b in (0xE8, 0xE9):
        off, ln, kind = i + 1, 5, "call" if b == 0xE8 else "jmp"
    elif b == 0x0F and 0x80 <= text[i + 1] <= 0x8F:
        off, ln, kind = i + 2, 6, "jcc"
    else:
        continue
    if off + 4 > n:
        continue
    tgt = (text_lo + i + ln + struct.unpack_from("<i", text, off)[0]) & 0xFFFFFFFF
    sn = in_vmp(tgt)
    if not sn:
        continue
    a = text_lo + i
    fl = ida_bytes.get_flags(a)
    head = ida_bytes.get_item_head(a)
    raw.append({"ea": a, "kind": kind, "target": tgt, "tseg": sn, "is_code_head": bool(ida_bytes.is_code(fl)),
                "unknown": bool(ida_bytes.is_unknown(fl)), "item_head": head,
                "item_is_code": bool(ida_bytes.is_code(ida_bytes.get_flags(head))), "func": func_of(a)})

# pointers from data sections into .vmp* (not IAT slots, which live inside .vmp1)
ptrs = []
for name in (".rdata", ".data"):
    for lo, hi in segs.get(name, []):
        data = ida_bytes.get_bytes(lo, hi - lo) or b""
        for i in range(0, len(data) - 3, 4):
            v = struct.unpack_from("<I", data, i)[0]
            sn = in_vmp(v)
            if sn:
                ptrs.append({"ea": lo + i, "seg": name, "value": v, "tseg": sn, "name": ida_name.get_name(lo + i),
                             "xrefs": [x for x in idautils.DataRefsTo(lo + i)][:8]})

# rel32 jmp/call patterns in .vmp* landing in .text (byte scan: junk produces false hits, aggregate later)
back = []
for lo, hi, name in vmp:
    data = ida_bytes.get_bytes(lo, hi - lo) or b""
    for i in range(len(data) - 5):
        b = data[i]
        if b in (0xE8, 0xE9):
            tgt = (lo + i + 5 + struct.unpack_from("<i", data, i + 1)[0]) & 0xFFFFFFFF
            if text_lo <= tgt < text_hi:
                fl = ida_bytes.get_flags(tgt)
                back.append({"ea": lo + i, "kind": "call" if b == 0xE8 else "jmp", "target": tgt,
                             "t_is_code_head": bool(ida_bytes.is_code(fl)), "t_func": func_of(tgt),
                             "t_is_func_start": bool(ida_funcs.get_func(tgt) and ida_funcs.get_func(tgt).start_ea == tgt)})

res = {
    "idb": idc.get_idb_path(), "segments": allsegs, "ncode_heads_text": ncode, "nfuncs": ida_funcs.get_func_qty(),
    "sites": sites, "other_refs": other_refs, "raw": raw, "ptrs": ptrs, "back": back,
    "imports": {"%08x" % k: v for k, v in imports.items()},
    "iat_refs": {"%08x" % k: v for k, v in iat_refs.items()},
    "gaps": [g for g in gaps if g[1] >= 8],
    "entry": idc.get_inf_attr(idc.INF_START_EA),
    "rdata_head_names": {"%08x" % a: ida_name.get_name(a) for a in range(segs[".rdata"][0][0], segs[".rdata"][0][0] + 0x2000, 4) if ida_name.get_name(a)},
}
with open(out_path, "w") as f:
    json.dump(res, f)
    f.flush()
print("vmp_scope: sites=%d raw=%d ptrs=%d back=%d imports=%d" % (len(sites), len(raw), len(ptrs), len(back), len(imports)))
idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

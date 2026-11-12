"""Evidence export for cross-binary function pairing. Works on x86 PE, i386 Mach-O and armv7
Mach-O databases. READ-ONLY: it never saves the database.

Run (on a database no other IDA holds; headless):
  TVHEADLESS=1 idat -A -S"tools/ida/export_evidence.py [OUTDIR]" -L<log> <TARGET>.i64
OUTDIR defaults to <directory of the .i64>/exports/<TARGET>-evidence. Addresses are JSON integers.
Check the strings afterwards with tools/verify_evidence_strings.py (Mach-O only). It writes:

  functions.jsonl  one object per function:
     address, size (entry chunk), name (raw), demangled (llvm-cxxfilt for Itanium names when it
     is on PATH, else IDA's demangler), thumb (ARM only), ranges (only if the function has tail
     chunks), blocks (basic blocks), callers (distinct functions that call or tail-jump to it,
     stubs resolved), data_refs (data xrefs to the entry: vtable slots, pointers), and
     strings   [text...]      string literals referenced, in first-reference order, deduplicated:
                              direct refs; PIC refs (IDA's, plus on i386 Mach-O this script's own
                              resolution against the PIC base, see x86_pic_bases); refs through
                              non-lazy pointers; and one pointer hop: CFString, ObjC selref, or a
                              data word holding the address of a string's first byte (`const char*
                              g = "..."`, the first entry of a name table). "" is dropped. Build
                              paths inside __FILE__ strings differ per build machine: normalise
                              them before comparing targets. PE images have no .reloc, so IDA's
                              offsets are guesses: a raw push/mov/displacement operand that holds the
                              address of a string's first byte (or of a pointer to one) counts too.
     typenames [text...]      Itanium type-name strings (_ZTS, e.g. "N3RBX8InstanceE") referenced:
                              inlined type_info compares (any_cast, get_deleter). Not literals.
     callees   [address...]   direct call / tail-jump targets, stubs resolved to the real body
     func_refs [address...]   functions whose address is taken (callbacks, atexit destructors)
     globals   [[address, name]...]  named data referenced (through non-lazy pointers too)
     imports   [name...]      imported functions called and imported data referenced (on the
                              VMProtect Players: the `__imp_` slots of the `.iat` segment, prefix
                              dropped; C++ imports stay MSVC-decorated)
     imms      [int...]       constants c with 0xFFFF < c < 0xFFFF0000 (or wider than 32 bits)
                              that are not addresses: instruction immediates (movw/movt halves
                              joined on ARM) plus 4- and 8-byte values read from read-only data or
                              ARM literal pools. That is where floats live, so 1.0f is 0x3f800000
                              on every target. Compiler specific all the same: x86 stores a double
                              as two 32-bit immediates where ARM loads one 64-bit literal, and
                              division magic numbers differ. Split 64-bit values before comparing.
  strings.jsonl    every string literal: address, seg, kind (c / utf16 / typename), text,
                   refs [function...]. Mach-O C-string sections are split at NULs exactly; anywhere
                   else a string is what IDA defined as one, 4+ characters.
  vtables.jsonl    one object per vtable (Itanium: see itanium_vtables()) or per class with a
                   vftable (MSVC: see msvc_vtables(), found through RTTI by msvc_rtti.py).
  summary.json     counts.
"""
import bisect
import json
import os
import re
import shutil
import struct
import subprocess
import sys

import ida_allins, ida_auto, ida_bytes, ida_funcs, ida_gdl, ida_ida, ida_nalt, ida_name
import ida_segment, ida_ua, ida_xref, idaapi, idautils, idc

ida_auto.auto_wait()

PS = 8 if ida_ida.inf_is_64bit() else 4
MASK = (1 << (8 * PS)) - 1
IS_ARM = ida_ida.inf_get_procname().upper().startswith("ARM")
IS_PE = ida_ida.inf_get_filetype() == ida_ida.f_PE
IS_X86_32 = ida_ida.inf_get_procname() == "metapc" and PS == 4
ptr = ida_bytes.get_qword if PS == 8 else ida_bytes.get_wide_dword

CSTR_SEGS = {"__cstring", "__objc_methname", "__objc_classname", "__objc_methtype"}  # NUL-split sections
PTR_SEGS = {"__nl_symbol_ptr", "__la_symbol_ptr", "__got", "__pointers", "__objc_selrefs", "__objc_classrefs",
            "__objc_superrefs", "__objc_protorefs", "__cls_refs", "__message_refs"}  # one pointer, follow it
SKIP_SEGS = {"__gcc_except_tab", "__eh_frame", "__unwind_info", "HEADER"}  # unwind data, never evidence

_segname = {}


def segname(seg):
    n = _segname.get(seg.start_ea)
    if n is None:
        n = _segname[seg.start_ea] = ida_segment.get_segm_name(seg)
    return n


# ---------------------------------------------------------------- names

def demangle_many(names):
    """{name: demangled or None}. IDA's demangler fails on about 8% of the g++ names here (the
    `LZN..E` address-of template arguments of Described<>/NonFactoryProduct<>), so Itanium names go
    through llvm-cxxfilt (the tool that wrote <TARGET>-symbols.tsv); IDA handles MSVC and the rest."""
    out = {}
    tool = shutil.which("llvm-cxxfilt") or shutil.which("c++filt")
    # optional IDA `j_` prefix and `_N` duplicate suffix, optional linker suffix (`$shim`), kept on the result
    pat = re.compile(r"(?:j_)?_*(_Z\w+?)(?:_\d+)?(\$\w+)?$")
    todo = [(n, m.group(1), m.group(2) or "") for n in names for m in [pat.match(n)] if m] if tool else []
    if todo:
        res = subprocess.run([tool], input="\n".join(z for _, z, _ in todo), capture_output=True, text=True).stdout.split("\n")
        for (n, z, suffix), d in zip(todo, res):
            if d and d != z:
                out[n] = d + suffix
    for n in names:
        if n not in out:
            out[n] = ida_name.demangle_name(n, 0)
    return out


def import_name(ea):
    n = ida_name.get_name(ea)
    return n[6:] if n.startswith("__imp_") else n  # IDA's prefix on function externs, not part of the symbol


# ---------------------------------------------------------------- strings

STRINGS = {}  # start ea -> [seg, kind, text, {function ea: None}]
_cstr_bytes = {}  # CSTR segment start -> its bytes
_str_at = {}  # ea -> start ea of the string containing it, or None


def _add_string(start, seg, kind, raw):
    if start not in STRINGS:
        text = raw.decode("utf-16-le" if kind == "utf16" else "utf-8", "replace")
        if kind == "c" and (ida_name.get_name(start) or "").lstrip("_").startswith("ZTS"):
            kind = "typename"
        STRINGS[start] = [segname(seg), kind, text, {}]
    return start


def string_at(ea):
    """Start address of the non-empty string literal containing ea (registered in STRINGS), or None."""
    if ea in _str_at:
        return _str_at[ea]
    r = None
    seg = ida_segment.getseg(ea)
    if seg and seg.type != ida_segment.SEG_XTRN and seg.type != ida_segment.SEG_BSS:
        name = segname(seg)
        if name in CSTR_SEGS:  # exact by the section's definition, whatever items IDA made
            data = _cstr_bytes.get(seg.start_ea)
            if data is None:
                data = _cstr_bytes[seg.start_ea] = ida_bytes.get_bytes(seg.start_ea, seg.end_ea - seg.start_ea)
            off = ea - seg.start_ea
            a = data.rfind(b"\0", 0, off) + 1
            b = data.find(b"\0", off)
            if b < 0:
                b = len(data)
            if b > a:
                r = _add_string(seg.start_ea + a, seg, "c", data[a:b])
        elif name not in SKIP_SEGS and seg.type != ida_segment.SEG_CODE:
            head = ida_bytes.get_item_head(ea)
            fl = ida_bytes.get_flags(head)
            if ida_bytes.is_strlit(fl):
                st = ida_nalt.get_str_type(head)
                code = ida_nalt.get_str_type_code(st) if st != idaapi.BADADDR else ida_nalt.STRTYPE_C
                if code in (ida_nalt.STRTYPE_C, ida_nalt.STRTYPE_C_16):
                    wide = code == ida_nalt.STRTYPE_C_16
                    raw = ida_bytes.get_bytes(head, ida_bytes.get_item_size(head)) or b""
                    raw = _utf16_cut(raw) if wide else raw.split(b"\0")[0]
                    if len(raw) >= (8 if wide else 4):  # shorter ones here are mostly IDA misreading data ("5")
                        r = _add_string(head, seg, "utf16" if wide else "c", raw)
            elif name == "__ustring":
                raw = _utf16_cut(ida_bytes.get_bytes(ea, min(4096, seg.end_ea - ea)) or b"")
                if raw:
                    r = _add_string(ea, seg, "utf16", raw)
            elif IS_PE and not ida_bytes.is_float(fl) and not ida_bytes.is_double(fl) and _pe_wide(ea):
                # L"..." IDA typed as something else: `push offset off_BD7C1C` in PLAYER 0x004034c0
                # is L"\\RbxLogs\\", made a dword offset because 0x0052005c lies in .text
                r = _add_string(ea, seg, "utf16", _pe_wide(ea))
            elif ida_bytes.is_unknown(ida_bytes.get_flags(ea)) and not (seg.perm & ida_segment.SEGPERM_WRITE):
                # referenced bytes IDA left undefined in read-only data: accept printable ASCII, 4+ chars
                raw = (ida_bytes.get_bytes(ea, min(1024, seg.end_ea - ea)) or b"").split(b"\0")[0]
                if len(raw) >= 4 and all(32 <= c < 127 or c in (9, 10, 13) for c in raw):
                    r = _add_string(ea, seg, "c", raw)
    _str_at[ea] = r
    return r


def _pe_wide(ea):
    """The UTF-16LE bytes of a NUL-terminated string of 4+ printable ASCII characters at ea, or None."""
    raw = _utf16_cut(ida_bytes.get_bytes(ea, 16384) or b"")  # WMV profile XML in PLAYER 0x00433f40: 4 KB+
    if len(raw) >= 8 and all(raw[k + 1] == 0 and (32 <= raw[k] < 127 or raw[k] in (9, 10, 13))
                             for k in range(0, len(raw), 2)) and len(raw) < 16382:
        return raw
    return None


def _utf16_cut(raw):
    for i in range(0, len(raw) - 1, 2):
        if raw[i] == 0 and raw[i + 1] == 0:
            return raw[:i]
    return raw[: len(raw) & ~1]


def all_strings():
    """Register every string literal, referenced or not."""
    for s in idautils.Segments():
        seg = ida_segment.getseg(s)
        name = segname(seg)
        if name in SKIP_SEGS or seg.type in (ida_segment.SEG_XTRN, ida_segment.SEG_BSS, ida_segment.SEG_CODE):
            continue
        if name in CSTR_SEGS:
            string_at(seg.start_ea)
            data = _cstr_bytes.get(seg.start_ea) or b""
            i = data.find(b"\0")
            while i >= 0 and i + 1 < len(data):
                if data[i + 1]:
                    string_at(seg.start_ea + i + 1)
                i = data.find(b"\0", i + 1)
            continue
        ea = seg.start_ea
        while ea != idaapi.BADADDR and ea < seg.end_ea:
            if ida_bytes.is_strlit(ida_bytes.get_flags(ea)):
                string_at(ea)
            ea = ida_bytes.next_head(ea, seg.end_ea)


# ---------------------------------------------------------------- functions

STATS = {"x86_pic_functions": 0, "x86_pic_refs": 0, "x86_pic_refs_ida_lacked": 0, "x86_pic_ida_absolute_dropped": 0,
         "pe_unmarked_string_refs": 0}
CODE_DATA_REFS = set()  # every non-string data address function code references (after non-lazy pointers)
_callee = {}  # raw call target -> ("f", ea) | ("i", name) | None


def resolve_code(t):
    """A call or function-pointer target: ("f", body ea) with stubs/thunks followed, ("i", import name), or None."""
    if t in _callee:
        return _callee[t]
    r, cur = None, t
    for _ in range(4):
        seg = ida_segment.getseg(cur)
        if not seg:
            break
        if seg.type == ida_segment.SEG_XTRN:
            r = ("i", import_name(cur))
            break
        fn = ida_funcs.get_func(cur)
        if not fn or fn.start_ea != cur:
            break
        r = ("f", cur)
        if fn.flags & ida_funcs.FUNC_THUNK:
            nxt = ida_funcs.calc_thunk_func_target(fn)
            nxt = nxt[0] if isinstance(nxt, (tuple, list)) else nxt
        elif ida_name.get_name(cur).endswith("$shim"):
            # armv7 linker branch island: 12 bytes that only jump to the real function. IDA does not
            # flag these as thunks (IOS_1029 0x00f16000); 3.4% of call edges went through one.
            nxt = next((t for h in idautils.FuncItems(cur) for t in idautils.CodeRefsFrom(h, 0)), None)
        else:
            break
        if nxt in (None, idaapi.BADADDR, cur):
            break
        cur = nxt
    _callee[t] = r
    return r


_x86_thunk = {}


def x86_pic_bases(f):
    """PIC bases of an i386 Mach-O function, ascending: the return address of every `call $+5`
    (popped into a register) and of every call to a `mov reg, [esp]; ret` thunk.
    Why this exists: IDA resolves [reg + disp] only while the base stays in the register it was
    popped into. After `mov eax, esi` it drops the reference or reads the displacement as an absolute
    address (RBX::StepJointsStage::jointsStepWorld, MACSTUDIO 0x005a44e8). The base is one value per
    function and a displacement that reaches a data section from it is megabytes, never a struct
    offset, so scan_function resolves [any reg + disp] against the base without tracking registers."""
    bases = []
    for h in idautils.FuncItems(f.start_ea):
        if ida_bytes.get_byte(h) != 0xE8:
            continue
        t = (h + 5 + ida_bytes.get_wide_dword(h + 1)) & 0xFFFFFFFF
        if t != h + 5:
            k = _x86_thunk.get(t)
            if k is None:
                tb = ida_bytes.get_bytes(t, 4) or b""
                k = _x86_thunk[t] = len(tb) == 4 and tb[0] == 0x8B and tb[1] & 0xC7 == 0x04 and tb[2:] == b"\x24\xC3"
            if not k:
                continue
        bases.append(h + 5)
    return bases


def pe_unmarked_string(v):
    """PE images here have no .reloc, so IDA only guesses which immediates are addresses. For a raw
    operand value in a data section: the string starting there, or the string a pointer stored
    there starts (the same one hop as for marked references), else None."""
    seg = ida_segment.getseg(v)
    if not seg or seg.type in (ida_segment.SEG_CODE, ida_segment.SEG_XTRN, ida_segment.SEG_BSS):
        return None
    s = string_at(v)
    if s is not None:
        return s if s == v and STRINGS[s][1] != "typename" else None
    raw = (ida_bytes.get_bytes(v, min(4096, seg.end_ea - v)) or b"").split(b"\0")[0]
    if len(raw) >= 4 and all(32 <= c < 127 or c in (9, 10, 13) for c in raw):
        # IDA typed the bytes as something else: `push 0B247E0h` in PLAYER 0x005e1530 points 4 bytes
        # into xmmword_B247DC, at "!dirty file: ..."
        return _add_string(v, seg, "c", raw)
    p = ptr(v)
    s = string_at(p) if p else None
    return s if s == p and s is not None and STRINGS[s][1] != "typename" else None


def keep_const(v):
    return 0xFFFF < v < 0xFFFF0000 or v > 0xFFFFFFFF


def scan_function(f):
    strings, typenames, callees, func_refs, globs, imports, imms = {}, {}, {}, {}, {}, {}, {}
    movw = {}  # ARM: reg -> low half waiting for its MOVT
    bases = x86_pic_bases(f) if IS_X86_32 and not IS_PE else ()
    STATS["x86_pic_functions"] += bool(bases)
    insn = ida_ua.insn_t()
    x = ida_xref.xrefblk_t()

    def code_ref(t, bucket):
        r = resolve_code(t)
        if r:
            (bucket if r[0] == "f" else imports)[r[1]] = None

    def data_ref(t, read, depth=0):
        seg = ida_segment.getseg(t)
        if not seg:
            return
        if seg.type == ida_segment.SEG_XTRN:
            imports[import_name(t)] = None
            return
        name = segname(seg)
        if name in SKIP_SEGS:
            return
        fl = ida_bytes.get_flags(t)
        if seg.type == ida_segment.SEG_CODE:
            if ida_bytes.is_code(fl):
                code_ref(t, func_refs)  # None for a mere label (landing pad, jump target)
            elif read:  # literal pool (ARM) or constant in .text
                if ida_bytes.is_off0(fl):
                    if depth < 2:
                        for p in idautils.DataRefsFrom(t):
                            data_ref(p, False, depth + 1)
                else:
                    const(t, fl)
            return
        s = string_at(t)
        if s is not None:
            (typenames if STRINGS[s][1] == "typename" else strings)[s] = None
            return
        if name in PTR_SEGS:
            if depth < 2:
                p = ptr(t)
                if IS_ARM and p & 1:  # pointer to a Thumb function
                    p = code_target(p) or p
                data_ref(p, False, depth + 1)
            return
        if name == "__cfstring":  # {isa, flags, chars, length}
            s = string_at(ptr(t + 2 * PS))
            if s is not None:
                strings[s] = None
            return
        CODE_DATA_REFS.add(t)
        if ida_bytes.has_user_name(fl):
            globs[t] = ida_name.get_name(t)
        p = ptr(t) if seg.type != ida_segment.SEG_BSS else 0
        if p and string_at(p) == p:  # the one hop: a word holding the address of a string's first byte
            if STRINGS[p][1] != "typename":
                strings[p] = None
        elif read and not ida_bytes.is_off0(fl) and not (seg.perm & ida_segment.SEGPERM_WRITE):
            const(t, fl)

    def const(t, fl):
        """A 4- or 8-byte constant the instruction in `insn` reads from read-only memory."""
        size = ida_bytes.get_item_size(t) if ida_bytes.is_data(fl) else 0
        if size not in (4, 8):
            sizes = [ida_ua.get_dtype_size(o.dtype) for o in insn.ops
                     if o.type in (ida_ua.o_mem, ida_ua.o_displ, ida_ua.o_phrase)]
            size = sizes[0] if len(sizes) == 1 else 0
        if size == 4:
            v = ida_bytes.get_wide_dword(t)
        elif size == 8:
            v = ida_bytes.get_qword(t)
        else:
            return
        if keep_const(v):
            imms[v] = None

    for h in idautils.FuncItems(f.start_ea):
        decoded = ida_ua.decode_insn(insn, h) > 0
        mine = []
        if bases and decoded:
            base = bases[max(bisect.bisect_right(bases, h) - 1, 0)]  # the one set up most recently
            for op in insn.ops:
                if op.type == ida_ua.o_displ and 0x1000 <= op.addr & MASK <= MASK - 0x1000:
                    seg = ida_segment.getseg((base + op.addr) & MASK)
                    if seg and seg.type != ida_segment.SEG_CODE and segname(seg) != "HEADER":
                        mine.append((base + op.addr) & MASK)
        for t in mine:
            data_ref(t, insn.itype != ida_allins.NN_lea)
        theirs = []
        ok = x.first_from(h, ida_xref.XREF_FAR)
        while ok:
            if x.iscode:
                if x.type in (ida_xref.fl_CF, ida_xref.fl_CN):
                    code_ref(x.to, callees)
                elif x.type in (ida_xref.fl_JF, ida_xref.fl_JN):
                    tf = ida_funcs.get_func(x.to)
                    if tf and tf.start_ea == x.to and tf.start_ea != f.start_ea:
                        code_ref(x.to, callees)  # tail call
            elif x.type != ida_xref.dr_I:
                theirs.append(x.to)
                if mine:
                    pass  # mine replaces IDA's for this instruction
                elif bases and any(o.type == ida_ua.o_displ and o.addr & MASK == x.to for o in insn.ops):
                    STATS["x86_pic_ida_absolute_dropped"] += 1  # displacement read as an absolute address
                else:
                    data_ref(x.to, decoded and x.type == ida_xref.dr_R)
            ok = x.next_from()
        if mine:
            STATS["x86_pic_refs"] += len(mine)
            STATS["x86_pic_refs_ida_lacked"] += sum(1 for t in mine if t not in theirs)
        if not decoded:
            continue
        fl = ida_bytes.get_full_flags(h)
        for op in insn.ops:
            if op.type == ida_ua.o_void:
                break
            if IS_PE and op.type == ida_ua.o_displ and not ida_bytes.is_off(fl, op.n) and op.addr & MASK not in theirs:
                t = pe_unmarked_string(op.addr & MASK)  # `mov eax, table[ecx*4]` with a raw displacement
                if t is not None:
                    strings[t] = None
                    STATS["pe_unmarked_string_refs"] += 1
            if op.type != ida_ua.o_imm:
                continue
            v = op.value & MASK
            off = ida_bytes.is_off(fl, op.n)
            if IS_PE and not off and v not in theirs:
                t = pe_unmarked_string(v)
                if t is not None:  # `push 0B01234h` that IDA did not make an offset (no .reloc)
                    strings[t] = None
                    STATS["pe_unmarked_string_refs"] += 1
                    continue
            if IS_ARM and insn.itype == ida_allins.ARM_mov and insn.size == 4 and v <= 0xFFFF:
                movw[insn.ops[0].reg] = None if off else v
            elif IS_ARM and insn.itype == ida_allins.ARM_movt:
                lo = movw.pop(insn.ops[0].reg, None)
                if lo is not None and not off and keep_const(lo | (v & 0xFFFF) << 16):
                    imms[lo | (v & 0xFFFF) << 16] = None
            elif not off and keep_const(v):
                imms[v] = None

    return strings, typenames, callees, func_refs, globs, imports, imms


def export_functions(outdir):
    rows, callers = [], {}
    for start in idautils.Functions():
        f = ida_funcs.get_func(start)
        strings, typenames, callees, func_refs, globs, imports, imms = scan_function(f)
        for s in list(strings) + list(typenames):
            STRINGS[s][3][start] = None
        for c in callees:
            callers.setdefault(c, set()).add(start)
        name = ida_name.get_name(start)
        row = {"address": start, "size": f.size(), "name": name, "demangled": None}
        if IS_ARM:
            row["thumb"] = bool(idc.get_sreg(start, "T"))
        ranges = [[a, b] for a, b in idautils.Chunks(start)]
        if len(ranges) > 1:
            row["ranges"] = ranges
        row.update(blocks=ida_gdl.FlowChart(f, flags=ida_gdl.FC_NOEXT).size,
                   data_refs=sum(1 for _ in idautils.DataRefsTo(start)),
                   strings=[STRINGS[s][2] for s in strings], typenames=[STRINGS[s][2] for s in typenames],
                   callees=list(callees), func_refs=list(func_refs),
                   globals=[[a, n] for a, n in globs.items()], imports=list(imports), imms=list(imms))
        rows.append(row)
    dem = demangle_many([row["name"] for row in rows])
    with open(os.path.join(outdir, "functions.jsonl"), "w") as out:
        for row in rows:
            row["demangled"] = dem[row["name"]]
            row["callers"] = len(callers.get(row["address"], ()))
            out.write(json.dumps(row) + "\n")
    return rows


# ---------------------------------------------------------------- vtables

def code_target(w):
    """The function a vtable word points at (Thumb bit cleared), an imported function, or None."""
    for t in ((w, w & ~1) if IS_ARM else (w,)):
        seg = ida_segment.getseg(t)
        if not seg:
            continue
        if seg.type == ida_segment.SEG_CODE and ida_bytes.is_code(ida_bytes.get_flags(t)):
            return t
        if seg.type == ida_segment.SEG_XTRN and not (ida_name.get_name(t) or "").lstrip("_").startswith("ZT"):
            return t  # an import, but not imported data (the __cxxabiv1 typeinfo vtables, _ZTI, _ZTS)
    return None


def itanium_vtables():
    """Every vtable in the image, one record each:
      {address, kind, name, class, typeinfo, code_refs, groups: [{offset_to_top, vbase_vcall_offsets,
       address_point, slots: [{ea, name}]}]}
    groups[0] is the primary vtable; the rest are the secondary vtables of the other bases (multiple
    and virtual inheritance), each with its own offset-to-top and the same typeinfo pointer. A slot's
    ea is 0 for a null entry. Both destructor slots (D1 complete, D0 deleting) are listed; MSVC has one.
    kind:
      "vtable"        a _ZTV symbol defined in the image (name = the symbol)
      "construction"  a _ZTC symbol (construction vtable for Base-in-Derived; class says so)
      "nosymbol"      no symbol (name = null): found by its [offset-to-top][pointer to a _ZTI
                      typeinfo][code pointer] shape, class taken from the typeinfo, address = the
                      offset-to-top word, vbase/vcall prefix not recovered. The Mac build strips local
                      symbols, so its hidden classes land here, and so do their construction vtables,
                      which carry the base's typeinfo. code_refs tells them apart: a real vtable is
                      stored by constructors and destructors, a construction vtable is only pointed
                      at by a VTT.
    code_refs = how many of the table's start and address points are referenced from function code.
    IDA's own guessed `_ZTV` labels are ignored (several sit on typeinfo base-class arrays). VTTs
    (_ZTT) are not exported."""
    macho = ida_ida.inf_get_filetype() == ida_ida.f_MACHO
    real = (lambda n, k: n.startswith("__" + k)) if macho else (lambda n, k: n.startswith("_" + k))
    allnames = list(idautils.Names())
    guess = re.compile(r"_ZT[VC]") if macho else re.compile(r"$^")  # IDA's guesses: one underscore on Mach-O
    names = sorted(ea for ea, n in allnames if not guess.match(n))
    lo, hi = ida_ida.inf_get_min_ea(), ida_ida.inf_get_max_ea()
    ti_cache = {}
    done = set()  # addresses of typeinfo words already inside an emitted vtable

    def is_ti(v):
        """v is the address of a typeinfo object: a _ZTI name, imported ones (not in idautils.Names) and
        IDA's own `_ZTI` labels for stripped ones included; those come from the typeinfo's layout and are sound."""
        r = ti_cache.get(v)
        if r is None:
            r = ti_cache[v] = lo <= v < hi and (ida_name.get_name(v) or "").lstrip("_").startswith("ZTI")
        return r

    def sgn(v):
        return v - (1 << (8 * PS)) if v >> (8 * PS - 1) else v

    def small(v):
        return abs(sgn(v)) < 0x100000 and code_target(v) is None

    def parse(ea, j, end):
        """Groups of the table at ea whose typeinfo word is word j; words are read up to `end`."""
        n = min((end - ea) // PS, 8192)
        w = struct.unpack("<%d%s" % (n, "Q" if PS == 8 else "I"), ida_bytes.get_bytes(ea, n * PS))
        ti, start, groups = w[j], 0, []
        while True:
            done.add(ea + j * PS)
            k, slots = j + 1, []
            while k < n:
                t = code_target(w[k])
                if t is None:
                    m = k
                    while m < n and w[m] == 0 and m - k < 4:
                        m += 1
                    if m == k or m >= n or code_target(w[m]) is None:
                        break  # not a slot: the next group, or the end
                    slots.extend({"ea": 0, "name": ""} for _ in range(m - k))  # null slots inside the table
                    k = m
                    continue
                slots.append({"ea": t, "name": ida_name.get_name(t)})
                k += 1
            groups.append({"offset_to_top": sgn(w[j - 1]), "vbase_vcall_offsets": [sgn(v) for v in w[start:j - 1]],
                           "address_point": ea + (j + 1) * PS, "slots": slots})
            nj = next((q for q in range(k + 1, min(n, k + 66)) if w[q] == ti), None) if ti else None
            if nj is None or not all(small(v) for v in w[k:nj]):
                return ti, groups
            start, j = k, nj

    def bound(ea, seg):
        i = bisect.bisect_right(names, ea)
        return min(names[i] if i < len(names) else seg.end_ea, seg.end_ea)

    recs = []
    for ea, name in allnames:  # 1. symbols
        seg = ida_segment.getseg(ea)
        kind = "vtable" if real(name, "ZTV") else "construction" if real(name, "ZTC") else None
        if not kind or not seg or seg.type == ida_segment.SEG_XTRN or segname(seg) in PTR_SEGS:
            continue
        end = bound(ea, seg)
        if end - ea < 2 * PS:
            continue
        head = [ptr(ea + k * PS) for k in range(min((end - ea) // PS, 64))]
        j = next((k for k in range(1, len(head)) if is_ti(head[k])), 1)  # index of the typeinfo word
        if not all(small(v) for v in head[:j]):
            j = 1
        recs.append((ea, kind, name, name) + parse(ea, j, end))
    for s in idautils.Segments():  # 2. no symbol
        seg = ida_segment.getseg(s)
        if (seg.type in (ida_segment.SEG_XTRN, ida_segment.SEG_BSS, ida_segment.SEG_CODE)
                or segname(seg) in SKIP_SEGS | PTR_SEGS | CSTR_SEGS):
            continue
        n = (seg.end_ea - seg.start_ea) // PS
        w = struct.unpack("<%d%s" % (n, "Q" if PS == 8 else "I"), ida_bytes.get_bytes(seg.start_ea, n * PS) or b"\0" * n * PS)
        for k in range(1, n - 1):
            a = seg.start_ea + k * PS
            if w[k - 1] or a in done or not is_ti(w[k]):  # a primary vtable has offset-to-top 0
                continue
            t = code_target(w[k + 1])
            tf = ida_funcs.get_func(t) if t is not None else None
            if t is not None and (tf is None or tf.start_ea == t):  # a function entry or an import, not mid-code
                ti, groups = parse(a - PS, 1, bound(a, seg))
                if groups[0]["slots"]:
                    recs.append((a - PS, "nosymbol", None, ida_name.get_name(w[k]), ti, groups))
    dem = demangle_many(sorted({r[3] for r in recs}))
    for ea, kind, name, key, ti, groups in sorted(recs, key=lambda r: r[0]):
        cls = re.sub(r"^`?(construction vtable|vtable|typeinfo) for'? ?", "", dem[key] or key)
        refs = sum(1 for a in {ea} | {g["address_point"] for g in groups} if a in CODE_DATA_REFS)
        yield {"address": ea, "kind": kind, "name": name, "class": cls, "typeinfo": ti, "code_refs": refs,
               "groups": groups}


def msvc_slot(t):
    """A vftable slot of a PE image: {ea, name} plus purecall: true, or thunk: {adjust, target,
    target_name[, vtordisp]} for an adjustor thunk (`sub ecx, N` / `add ecx, N`, after
    `sub ecx, [ecx-4]` for a vtordisp thunk, then `jmp`)."""
    r = {"ea": t, "name": ida_name.get_name(t)}
    c = resolve_code(t)
    if c and c[0] == "i" and c[1].lstrip("_") == "purecall" or r["name"].lstrip("_") == "purecall":
        r["purecall"] = True
        return r
    b = ida_bytes.get_bytes(t, 24) or b""
    i, adj, vtordisp = 0, 0, False
    while i < len(b):
        if b[i:i + 2] == b"\x83\xe9":
            adj -= struct.unpack_from("b", b, i + 2)[0]; i += 3
        elif b[i:i + 2] == b"\x83\xc1":
            adj += struct.unpack_from("b", b, i + 2)[0]; i += 3
        elif b[i:i + 2] == b"\x81\xe9":
            adj -= struct.unpack_from("<i", b, i + 2)[0]; i += 6
        elif b[i:i + 2] == b"\x81\xc1":
            adj += struct.unpack_from("<i", b, i + 2)[0]; i += 6
        elif b[i:i + 3] == b"\x2b\x49\xfc":
            vtordisp = True; i += 3
        else:
            break
    if i and i + 5 <= len(b) and b[i] == 0xE9:
        tgt = (t + i + 5 + struct.unpack_from("<i", b, i + 1)[0]) & MASK
    elif i and i + 2 <= len(b) and b[i] == 0xEB:
        tgt = (t + i + 2 + struct.unpack_from("b", b, i + 1)[0]) & MASK
    else:
        return r
    r["thunk"] = {"adjust": adj, "target": tgt, "target_name": ida_name.get_name(tgt)}
    if vtordisp:
        r["thunk"]["vtordisp"] = True
    return r


MSVC_STATS = {}


def msvc_vtables():
    """One record per class with at least one vftable, found through RTTI (tools/ida/msvc_rtti.py):
      {address, kind "vtable", name, class, typeinfo, type_descriptor, code_refs,
       hierarchy: {attributes, bases: [{class, num_contained, mdisp, pdisp, vdisp, attributes}]},
       groups: [{offset_to_top, vbase_vcall_offsets [], address_point, name, col, col_offset,
                 cd_offset, for [class...], slots: [{ea, name, purecall? | thunk?}]}]}
    Same shape as the Itanium records so one pairing script reads both: a group is one vftable,
    address_point is the vftable itself (MSVC's vfptr points at slot 0; the COL pointer is at
    [-4]), offset_to_top is minus the COL offset, address/name are the first group's. `for` is
    the base path MSVC names the vftable after (empty for `6B@`). hierarchy.bases[0] is the class
    itself (the RTTI base class array, preorder). MSVC has one destructor slot (the scalar deleting
    destructor) where Itanium has two."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import msvc_rtti as M
    R = M.scan(M.from_ida())

    def cls(td):
        n = R.tds[td]
        d = ida_name.demangle_name("??_R0" + n[1:] + "@8", 0) or ""
        return re.sub(r"^(class|struct|union|enum) ", "", d.replace(" `RTTI Type Descriptor'", "")) or n

    MSVC_STATS.update(rtti_type_descriptors=len(R.tds), rtti_class_type_descriptors=sum(
        1 for n in R.tds.values() if n[:4] in (".?AV", ".?AU")), rtti_cols=len(R.cols),
        rtti_classes_with_vftable=len(R.classes), vftables=len(R.vfts))
    purecall = thunks = unnamed = 0
    for td, group in sorted(R.classes.items(), key=lambda kv: kv[1][0].ea):
        chd = R.chds[R.cols[group[0].col].chd]
        paths = M.vfptr_paths(R, td)
        gs = []
        for v in group:
            col = R.cols[v.col]
            slots = [msvc_slot(t) for t in v.slots]
            purecall += sum(1 for x in slots if x.get("purecall"))
            thunks += sum(1 for x in slots if x.get("thunk"))
            name = M.vftable_name(R, v)
            unnamed += name is None
            gs.append({"offset_to_top": -col.offset, "vbase_vcall_offsets": [], "address_point": v.ea,
                       "name": name, "col": v.col, "col_offset": col.offset, "cd_offset": col.cd_offset,
                       "for": [cls(t) for t in paths.get(col.offset, [])], "slots": slots})
        bases = [{"class": cls(b.td), "num_contained": b.num_contained, "mdisp": b.mdisp, "pdisp": b.pdisp,
                  "vdisp": b.vdisp, "attributes": b.attributes} for b in (R.bcds[x] for x in chd.bases)]
        yield {"address": group[0].ea, "kind": "vtable", "name": gs[0]["name"], "class": cls(td), "typeinfo": td,
               "type_descriptor": R.tds[td], "code_refs": sum(1 for v in group if v.ea in CODE_DATA_REFS),
               "hierarchy": {"attributes": chd.attributes, "bases": bases}, "groups": gs}
    MSVC_STATS.update(purecall_slots=purecall, adjustor_thunk_slots=thunks, vftables_name_unresolved=unnamed)


VTABLE_SCANNERS = {"itanium": itanium_vtables, "msvc": msvc_vtables}


def main():
    idb = idc.get_idb_path()
    target = os.path.splitext(os.path.basename(idb))[0]
    outdir = idc.ARGV[1] if len(idc.ARGV) > 1 else os.path.join(os.path.dirname(idb), "exports", target + "-evidence")
    os.makedirs(outdir, exist_ok=True)

    rows = export_functions(outdir)
    all_strings()
    with open(os.path.join(outdir, "strings.jsonl"), "w") as out:
        for ea in sorted(STRINGS):
            seg, kind, text, refs = STRINGS[ea]
            out.write(json.dumps({"address": ea, "seg": seg, "kind": kind, "text": text, "refs": list(refs)}) + "\n")

    scanner = VTABLE_SCANNERS.get("msvc" if IS_PE else "itanium")
    vts = list(scanner()) if scanner else []
    with open(os.path.join(outdir, "vtables.jsonl"), "w") as out:
        for v in vts:
            out.write(json.dumps(v) + "\n")

    summary = {
        "target": target,
        "functions": len(rows),
        "functions_with_string": sum(1 for r in rows if r["strings"]),
        "functions_with_typename": sum(1 for r in rows if r["typenames"]),
        "functions_with_callee": sum(1 for r in rows if r["callees"]),
        "functions_with_imm": sum(1 for r in rows if r["imms"]),
        "functions_with_global_or_import": sum(1 for r in rows if r["globals"] or r["imports"]),
        "strings": len(STRINGS),
        "strings_referenced": sum(1 for s in STRINGS.values() if s[3]),
        "vtables": len(vts),
        "vtables_by_kind": {k: sum(1 for v in vts if v["kind"] == k) for k in ("vtable", "construction", "nosymbol")},
        "vtables_nosymbol_with_code_refs": sum(1 for v in vts if v["kind"] == "nosymbol" and v["code_refs"]),
        "vtable_groups": sum(len(v["groups"]) for v in vts),
        "vtable_slots": sum(len(g["slots"]) for v in vts for g in v["groups"]),
        "vtables_implemented": scanner is not None,
    }
    summary.update(STATS)
    summary.update(MSVC_STATS)
    with open(os.path.join(outdir, "summary.json"), "w") as out:
        json.dump(summary, out, indent=1)
    print("export_evidence:", json.dumps(summary))


try:
    main()
except Exception:
    import traceback
    print("export_evidence: FAILED\n" + traceback.format_exc())
finally:
    idaapi.set_database_flag(idaapi.DBFL_KILL)  # never save
    idc.qexit(0)

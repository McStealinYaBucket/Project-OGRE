"""Phase 1 function discovery for the Windows databases (PLAYER, PLAYER_1015, STUDIO).

Repairs what the first auto-analysis got wrong in .text and creates functions only where
the binary itself gives evidence. It never touches .vmp0/.vmp1 and never patches bytes.

Passes, in order:
  1. Original IAT (VMProtect builds only). VMProtect moved the import directory into .vmp1,
     but the code still calls through the linker's IAT at the start of .rdata, whose slots
     still hold their IMAGE_THUNK_DATA (hint/name RVA or ordinal). That range becomes an
     extern segment `.iat` (what IDA makes of a normal .idata): each slot is named
     `__imp_<import>` and typed from its twin in the relocated IAT. IDA had read those RVAs
     as offsets into .text (and remembered them as callees of `call reg`); those references
     are removed and the hull of the bogus targets is undefined and rebuilt.
  2. Garbage splits: an unaligned function whose predecessor instruction falls through
     into it and that nothing calls is deleted and its predecessor re-created.
  3. Discovery rounds, until nothing new: targets of RTTI vftable slots (found through
     Complete Object Locators), `jmp` targets at the end of unwind funclets (destructors),
     `call rel32` targets from code in functions, 16-aligned int3-padded targets of pointers
     in .rdata/.data, `jmp ds:[IAT]` import thunks, and 16-aligned code that directly
     follows int3 padding or a function end. A vftable, funclet or call target that sits
     inside another function at a padded 16-aligned address is split out of it.
  4. C++ EH: every `mov eax, offset FuncInfo / jmp __CxxFrameHandler3` stub is located by
     the FuncInfo magic. Unwind funclets and stubs become tail chunks of the function(s)
     that push the stub (what IDA does by itself when it knows __CxxFrameHandler3, as in
     STUDIO), or stand-alone functions when no pusher is known. Catch blocks are folded
     into the function that owns the FuncInfo. So funclets never appear in the function
     list, and a function's size in the CSV is its body without them.
  5. Trailing int3 is trimmed off function ends; unexplored int3 runs become align items.

A data item in .text is only undefined when a proven entry lands in it and no instruction
of a function addresses it as memory (jump and index tables are read by their switch).
Inter-function padding is int3; nop sleds are loop alignment and never split a function.
The pass is idempotent: a second run changes nothing and keeps the first run's .tsv.

Run (snapshot first; the database is saved packed and <TARGET>-functions.csv rewritten):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-01-pre-discovery.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/discover_functions.py" \
        -Lida/logs/PLAYER-01-discovery.log ida/PLAYER.i64
Output: before/after counts in the log (lines starting `discover:`), every change in
<db dir>/logs/<TARGET>-01-discovery.tsv (address, end, reason), and the function list in
<db dir>/exports/<TARGET>-functions.csv. Outside the repo layout both land next to the database.
"""
import bisect, collections, os, re, struct
import ida_allins, ida_auto, ida_bytes, ida_funcs, ida_ida, ida_idaapi, ida_idp, ida_loader
import ida_nalt, ida_name, ida_netnode, ida_segment, ida_typeinf, ida_ua, ida_xref, idaapi, idautils, idc

BAD = ida_idaapi.BADADDR
ida_auto.auto_wait()
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)


def out_path(sub, name):
    d = os.path.join(DBDIR, sub)
    return os.path.join(d if os.path.isdir(d) else DBDIR, name)


def say(*a):
    print("discover: " + " ".join(str(x) for x in a))


def seg(name):
    s = ida_segment.get_segm_by_name(name)
    return s.start_ea, s.end_ea


T0, T1 = seg(".text")
R0, R1 = seg(".rdata")
D0, D1 = seg(".data")
TEXT = ida_bytes.get_bytes(T0, T1 - T0)
RDATA = ida_bytes.get_bytes(R0, R1 - R0)
DATA = ida_bytes.get_bytes(D0, D1 - D0)
LOG = []                               # (address, end, reason)
NOTES = set()                          # things left alone, same shape
STAT = collections.Counter()
in_text = lambda ea: T0 <= ea < T1
ptr_table = lambda ea: all(in_text(v) for v in struct.unpack_from("<2I", TEXT, ea - T0))
flags = ida_bytes.get_full_flags
insn = ida_ua.insn_t()
CALLS = (ida_allins.NN_call, ida_allins.NN_callni, ida_allins.NN_callfi)


def rdw(ea):
    return struct.unpack_from("<I", RDATA, ea - R0)[0] if R0 <= ea <= R1 - 4 else None


def is_code_head(ea):
    f = flags(ea)
    return ida_bytes.is_code(f) and ida_bytes.is_head(f)


def chunk(ea):
    ci = ida_funcs.fchunk_info_t()
    return ci if ida_funcs.get_fchunk_info(ci, ea) else None


def func_start(ea):
    fn = ida_funcs.get_func(ea)
    return fn.start_ea if fn else None


def coverage():
    """Byte totals for .text by what IDA currently thinks each byte is."""
    c = collections.Counter()
    ea = T0
    while ea < T1:
        f = flags(ea)
        if ida_bytes.is_code(f):
            n = ida_bytes.get_item_size(ea)
            c["in_function" if ida_funcs.get_func(ea) else "code_no_function"] += n
        elif ida_bytes.is_data(f):
            n = ida_bytes.get_item_size(ea)
            c["align" if ida_bytes.is_align(f) else "data"] += n
        else:
            nxt = ida_bytes.next_head(ea, T1)
            n = (T1 if nxt == BAD else nxt) - ea
            cc = TEXT[ea - T0:ea - T0 + n].count(b"\xcc")
            c["unexplored_int3"] += cc
            c["unexplored_other"] += n - cc
        ea += n
    return c


def report(tag):
    nf = sum(1 for _ in idautils.Functions(T0, T1))
    tails = sum(len(list(idautils.Chunks(f))) - 1 for f in idautils.Functions(T0, T1))
    c = coverage()
    say("%s: functions=%d (in .text %d, tail chunks %d) .text bytes: %s" % (
        tag, ida_funcs.get_func_qty(), nf, tails, " ".join("%s=%d" % kv for kv in sorted(c.items()))))
    return nf, c


# ---------------------------------------------------------------- 1. original IAT
def name_original_iat():
    """Turn the linker's IAT at the start of .rdata into an extern segment `.iat`, the shape IDA gives
    a normal PE's .idata: unloaded dwords named `__imp_<import>`, typed from the twin slot of the
    relocated IAT. Returns (iat_start, iat_end, bogus .text targets IDA derived from the thunk RVAs)."""
    mods = []
    for i in range(ida_nalt.get_import_module_qty()):
        names, ords = {}, {}

        def cb(ea, name, ordinal, names=names, ords=ords):
            if name:
                names[name] = ea
            if ordinal:
                ords[ordinal] = ea
            return True
        ida_nalt.enum_import_names(i, cb)
        mods.append((ida_nalt.get_import_module_name(i) or "?", names, ords))
    base = idaapi.get_imagebase()
    ea, found, end, misses = R0, [], R0, 0
    while ea < R1 - 8:
        group = []
        while rdw(ea):
            group.append(ea)
            ea += 4
        if not group:
            break                                   # two zero dwords in a row: past the IAT
        ea += 4
        dec = []
        for slot in group:
            v = rdw(slot)
            if v & 0x80000000:
                dec.append(("o", v & 0xffff))
                continue
            s = base + v + 2
            raw = RDATA[s - R0:s - R0 + 512].split(b"\0")[0] if R0 <= s < R1 else b""
            ok = raw and all(0x21 <= ch < 0x7f for ch in raw)
            dec.append(("n", raw.decode()) if ok else (None, None))
        hits = [m for m in mods if len(set(m[1].values()) | set(m[2].values())) == len(group) and
                all((k == "n" and v in m[1]) or (k == "o" and v in m[2]) for k, v in dec)]
        if len(hits) != 1:
            misses += 1
            if misses > 1 and not any(k for k, _ in dec):
                break                               # nothing decodes twice running: past the IAT
            STAT["iat group unmatched (%d slots at %#x)" % (len(group), group[0])] += 1
            continue
        misses = 0
        mod, names, ords = hits[0]
        for slot, (k, v) in zip(group, dec):
            twin = names[v] if k == "n" else ords[v]
            found.append((slot, mod, v if k == "n" else (ida_name.get_name(twin) or "%s_%d" % (mod, v)), twin))
        end = ea
    if not found:
        done = ida_segment.get_segm_by_name(".iat")
        say("original IAT: " + ("already an extern segment at %#x" % done.start_ea if done else
                                "none in .rdata (imports are where the import directory says)"))
        return 0, 0, []
    bogus = sorted({rdw(slot) for slot, _, _, _ in found if in_text(rdw(slot))})
    sites = set()                                   # instructions whose xrefs came from the thunk values
    for slot, _, _, _ in found:
        sites.update(x.frm for x in idautils.XrefsTo(slot) if in_text(x.frm))
    for t in bogus:
        for x in list(idautils.XrefsTo(t)):
            if x.type in (ida_xref.fl_CN, ida_xref.fl_JN, ida_xref.fl_CF, ida_xref.fl_JF) and in_text(x.frm) \
                    and TEXT[x.frm - T0] not in (0xE8, 0xE9, 0xEB, 0x0F) and not 0x70 <= TEXT[x.frm - T0] < 0x80:
                ida_xref.del_cref(x.frm, t, False)  # `call ds:[slot]` / `call reg`, resolved through the RVA
                sites.add(x.frm)
    bset, n = set(bogus), ida_netnode.netnode("$ vmm functions")
    idx = n.altfirst() if ida_netnode.exist(n) else ida_netnode.BADNODE
    while idx != ida_netnode.BADNODE:               # x86 module's memory of resolved `call reg` targets
        nxt, v = n.altnext(idx), n.altval(idx)
        if v - 1 in bset and in_text(idx) and TEXT[idx - T0] not in (0xE8, 0xE9):
            n.altdel(idx)
            ida_xref.del_cref(idx, v - 1, False)
            sites.add(idx)
        idx = nxt
    rs = ida_segment.get_segm_by_name(".rdata")
    seg_t = ida_segment.segment_t()
    seg_t.start_ea, seg_t.end_ea, seg_t.sel, seg_t.bitness = R0, end, rs.sel, rs.bitness
    seg_t.align, seg_t.comb, seg_t.perm = rs.align, rs.comb, ida_segment.SEGPERM_READ
    if not ida_segment.set_segm_start(R0, end, ida_segment.SEGMOD_KEEP) or \
            not ida_segment.add_segm_ex(seg_t, ".iat", "DATA", ida_segment.ADDSEG_NOSREG | ida_segment.ADDSEG_NOAA):
        raise RuntimeError("could not carve the .iat segment out of .rdata")
    g = ida_segment.get_segm_by_name(".iat")
    g.type = ida_segment.SEG_XTRN
    g.update()
    for slot, mod, label, twin in found:
        rva = rdw(slot)
        ida_bytes.del_items(slot, ida_bytes.DELIT_SIMPLE, 4)
        for k in range(4):
            ida_bytes.del_value(slot + k)           # like a loader-made .idata: the slot has no static value
        ida_bytes.create_dword(slot, 4)
        ida_name.set_name(slot, "__imp_" + label, ida_name.SN_NOWARN | ida_name.SN_FORCE)
        ida_bytes.set_cmt(slot, "%s (thunk data %#x in the file; loader IAT twin at %#x)" % (mod, rva, twin), True)
        tif = ida_typeinf.tinfo_t()
        if ida_nalt.get_tinfo(tif, twin):
            if tif.is_func():
                ptr = ida_typeinf.tinfo_t()
                ptr.create_ptr(tif)
                tif = ptr
            ida_typeinf.apply_tinfo(slot, tif, ida_typeinf.TINFO_DEFINITE)
    for x in sites:
        ida_auto.plan_ea(x)
    ida_auto.auto_wait()
    say("original IAT: %d slots in %#x-%#x made extern segment .iat; %d bogus .text targets, %d sites re-analysed"
        % (len(found), R0, end, len(bogus), len(sites)))
    LOG.append((R0, end, "segment .iat (SEG_XTRN) carved out of .rdata, %d import slots named" % len(found)))
    return R0, end, bogus


def wipe_polluted(bogus):
    """Undefine the hull of the bogus IAT targets, snapped outward to clean function starts."""
    def clean(ea):
        return func_start(ea) == ea and TEXT[ea - 1 - T0] == 0xCC
    h0 = min(bogus) & ~15
    while h0 > T0 and not clean(h0):
        h0 -= 16
    h1 = (max(bogus) + 16) & ~15
    while h1 < T1 and not clean(h1):
        h1 += 16
    readd = []
    for f in list(idautils.Functions(T0, T1)):
        if any(a < h1 and b > h0 for a, b in idautils.Chunks(f)):
            if not h0 <= f < h1:
                readd.append(f)
            ida_funcs.del_func(f)
            STAT["polluted region: functions deleted"] += 1
    ida_bytes.del_items(h0, ida_bytes.DELIT_SIMPLE, h1 - h0)
    for f in readd:
        ida_funcs.add_func(f)
    say("polluted region %#x-%#x undefined (%d bytes)" % (h0, h1, h1 - h0))
    LOG.append((h0, h1, "undefined: hull of bogus IAT-RVA targets"))


# ---------------------------------------------------------------- scans
STUB = re.compile(
    rb"(?:\x8b\x54\x24\x08(?:\x8d\x02|\x8d\x42.|\x8d\x82....)(?:\x8b\x0a|\x8b\x4a.|\x8b\x8a....)\x33\xc8\xe8...."
    rb"(?:(?:\x83\xc0.|\x05....)?(?:\x8b\x0a|\x8b\x4a.|\x8b\x8a....)\x33\xc8\xe8....)?)?\xb8(....)\xe9....", re.S)


def flow_end(ea, limit=0x400):
    """Linear extent of a funclet: decode to the first stop instruction past every forward branch."""
    start, maxfwd, heads = ea, ea, []
    while ea - start < limit:
        n = ida_ua.decode_insn(insn, ea)
        if n <= 0 or TEXT[ea - T0] == 0xCC:
            return None
        heads.append(ea)
        nxt, it = ea + n, insn.itype
        if ida_allins.NN_ja <= it <= ida_allins.NN_jz or it == ida_allins.NN_jmpshort:
            if nxt <= insn.Op1.addr < start + limit:
                maxfwd = max(maxfwd, insn.Op1.addr)
        if (insn.get_canon_feature() & ida_idp.CF_STOP) and nxt > maxfwd:
            return nxt, heads
        ea = nxt
    return None


def scan_eh():
    """FuncInfo-driven map of text$x. Returns stubs {start: funcinfo}, extents {start: (end, heads)},
    refs {extent start: set of funcinfo}, catches {handler: funcinfo}."""
    stubs, ext, refs, catches = {}, {}, collections.defaultdict(set), {}
    for m in STUB.finditer(TEXT):
        fi = struct.unpack("<I", m.group(1))[0]
        mg = rdw(fi)
        if mg is None or (mg & 0x1fffffff) not in (0x19930520, 0x19930521, 0x19930522):
            continue
        s, e = T0 + m.start(), T0 + m.end()
        heads, a = [], s
        while a < e:
            heads.append(a)
            a += ida_ua.decode_insn(insn, a) or 1
        if a != e:
            STAT["eh: stub does not decode"] += 1
            continue
        stubs[s] = fi
        ext[s] = (e, heads)
        refs[s].add(fi)
    for s, fi in stubs.items():
        _, maxstate, punw, ntry, ptry = struct.unpack_from("<5I", RDATA, fi - R0)
        if maxstate > 0x4000 or ntry > 0x400:
            STAT["eh: implausible FuncInfo"] += 1
            continue
        for i in range(maxstate):
            a = rdw(punw + i * 8 + 4)
            if a and in_text(a):
                if a not in ext:
                    r = flow_end(a)
                    if not r:
                        STAT["eh: unwind funclet does not decode"] += 1
                        continue
                    ext[a] = r
                refs[a].add(fi)
        for i in range(ntry):
            ncatch, ph = struct.unpack_from("<2I", RDATA, ptry + i * 20 + 12 - R0)
            for j in range(ncatch):
                h = rdw(ph + j * 16 + 12)
                if h and in_text(h):
                    catches[h] = fi
    # an extent that runs into the next entry is not a clean funclet: drop both sides of the clash
    order = sorted(ext)
    for a, b in zip(order, order[1:]):
        if ext[a] and ext[a][0] > b:
            STAT["eh: overlapping extents dropped"] += 1
            ext[a] = ext[b] = None
    ext = {a: v for a, v in ext.items() if v}
    jmps = set()
    for a, (e, heads) in ext.items():
        if a not in stubs and TEXT[heads[-1] - T0] == 0xE9:
            t = (e + struct.unpack_from("<i", TEXT, heads[-1] + 1 - T0)[0]) & 0xffffffff
            if in_text(t):
                jmps.add(t)
    return stubs, ext, refs, catches, jmps


def scan_vftables():
    cols = set()
    for off in range(0, len(RDATA) - 20, 4):
        sig, o, cd, td, chd = struct.unpack_from("<5I", RDATA, off)
        if sig == 0 and D0 <= td < D1 and R0 <= chd < R1 and o < 0x10000 and cd < 0x10000 \
                and DATA[td + 8 - D0:td + 11 - D0] == b".?A":
            cols.add(R0 + off)
    targets, n = set(), 0
    for off in range(0, len(RDATA) - 8, 4):
        if struct.unpack_from("<I", RDATA, off)[0] in cols:
            ea, k = R0 + off + 4, 0
            while True:
                t = rdw(ea)
                if t is None or not in_text(t) or (k and ida_bytes.has_xref(flags(ea))):
                    break
                targets.add(t)
                ea += 4
                k += 1
            n += 1 if k else 0
    say("RTTI: %d complete object locators, %d vftables, %d distinct slot targets" % (len(cols), n, len(targets)))
    return targets


# ---------------------------------------------------------------- function creation
def prev_ok(ea, strong=False):
    """Does a function boundary plausibly sit right before ea?"""
    if ea == T0:
        return True
    p = ida_bytes.get_item_head(ea - 1)
    f = flags(p)
    if ida_bytes.is_code(f):
        if TEXT[p - T0] == 0xCC and ida_bytes.get_item_size(p) == 1:
            return True
        ida_ua.decode_insn(insn, p)
        if insn.get_canon_feature() & ida_idp.CF_STOP:
            return True
        return strong and insn.itype in CALLS       # a call that does not return (throw, abort)
    if ida_bytes.is_data(f) and not ida_bytes.is_align(f) and data_is_legit(p):
        return True                                 # directly after the previous function's jump table
    return TEXT[ea - 1 - T0] == 0xCC


def data_is_legit(head):
    """A data item in .text is real when an instruction inside a function addresses it as memory
    (`jmp ds:tbl[eax*4]`, `movzx eax, byte ptr tbl[ecx]`). `push offset x` proves nothing."""
    tmp = ida_ua.insn_t()
    for a in range(head, ida_bytes.get_item_end(head)):
        x = ida_xref.get_first_dref_to(a)
        while x != BAD:
            if in_text(x) and is_code_head(x) and ida_funcs.get_func(x) and ida_ua.decode_insn(tmp, x) \
                    and any(op.type in (ida_ua.o_mem, ida_ua.o_displ) and op.addr == a for op in tmp.ops):
                return True
            x = ida_xref.get_next_dref_to(a, x)
    return False


def undefine_item(h):
    """Undefine a function-less item (a whole misdecoded code run, or a data item no switch reads)."""
    f = flags(h)
    if ida_funcs.get_func(h):
        return False
    if ida_bytes.is_data(f):
        if ida_bytes.is_align(f) or data_is_legit(h):
            return False
        ida_bytes.del_items(h, ida_bytes.DELIT_SIMPLE, ida_bytes.get_item_end(h) - h)
        STAT["bogus data items undefined"] += 1
        return True
    a = b = h
    while True:
        p = ida_bytes.get_item_head(a - 1)
        if p >= a or not is_code_head(p) or ida_funcs.get_func(p):
            break
        a = p
    while b < T1 and is_code_head(b) and not ida_funcs.get_func(b):
        b = ida_bytes.get_item_end(b)
    ida_bytes.del_items(a, ida_bytes.DELIT_SIMPLE, b - a)
    STAT["misdecoded code runs undefined"] += 1
    return True


def add_func_repair(ea, end=BAD):
    """add_func; when the flow hits bytes IDA cannot turn into an instruction because a misdecoded
    function-less item sits on them, undefine that item and try again."""
    for _ in range(64):
        if ida_funcs.add_func(ea, end):
            return True
        fn = ida_funcs.func_t()
        fn.start_ea = ea
        if end != BAD or ida_funcs.find_func_bounds(fn, ida_funcs.FIND_FUNC_DEFINE) != ida_funcs.FIND_FUNC_UNDEF:
            return False
        bad = fn.end_ea
        n = ida_ua.decode_insn(insn, bad) if in_text(bad) else 0
        if n <= 0:
            return False
        changed, a = False, bad
        while a < bad + n:
            h = ida_bytes.get_item_head(a)
            if ida_bytes.is_unknown(flags(h)):
                a += 1
                continue
            g = ida_funcs.get_func(h)
            if g and bad < g.start_ea < bad + n and g.start_ea != ea:
                ida_funcs.del_func(g.start_ea)      # a "function" that starts inside this instruction
                LOG.append((g.start_ea, g.start_ea, "function deleted: starts inside the instruction at %#x" % bad))
                STAT["deleted: function starting mid-instruction of proven code"] += 1
            if not undefine_item(h):
                return False
            changed = True
        if not changed:
            return False
    return False


def make_func(ea, why, end=BAD):
    if TEXT[ea - T0] == 0xCC:
        STAT["rejected %s: starts with int3" % why] += 1
        return False
    h = ida_bytes.get_item_head(ea)
    f = flags(h)
    if ida_bytes.is_data(f):
        if ida_bytes.is_align(f) or data_is_legit(h):
            STAT["rejected %s: inside real data" % why] += 1
            return False
        if ptr_table(ea) and why in ("carve", "dataref"):
            STAT["rejected %s: looks like a pointer table" % why] += 1
            return False
        undefine_item(h)
    elif ida_bytes.is_code(f) and h != ea:
        if not undefine_item(h):
            STAT["rejected %s: mid-instruction of a function" % why] += 1
            return False
    elif ida_bytes.is_unknown(f) and ptr_table(ea) and why in ("carve", "dataref"):
        STAT["rejected %s: looks like a pointer table" % why] += 1
        return False
    if not add_func_repair(ea, end):
        STAT["rejected %s: add_func failed" % why.split(" (")[0]] += 1
        NOTES.add((ea, ea, "NOT CREATED: add_func failed for " + why))
        return False
    LOG.append((ea, ida_funcs.get_func(ea).end_ea, why))
    STAT["created: " + why.split(" (")[0]] += 1
    return True


def ensure_func(ea, why, split=False):
    fn = ida_funcs.get_func(ea)
    if fn is None:
        return make_func(ea, why)
    if fn.start_ea == ea:
        return False
    if not (split and ea & 15 == 0 and is_code_head(ea) and prev_ok(ea, True)):
        STAT["conflict %s: target inside a function, not a padded 16-aligned start" % why] += 1
        NOTES.add((ea, fn.start_ea, "NOT SPLIT: %s inside function, not a padded 16-aligned start" % why))
        return False
    owners, first = [], fn.start_ea
    ci = chunk(ea)
    if ci.is_tail() and ci.start_ea == ea:          # a whole tail (IDA merged a jmp target): detach it
        for _ in range(4096):                       # one referer at a time, there can be hundreds
            g = ida_funcs.get_func(ea)
            if g is None or not ida_funcs.remove_func_tail(g, ea):
                break
    while True:
        g = ida_funcs.get_func(ea)
        if g is None or g.start_ea == ea or len(owners) > 8:
            break
        owners.append(g.start_ea)
        ida_funcs.del_func(g.start_ea)
    ok = make_func(ea, why + " (split out of %#x)" % first)
    STAT["splits: target was inside another function"] += ok
    for o in owners:
        if not ida_funcs.add_func(o):
            STAT["split: owner could not be re-created"] += 1
            LOG.append((o, o, "LOST: owner not re-created after split at %#x" % ea))
    return ok


def merge_fallthrough_splits():
    """Unaligned function start that the previous instruction falls into and nothing calls."""
    for s in list(idautils.Functions(T0, T1)):
        fn = ida_funcs.get_func(s)
        if s & 15 == 0 or fn is None or fn.start_ea != s or (fn.flags & ida_funcs.FUNC_LIB) \
                or in_eh(s):
            continue
        if ida_name.get_name(s)[:4] not in ("sub_", "SEH_") \
                or any(x.type in (ida_xref.fl_CN, ida_xref.fl_CF) for x in idautils.XrefsTo(s)):
            continue
        p = ida_bytes.get_item_head(s - 1)
        pf = ida_funcs.get_func(p)
        if ida_bytes.is_data(flags(p)) and not ida_bytes.is_align(flags(p)) and pf is None \
                and TEXT[s - 1 - T0] != 0xCC and not data_is_legit(p):
            ida_funcs.del_func(s)                   # operand bytes had been made data, code resumed after
            LOG.append((s, s, "function deleted: unaligned start right after a bogus data item"))
            STAT["deleted: unaligned start after bogus data"] += 1
            continue
        if not is_code_head(p) or pf is None or p + ida_bytes.get_item_size(p) != s:
            continue
        ida_ua.decode_insn(insn, p)
        if (insn.get_canon_feature() & ida_idp.CF_STOP) or insn.itype in CALLS or TEXT[p - T0] == 0xCC:
            continue
        owner = pf.start_ea
        ida_funcs.del_func(s)
        ida_funcs.del_func(owner)
        if ida_funcs.add_func(owner):
            LOG.append((s, ida_funcs.get_func(owner).end_ea, "merged back into %#x (fallthrough split)" % owner))
            STAT["merged: fallthrough split"] += 1
        else:
            STAT["merge: owner could not be re-created"] += 1
            LOG.append((owner, owner, "LOST: owner not re-created after merge of %#x" % s))


def extend_truncated():
    """A function whose last instruction falls through into function-less code was cut short."""
    for s in list(idautils.Functions(T0, T1)):
        fn = ida_funcs.get_func(s)
        e = fn.end_ea
        p = ida_bytes.get_item_head(e - 1)
        if e >= T1 or in_eh(s) or not is_code_head(p) or not is_code_head(e) or ida_funcs.get_func(e):
            continue
        ida_ua.decode_insn(insn, p)
        if (insn.get_canon_feature() & ida_idp.CF_STOP) or insn.itype in CALLS or TEXT[e - T0] == 0xCC:
            continue
        ida_funcs.del_func(s)
        if not ida_funcs.add_func(s):
            ida_funcs.add_func(s, e)
            STAT["truncated function could not be re-created, restored"] += 1
        elif ida_funcs.get_func(s).end_ea != e:
            STAT["truncated functions extended"] += 1
            LOG.append((s, ida_funcs.get_func(s).end_ea, "re-created: fell through into function-less code at %#x" % e))
    ida_auto.auto_wait()


def in_eh(ea):
    i = bisect.bisect_right(EH_STARTS, ea) - 1
    return i >= 0 and ea < EXT[EH_STARTS[i]][0]


def call_targets():
    out = set()
    for m in re.finditer(rb"\xe8(....)", TEXT, re.S):
        src = T0 + m.start()
        t = (src + 5 + struct.unpack("<i", m.group(1))[0]) & 0xffffffff
        if in_text(t) and func_start(t) != t and is_code_head(src) and ida_funcs.get_func(src) \
                and not in_eh(src):
            out.add(t)
    return out


def pointer_targets():
    out = set()
    for blob in (RDATA, DATA):
        for (t,) in struct.iter_unpack("<I", blob[:len(blob) & ~3]):
            if in_text(t) and t & 15 == 0:
                out.add(t)
    return out


def discovery_round(vt, iat):
    before = STAT.copy()
    merge_fallthrough_splits()
    for t in sorted(vt):
        if in_eh(t) or func_start(t) == t:
            continue
        if t & 15 == 0 or ida_funcs.get_func(t):
            ensure_func(t, "vftable slot", True)
        elif TEXT[t - T0:t - T0 + 2] == b"\xff\x25" or prev_ok(t, True):
            make_func(t, "vftable slot")
        else:
            STAT["skipped vftable slot: unaligned, no boundary before it"] += 1
            NOTES.add((t, t, "NOT CREATED: unaligned vftable slot target with no boundary before it"))
    for t in sorted(FJMPS):                         # `jmp dtor` at the end of an unwind funclet
        if not in_eh(t) and func_start(t) != t:
            ensure_func(t, "funclet jmp target", True)
    for t in sorted(call_targets()):
        if in_eh(t):
            continue
        if t & 15 == 0 and (ida_funcs.get_func(t) or prev_ok(t, True)):
            ensure_func(t, "call target", True)
        elif ida_funcs.get_func(t) is None and prev_ok(t):
            make_func(t, "call target")
        else:
            STAT["skipped call target: inside a function or no boundary before it"] += 1
            NOTES.add((t, t, "NOT CREATED: call target inside a function or with no boundary before it"))
    for t in sorted(PTRS):
        if not in_eh(t) and ida_funcs.get_func(t) is None and TEXT[t - T0] != 0xCC and prev_ok(t):
            make_func(t, "dataref")
    for m in re.finditer(rb"\xff\x25(....)", TEXT, re.S):
        ea, slot = T0 + m.start(), struct.unpack("<I", m.group(1))[0]
        if any(a <= slot < b for a, b in iat) and ida_funcs.get_func(ea) is None and not in_eh(ea) \
                and not ida_bytes.is_data(flags(ida_bytes.get_item_head(ea))) and prev_ok(ea):
            make_func(ea, "import thunk", ea + 6)
    ea = T0
    while ea < T1:                                  # carve: walk the function-less gaps
        ci = chunk(ea)
        if ci:
            ea = ci.end_ea
            continue
        nxt = ida_funcs.get_next_fchunk_ea(ea)
        gap_end = T1 if nxt == BAD or nxt > T1 else nxt
        a = (ea + 15) & ~15
        while a < gap_end:
            if TEXT[a - T0] != 0xCC and not in_eh(a) and ida_funcs.get_func(a) is None:
                f = flags(a)
                inside_data = ida_bytes.is_data(flags(ida_bytes.get_item_head(a)))
                if inside_data:
                    ok = TEXT[a - 1 - T0] == 0xCC and not ida_bytes.is_align(flags(ida_bytes.get_item_head(a)))
                else:
                    # nop-sled padding (asm libraries) counts only when data or a call points here;
                    # a loop head inside undiscovered code has jump references only
                    ok = (ida_bytes.is_unknown(f) or is_code_head(a)) and (prev_ok(a) or (
                        ida_bytes.is_align(flags(ida_bytes.get_item_head(a - 1))) and
                        any(x.type in (ida_xref.dr_O, ida_xref.fl_CN) for x in idautils.XrefsTo(a))))
                if ok:
                    make_func(a, "carve")
            c2 = chunk(a)
            a = ((c2.end_ea + 15) & ~15) if c2 else a + 16
        ea = gap_end
    ida_auto.auto_wait()
    return sum(v for k, v in STAT.items() if k.startswith("created")) - \
        sum(v for k, v in before.items() if k.startswith("created"))


# ---------------------------------------------------------------- 4. C++ EH
def eh_parents(stubs):
    """stub start -> function starts that load its address (push offset stub / mov eax, offset stub)."""
    par = collections.defaultdict(set)
    for m in re.finditer(rb"[\x68\xb8](....)", TEXT, re.S):
        v = struct.unpack("<I", m.group(1))[0]
        if v in stubs:
            site = T0 + m.start()
            f = func_start(site) if is_code_head(site) else None
            if f is not None and not in_eh(f):
                par[v].add(f)
            else:
                STAT["eh: stub reference outside any function"] += 1
                NOTES.add((site, v, "stub reference outside any function"))
    return par


def rebuild_eh(stubs, refs, par):
    fi_owners = collections.defaultdict(set)
    for s, fi in stubs.items():
        fi_owners[fi] |= par.get(s, set())
    owners = {a: frozenset().union(*(fi_owners[fi] for fi in refs[a])) for a in EXT}
    ends = {v[0] for v in EXT.values()}
    for a in EH_STARTS:                             # tails IDA cut in the middle of a funclet: dissolve
        for x in (a, EXT[a][0] - 1):
            ci = chunk(x)
            if ci and ci.is_tail() and in_eh(ci.start_ea) and (ci.start_ea not in EXT or ci.end_ea not in ends):
                for _ in range(64):
                    g, c2 = ida_funcs.get_func(x), chunk(x)
                    if g is None or c2 is None or not c2.is_tail() or not ida_funcs.remove_func_tail(g, x):
                        break
                STAT["eh: malformed tails dissolved"] += 1
    native = set()
    for a in EH_STARTS:                             # make every extent exact code
        b, heads = EXT[a]
        ca, cb = chunk(a), chunk(b - 1)
        if ca and cb and ca.is_tail() and ca.start_ea == cb.start_ea:
            native.add(a)                           # IDA already made it a tail (STUDIO)
            continue
        if not owners[a] and func_start(a) == a and ida_funcs.get_func(a).end_ea == b:
            continue                                # orphan already made stand-alone
        clash = False
        for h in heads:
            g = ida_funcs.get_func(h)
            if g:
                if in_eh(g.start_ea):
                    ida_funcs.del_func(g.start_ea)  # funclet or stub that had been made a function
                    STAT["eh: stand-alone funclet functions dissolved"] += 1
                else:
                    clash = True
        if clash:
            STAT["eh: extent overlaps a real function, left alone"] += 1
            NOTES.add((a, b, "EH extent overlaps function %#x, left alone" % func_start(a if func_start(a) else heads[-1])))
            owners[a] = None
            continue
        if not all(is_code_head(h) and ida_bytes.get_item_end(h) == n for h, n in zip(heads, heads[1:] + [b])):
            h0 = ida_bytes.get_item_head(a)
            ida_bytes.del_items(h0, ida_bytes.DELIT_SIMPLE, b - h0)
            for h in heads:
                ida_ua.create_insn(h)
            STAT["eh: extents re-created as code"] += 1
    ida_auto.auto_wait()
    i = 0
    while i < len(EH_STARTS):                       # merge neighbours with identical owners
        a = EH_STARTS[i]
        b = EXT[a][0]
        j = i + 1
        while j < len(EH_STARTS) and EH_STARTS[j] == b and owners[EH_STARTS[j]] == owners[a] \
                and owners[a] and EH_STARTS[j] not in native and a not in native:
            b = EXT[EH_STARTS[j]][0]
            j += 1
        i = j
        if a in native:
            STAT["eh: extents already tails (kept)"] += 1
            continue
        if owners[a] is None:
            continue
        if not is_code_head(a):
            STAT["eh: extent is not code after repair"] += 1
            continue
        if not owners[a]:
            if func_start(a) != a:
                make_func(a, "eh orphan (no function pushes its handler)", b)
            continue
        for o in sorted(owners[a]):
            pf = ida_funcs.get_func(o)
            if pf and pf.start_ea == o and ida_funcs.append_func_tail(pf, a, b):
                STAT["eh: tail attachments"] += 1
            else:
                STAT["eh: tail attachment failed"] += 1
        LOG.append((a, b, "eh tail of %s" % ",".join("%#x" % o for o in sorted(owners[a])[:4])))
        STAT["eh: tail chunks created"] += 1


def attach_catches(catches, stubs, par):
    fi_owners = collections.defaultdict(set)
    for s, fi in stubs.items():
        fi_owners[fi] |= par.get(s, set())
    for h in sorted(catches):
        fn = ida_funcs.get_func(h)
        if fn and fn.start_ea == h and not any(x.type in (ida_xref.fl_CN, ida_xref.fl_CF) for x in idautils.XrefsTo(h)):
            ida_funcs.del_func(h)                   # a catch block that had been made a function
            STAT["catch: stand-alone catch functions dissolved"] += 1
            fn = None
        if fn:
            STAT["catch: already inside a function"] += 1
            continue
        cands = [p for p in fi_owners[catches[h]] if p < h]
        pf = ida_funcs.get_func(max(cands)) if cands else None
        if pf is None:
            STAT["catch: owner unknown, left alone"] += 1
            NOTES.add((h, h, "catch handler not folded: no function pushes its handler stub"))
            continue
        nxt, own = ida_funcs.get_next_fchunk_ea(pf.start_ea), True
        while nxt != BAD and nxt <= h and func_start(nxt) == pf.start_ea and nxt != pf.start_ea:
            nxt, own = ida_funcs.get_next_fchunk_ea(nxt), False
        if nxt != BAD and nxt <= h:
            STAT["catch: another function between owner and handler, left alone"] += 1
            NOTES.add((h, nxt, "catch handler of %#x not folded: chunk at %#x lies between" % (pf.start_ea, nxt)))
            continue
        hd = ida_bytes.get_item_head(h)
        if ida_bytes.is_data(flags(hd)) and not ida_bytes.is_align(flags(hd)) and not data_is_legit(hd):
            ida_bytes.del_items(hd, ida_bytes.DELIT_SIMPLE, ida_bytes.get_item_end(hd) - hd)
        if not ida_funcs.add_func(h):               # let IDA find the block's extent, then fold it in
            STAT["catch: block does not form code, left alone"] += 1
            NOTES.add((h, h, "catch handler not folded: add_func failed"))
            continue
        e, o = ida_funcs.get_func(h).end_ea, pf.start_ea
        ida_funcs.del_func(h)
        if (own and ida_funcs.set_func_end(o, e)) or ida_funcs.append_func_tail(ida_funcs.get_func(o), h, e):
            STAT["catch: blocks folded into owner"] += 1
            LOG.append((h, e, "catch block folded into %#x" % o))
        else:
            STAT["catch: fold failed"] += 1
    ida_auto.auto_wait()


def trim_int3():
    for s in list(idautils.Functions(T0, T1)):
        fn = ida_funcs.get_func(s)
        e = fn.end_ea
        while e - 1 > s and TEXT[e - 1 - T0] == 0xCC and is_code_head(e - 1):
            e -= 1
        n = fn.end_ea - e
        if n and ida_funcs.set_func_end(s, e):
            ida_bytes.del_items(e, ida_bytes.DELIT_SIMPLE, n)
            STAT["trailing int3 runs trimmed"] += 1


def pad_int3():
    """Unexplored int3 runs that end on a 16-byte boundary are inter-function padding: show them as align."""
    ea = T0
    while ea < T1:
        if ida_bytes.is_unknown(flags(ea)) and TEXT[ea - T0] == 0xCC:
            e = ea
            while e < T1 and TEXT[e - T0] == 0xCC and ida_bytes.is_unknown(flags(e)):
                e += 1
            if e & 15 == 0 and ida_bytes.create_align(ea, e - ea, 0):
                STAT["int3 padding runs marked align"] += 1
            ea = e
        else:
            ea = ea + 1 if ida_bytes.is_unknown(flags(ea)) else ida_bytes.get_item_end(ea)


# ---------------------------------------------------------------- main
af = ida_ida.inf_get_af()
ida_ida.inf_set_af(af & ~ida_ida.AF_FINAL)          # the final pass is what turned funclets into dd arrays
OUTSIDE = {f for f in idautils.Functions() if not in_text(f)}
n_before, cov_before = report("before")
iat0, iat1, bogus = name_original_iat()
R0, R1 = seg(".rdata")                              # .rdata now starts after the .iat segment
RDATA = ida_bytes.get_bytes(R0, R1 - R0)
IAT = [(iat0, iat1)] if iat1 else []
for i in range(ida_segment.get_segm_qty()):
    s = ida_segment.getnseg(i)
    if s.type == ida_segment.SEG_XTRN:
        IAT.append((s.start_ea, s.end_ea))
STUBS, EXT, REFS, CATCHES, FJMPS = scan_eh()
EH_STARTS = sorted(EXT)
say("C++ EH: %d handler stubs, %d funclet/stub extents (%d bytes), %d catch handlers" % (
    len(STUBS), len(EXT), sum(v[0] - a for a, v in EXT.items()), len(CATCHES)))
VT = scan_vftables()
PTRS = pointer_targets()
if bogus:
    wipe_polluted(bogus)
for rnd in range(1, 13):
    n = discovery_round(VT, IAT)
    say("round %d: %d functions created" % (rnd, n))
    if not n:
        break
extend_truncated()
PARENTS = eh_parents(STUBS)
rebuild_eh(STUBS, REFS, PARENTS)
attach_catches(CATCHES, STUBS, PARENTS)
for rnd in range(13, 17):                           # code freed by the EH pass
    n = discovery_round(VT, IAT)
    say("round %d: %d functions created" % (rnd, n))
    if not n:
        break
extend_truncated()
PARENTS = eh_parents(STUBS)                         # re-creating a function drops its tails:
rebuild_eh(STUBS, REFS, PARENTS)                    # attach again whatever the last steps detached
trim_int3()
pad_int3()
ida_auto.auto_wait()
state = collections.Counter()
for a in EH_STARTS:
    ci = chunk(a)
    state["tail chunk" if ci and ci.is_tail() else "stand-alone function" if func_start(a) == a else
          "inside a function body" if ci else "not in any function"] += 1
say("EH extents by final state: %s" % dict(state))
for f in [f for f in idautils.Functions() if not in_text(f) and f not in OUTSIDE]:
    ida_funcs.del_func(f)                           # auto-analysis followed a call out of .text: not ours
    STAT["functions auto-analysis made outside .text, removed again"] += 1
for k, v in sorted(STAT.items()):
    say("  %-70s %d" % (k, v))
n_after, cov_after = report("after")
say("functions in .text: %d -> %d" % (n_before, n_after))
ida_ida.inf_set_af(af)

if LOG:                                             # a second run changes nothing: keep the first run's record
    with open(out_path("logs", TARGET + "-01-discovery.tsv"), "w") as f:
        f.write("address\tend\treason\n")
        for a, b, why in LOG + sorted(NOTES):
            f.write("0x%08x\t0x%08x\t%s\n" % (a, b, why))
with open(out_path("exports", TARGET + "-functions.csv"), "w") as f:
    f.write("address,size,flags,name\n")
    for ea in idautils.Functions():
        fn = ida_funcs.get_func(ea)
        f.write("0x%08x,%d,0x%x,%s\n" % (ea, fn.size(), fn.flags, ida_name.get_name(ea)))
ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
idc.qexit(0)

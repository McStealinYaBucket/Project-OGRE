"""Per-function x86 ABI facts for checking proposed names against their signatures. READ-ONLY.

Run on a scratch copy (never a canonical .i64), headless:
  TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/abi_facts.py ADDRS.txt OUT.jsonl [nohr]" -L<log> COPY.i64
ADDRS.txt: one hex address per line. OUT.jsonl, one object per address:
  ea, start (containing function or null), argsize (IDA's purged bytes), rets (set of retn
  immediates in the entry chunk; [] = none), tail_chunk_rets (rets in tail chunks: EH
  funclets), noret,  tails [[target, its argsize]...] (tail jumps to
  functions not marked noreturn), ecx_in / edx_in (register read before any write on some
  path from the entry: own liveness over the flowchart), and unless "nohr": hr_type,
  hr_args [[name, size, loc]] from Hex-Rays.
"""
import json, sys
import ida_auto, ida_funcs, ida_gdl, ida_ua, ida_allins, idaapi, idc

ida_auto.auto_wait()
args = idc.ARGV[1:]
addrs = [int(l.split()[0], 16) for l in open(args[0]) if l.strip()]
out = open(args[1], "w")
use_hr = not (len(args) > 2 and args[2] == "nohr")
if use_hr:
    import ida_hexrays, ida_typeinf
    use_hr = ida_hexrays.init_hexrays_plugin()

ECX, EDX = 1, 2  # IDA x86 register numbers (eax=0, ecx=1, edx=2)
CALLS = (ida_allins.NN_call, ida_allins.NN_callfi, ida_allins.NN_callni)


def reg_use(insn, reg):
    """(reads, writes) of a 32/16/8-bit view of reg by one instruction (approximate)."""
    feat = insn.get_canon_feature()
    rd = wr = False
    lo8 = {ECX: 1, EDX: 2}[reg]  # cl=1, dl=2 share numbers in IDA's 8-bit view; good enough
    if insn.itype == ida_allins.NN_push and insn.ops[0].type == idaapi.o_reg:
        return False, False  # MSVC's `push ecx` reserves a stack slot; not an argument read
    for i in range(8):
        op = insn.ops[i]
        if op.type == idaapi.o_void:
            break
        if op.type == idaapi.o_reg and op.reg in (reg, lo8):
            use = feat & (idaapi.CF_USE1 << i)
            chg = feat & (idaapi.CF_CHG1 << i)
            # xor ecx,ecx / sub ecx,ecx is a pure write
            if insn.itype in (ida_allins.NN_xor, ida_allins.NN_sub) and i == 0 and \
                    insn.ops[1].type == idaapi.o_reg and insn.ops[1].reg == op.reg:
                return False, True
            if use:
                rd = True
            if chg and not use:
                wr = True
            elif chg:
                wr = wr  # read-modify-write: counts as a read (already set)
        elif op.type in (idaapi.o_phrase, idaapi.o_displ):
            # base or index register
            if op.reg == reg or (op.specflag1 and ((op.specflag2 >> 3) & 7) == reg):
                rd = True
    m = insn.get_canon_mnem()
    if insn.itype in (ida_allins.NN_rep_movs if hasattr(ida_allins, "NN_rep_movs") else -1,):
        rd = True
    if m and m.startswith(("rep", "loop", "jecxz")) and reg == ECX:
        rd = True
    if insn.itype in (ida_allins.NN_cdq,) and reg == EDX:
        wr = True
    if insn.itype in (ida_allins.NN_mul, ida_allins.NN_imul, ida_allins.NN_div, ida_allins.NN_idiv) and reg == EDX:
        if insn.itype in (ida_allins.NN_div, ida_allins.NN_idiv):
            rd = True
        elif insn.ops[1].type == idaapi.o_void:
            wr = True
    if insn.itype in CALLS:
        wr = True  # caller-saved: clobbered by the call
    return rd, wr


def live_in(f, reg):
    fc = ida_gdl.FlowChart(f, flags=ida_gdl.FC_PREDS)
    gen, kill, succ = {}, {}, {}
    for b in fc:
        g = k = False
        ea = b.start_ea
        while ea < b.end_ea:
            insn = ida_ua.insn_t()
            if not ida_ua.decode_insn(insn, ea):
                break
            r, w = reg_use(insn, reg)
            if r and not k:
                g = True
            if w:
                k = True
            ea += insn.size
        gen[b.id], kill[b.id] = g, k
        succ[b.id] = [s.id for s in b.succs()]
    live = {i: False for i in gen}
    changed = True
    while changed:
        changed = False
        for i in gen:
            v = gen[i] or (not kill[i] and any(live[s] for s in succ[i]))
            if v and not live[i]:
                live[i] = True
                changed = True
    entry = [b.id for b in fc if b.start_ea == f.start_ea]
    return bool(entry and live[entry[0]])


for ea in addrs:
    rec = {"ea": ea}
    f = ida_funcs.get_func(ea)
    if not f:
        rec["start"] = None
        out.write(json.dumps(rec) + "\n")
        continue
    rec["start"] = f.start_ea
    rec["argsize"] = f.argsize
    rets, tails, tail_rets = set(), set(), set()
    fii = ida_funcs.func_item_iterator_t()
    ok = fii.set(f)
    while ok:
        a = fii.current()
        insn = ida_ua.insn_t()
        if ida_ua.decode_insn(insn, a) and insn.itype in (ida_allins.NN_retn, ida_allins.NN_retf):
            v = insn.ops[0].value if insn.ops[0].type == idaapi.o_imm else 0
            # a ret in a tail chunk is usually an EH funclet's (catch blocks return with plain ret)
            (rets if f.contains(a) and f.start_ea <= a < f.end_ea else tail_rets).add(v)
        elif insn.itype == ida_allins.NN_jmp and insn.ops[0].type == idaapi.o_near:
            g = ida_funcs.get_func(insn.ops[0].addr)
            if g and g.start_ea == insn.ops[0].addr and g.start_ea != f.start_ea \
                    and not (g.flags & ida_funcs.FUNC_NORET):
                tails.add(g.start_ea)
        ok = fii.next_code()
    rec["rets"] = sorted(rets)
    rec["tail_chunk_rets"] = sorted(tail_rets)
    rec["noret"] = bool(f.flags & ida_funcs.FUNC_NORET)
    # tail jumps to other functions: the purge of a body without ret is its tail callee's
    rec["tails"] = [[t, ida_funcs.get_func(t).argsize] for t in sorted(tails)]
    try:
        rec["ecx_in"] = live_in(f, ECX)
        rec["edx_in"] = live_in(f, EDX)
    except Exception as e:
        rec["live_err"] = str(e)
    if use_hr:
        try:
            cf = ida_hexrays.decompile(f.start_ea)
            fi = ida_typeinf.func_type_data_t()
            if cf and cf.type.get_func_details(fi):
                rec["hr_type"] = str(cf.type)
                al = []
                for a in fi:
                    loc = a.argloc
                    if loc.is_reg1():
                        l = "reg%d" % loc.reg1()
                    elif loc.is_stkoff():
                        l = "stk%d" % loc.stkoff()
                    else:
                        l = "other"
                    al.append([a.name, a.type.get_size(), l])
                rec["hr_args"] = al
                rec["hr_ret"] = str(fi.rettype)
        except Exception as e:
            rec["hr_err"] = str(e)[:200]
    out.write(json.dumps(rec) + "\n")
    out.flush()

out.close()
idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

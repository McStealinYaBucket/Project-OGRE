"""Split adjustor-thunk targets that IDA buried inside another function into their own functions.

Input: ida/exports/<TARGET>-evidence/vtables.jsonl (slots with "thunk": {adjust, target}). A target
that is not a function start is split off only when the bytes are unambiguous:
  - the last instruction before it (skipping int3/nop padding) is ret or an unconditional jmp, so
    nothing falls through into it;
    (or a call to a noreturn function);
  - the target's own code (heads reachable from it by flow and jumps, not calls) contains no code of
    the owner's entry chunk before the target (shared code), and no jump from outside that code lands
    inside it except on the target itself (a jmp to the target is a tail call or the thunk).
Then: target at a tail-chunk start -> remove_func_tail + add_func; target inside a tail chunk ->
the chunk is cut at the target (part before stays a tail of the owner); target inside the owner's
main body -> the owner ends at the target, add_func(target). Code left with no function after that
(the owner's bytes beyond the new function's end) is given back to the owner as a tail chunk when
the owner jumps into it, else made a function of its own, and logged.
Afterwards an oracle name (_Z, not a _ZTh/_ZTv thunk name) left on what is now a <=16-byte adjustor
thunk whose target has an auto name moves, with its comment, to the target (the pairing had seen the
target's body inside the thunk's function). A rerun splits nothing new.
Every target goes to ida/logs/<TARGET>-thunk-split.tsv with its verdict; lines start with `split_thunks:`.

Run (snapshot first; saves packed):
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/split_thunk_targets.py" -Lida/logs/PLAYER-05-split.log ida/PLAYER.i64
Argument "nosave": classify and split, then discard (scratch copies / dry run).
"""
import json
import os
from collections import Counter

import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_name, idaapi, idautils, idc

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
STAT = Counter()
LOG = []


def say(*a):
    print("split_thunks: " + " ".join(str(x) for x in a))


def is_jump(xr):
    return xr.type in (idaapi.fl_JN, idaapi.fl_JF)


def prev_insn(t):
    ea = t
    while ida_bytes.get_byte(ea - 1) in (0xCC, 0x90):       # int3 / nop padding
        ea -= 1
    p = idc.prev_head(ea, 0)
    if p == idaapi.BADADDR or not ida_bytes.is_code(ida_bytes.get_flags(p)) or p + ida_bytes.get_item_size(p) != ea:
        return None, "not code"
    return p, idc.print_insn_mnem(p)


def noret_call(p):
    if idc.print_insn_mnem(p) != "call":
        return False
    g = ida_funcs.get_func(idc.get_operand_value(p, 0))
    return g is not None and bool(g.flags & ida_funcs.FUNC_NORET) and \
        not any(xr.type == idaapi.fl_F for xr in idautils.XrefsFrom(p, 0))


def body_of(t, limit=0x10000):
    """Heads reachable from t by ordinary flow and jumps (not calls): the target's own code."""
    seen, todo = set(), [t]
    while todo:
        h = todo.pop()
        if h in seen or not ida_bytes.is_code(ida_bytes.get_flags(h)) or abs(h - t) > limit:
            continue
        seen.add(h)
        todo.extend(xr.to for xr in idautils.XrefsFrom(h, 0) if xr.type in (idaapi.fl_F, idaapi.fl_JN, idaapi.fl_JF))
    return seen


def classify(t):
    f = ida_funcs.get_func(t)
    if f is None:
        return "nofunc", None, None, ""
    if f.start_ea == t:
        return "already", f, None, ""
    ch = ida_funcs.get_fchunk(t)
    p, mn = prev_insn(t)
    if p is None or not (mn in ("retn", "ret", "jmp") or noret_call(p)):
        return "falls-through", f, ch, "prev %s at %s" % (mn, hex(p) if p else "-")
    body = body_of(t)
    for h in body:
        if f.start_ea <= h < t or (ch.flags & ida_funcs.FUNC_TAIL and ida_funcs.func_contains(f, h)
                                   and ida_funcs.get_fchunk(h).start_ea == f.start_ea):
            return "branch-back", f, ch, "code reachable from the target at 0x%x is the owner's" % h
        if h == t:
            continue
        for xr in idautils.XrefsTo(h, 0):
            if xr.iscode and xr.frm not in body and (is_jump(xr) or xr.type == idaapi.fl_F):
                return "branch-in", f, ch, "jump from 0x%x to 0x%x" % (xr.frm, h)
    return "ok", f, ch, "prev %s 0x%x, %d heads" % (mn, p, len(body))


def split(t, f, ch):
    owner = f.start_ea
    old_end = ch.end_ea
    if ch.flags & ida_funcs.FUNC_TAIL:
        cstart = ch.start_ea
        it = ida_funcs.func_parent_iterator_t(ch)
        parents = []
        ok = it.first()
        while ok:
            parents.append(it.parent())
            ok = it.next()
        del it                                    # the iterator locks the chunk
        for p in parents:
            if not ida_funcs.remove_func_tail(ida_funcs.get_func(p), cstart):
                return "remove_func_tail failed (parent 0x%x)" % p
        if cstart != t:
            for p in parents:
                ida_funcs.append_func_tail(ida_funcs.get_func(p), cstart, t)
        if not ida_funcs.add_func(t):
            for p in parents:                     # undo: the chunk goes back to every parent
                ida_funcs.append_func_tail(ida_funcs.get_func(p), t, old_end)
            return "add_func failed (restored)"
        kind = "tail of %d parent(s)" % len(parents)
    else:
        if not ida_funcs.set_func_end(owner, t):
            return "set_func_end failed"
        if not ida_funcs.add_func(t):
            ida_funcs.set_func_end(owner, old_end)
            return "add_func failed (restored)"
        kind = "body"
    nf = ida_funcs.get_func(t)
    note = "%s split, new size %d" % (kind, nf.size())
    # bytes of the old chunk beyond the new function: give them back or make functions
    ea = t
    while ea < old_end:
        g = ida_funcs.get_func(ea)
        if g is not None:
            ea = ida_funcs.get_fchunk(ea).end_ea
            continue
        if not ida_bytes.is_code(ida_bytes.get_flags(ea)):
            ea = idc.next_head(ea, old_end) if idc.next_head(ea, old_end) != idaapi.BADADDR else old_end
            continue
        if ida_funcs.add_func(ea):
            STAT["leftover made a function"] += 1
            note += "; leftover 0x%x -> function" % ea
        else:
            end = ea
            while end < old_end and ida_funcs.get_func(end) is None and ida_bytes.is_code(ida_bytes.get_flags(end)):
                end = idc.next_head(end, old_end)
                if end == idaapi.BADADDR:
                    end = old_end
            ida_funcs.append_func_tail(ida_funcs.get_func(owner), ea, end)
            STAT["leftover given back to owner as tail"] += 1
            note += "; leftover 0x%x-0x%x -> tail of owner" % (ea, end)
            ea = end
    return note


targets, thunks = set(), set()
for line in open(os.path.join(DBDIR, "exports", TARGET + "-evidence", "vtables.jsonl")):
    v = json.loads(line)
    for g in v["groups"]:
        for s in g["slots"]:
            if s.get("thunk"):
                targets.add(s["thunk"]["target"])
                thunks.add((s["ea"], s["thunk"]["target"]))

for t in sorted(targets):
    verdict, f, ch, why = classify(t)
    if verdict == "already":
        continue
    where = "tail" if ch is not None and ch.flags & ida_funcs.FUNC_TAIL else "body"
    owner = "0x%08x" % f.start_ea if f else "-"
    if verdict == "ok":
        res = split(t, f, ch)
        verdict = "split" if "failed" not in res else "split failed"
        why = res
    STAT["%s (%s)" % (verdict, where)] += 1
    LOG.append(("0x%08x" % t, verdict, where, owner, why))

# An oracle name paired to an owner whose body was the target's (e.g. "caller of paired <dtor>" on a
# deleting destructor) sits on what is now an adjustor thunk: move it, with its comment, to the target.
for th, t in sorted(thunks):
    f, g = ida_funcs.get_func(th), ida_funcs.get_func(t)
    if not f or not g or f.start_ea != th or g.start_ea != t or f.size() > 16:
        continue
    n = ida_name.get_name(th)
    if not n.startswith("_Z") or n.startswith(("_ZTh", "_ZTv", "_ZTc")) or \
            not ida_bytes.has_dummy_name(ida_bytes.get_flags(t)):
        continue
    cmt = idc.get_func_cmt(th, 1) or ""
    ida_name.set_name(th, "", ida_name.SN_NOWARN)
    if ida_name.set_name(t, n, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
        idc.set_func_cmt(t, cmt, 1)
        idc.set_func_cmt(th, "", 1)
        STAT["oracle name moved from the adjustor thunk to its target"] += 1
        LOG.append(("0x%08x" % t, "name moved", "body", "0x%08x" % th, n))
    else:
        ida_name.set_name(th, n, ida_name.SN_NOWARN | ida_name.SN_NOCHECK)

with open(os.path.join(DBDIR, "logs", TARGET + "-thunk-split.tsv"), "w") as fh:
    fh.write("target\tverdict\twhere\towner\tdetail\n")
    for r in LOG:
        fh.write("\t".join(r) + "\n")
say("functions now", ida_funcs.get_func_qty())
for k, v in sorted(STAT.items()):
    say("%-45s %d" % (k, v))
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

"""Apply the VS2008 SP1 FLIRT signatures (toolchain/sig/vc9sp1-*.sig) to a Windows database.

Build the sigs first: python3 -P tools/ida/build_flirt.py. Order (first match wins on a function;
the order matters only where two sigs match one body, e.g. msvcprt's ??_G_Fac_node and atls's
??_GCWin32Heap, and the referenced-name check below settles those):
  1. vc9sp1-msvcrt    /MD static CRT code: startup, EH prologs, cookie, 64-bit helpers, RTC
  2. vc9sp1-msvcprt   static part of msvcprt.lib (locale facet registration)
  3. vc9sp1-comsuppw  _com_error / _bstr_t support (native wchar_t build)
  4. vc9sp1-atls      static ATL
FLIRT only renames functions that still have a dummy name (sub_...); it marks them FUNC_LIB.
After FLIRT (a rejected hit gets its old name back, loses FUNC_LIB and is logged "(rejected)"):
  - referenced names: FLIRT ignores a module's `^off name` references when it applies, so a
    scalar deleting destructor calling a different destructor matches. A hit is rejected when a
    direct call/jmp or import call at such an offset reaches a named function/import with another
    name (dummy names, unknown_libname and import thunks do not count against it).
  - duplicates: one name on several hits in one segment cannot be told apart.
  - then data: dummy-named globals that accepted hits reference at `^off` offsets get that name
    (___security_cookie, vftables, SEH tables), and functions still unnamed are tried again: a
    short module with a `REF` on a global (@__security_check_cookie@4) only matches after that.
Measured 2026-11-12 on scratch copies (probe mode; per-hit tables in
ida/logs/<TARGET>-flirt-scratch-measure.tsv): PLAYER 97 functions accepted (msvcrt 42 in .text +
15 in .vmp0, msvcprt 6, comsuppw 7, atls 27; 4 of them unknown_libname) and 26 globals named;
3 rejected (??_GCWin32Heap at 0x009ef047 matched as ??_G_Fac_node, and two unverifiable
??_G_Fac_node in .vmp0). STUDIO 26 accepted (atls 23, msvcrt 2, msvcprt 1) and 5 globals; IDA's
vc32rtf had already named its CRT (45 already-named functions probe as VC9 matches, all under the
same names); 1 rejected (0x00a7cf4b, the same CWin32Heap case). No hit had vftable or
assert-string evidence of being Roblox code.

Run on a canonical database after a snapshot (saves packed):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-NN-pre-flirt.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/apply_flirt.py" -Lida/logs/PLAYER-NN-flirt.log ida/PLAYER.i64
Script arguments (-S"tools/ida/apply_flirt.py nosave probe"):
  nosave  measure only (scratch copies): the database is not saved
  probe   also try every sig on each function that already has a non-dummy name and report
          where a sig would match (FLIRT skips those, so a clash would go unseen); implies nosave
Every hit goes to <db dir>/logs/<TARGET>-flirt.tsv (address, sig, old name, new name, evidence):
  vft=<class>  the function is a vftable slot of an RTTI class that is not CRT/ATL/STL/comsupp
  str=<text>   it references an assert/__FILE__ string (BuildAgent, .cpp, .h paths)
  seg=<name>   the hit is outside .text (PLAYER's .vmp0 holds a second, VMProtect-owned CRT copy)
vft/str mark a likely false positive. Log lines start with `apply_flirt:`.
"""
import os
import re
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_name, ida_nalt, ida_segment, idaapi, idautils, idc
import msvc_rtti as M

ida_auto.auto_wait()
ARGS = idc.ARGV[1:]
PROBE = "probe" in ARGS
SAVE = "nosave" not in ARGS and not PROBE
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SIGS = ["vc9sp1-msvcrt", "vc9sp1-msvcprt", "vc9sp1-comsuppw", "vc9sp1-atls"]
LIBCLASS = re.compile(r"\.\?A[VUTW](.*@ATL@@|.*@std@@|_com_error@@|type_info@@|bad_\w+@std@@|exception@@)$")
ASSERTSTR = re.compile(r"BuildAgent|\.(cpp|h|hpp|inl)\b", re.I)


def say(*a):
    msg = "apply_flirt: " + " ".join(str(x) for x in a)
    print(msg)
    sys.stdout.flush()


def names():
    return {ea: (ida_name.get_name(ea), ida_funcs.get_func(ea).flags) for ea in idautils.Functions()}


def evidence(ea, vft_owner):
    ev = []
    if ea in vft_owner:
        ev.append("vft=" + vft_owner[ea])
    seg = ida_segment.get_segm_name(ida_segment.getseg(ea))
    if seg != ".text":
        ev.append("seg=" + seg)
    f = ida_funcs.get_func(ea)
    for item in (idautils.FuncItems(ea) if f else []):
        for d in idautils.DataRefsFrom(item):
            if ida_bytes.is_strlit(ida_bytes.get_flags(d)):
                s = ida_bytes.get_strlit_contents(d, -1, ida_nalt.get_str_type(d)) or b""
                s = s.decode("latin-1")
                if ASSERTSTR.search(s):
                    ev.append("str=" + s[:60])
    return ev


R = M.scan(M.from_ida())
vft_owner = {}
for v in R.vfts.values():
    td = R.cols[v.col].td
    if not LIBCLASS.match(R.tds[td]):
        for s in v.slots:
            vft_owner.setdefault(s, R.tds[td])
say("target", TARGET, "functions", ida_funcs.get_func_qty(), "non-library vftable slot targets", len(vft_owner))
for n in range(ida_funcs.get_idasgn_qty()):
    say("already applied:", ida_funcs.get_idasgn_desc_with_matches(n))

# FLIRT does not check a module's referenced names (`^off name` in the .pat) when it applies, so a
# scalar deleting destructor that calls a different destructor still matches (PLAYER 0x009ef047,
# ??_GCWin32Heap, came out as ??_G_Fac_node). Reject a hit when a call/jump it makes at a
# referenced-name offset lands on a function or import whose name is a different one. Repeat until
# nothing changes, since a rejected name can be the evidence for another hit.
pats, allnames = {}, set()  # allnames: every public name, aliases (`:0016 __alloca_probe_8`) too
for sig in SIGS:
    for line in open(os.path.join(REPO, "toolchain", "sig", sig + ".pat"), errors="replace"):
        t = line.split()
        allnames.update(t[k + 1] for k in range(4, len(t) - 1) if t[k].startswith(":"))
        if len(t) > 5 and t[4] == ":0000":
            pats.setdefault(t[5], []).append([(int(t[k][1:], 16), t[k + 1]) for k in range(6, len(t) - 1)
                                              if t[k].startswith("^")])


before = names()
nondummy = {ea for ea in before if not ida_bytes.has_dummy_name(ida_bytes.get_flags(ea))}
say("functions with a non-dummy name before:", len(nondummy))
rows, per = [], Counter()
PATHS = {sig: os.path.join(REPO, "toolchain", "sig", sig + ".sig") for sig in SIGS}


def apply_round(first):
    """First round: plan every sig. Later rounds (after data names were added): FLIRT will not
    re-plan a sig, so try each sig on each function that still has a dummy name."""
    out = []
    for sig in SIGS:
        if not os.path.exists(PATHS[sig]):
            say("MISSING", PATHS[sig], "- run tools/ida/build_flirt.py")
            idc.qexit(1)
        cur = names()
        if first:
            ida_funcs.plan_to_apply_idasgn(PATHS[sig])
        else:
            for ea, (nm, fl) in cur.items():
                if ida_bytes.has_dummy_name(ida_bytes.get_flags(ea)) and ea not in rejected:
                    ida_funcs.apply_idasgn_to(PATHS[sig], ea, False)
        ida_auto.auto_wait()
        for ea, (nm, fl) in sorted(names().items()):
            old = cur.get(ea)
            if old is None or old[0] != nm or (fl & ida_funcs.FUNC_LIB and not old[1] & ida_funcs.FUNC_LIB):
                out.append((ea, sig, old[0] if old else "(new function)", nm, evidence(ea, vft_owner)))
                per[sig] += 1
    return out


def same(have, want):
    """IDA drops one leading underscore and the @N of a C import name (`malloc` for `_malloc`,
    `_CxxThrowException` for `__CxxThrowException@8`) and adds `_N` to duplicate names."""
    have = have.replace("__imp_", "", 1)
    want = want.replace("__imp_", "", 1)
    alts = {want, want[1:] if want.startswith("_") else want}
    alts |= {re.sub(r"@\d+$", "", w) for w in alts}
    return any(have == w or have == "j_" + w or re.fullmatch(re.escape(w) + r"_\d+", have) for w in alts)


def known(t):
    """A name that can contradict a reference: not a dummy, not unknown_libname, not an import thunk
    (IDA names `jmp ds:__imp_X` after X, so a wrapper the lib defines can show up under X)."""
    nm = ida_name.get_name(t)
    f = ida_funcs.get_func(t)
    return bool(nm) and not ida_bytes.has_dummy_name(ida_bytes.get_flags(t)) \
        and not nm.startswith("unknown_libname") and not (f and f.flags & ida_funcs.FUNC_THUNK)


def ref_problems(ea, nm):
    base = nm if nm in pats else re.sub(r"_\d+$", "", nm)
    best = None
    for refs in pats.get(base, [[]]):
        bad = []
        for off, want in refs:
            h = ida_bytes.get_item_head(ea + off)
            # direct call/jmp only: through a pointer IDA adds a code ref to the pointer's
            # initial value (`call ?__errorPfn` -> _com_raise_error), which is not the reference
            direct = idc.get_operand_type(h, 0) in (idc.o_near, idc.o_far)
            tgts = list(idautils.CodeRefsFrom(h, 0)) if direct else []
            tgts += [t for t in idautils.DataRefsFrom(h) if ida_segment.getseg(t) and
                     ida_segment.getseg(t).type == ida_segment.SEG_XTRN]  # call ds:[import]
            for t in tgts:
                f = ida_funcs.get_func(t)
                is_fn = f is not None and f.start_ea == t
                is_imp = ida_segment.getseg(t).type == ida_segment.SEG_XTRN
                if (is_fn or is_imp) and known(t) and not same(ida_name.get_name(t), want):
                    bad.append("ref+0x%x=%s want %s" % (off, ida_name.get_name(t), want))
        if not bad:
            return []
        best = bad if best is None or len(bad) < len(best) else best
    return best or []


rejected = set()


def check(hit_rows):
    while True:
        new = []
        for ea, sig, old, nm, ev in hit_rows:
            if ea in rejected:
                continue
            bad = ref_problems(ea, ida_name.get_name(ea))
            if bad:
                new.append((ea, sig, old, nm, bad))
        if not new:  # then: one name on several hits in one segment cannot be told apart (PLAYER
            # .vmp0, a second VMProtect-owned CRT copy, had two ??_G_Fac_node hits with unnamed callees)
            live = {}
            for ea, sig, old, nm, ev in hit_rows:
                if ea not in rejected and ida_funcs.get_func(ea).flags & ida_funcs.FUNC_LIB:
                    n = ida_name.get_name(ea)
                    if n.startswith("unknown_libname"):
                        continue
                    exact = n in allnames or n[1:] in allnames  # IDA may add a leading underscore
                    key = (n if exact else re.sub(r"_\d+$", "", n), ida_segment.getseg(ea).start_ea)
                    live.setdefault(key, []).append((ea, sig, old, nm))
            new = [(ea, sig, old, nm, ["same name on %d hits in this segment" % len(v)])
                   for v in live.values() if len(v) > 1
                   for ea, sig, old, nm in v]
            if not new:
                break
        for ea, sig, old, nm, bad in new:
            rejected.add(ea)
            f = ida_funcs.get_func(ea)
            f.flags &= ~ida_funcs.FUNC_LIB
            ida_funcs.update_func(f)
            if old != nm:  # a name that was there before FLIRT stays; otherwise back to sub_
                ida_name.set_name(ea, "" if old.startswith(("(", "sub_")) else old, ida_name.SN_NOWARN)
            rows.append((ea, sig + " (rejected)", nm, ida_name.get_name(ea), bad))
            say("rejected 0x%08x %s: %s" % (ea, nm, "; ".join(bad)))


def name_data(hit_rows):
    """FLIRT names no data, but a short module can carry `REF off name` on a global it reads
    (@__security_check_cookie@4 needs ___security_cookie). PLAYER has no load-config directory, so
    IDA never named the cookie and the check function went unmatched. Name each dummy-named global
    an accepted hit references at a `^off` offset (its own fixup word must hold the address), then
    a second round can match the modules that needed it."""
    done = []
    for ea, sig, old, nm, ev in hit_rows:
        if ea in rejected:
            continue
        n = ida_name.get_name(ea)
        lists = pats.get(n if n in pats else re.sub(r"_\d+$", "", n), [])
        if len(lists) != 1:
            continue
        for off, want in lists[0]:
            t = ida_bytes.get_dword(ea + off)
            seg = ida_segment.getseg(t)
            if want.startswith("$") or not seg or seg.type in (ida_segment.SEG_XTRN, ida_segment.SEG_CODE) \
                    or not ida_bytes.has_dummy_name(ida_bytes.get_flags(t)) \
                    or ida_name.get_name_ea(idaapi.BADADDR, want) != idaapi.BADADDR:
                continue
            if ida_name.set_name(t, want, ida_name.SN_NOCHECK | ida_name.SN_NOWARN):
                done.append((t, sig + " (data name)", "(data)", want, ["referenced by %s at 0x%08x" % (n, ea + off)]))
    return done


hits = apply_round(True)
rows += hits
check(hits)
for rnd in range(2, 5):
    data = name_data(hits)
    rows += data
    say("round %d: named %d globals from accepted hits" % (rnd, len(data)), [(hex(r[0]), r[3]) for r in data])
    if not data:
        break
    more = apply_round(False)
    say("round %d: %d more hits" % (rnd, len(more)), [(hex(r[0]), r[3]) for r in more])
    rows += more
    hits += more
    check(hits)
for n in range(ida_funcs.get_idasgn_qty()):
    say("sig:", ida_funcs.get_idasgn_desc_with_matches(n))
for sig in SIGS:
    say("hits %-16s %d" % (sig, per[sig]))
say("hits rejected (referenced-name check, duplicate names):", len(rejected))

if PROBE:  # would one of our sigs match a function FLIRT skipped because it is already named?
    # apply_idasgn_to wants the sig as loaded (full path); it returns 0 (LIBFUNC_FOUND) on a match
    # and does not rename. Control: it must return 1 on functions FLIRT just left unnamed.
    paths = {sig: os.path.join(REPO, "toolchain", "sig", sig + ".sig") for sig in SIGS}
    ctl = [ea for ea in idautils.Functions() if ida_bytes.has_dummy_name(ida_bytes.get_flags(ea))][::200]
    say("probe control: %d of %d unmatched functions probe as a match (want 0)" % (
        sum(1 for ea in ctl if any(ida_funcs.apply_idasgn_to(paths[s], ea, False) == 0 for s in SIGS)), len(ctl)))
    for ea in sorted(nondummy):
        nm = ida_name.get_name(ea)
        for sig in SIGS:
            if ida_funcs.apply_idasgn_to(paths[sig], ea, False) == 0:
                rows.append((ea, sig + " (probe)", nm, "(would match)", evidence(ea, vft_owner)))
                per[sig + " probe"] += 1
                break
    say("probe matches on already-named functions:", sum(v for k, v in per.items() if k.endswith("probe")))

fp = [r for r in rows if not r[1].endswith("(data name)") and any(not e.startswith("seg=") for e in r[4])]
say("flagged rows (rejected, or vftable/assert evidence):", len(fp))
for r in fp:
    say("  FP? 0x%08x %s %s -> %s %s" % (r[0], r[1], r[2], r[3], "; ".join(r[4])))
os.makedirs(os.path.join(os.path.dirname(IDB), "logs"), exist_ok=True)
tsv = os.path.join(os.path.dirname(IDB), "logs", TARGET + "-flirt.tsv")
with open(tsv, "w") as f:
    f.write("address\tsig\told_name\tnew_name\tevidence\n")
    for ea, sig, old, new, ev in rows:
        f.write("0x%08x\t%s\t%s\t%s\t%s\n" % (ea, sig, old, new, "; ".join(ev)))
say("wrote", tsv, len(rows), "rows; save" if SAVE else "; not saved")
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

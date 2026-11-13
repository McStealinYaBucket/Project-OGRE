"""Push the third-party library matches into a Windows database (canonical: snapshot first).

Input: ida/exports/<TARGET>-lib-resolved.csv, written by tools/lib/lib_consistency.py from the
library CSVs (tools/lib/{lua,raknet,g3d,boost}_match.py). Per address, by status:
  identical  name = the compiled MSVC decorated name; repeatable comment
             "match: <library> <version> identical | <demangled>" (+ "also: <folded names>").
  located    name likewise (the compiled counterpart is known; the bytes differ); comment
             "lib: <library> <version> located, not identical | <demangled>".
  span       comment only: "lib: <library> <version> span, no compiled counterpart | <note>".
  conflict, inside   nothing (see the resolved CSV's note column); match:/lib: lines from an earlier
             run are removed.
A name is set only when the function's current name is an IDA dummy/auto name (sub_, nullsub_,
unknown_libname_, IDA's jmp-thunk name j_<target>), an ios:/mac: oracle name, or a name an earlier run of
this script set (its match:/lib: line is there) that the row now corrects, and never a name
tools/ida/apply_db_updates.py set from a fact (a "p4 <tag>" line that is not a reccmp line). lib_consistency.py has already rejected every row whose
oracle name names Roblox code; an oracle name that names other library code at an identical row is
superseded by the byte match and kept as a "superseded <tag> <name>" comment line. FLIRT (FUNC_LIB) and
any other name are never overwritten. A row at an address with no IDA function (IDA missed it) gets one
(add_func); one IDA made a tail chunk of its caller is split off into its own function. Lines this script wrote before (match:/lib:/superseded) are replaced, so a rerun changes
nothing. Every action goes to ida/logs/<TARGET>-libnames.tsv; log lines start with `apply_lib_names:`.

Run (saves packed):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-04-pre-libs.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/apply_lib_names.py" -Lida/logs/PLAYER-04-libnames.log ida/PLAYER.i64
Argument "nosave": measure only (scratch copies).
"""
import csv
import os
import re
from collections import Counter

import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_name, idaapi, idc

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
EXP = os.path.join(DBDIR, "exports")
OURS = ("match: ", "lib: ", "superseded ")
AUTO = re.compile(r"^(?:j_)*(?:sub|nullsub|unknown_libname)_[0-9A-Fa-f]+$|^j_")   # j_<x>: IDA's thunk name
STAT = Counter()
LOG = []


def say(*a):
    print("apply_lib_names: " + " ".join(str(x) for x in a))


def cut(line, n=900):
    return line if len(line) <= n else line[:n - 3] + "..."


def oracle_line(ea):
    for l in (idc.get_func_cmt(ea, 1) or "").split("\n"):
        if l.startswith(("ios: ", "mac: ")):
            return l
    return None


def set_comment(ea, lines):
    old = idc.get_func_cmt(ea, 1) or ""
    keep = [l for l in old.split("\n") if l and not l.startswith(OURS)]
    new = "\n".join([cut(l) for l in lines] + keep)
    if new != old:
        idc.set_func_cmt(ea, new, 1)
        STAT["comment set"] += 1


for r in csv.DictReader(open(os.path.join(EXP, TARGET + "-lib-resolved.csv"))):
    ea = int(r["address"], 16)
    st = r["status"]
    if st in ("conflict", "inside"):
        STAT["skipped " + st] += 1
        f = ida_funcs.get_func(ea)
        if f is not None and f.start_ea == ea:
            set_comment(ea, [])     # drop lines an earlier run wrote before the row became a conflict
        continue
    f = ida_funcs.get_func(ea)
    if f is None:
        if not ida_funcs.add_func(ea):
            STAT["add_func failed"] += 1
            LOG.append((r["address"], "add_func failed", ""))
            continue
        f = ida_funcs.get_func(ea)
        STAT["function created"] += 1
        LOG.append((r["address"], "function created", "size %d" % f.size()))
    ch = ida_funcs.get_fchunk(ea)
    if f.start_ea != ea and ch and ch.start_ea == ea and ch.flags & ida_funcs.FUNC_TAIL:
        # IDA attached a separately compiled function to its only caller as a tail chunk.
        owner = f.start_ea
        if ida_funcs.remove_func_tail(f, ea) and ida_funcs.add_func(ea):
            f = ida_funcs.get_func(ea)
            STAT["tail chunk split into a function"] += 1
            LOG.append((r["address"], "tail chunk of 0x%08x split into a function" % owner, "size %d" % f.size()))
    if f.start_ea != ea:
        STAT["not a function start (skipped)"] += 1
        LOG.append((r["address"], "not a function start", "in 0x%08x" % f.start_ea))
        continue
    lib = "%s %s" % (r["library"], r["version"])
    others = [n for n in r["names"].split("|") if n and n != r["name"]]
    if st == "span":
        set_comment(ea, ["lib: %s span, no compiled counterpart | %s" % (lib, r["note"] or "inside the library block")])
        STAT["span comment"] += 1
        continue
    head = "match: %s identical" % lib if st == "identical" else "lib: %s located, not identical" % lib
    lines = [head + " | " + r["demangled"]]
    if others:
        lines.append("match: also (folded): " + " ".join(others[:10]) + (" ..." if len(others) > 10 else ""))
    cur = ida_name.get_name(ea)
    orc = oracle_line(ea)
    auto = bool(AUTO.match(cur)) or ida_bytes.has_dummy_name(ida_bytes.get_flags(ea))
    oracle_named = orc is not None and (cur.startswith("_Z") or not auto)
    # a name an earlier run of this script set from a row the matchers have since corrected (g3d-2013.md:
    # Color3::one -> gray, Vector3::ignore -> unitZ, rayAABox -> collisionLocationForMovingPointFixedAABox)
    stale = any(l.startswith(("match: ", "lib: ")) for l in (idc.get_func_cmt(ea, 1) or "").split("\n"))
    # a name tools/ida/apply_db_updates.py set from a hand-checked fact ("p4 <tag> ...", not a reccmp line)
    # stays: e.g. 0x0066ee10 EventSource::raiseEventInvocation (rtti:), ICF-folded with a RakNet `ret 0Ch` body
    fact = any(l.startswith("p4 ") and not l.startswith("p4 match: reccmp")
               for l in (idc.get_func_cmt(ea, 1) or "").split("\n"))
    action = "kept"
    if cur == r["name"]:
        action = "already"
        lines += [l for l in (idc.get_func_cmt(ea, 1) or "").split("\n") if l.startswith("superseded ")]
    elif f.flags & ida_funcs.FUNC_LIB:
        action = "kept (FUNC_LIB)"
    elif fact and not auto:
        action = "kept (p4 fact)"
    elif auto or oracle_named or stale:
        if oracle_named and "oracle disagrees" in r["note"]:
            lines.append("superseded " + orc)
        if ida_name.set_name(ea, r["name"], ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
            action = "renamed from oracle" if oracle_named else "renamed (stale library name)" \
                if stale and not auto else "renamed"
        else:
            action = "set_name failed (name in use?)"
    STAT[st + ": " + action] += 1
    LOG.append((r["address"], st + ": " + action, cur + " -> " + r["name"]))
    set_comment(ea, lines)

with open(os.path.join(DBDIR, "logs", TARGET + "-libnames.tsv"), "w") as fh:
    fh.write("address\taction\tdetail\n")
    for row in LOG:
        fh.write("\t".join(row) + "\n")
for k, v in sorted(STAT.items()):
    say("%-50s %d" % (k, v))
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

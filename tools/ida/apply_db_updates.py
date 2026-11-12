"""Apply ida/exports/<TARGET>-db-updates.csv (tools/ida/match_names.py) to a Windows database (canonical:
snapshot first).

Per row (address,action,name,tag,line):
  rename      set `name` (an existing different name is replaced: the planner decided); rename-lib also
              clears FUNC_LIB (a FLIRT false positive)
  unname      give the function back its dummy name
  comment     name untouched
Every row writes `line` into the repeatable function comment; lines starting "p4 " are this script's and
are replaced on a rerun, so a second run changes nothing. A fact row (not a "p4 match: reccmp" line) that
renames or unnames also drops the apply_lib_names.py lines ("match: ", "lib: ") it contradicts.
A name already used at another address is not set (logged). Every action goes to
ida/logs/<TARGET>-db-updates.tsv; log lines start with `apply_db_updates:`.

Run (saves packed):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-06-phase4-sync.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/apply_db_updates.py" -Lida/logs/PLAYER-06-db-updates.log ida/PLAYER.i64
Argument "nosave": measure only (scratch copies).
"""
import csv
import os
from collections import Counter

import ida_auto, ida_funcs, ida_loader, ida_name, idaapi, idc

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
STAT, LOG = Counter(), []


def say(*a):
    print("apply_db_updates: " + " ".join(str(x) for x in a))


for r in csv.DictReader(open(os.path.join(DBDIR, "exports", TARGET + "-db-updates.csv"))):
    ea, act, want, line = int(r["address"], 16), r["action"], r["name"], r["line"]
    f = ida_funcs.get_func(ea)
    if f is None or f.start_ea != ea:
        STAT["skipped: not a function start"] += 1
        LOG.append((ea, "skip", "not a function start", ""))
        continue
    cur = ida_name.get_name(ea)
    fact = not line.startswith("p4 match: reccmp")
    old = idc.get_func_cmt(ea, 1) or ""
    keep = [l for l in old.split("\n") if l and not l.startswith("p4 ")]
    if fact and act in ("rename", "unname"):
        keep = [l for l in keep if not l.startswith(("match: ", "lib: "))]
    new = "\n".join([line[:900]] + keep)
    if new != old:
        idc.set_func_cmt(ea, new, 1)
        STAT["comment written"] += 1
    if act == "comment":
        STAT["comment only (%s)" % r["tag"]] += 1
        continue
    if act == "unname":
        if ida_name.set_name(ea, "", ida_name.SN_NOWARN):
            STAT["unnamed"] += 1
            LOG.append((ea, "unname", ida_name.get_name(ea), cur))
        continue
    if cur == want:
        STAT["name already set"] += 1
        continue
    other = ida_name.get_name_ea(idaapi.BADADDR, want)
    if other != idaapi.BADADDR and other != ea:
        STAT["skipped: name used at another address"] += 1
        LOG.append((ea, "skip", "name in use at 0x%08x: %s" % (other, want), cur))
        continue
    if act == "rename-lib" and f.flags & ida_funcs.FUNC_LIB:
        f.flags &= ~ida_funcs.FUNC_LIB
        ida_funcs.update_func(f)
        STAT["FUNC_LIB cleared"] += 1
    if ida_name.set_name(ea, want, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
        STAT["renamed (%s)" % r["tag"]] += 1
        LOG.append((ea, act, want, cur))
    else:
        STAT["set_name failed"] += 1
        LOG.append((ea, "fail", want, cur))

with open(os.path.join(DBDIR, "logs", TARGET + "-db-updates.tsv"), "w") as fh:
    fh.write("address\taction\tname\told_name\n")
    for ea, act, det, cur in LOG:
        fh.write("0x%08x\t%s\t%s\t%s\n" % (ea, act, det, cur))
for k, v in sorted(STAT.items()):
    say("%-45s %d" % (k, v))
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

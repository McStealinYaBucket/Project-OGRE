"""Apply ida/exports/<TARGET>-db-updates.csv (tools/ida/match_names.py) to a Windows database (canonical:
snapshot first).

Per row (address,action,name,tag,line):
  rename      set `name` (an existing different name is replaced: the planner decided); rename-lib also
              clears FUNC_LIB (a FLIRT false positive); so does any match: rename (a dummy FLIRT flagged)
  unname      give the function back its dummy name
  comment     name untouched
  data1/data4/data8 a global: define a byte/dword/qword at the address, set `name`, `line` as its repeatable comment
Every row writes `line` into the repeatable function comment; lines starting "p4 " are this script's and
are replaced on a rerun, so a second run changes nothing. A fact row (not a "p4 match: reccmp" line) that
renames or unnames also drops the apply_lib_names.py lines ("match: ", "lib: ") it contradicts.
A name already used at another address is not set (logged), unless that address is planned to take
another name itself (a swap: its name is freed first). Every action goes to
ida/logs/<TARGET>-db-updates.tsv; log lines start with `apply_db_updates:`.

Run (saves packed):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-06-phase4-sync.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/apply_db_updates.py" -Lida/logs/PLAYER-06-db-updates.log ida/PLAYER.i64
Argument "nosave": measure only (scratch copies).
"""
import csv
import os
from collections import Counter

import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_name, idaapi, idc

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
STAT, LOG = Counter(), []


def say(*a):
    print("apply_db_updates: " + " ".join(str(x) for x in a))


ROWS = list(csv.DictReader(open(os.path.join(DBDIR, "exports", TARGET + "-db-updates.csv"))))
PLANNED = {int(r["address"], 16): r["name"] for r in ROWS if r["action"].startswith("rename")}

for r in ROWS:
    ea, act, want, line = int(r["address"], 16), r["action"], r["name"], r["line"]
    if act in ("data1", "data4", "data8"):
        # a global: define it as a byte/dword/qword (when it is not data of that size yet), name it, and keep
        # the evidence as its repeatable comment (non-"p4 " lines kept)
        n = int(act[4:])
        if not ida_bytes.is_data(ida_bytes.get_flags(ea)) or ida_bytes.get_item_size(ea) != n:
            ida_bytes.del_items(ea, ida_bytes.DELIT_SIMPLE, n)
            {1: ida_bytes.create_byte, 4: ida_bytes.create_dword, 8: ida_bytes.create_qword}[n](ea, n)
        old = idc.get_cmt(ea, 1) or ""
        new = "\n".join([line[:900]] + [l for l in old.split("\n") if l and not l.startswith("p4 ")])
        if new != old:
            idc.set_cmt(ea, new, 1)
        cur = ida_name.get_name(ea)
        if cur == want:
            STAT["data name already set"] += 1
        elif ida_name.set_name(ea, want, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
            STAT["data renamed (%s)" % r["tag"]] += 1
            LOG.append((ea, act, want, cur))
        else:
            STAT["data set_name failed"] += 1
            LOG.append((ea, "fail", want, cur))
        continue
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
    # rename-lib, or a reccmp-proven name on a dummy that FLIRT flagged as library code (no FLIRT name; the
    # planner never plans library-owned addresses): the flag would make export_names.py call it flirt:
    if (act == "rename-lib" or r["tag"] == "match:") and f.flags & ida_funcs.FUNC_LIB:
        f.flags &= ~ida_funcs.FUNC_LIB
        ida_funcs.update_func(f)
        STAT["FUNC_LIB cleared"] += 1
    if cur == want:
        STAT["name already set"] += 1
        continue
    other = ida_name.get_name_ea(idaapi.BADADDR, want)
    if other != idaapi.BADADDR and other != ea and PLANNED.get(other, want) != want:
        # a swap: the holder is planned to take another name (its own row renames it later), so free it now
        ida_name.set_name(other, "", ida_name.SN_NOWARN)
        LOG.append((other, "freed", "for 0x%08x" % ea, want))
        other = idaapi.BADADDR
    if other != idaapi.BADADDR and other != ea:
        STAT["skipped: name used at another address"] += 1
        LOG.append((ea, "skip", "name in use at 0x%08x: %s" % (other, want), cur))
        continue
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

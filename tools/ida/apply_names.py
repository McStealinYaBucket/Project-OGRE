"""Push the oracle names from tools/pair_names.py into a Windows database (canonical: snapshot first).

Inputs (repo layout): ida/exports/<TARGET>-names-proposed.csv and, if present,
ida/exports/<TARGET>-app-decomp-map.csv. For each proposed address:
  - name: the first non-guess row's Itanium-mangled oracle name (pair_names.py puts the
    most-voted name first at a folded address), set only when the function still has an IDA
    dummy/auto name (sub_..., nullsub_..., j_<anything> IDA named itself). FLIRT/library functions (FUNC_LIB) and any
    other existing name are never overwritten; those keep their name and get the comment only.
  - repeatable function comment, one line per row: "<source_tag> <demangled> | <evidence>"
    (guess: rows only ever reach the comment; a line is cut at 900 characters); at most 25 rows, then a count of the rest. Rows
    from the App-decomp map add "prior: in 2013 App decomp: <files>". Lines this script wrote
    before (starting ios:/mac:/guess:/prior:/"... ") are replaced, other comment text is kept, so
    a second run changes nothing.
Every action goes to <db dir>/logs/<TARGET>-names.tsv; log lines start with `apply_names:`.

Run (saves packed):
    cp ida/PLAYER.i64 ida/snapshots/PLAYER-NN-pre-names.i64
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/apply_names.py" -Lida/logs/PLAYER-NN-names.log ida/PLAYER.i64
Argument "nosave": measure only (scratch copies).
"""
import csv
import os
import re
from collections import Counter, defaultdict

import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_name, idaapi, idc

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
EXP = os.path.join(DBDIR, "exports")
MAXROWS = 25
OURS = ("ios: ", "mac: ", "guess: ", "prior: in 2013 App decomp: ", "... ")   # other prior: lines: apply_prior_names.py
AUTO = re.compile(r"^(?:j_)*(?:sub|nullsub)_[0-9A-Fa-f]+$")
STAT = Counter()


def cut(line, n=900):
    """IDA truncates very long comment lines on its own; cut them here so a rerun compares equal."""
    return line if len(line) <= n else line[:n - 3] + "..."


def say(*a):
    print("apply_names: " + " ".join(str(x) for x in a))


rows = defaultdict(list)
for r in csv.DictReader(open(os.path.join(EXP, TARGET + "-names-proposed.csv"))):
    rows[int(r["address"], 16)].append(r)
prior = defaultdict(set)
amap = os.path.join(EXP, TARGET + "-app-decomp-map.csv")
if os.path.exists(amap):
    for r in csv.DictReader(open(amap)):
        if r["player_address"]:
            prior[int(r["player_address"], 16)].update(x for x in r["files"].split(";") if x)

log = []
for ea in sorted(set(rows) | set(prior)):
    f = ida_funcs.get_func(ea)
    if not f or f.start_ea != ea:
        STAT["skipped: not a function start"] += 1
        log.append((ea, "skip", "not a function start", ""))
        continue
    rs = rows.get(ea, [])
    lines = [cut("%s %s | %s" % (r["source_tag"], r["demangled"], r["evidence"])) for r in rs[:MAXROWS]]
    if len(rs) > MAXROWS:
        lines.append("... and %d more names (folded); see ida/exports/%s-names-proposed.csv" % (len(rs) - MAXROWS, TARGET))
    if ea in prior:
        lines.append(cut("prior: in 2013 App decomp: " + ", ".join(sorted(prior[ea]))))
    old = idc.get_func_cmt(ea, 1) or ""
    keep = [l for l in old.split("\n") if l and not l.startswith(OURS)]
    new = "\n".join(keep + lines)
    if new != old:
        idc.set_func_cmt(ea, new, 1)
        STAT["comments written"] += 1
    strong = [r for r in rs if r["source_tag"] != "guess:"]
    cur = ida_name.get_name(ea)
    if not strong:
        STAT["comment only: guess or prior rows"] += 1
        log.append((ea, "comment", "no strong row", cur))
        continue
    want = strong[0]["mangled"]
    if cur == want:
        STAT["name already set"] += 1
        continue
    flags = ida_bytes.get_flags(ea)
    # IDA renames a jmp thunk `j_<target>` on its own when the target is renamed: still auto
    tgt = ida_funcs.calc_thunk_func_target(f)[0] if cur.startswith("j_") else idaapi.BADADDR
    auto = ida_bytes.has_dummy_name(flags) or AUTO.match(cur) or \
        (tgt != idaapi.BADADDR and cur == "j_" + ida_name.get_name(tgt))
    if (f.flags & ida_funcs.FUNC_LIB) or not auto:
        STAT["kept existing name (library or non-auto)"] += 1
        log.append((ea, "kept", "existing name %s; proposed %s" % (cur, want), cur))
        continue
    if ida_name.get_name_ea(idaapi.BADADDR, want) != idaapi.BADADDR:
        STAT["skipped: name already used elsewhere"] += 1
        log.append((ea, "skip", "name in use at 0x%x: %s" % (ida_name.get_name_ea(idaapi.BADADDR, want), want), cur))
        continue
    if ida_name.set_name(ea, want, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
        STAT["renamed (%s)" % strong[0]["source_tag"]] += 1
        log.append((ea, "name", want, cur))
    else:
        STAT["set_name failed"] += 1
        log.append((ea, "fail", want, cur))

d = os.path.join(DBDIR, "logs")
with open(os.path.join(d if os.path.isdir(d) else DBDIR, TARGET + "-names.tsv"), "w") as fh:
    fh.write("address\taction\tdetail\told_name\n")
    for ea, act, det, cur in log:
        fh.write("0x%08x\t%s\t%s\t%s\n" % (ea, act, det, cur))
for k, v in sorted(STAT.items()):
    say("  %-50s %d" % (k, v))
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

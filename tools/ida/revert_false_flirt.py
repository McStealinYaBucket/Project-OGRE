"""Undo IDA's first-pass vc32rtf FLIRT names that the evidence contradicts (STUDIO; harmless elsewhere).

A function flagged FUNC_LIB whose address has a strong oracle row in ida/exports/<T>-names-proposed.csv
(not guess:) or an identical/located row in ida/exports/<T>-lib-resolved.csv naming something else
loses the library flag and its name (back to sub_). Then rerun tools/ida/apply_names.py and
tools/ida/apply_lib_names.py, which name it by their own rules. Evidence recorded per function:
  - the oracle row's class and evidence (unique string, body identical to a PLAYER function, vftable slot)
    or the library body match;
  - rtti: for a constructor/destructor name, whether the body references that class's vftable;
  - flirt: how many FLIRT functions carry the same name (less IDA's _N suffix); one library function
    under 70 different addresses is a signature matching a common shape, not that function.
A repeatable comment line "revert: <old FLIRT name> removed | ..." records it. Log: ida/logs/<T>-flirt-revert.tsv.

Run (snapshot first; saves packed):
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/revert_false_flirt.py" -Lida/logs/STUDIO-05-flirt-revert.log ida/STUDIO.i64
Argument "nosave": measure only.
"""
import csv
import os
import re
from collections import Counter

import ida_auto, ida_funcs, ida_loader, ida_name, idaapi, idautils, idc

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
EXP = os.path.join(DBDIR, "exports")
STAT = Counter()
LOG = []

want = {}
for r in csv.DictReader(open(os.path.join(EXP, TARGET + "-names-proposed.csv"))):
    if r["source_tag"] != "guess:":
        want.setdefault(int(r["address"], 16), (r["mangled"], r["demangled"], "%s %s: %s" % (
            r["source_tag"], r["evidence_class"], r["evidence"][:120])))
for r in csv.DictReader(open(os.path.join(EXP, TARGET + "-lib-resolved.csv"))):
    if r["status"] in ("identical", "located"):
        want[int(r["address"], 16)] = (r["name"], r["demangled"], "lib %s %s %s" % (r["library"], r["version"], r["status"]))


def rtti_check(ea, dem):
    m = re.match(r"(?:.*\s)?((?:\w+::)*)(~?)(\w+)\(", dem)
    if not m or not m.group(1) or m.group(1).rstrip(":").split("::")[-1] != m.group(3):
        return "n/a"
    cls = m.group(1)[:-2]
    for h in idautils.FuncItems(ea):
        for x in idautils.DataRefsFrom(h):
            d = ida_name.demangle_name(ida_name.get_name(x), 0) or ""
            if d.startswith("const " + cls + "::`vftable'") or ("const %s::`vftable'" % cls) in d:
                return "vftable 0x%08x referenced" % x
    return "no vftable reference"


BASE_COUNT = Counter(re.sub(r"_\d+$", "", ida_name.get_name(ea)) for ea in idautils.Functions()
                     if ida_funcs.get_func(ea).flags & ida_funcs.FUNC_LIB)


def same_name_check(name):
    base = re.sub(r"_\d+$", "", name)
    if base.startswith(("sub_", "unknown_libname")):
        return "FLIRT gave no name"
    return "%d FLIRT functions carry %s" % (BASE_COUNT[base], base)


for ea in idautils.Functions():
    f = ida_funcs.get_func(ea)
    if not (f.flags & ida_funcs.FUNC_LIB) or ea not in want:
        continue
    cur = ida_name.get_name(ea)
    name, dem, ev = want[ea]
    if cur == name:
        continue
    rt, dl = rtti_check(ea, dem), same_name_check(cur)
    f.flags &= ~ida_funcs.FUNC_LIB
    ida_funcs.update_func(f)
    ida_name.set_name(ea, "", ida_name.SN_NOWARN)
    line = "revert: FLIRT name %s removed | evidence %s -> %s | rtti: %s | flirt: %s" % (cur, ev, dem, rt, dl)
    old = idc.get_func_cmt(ea, 1) or ""
    keep = [l for l in old.split("\n") if l and not l.startswith("revert: ")]
    idc.set_func_cmt(ea, "\n".join(keep + [line[:900]]), 1)
    kind = "destructor" if "::~" in dem else "other"
    STAT["reverted (%s, %s)" % (kind, "named FLIRT" if not cur.startswith(("sub_", "unknown_libname_")) else "dummy FLIRT")] += 1
    STAT["rtti " + rt.split(" 0x")[0]] += 1
    LOG.append(("0x%08x" % ea, cur, ida_name.get_name(ea), dem, ev, rt, dl))

with open(os.path.join(DBDIR, "logs", TARGET + "-flirt-revert.tsv"), "w") as fh:
    fh.write("address\told_name\tnew_name\tevidence_name\tevidence\trtti\tflirt_name_count\n")
    for r in LOG:
        fh.write("\t".join(r) + "\n")
for k, v in sorted(STAT.items()):
    print("revert_false_flirt: %-45s %d" % (k, v))
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
    print("revert_false_flirt: saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

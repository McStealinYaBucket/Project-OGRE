"""Export a database's names and function list as text (READ-ONLY; CLAUDE.md "The i64 rule").

Writes (repo layout, next to the database otherwise):
  ida/exports/<TARGET>-names.csv      address, name, demangled, kind, size, source
      every function with a non-dummy name and every non-auto data name. source is where the name
      came from: ios:/mac: (oracle transfer, tools/ida/apply_names.py; the function's repeatable
      comment starts with that tag), flirt: (library function, FUNC_LIB), rtti: (the item's
      comment starts "rtti: ", tools/ida/name_rtti.py), match:/lib: (third-party library body,
      identical / located, tools/ida/apply_lib_names.py), ida: (anything else IDA or the loader
      named: import slots, PE exports, its own thunk names j_<target>), prior: (an auto name replaced
      by the prior App decomp's name, tools/ida/apply_prior_names.py; not verified). A "p4 <tag> ..." comment
      line (tools/ida/apply_db_updates.py) gives the source first: match: for a reccmp 100% name, else its tag.
      demangled: from <TARGET>-names-proposed.csv for oracle names, else IDA's demangler.
  ida/exports/<TARGET>-functions.csv  address, size, flags, name (same format as before)
  ida/exports/<TARGET>-types.h        every local type (print_decls, as File > Produce file > Create C header)
Run on the canonical database after a session (never saves) or on a copy:
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/export_names.py" -Lida/logs/PLAYER-export-names.log ida/PLAYER.i64
"""
import csv
import os
import re
from collections import Counter

import ida_auto, ida_bytes, ida_funcs, ida_name, idaapi, idautils, idc

ida_auto.auto_wait()
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
EXP = os.path.join(DBDIR, "exports") if os.path.isdir(os.path.join(DBDIR, "exports")) else DBDIR
DUMMY = re.compile(r"^(?:j_)*(?:sub|nullsub|unknown_libname)_[0-9A-Fa-f]+$")
TAGS = ("ios:", "mac:", "match:", "prior:", "guess:")

dem = {}
prop = os.path.join(EXP, TARGET + "-names-proposed.csv")
if os.path.exists(prop):
    for r in csv.DictReader(open(prop)):
        dem.setdefault(r["mangled"], r["demangled"])


def demangled(n):
    if n in dem:
        return dem[n]
    d = ida_name.demangle_name(n, 0)
    return d or ""


def source(ea, n, func):
    if func:
        f = ida_funcs.get_func(ea)
        if f.flags & ida_funcs.FUNC_LIB:
            return "flirt:"
        c = idc.get_func_cmt(ea, 1) or ""
        for l in c.split("\n"):
            # tools/ida/apply_db_updates.py: "p4 <tag> ..." decides when that script set the current name
            # (a reccmp line only when the name is the one it records; a fact line whenever it set a name)
            if l.startswith("p4 ") and not DUMMY.match(n):
                tag = l.split(" ")[1]
                if l.startswith("p4 match: reccmp "):
                    if (" vs our %s " % n) in l:
                        return "match:"
                elif tag != "guess:":
                    return tag
        for l in c.split("\n"):
            if l.startswith("prior: ") and " | named from the 2013 App decomp" in l:
                return "prior:"     # named from the prior App decomp (tools/ida/apply_prior_names.py)
        if n.startswith("_Z") or n in dem:
            for l in c.split("\n"):
                if l.startswith(("ios: ", "mac: ")):
                    return l.split(" ", 1)[0]
        for l in c.split("\n"):
            if l.startswith("match: ") and " identical" in l:
                return "match:"     # third-party library body, byte-identical (tools/ida/apply_lib_names.py)
            if l.startswith("lib: ") and " located" in l:
                return "lib:"       # third-party library, located, not identical
        return "ida:"
    c = (idc.get_cmt(ea, 1) or idc.get_cmt(ea, 0) or "")
    return "rtti:" if c.startswith("rtti: ") else "ida:"


rows, fn_rows, stat = [], [], Counter()
for ea in idautils.Functions():
    f = ida_funcs.get_func(ea)
    n = ida_name.get_name(ea)
    fn_rows.append((ea, f.size(), f.flags, n))
    if DUMMY.match(n) or ida_bytes.has_dummy_name(ida_bytes.get_flags(ea)):
        stat["function, dummy name"] += 1
        continue
    s = source(ea, n, True)
    stat["function " + s] += 1
    rows.append(("0x%08x" % ea, n, demangled(n), "func", f.size(), s))
for ea, n in idautils.Names():
    fl = ida_bytes.get_flags(ea)
    if ida_funcs.get_func(ea) and ida_funcs.get_func(ea).start_ea == ea:
        continue
    if not ida_bytes.is_data(fl) and not ida_bytes.is_unknown(fl):
        continue  # labels inside code
    s = source(ea, n, False)
    if s != "rtti:" and (ida_bytes.has_auto_name(fl) or ida_bytes.has_dummy_name(fl)):
        continue
    stat["data " + s] += 1
    rows.append(("0x%08x" % ea, n, demangled(n), "data", ida_bytes.get_item_size(ea), s))
rows.sort()
with open(os.path.join(EXP, TARGET + "-names.csv"), "w", newline="") as fh:
    w = csv.writer(fh)
    w.writerow(["address", "name", "demangled", "kind", "size", "source"])
    w.writerows(rows)
with open(os.path.join(EXP, TARGET + "-functions.csv"), "w") as fh:
    fh.write("address,size,flags,name\n")
    for ea, sz, fl, n in fn_rows:
        fh.write("0x%08x,%d,0x%x,%s\n" % (ea, sz, fl, n))
# every local type, as File > Produce file > Create C header file writes them
import ida_typeinf  # noqa: E402


class _Sink(ida_typeinf.text_sink_t):
    def __init__(self, fh):
        ida_typeinf.text_sink_t.__init__(self)
        self.fh = fh

    def _print(self, text):
        self.fh.write(text)
        return 0


with open(os.path.join(EXP, TARGET + "-types.h"), "w") as fh:
    ords = list(range(1, ida_typeinf.get_ordinal_limit(None)))
    got = ida_typeinf.print_decls(_Sink(fh), None, ords, ida_typeinf.PDF_DEF_FWD | ida_typeinf.PDF_DEF_BASE |
                                  ida_typeinf.PDF_HEADER_CMT)
print("export_names: local types %d, print_decls %d" % (len(ords), got))
for k, v in sorted(stat.items()):
    print("export_names: %-40s %d" % (k, v))
print("export_names: functions %d, names rows %d" % (len(fn_rows), len(rows)))
idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

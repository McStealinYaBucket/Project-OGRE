"""Phase 1 step 1: first auto-analysis of a fresh database. Waits for the queue, writes the
function list to ida/exports/<TARGET>-functions.csv, saves packed and exits.
Run: idat -A -Stools/ida/first_analysis.py -Lida/logs/<TARGET>-00.log -oida/<TARGET>.i64 <binary>"""
import os
import ida_auto, ida_funcs, ida_loader, ida_name, idautils, idc

ida_auto.auto_wait()
idb = idc.get_idb_path()
target = os.path.splitext(os.path.basename(idb))[0]
out = os.path.join(os.path.dirname(idb), "exports", target + "-functions.csv")
with open(out, "w") as f:
    f.write("address,size,flags,name\n")
    n = 0
    for ea in idautils.Functions():
        fn = ida_funcs.get_func(ea)
        f.write("0x%08x,%d,0x%x,%s\n" % (ea, fn.size(), fn.flags, ida_name.get_name(ea)))
        n += 1
print("first_analysis: %s functions=%d" % (target, n))
ida_loader.save_database(idb, ida_loader.DBFL_COMP)
idc.qexit(0)

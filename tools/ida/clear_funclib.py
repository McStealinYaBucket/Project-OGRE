"""Clear FUNC_LIB on the given functions and save packed (canonical database: snapshot first).

    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/clear_funclib.py 0x004fabe0 0x00731060" -Lida/logs/STUDIO-10-funclib.log ida/STUDIO.i64

For a function FLIRT flagged as library code without naming it (a dummy name) that a reccmp 100% match then
named after our own function: the flag makes tools/ida/export_names.py report the name as flirt:.
tools/ida/apply_db_updates.py now clears it when it renames; this fixes addresses renamed before that
(STUDIO, 2026-11-13: 0x004fabe0 ContentId::c_str, 0x00731060 / 0x00731900 / 0x007321a0 shared_ptr<Value<T>> ctors).
"""
import ida_auto, ida_funcs, ida_loader, idc

ida_auto.auto_wait()
for a in idc.ARGV[1:]:
    f = ida_funcs.get_func(int(a, 16))
    if f is None or not f.flags & ida_funcs.FUNC_LIB:
        print("clear_funclib: %s not a library-flagged function" % a)
        continue
    f.flags &= ~ida_funcs.FUNC_LIB
    ida_funcs.update_func(f)
    print("clear_funclib: %s %s cleared" % (a, idc.get_func_name(f.start_ea)))
ida_loader.save_database(idc.get_idb_path(), ida_loader.DBFL_COMP)
print("clear_funclib: saved packed")
idc.qexit(0)

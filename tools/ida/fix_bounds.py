"""Fix function bounds the work reports proved wrong in a Windows database (canonical: snapshot first).

    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/fix_bounds.py" -Lida/logs/PLAYER-10-bounds.log ida/PLAYER.i64
    (argument "nosave": measure only, on a scratch copy)

1. Bridge<T>::on_newindex(lua_State*) wrappers (near-miss-2.md): they call an on_newindex(T&, const char*,
   lua_State*) that always throws, so IDA marked the callee noreturn and ended the wrapper at the call, but the
   compiler still emitted `add esp,0x24; pop edi; xor eax,eax; pop esi; ret` (83 c4 24 5f 33 c0 5e c3) after it.
   Every function whose end is followed by exactly those 8 bytes, with no function starting there, is extended
   over them (PLAYER 25, STUDIO 25 on 2026-11-13; e.g. 0x00670170 0x28 -> 0x30).
2. Functions IDA never created (ADD): STUDIO 0x00a0e170 IndexedMesh::isUpperRoot (math-5a.md; the same bytes
   as PLAYER 0x008f90b0, a function there).
Log lines start with `fix_bounds:`.
"""
import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_ua, idaapi, idc
import os

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
TARGET = os.path.splitext(os.path.basename(idc.get_idb_path()))[0]
EPILOGUE = bytes.fromhex("83c4245f33c05ec3")
ADD = {"STUDIO": [0x00a0e170]}


def say(*a):
    print("fix_bounds: " + " ".join(str(x) for x in a))


n = 0
for ea in list(__import__("idautils").Functions()):
    f = ida_funcs.get_func(ea)
    end = f.end_ea
    if ida_bytes.get_bytes(end, 8) != EPILOGUE or ida_funcs.get_func(end) is not None:
        continue
    ida_bytes.del_items(end, ida_bytes.DELIT_SIMPLE, 8)
    k = end
    while k < end + 8:
        size = ida_ua.create_insn(k)
        if not size:
            break
        k += size
    if k == end + 8 and ida_funcs.set_func_end(ea, end + 8):
        n += 1
        say("extended 0x%08x %s: 0x%x -> 0x%x" % (ea, idc.get_func_name(ea), end - ea, end + 8 - ea))
    else:
        say("FAILED to extend 0x%08x" % ea)
say("extended", n)
for ea in ADD.get(TARGET, []):
    owner = ida_funcs.get_func(ea)
    if owner is not None:
        ch = ida_funcs.get_fchunk(ea)
        if owner.start_ea == ea or ch is None or ch.start_ea != ea or not (ch.flags & ida_funcs.FUNC_TAIL):
            say("0x%08x already in a function (start 0x%08x)" % (ea, owner.start_ea))
            continue
        # a tail chunk IDA attached to the function that jumps to it (a tail call): give it back
        say("0x%08x: removing tail chunk 0x%08x-0x%08x from 0x%08x %s" % (ea, ch.start_ea, ch.end_ea, owner.start_ea,
                                                                       idc.get_func_name(owner.start_ea)))
        ida_funcs.remove_func_tail(owner, ea)
    ok = ida_funcs.add_func(ea)
    ida_auto.auto_wait()
    f = ida_funcs.get_func(ea)
    say("add 0x%08x: %s, size 0x%x" % (ea, ok, f.size() if f else 0))
if SAVE:
    ida_loader.save_database(idc.get_idb_path(), ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

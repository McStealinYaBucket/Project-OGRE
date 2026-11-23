"""Delete named local types from a Windows database (saves packed). For a proven layout whose base chain changed
(the old base types stay behind after tools/ida/import_types.py apply, which only adds and replaces).

    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/del_types.py names.txt" -Lida/logs/PLAYER-NN-del-types.log ida/PLAYER.i64

names.txt: one C++ type name per line, spelled as import_types.py wrote it (its log's "type ..." lines).
A type still used by another local type is kept (logged): deleting it would leave a dangling reference.
A use is the whole name (not a prefix of a template instance: RBX::X is not used by RBX::X<A, B>).
"""
import re

import ida_auto, ida_loader, ida_typeinf, idc

ida_auto.auto_wait()
names = [l.strip() for l in open(idc.ARGV[1]) if l.strip()]
til = ida_typeinf.get_idati()


def users():
    """names (of `names`) that another local type's declaration mentions"""
    out = set()
    for ordinal in range(1, ida_typeinf.get_ordinal_limit(til)):
        t = ida_typeinf.tinfo_t()
        if t.get_numbered_type(til, ordinal):
            decl = t._print(None, ida_typeinf.PRTYPE_DEF | ida_typeinf.PRTYPE_MULTI) or ""
            own = t.get_type_name() or ""
            out.update(n for n in names if n != own and re.search(r"(?<![\w:])" + re.escape(n) + r"(?![\w<])", decl))
    return out


left = list(names)
while left:     # a chain of stale types (A uses B) goes in several rounds
    used = users()
    gone = [n for n in left if n not in used and ida_typeinf.del_named_type(til, n, ida_typeinf.NTF_TYPE)]
    for n in gone:
        print("del_types: deleted", n)
    left = [n for n in left if n not in gone]
    names = left
    if not gone:
        break
for n in left:
    print("del_types: kept (still referenced or not found)", n)
ida_loader.save_database(idc.get_idb_path(), ida_loader.DBFL_COMP)
print("del_types: saved packed")
idc.qexit(0)

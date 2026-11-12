"""Write Hex-Rays pseudocode (or the disassembly, when decompilation fails) of a list of
functions to a JSON-lines file. Read-only: the database is killed, never saved.

Run on a scratch COPY of a database, never on ida/*.i64:
  TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/dump_pseudocode.py <addrs.txt> <out.jsonl>" \
      -L<log> /tmp/.../COPY.i64
addrs.txt holds one address per line (hex with 0x, or decimal).
"""
import json

import ida_auto
import ida_funcs
import ida_hexrays
import ida_lines
import idaapi
import idautils
import idc


def disasm(f):
    out = []
    for ch in idautils.Chunks(f.start_ea):
        for ea in idautils.Heads(ch[0], ch[1]):
            out.append("%08x  %s" % (ea, idc.generate_disasm_line(ea, 0)))
    return "\n".join(out)


def main():
    ida_auto.auto_wait()
    src, dst = idc.ARGV[1], idc.ARGV[2]
    ok = ida_hexrays.init_hexrays_plugin()
    with open(dst, "w") as fo:
        for line in open(src):
            line = line.strip()
            if not line:
                continue
            ea = int(line, 0)
            f = ida_funcs.get_func(ea)
            rec = {"ea": ea, "func": f.start_ea if f else None,
                   "name": idc.get_func_name(ea), "size": (f.end_ea - f.start_ea) if f else None}
            if f:
                rec["disasm"] = disasm(f)
                if ok:
                    try:
                        c = ida_hexrays.decompile(f.start_ea)
                        rec["pseudo"] = "\n".join(ida_lines.tag_remove(l.line) for l in c.get_pseudocode())
                    except Exception as e:  # noqa: BLE001 - report and keep going
                        rec["error"] = str(e)
            fo.write(json.dumps(rec) + "\n")
            fo.flush()
    idaapi.set_database_flag(idaapi.DBFL_KILL)
    idc.qexit(0)


main()

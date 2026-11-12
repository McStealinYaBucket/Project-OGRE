"""Dump Hex-Rays pseudocode (disassembly when decompilation fails) for a list of functions, for
side-by-side review. READ-ONLY by intent: renames, if asked for, go only into the open database,
which this script never saves (DBFL_KILL). Run it on a scratch COPY of a canonical .i64.

  TVHEADLESS=1 idat -A -S"tools/ida/decomp_dump.py LIST OUT [RENAMES_CSV]" -L<log> <copy>.i64

LIST: one item per line, either 0xADDRESS or a symbol name as IDA spells it (Mach-O `__Z...`).
OUT: text file, one `==== item` block per item. RENAMES_CSV (optional): a names-proposed CSV;
its first name per address is applied as `P_<qualified name>` so callees read by name.
"""
import csv
import re

import ida_auto
import ida_funcs
import ida_hexrays
import ida_lines
import ida_name
import idaapi
import idc

MAXL = 160


def main():
    args = idc.ARGV[1:]
    lst, out = args[0], args[1]
    ida_auto.auto_wait()
    if len(args) > 2:
        seen = set()
        for r in csv.DictReader(open(args[2])):
            ea = int(r["address"], 16)
            if ea in seen:
                continue
            seen.add(ea)
            q = re.sub(r"\(.*", "", r["demangled"].split(" [")[0])
            q = re.sub(r"<[^<>]*>", "", re.sub(r"<[^<>]*>", "", q))
            n = "P_" + re.sub(r"[^0-9A-Za-z_]+", "_", q)
            ida_name.set_name(ea, n, ida_name.SN_NOWARN | ida_name.SN_NOCHECK | ida_name.SN_FORCE)
    hx = ida_hexrays.init_hexrays_plugin()
    with open(out, "w") as f:
        for item in open(lst).read().split():
            ea = int(item, 16) if item.startswith("0x") else ida_name.get_name_ea(idaapi.BADADDR, item)
            fn = ida_funcs.get_func(ea) if ea != idaapi.BADADDR else None
            f.write("==== %s\n" % item)
            if not fn:
                f.write("  (no function)\n")
                continue
            f.write("  ea=0x%x size=%d name=%s\n" % (fn.start_ea, fn.end_ea - fn.start_ea, ida_funcs.get_func_name(fn.start_ea)))
            text = None
            if hx:
                try:
                    c = ida_hexrays.decompile(fn.start_ea)
                    text = [ida_lines.tag_remove(l.line) for l in c.get_pseudocode()]
                except Exception as e:  # noqa: BLE001
                    f.write("  (decompile failed: %s)\n" % e)
            if text is None:
                text = []
                for h in idaapi.Heads(fn.start_ea, fn.end_ea) if hasattr(idaapi, "Heads") else []:
                    text.append("%x  %s" % (h, idc.generate_disasm_line(h, 0)))
                if not text:
                    import idautils
                    text = ["%x  %s" % (h, idc.generate_disasm_line(h, 0)) for h in idautils.Heads(fn.start_ea, fn.end_ea)]
            for l in text[:MAXL]:
                f.write(l + "\n")
            if len(text) > MAXL:
                f.write("  ... (%d more lines)\n" % (len(text) - MAXL))
            f.flush()
    idaapi.set_database_flag(idaapi.DBFL_KILL)
    idc.qexit(0)


main()

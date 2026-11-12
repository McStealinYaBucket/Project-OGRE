"""Import the proven class layouts (classes whose DECOMP_SIZE_ASSERT passes and whose ctor/dtor match at 100%)
into a Windows database as local types, and type the `this` of their constructors and destructors.

Two steps:
  1. dump (no IDA; reccmp's cvdump.exe under wine, streamed, about 0.1 GB): read the class layouts from our
     recompiled PDB's type records and write them, with every base class they need, as JSON:
         tools/capped.sh python3 -P tools/ida/import_types.py dump build/RobloxPlayerBeta.pdb ida/exports/PLAYER-proven-types.json
  2. apply (IDA, saves packed): create one local type per class (C++ name, e.g. RBX::Instance), members at
     the PDB's offsets, gaps as gap_XX byte arrays, base classes as base-class members, vfptr as
     `void **__vftable`; members whose type is another imported class get that type, pointers to an imported
     class get a typed pointer, everything else a byte array of the member's size whose comment names the
     C++ type. Then every function named as a ctor/dtor of an imported class (MSVC ??0/??1/??_G/??_E, or
     Itanium C1/C2/D0/D1/D2) gets `this` typed as Class*, keeping the rest of its prototype.
         TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/import_types.py apply" -Lida/logs/PLAYER-06-types.log ida/PLAYER.i64
     The struct sizes are checked against the PDB's; a mismatch is logged and the type is not applied.
Classes and their 2013 proof: see CLASSES. A base class not in CLASSES is imported too (it is part of the
proven layout), flagged "base" in the JSON and the type comment.
"""
import json
import os
import re
import sys

CLASSES = [
    "RBX::Debugable", "RBX::Object", "RBX::Reflection::EventSource", "RBX::Reflection::DescribedBase",
    "RBX::GuidItem<RBX::Instance>", "RBX::Limits::Countable", "RBX::Instance", "RBX::ServiceProvider",
    "RBX::VerbContainer", "RBX::Verb", "RBX::Reflection::ClassDescriptor",
    "RBX::Reflection::MemberDescriptorContainer<RBX::Reflection::PropertyDescriptor>", "RBX::Name",
    "RBX::DataModelJob", "RBX::DataModel::GenericJob", "RBX::GcJob", "RBX::LibraryService",
    "RBX::MovingAssemblyStage", "RBX::DataModelArbiter",
]


CVDUMP = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))),
                      "toolchain/venv/lib/python3.14/site-packages/reccmp/bin/cvdump.exe")
PRIM_SIZE = {"T_INT4": 4, "T_UINT4": 4, "T_LONG": 4, "T_ULONG": 4, "T_SHORT": 2, "T_USHORT": 2, "T_CHAR": 1,
             "T_UCHAR": 1, "T_RCHAR": 1, "T_BOOL08": 1, "T_REAL32": 4, "T_REAL64": 8, "T_QUAD": 8, "T_UQUAD": 8,
             "T_INT8": 8, "T_UINT8": 8, "T_WCHAR": 2, "T_INT2": 2, "T_UINT2": 2, "T_INT1": 1, "T_UINT1": 1,
             "T_HRESULT": 4, "T_REAL80": 10}
HEAD = re.compile(r"^(0x[0-9a-fA-F]+) : Length = \d+, Leaf = 0x[0-9a-fA-F]+ (\w+)")
UDTLINE = re.compile(r"Size = (\d+)\s*,\s*class name = (.*), unique name = (.*), UDT\((0x[0-9a-fA-F]+)\)")


def dump(pdb, out):
    """Lean reader (2026-11-12: reccmp's Cvdump types parser held 13-22 GB for this PDB). cvdump -t is
    streamed to a temporary file while only the byte offset of each record and the UDT index of each named
    class are kept; then only the records reachable from CLASSES through base classes and member types are
    read back. Identity is the unique (decorated) name; its newest definition record wins (an incrementally
    linked PDB still carries the records of older compiles, with stale sizes; cvdump's UDT() index is per
    printed name, so it cannot tell template instantiations apart). Names that stand for several
    classes (the PDB prints Described<...> as RBX::Reflection::Described) become their undecorated unique
    name (llvm-undname)."""
    import subprocess
    import tempfile
    tmp = tempfile.TemporaryFile(dir=os.path.dirname(os.path.abspath(out)))
    win = "Z:" + os.path.abspath(pdb).replace("/", "\\")
    proc = subprocess.Popen(["wine", CVDUMP, "-t", win], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    offs, uniq_at, newest, names = {}, {}, {}, {}
    pos, cur = 0, None
    prev = ""
    for raw in proc.stdout:
        tmp.write(raw)
        line = raw.decode("latin-1")
        m = HEAD.match(line)
        if m:
            cur = int(m.group(1), 16)
            offs[cur] = pos
        elif cur is not None and "class name = " in line:
            u = UDTLINE.search(line)
            if u:
                uniq_at[cur] = u.group(3)
                if u.group(1) != "0" or "FORWARD REF" not in prev:
                    newest[u.group(3)] = max(newest.get(u.group(3), 0), cur)
                    names.setdefault(u.group(2), set()).add(u.group(3))
        prev = line
        pos += len(raw)
    if proc.wait() != 0:
        sys.exit("cvdump failed")

    def record(k):
        tmp.seek(offs[k])
        lines = [tmp.readline().decode("latin-1")]
        while True:
            l = tmp.readline().decode("latin-1")
            if not l.strip() or HEAD.match(l):
                return lines
            lines.append(l)

    def tkey(s):
        """'0x0002CD06' -> int; 'T_ULONG(0022)' -> 'T_ULONG'."""
        return int(s, 16) if s.startswith("0x") else s.split("(")[0]

    cache = {}

    def info(k):
        """(leaf, dict) of one record, parsed for what the layout needs."""
        if k in cache:
            return cache[k]
        ls = record(k)
        leaf = HEAD.match(ls[0]).group(2)
        body = " ".join(l.strip() for l in ls[1:])
        d = {}
        if leaf in ("LF_CLASS", "LF_STRUCTURE", "LF_UNION"):
            u = UDTLINE.search(body)
            d["size"], d["name"], d["unique"] = int(u.group(1)), u.group(2), u.group(3)
            d["fields"] = int(re.search(r"field list type (0x[0-9a-fA-F]+)", body).group(1), 16)
        elif leaf == "LF_POINTER":
            d["elem"] = tkey(re.search(r"Element type : (\S+)", body).group(1))
            d["size"] = int(re.search(r"Size: (\d+)", body).group(1))
        elif leaf == "LF_MODIFIER":
            d["elem"] = tkey(re.search(r"modifies type (\S+)", body).group(1))
        elif leaf == "LF_ARRAY":
            d["size"] = int(re.search(r"length = (\d+)", body).group(1))
        elif leaf == "LF_ENUM":
            d["elem"] = tkey(re.search(r"type = (\S+)", body).group(1))
            d["name"] = re.search(r"enum name = (.*), UDT", body).group(1)
        elif leaf == "LF_BITFIELD":
            m = re.search(r"bits = (\d+), starting position = (\d+), Type = (\S+)", body)
            d["bits"], d["pos"], d["elem"] = int(m.group(1)), int(m.group(2)), tkey(m.group(3))
        elif leaf == "LF_FIELDLIST":
            d["list"] = []
            for l in re.split(r"\s(?=list\[\d+\] = )", body):
                m = re.match(r"list\[\d+\] = (LF_\w+), (.*)", l.strip())
                if m:
                    d["list"].append((m.group(1), m.group(2)))
        cache[k] = (leaf, d)
        return cache[k]

    def udt(k):
        """The definition of a class record (or forward reference): the newest record with its unique name."""
        return newest.get(uniq_at.get(k), k)

    undn = {}

    def cname(k):
        n = info(k)[1]["name"]
        if len(names.get(n, ())) <= 1:
            return n
        if k not in undn:
            o = subprocess.run(["llvm-undname"], input="?x@@3%sA\n" % uniq_at[k], capture_output=True,
                               text=True).stdout.split("\n")
            t = o[1] if len(o) > 1 and o[1].endswith(" x") else n + "[" + uniq_at[k] + "]"
            undn[k] = re.sub(r"\b(?:class|struct|union|enum) ", "", t[:-2] if t.endswith(" x") else t)
        return undn[k]

    def describe(t):
        """(kind, type name, size, scalar) of a member type."""
        if isinstance(t, str):
            if t.startswith("T_32P") or t.startswith("T_PV"):
                return "ptr", None, 4, None
            return "scalar", None, PRIM_SIZE.get(t), t
        leaf, d = info(t)
        if leaf == "LF_MODIFIER":
            return describe(d["elem"])
        if leaf == "LF_POINTER":
            e = d["elem"]
            while not isinstance(e, str) and info(e)[0] == "LF_MODIFIER":
                e = info(e)[1]["elem"]
            ref = cname(udt(e)) if not isinstance(e, str) and info(e)[0] in ("LF_CLASS", "LF_STRUCTURE",
                                                                            "LF_UNION") else None
            return "ptr", ref, d["size"], None
        if leaf in ("LF_CLASS", "LF_STRUCTURE", "LF_UNION"):
            k = udt(t)
            return leaf[3:].lower(), cname(k), info(k)[1]["size"], None
        if leaf == "LF_ENUM":
            e = describe(d["elem"])
            return "enum", d["name"], e[2], e[3]
        if leaf == "LF_ARRAY":
            return "array", None, d["size"], None
        if leaf == "LF_BITFIELD":
            e = describe(d["elem"])
            return "bitfield", e[1], e[2], e[3]
        return leaf, None, None, None

    def fields(fl):
        """A field list, following LF_INDEX continuations (lists over 64 KB)."""
        out = []
        while fl:
            nxt = 0
            for f, rest in info(fl)[1]["list"]:
                if f == "LF_INDEX":
                    nxt = int(re.search(r"(0x[0-9a-fA-F]+)", rest).group(1), 16)
                else:
                    out.append((f, rest))
            fl = nxt
        return out

    def has_members(k):
        return any(f in ("LF_MEMBER", "LF_VFUNCTAB", "LF_BCLASS") for f, _ in fields(info(k)[1]["fields"]))

    res, missing, todo = {}, [], []
    for n in CLASSES:
        ks = names.get(n, set())
        if len(ks) != 1:
            missing.append(n if not ks else "%s (%d definitions)" % (n, len(ks)))
            continue
        todo.append((n, newest[ks.pop()]))
    listed = {n for n, _ in todo}
    while todo:
        n, key = todo.pop(0)
        if n in res:
            continue
        c = info(key)[1]
        bases, mem, vf, vb = [], [], False, False
        bits = {}
        for f, rest in fields(c["fields"]):
            if f == "LF_VFUNCTAB":
                vf = True
            elif f in ("LF_VBCLASS", "LF_IVBCLASS"):
                vb = True
            elif f == "LF_BCLASS":
                m = re.search(r"type = (\S+), offset = (\d+)", rest)
                bk = udt(tkey(m.group(1).rstrip(",")))
                bn = cname(bk)
                bases.append({"name": bn, "offset": int(m.group(2)), "size": info(bk)[1]["size"],
                              "empty": not has_members(bk)})
                todo.append((bn, bk))
            elif f == "LF_MEMBER":
                m = re.search(r"type = (\S+), offset = (\d+)\s+member name = '(.*)'", rest)
                t, off, name = tkey(m.group(1).rstrip(",")), int(m.group(2)), m.group(3)
                kind, ref, size, scalar = describe(t)
                if kind == "bitfield":
                    bf = info(t)[1]
                    if off in bits:     # one storage unit for the bitfields sharing an offset
                        bits[off]["bits"].append("%s:%d@%d" % (name, bf["bits"], bf["pos"]))
                        continue
                    row = {"name": name, "offset": off, "size": size, "kind": "bitfield", "type": None,
                           "scalar": scalar, "bits": ["%s:%d@%d" % (name, bf["bits"], bf["pos"])]}
                    bits[off] = row
                    mem.append(row)
                    continue
                mem.append({"name": name, "offset": off, "size": size, "kind": kind, "type": ref,
                            "scalar": scalar})
        bases.sort(key=lambda b: b["offset"])
        mem.sort(key=lambda m: m["offset"])
        res[n] = {"size": c["size"], "vftable": vf, "virtual_bases": vb, "bases": bases, "members": mem,
                  "base": n not in listed, "unique": uniq_at.get(key, "")}
    json.dump({"pdb": os.path.basename(pdb), "classes": res, "missing": missing}, open(out, "w"), indent=1)
    print("import_types dump: %d classes (%d listed, %d bases), records read %d of %d, missing %s" % (
        len(res), sum(not v["base"] for v in res.values()), sum(v["base"] for v in res.values()),
        len(cache), len(offs), missing))


SCALARS = {"T_INT4": "int", "T_UINT4": "unsigned int", "T_LONG": "int", "T_ULONG": "unsigned int",
           "T_SHORT": "__int16", "T_USHORT": "unsigned __int16", "T_CHAR": "char", "T_UCHAR": "unsigned char",
           "T_RCHAR": "char", "T_BOOL08": "bool", "T_REAL32": "float", "T_REAL64": "double",
           "T_QUAD": "__int64", "T_UQUAD": "unsigned __int64", "T_INT8": "__int64", "T_UINT8": "unsigned __int64",
           "T_WCHAR": "wchar_t", "T_32PVOID": "void *"}

CTOR_DTOR = re.compile(r"^\?\?(?:0|1|_G|_E)(?P<msvc>.+?)@@[QUAIEM][AE]|^_ZN(?P<ita>.+?)(?:C[12]|D[012])E")


def apply():
    import ida_auto, ida_funcs, ida_loader, ida_name, ida_typeinf, idaapi, idautils, idc
    ida_auto.auto_wait()
    idb = idc.get_idb_path()
    target = os.path.splitext(os.path.basename(idb))[0]
    data = json.load(open(os.path.join(os.path.dirname(idb), "exports", target + "-proven-types.json")))
    classes = data["classes"]
    log = []

    def say(*a):
        s = "import_types: " + " ".join(str(x) for x in a)
        print(s)
        log.append(s)

    def arr(n):
        t = ida_typeinf.tinfo_t()
        t.create_array(ida_typeinf.tinfo_t(ida_typeinf.BTF_UINT8), n)
        return t

    def named(n):
        t = ida_typeinf.tinfo_t()
        return t if t.get_named_type(None, n) else None

    def scalar(s):
        t = ida_typeinf.tinfo_t()
        decl = SCALARS.get(s)
        if decl and ida_typeinf.parse_decl(t, None, decl + " x;", ida_typeinf.PT_SIL) is not None:
            return t
        return None

    # pass 1: empty placeholders so members can refer to each other; pass 2: the real layout (bases first)
    order, seen = [], set()

    def visit(n):
        if n in seen or n not in classes:
            return
        seen.add(n)
        for b in classes[n]["bases"]:
            visit(b["name"])
        order.append(n)
    for n in classes:
        visit(n)
    ok = set()

    def build(final):
        """One pass over the classes; the second pass sees every name, so pointers between them get typed."""
        note = say if final else (lambda *x: None)
        for n in order:
            c = classes[n]
            udt = ida_typeinf.udt_type_data_t()
            pos = 0

            def add(name, off, t, cmt="", base=False):
                nonlocal pos
                if off > pos:
                    g = ida_typeinf.udm_t()
                    g.name, g.offset, g.size = "gap_%X" % pos, pos * 8, (off - pos) * 8
                    g.type = arr(off - pos)
                    udt.push_back(g)
                u = ida_typeinf.udm_t()
                u.name, u.offset, u.type, u.cmt = name, off * 8, t, cmt
                u.size = t.get_size() * 8
                if base:
                    u.set_baseclass()
                udt.push_back(u)
                pos = off + t.get_size()

            first = sorted([(b["offset"], "b", b) for b in c["bases"]] + [(m["offset"], "m", m) for m in c["members"]],
                           key=lambda x: (x[0], x[1]))
            if c["vftable"] and not any(o == 0 and k == "b" and classes.get(b["name"], {}).get("vftable")
                                        for o, k, b in first):
                vt = ida_typeinf.tinfo_t()
                ida_typeinf.parse_decl(vt, None, "void **x;", ida_typeinf.PT_SIL)
                add("__vftable", 0, vt)
            for off, k, x in first:
                if off < pos:
                    note("overlap in %s at 0x%x (%s) skipped" % (n, off, x["name"]))
                    continue
                if k == "b" and x.get("empty"):
                    continue    # an empty base (EBO or a 1-byte pad): left to the gap bytes
                if k == "b":
                    t = named(x["name"]) if x["name"] in ok else None
                    add(x["name"].split("::")[-1] if t else "base_" + re.sub(r"\W", "_", x["name"])[:40], off,
                        t or arr(x["size"]), "" if t else "base " + x["name"], base=bool(t))
                    continue
                t, cmt = None, ""
                if x["kind"] == "ptr":
                    if x["type"] in ok:
                        t = ida_typeinf.tinfo_t()
                        t.create_ptr(named(x["type"]))
                    else:
                        t = scalar("T_32PVOID")
                        cmt = "%s *" % (x["type"] or "?")
                elif x["kind"] in ("scalar", "bitfield", "enum"):
                    t = scalar(x["scalar"])
                    if t and t.get_size() != x["size"]:
                        t = None
                    if x["kind"] == "bitfield":
                        cmt = "bitfields " + ", ".join(x["bits"]) + (" (%s)" % x["type"] if x["type"] else "")
                    elif x["kind"] == "enum" and t:
                        cmt = "enum " + x["type"]
                elif x["type"] in ok:
                    t = named(x["type"])
                if t is None:
                    if not x["size"]:
                        note("member %s::%s has no size, skipped" % (n, x["name"]))
                        continue
                    t, cmt = arr(x["size"]), x["type"] or x["kind"]
                add(x["name"], off, t, cmt)
            if c["size"] and c["size"] > pos:
                g = ida_typeinf.udm_t()
                g.name, g.offset, g.size, g.type = "gap_%X" % pos, pos * 8, (c["size"] - pos) * 8, arr(c["size"] - pos)
                udt.push_back(g)
            udt.taudt_bits |= ida_typeinf.TAUDT_CPPOBJ
            t = ida_typeinf.tinfo_t()
            if not t.create_udt(udt, ida_typeinf.BTF_STRUCT):
                note("create_udt failed for", n)
                continue
            err = t.set_named_type(None, n, ida_typeinf.NTF_REPLACE)
            if err != ida_typeinf.TERR_OK:
                note("set_named_type failed for %s: %s" % (n, ida_typeinf.tinfo_errstr(err)))
                continue
            got = named(n)
            if got is None or got.get_size() != c["size"]:
                note("SIZE MISMATCH %s: IDA %s, PDB 0x%x" % (n, got and hex(got.get_size()), c["size"]))
                continue
            ok.add(n)
            if final:
                say("type %-80s size 0x%x%s" % (n, c["size"], " (base)" if c["base"] else ""))

    build(False)
    build(True)

    # `this` of ctors/dtors
    typed = methods = 0
    for ea in idautils.Functions():
        nm = ida_name.get_name(ea)
        if not nm.startswith(("?", "_ZN")):
            continue
        dem = ida_name.demangle_name(nm, ida_name.MNG_SHORT_FORM) or ""
        m = re.match(r"^(?:[\w\s]*?)?(?P<cls>[\w:<>,\s\*&]+)::(?P<fn>~?[\w<>,:\s\*&]+?|`[^']+')\(", dem.strip())
        if not m:
            continue
        cls, fn = m.group("cls").strip(), m.group("fn").strip()
        cls_n = re.sub(r"\s+", "", cls)
        hit = next((c for c in ok if re.sub(r"\s+", "", c) == cls_n), None)
        if hit is None:
            continue
        base = hit.split("::")[-1].split("<")[0]
        structor = fn.startswith(base) or fn.startswith("~" + base) or "deleting destructor" in fn
        if not structor and not nm.startswith("?"):
            continue    # an Itanium name does not say whether a method is static
        tif = ida_typeinf.tinfo_t()
        if not (idaapi.get_tinfo(tif, ea) or ida_typeinf.guess_tinfo(tif, ea)) or not tif.is_func():
            say("no prototype for 0x%08x %s" % (ea, dem))
            continue
        fd = ida_typeinf.func_type_data_t()
        if not tif.get_func_details(fd) or fd.size() == 0:
            say("no arguments for 0x%08x %s" % (ea, dem))
            continue
        cc = fd.get_cc()
        # IDA's prototype from an Itanium name passes `this` as the first stack argument (cdecl/stdcall);
        # MSVC passes it in ecx with the other arguments on the stack in the same order, so thiscall fits.
        itanium = nm.startswith("_ZN") and cc in (ida_typeinf.CM_CC_CDECL, ida_typeinf.CM_CC_STDCALL)
        if not structor and cc != ida_typeinf.CM_CC_THISCALL:
            continue    # static member functions (MSVC names say so; IDA's prototype is not thiscall)
        if not (cc == ida_typeinf.CM_CC_THISCALL or (cc == ida_typeinf.CM_CC_FASTCALL and fd.size() == 1)
                or itanium):
            say("not thiscall (cc 0x%x, %d args), left alone: 0x%08x %s" % (cc, fd.size(), ea, dem))
            continue
        pt = ida_typeinf.tinfo_t()
        pt.create_ptr(named(hit))
        fd[0].type, fd[0].name = pt, "this"
        fd.set_cc(ida_typeinf.CM_CC_THISCALL)
        nt = ida_typeinf.tinfo_t()
        nt.create_func(fd)
        if ida_typeinf.apply_tinfo(ea, nt, ida_typeinf.TINFO_DEFINITE):
            typed += 1
            methods += not structor
            say("this 0x%08x %s -> %s *" % (ea, dem, hit))
        else:
            say("apply_tinfo failed 0x%08x %s" % (ea, dem))
    say("types imported %d of %d (%d listed); this typed %d (%d ctor/dtor, %d other methods)" % (
        len(ok), len(classes), sum(1 for n in ok if not classes[n]["base"]), typed, typed - methods, methods))
    for n in classes:
        if n not in ok:
            say("NOT IMPORTED", n)
    for n in data["missing"]:
        say("NOT IN PDB", n)
    if "nosave" in idc.ARGV[1:]:
        idaapi.set_database_flag(idaapi.DBFL_KILL)
    else:
        ida_loader.save_database(idb, ida_loader.DBFL_COMP)
        say("saved packed")
    idc.qexit(0)


if __name__ == "__main__":
    try:
        import idc  # noqa: F401
        apply()
    except ImportError:
        if sys.argv[1:2] != ["dump"] or len(sys.argv) != 4:
            sys.exit(__doc__)
        dump(sys.argv[2], sys.argv[3])

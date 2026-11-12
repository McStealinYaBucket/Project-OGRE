"""Cross-check the third-party library CSVs (ida/exports/PLAYER-lib-*.csv) against each other and against
the oracle names, and write the resolved per-address table the IDA apply step and gen_reccmp_csv read.

Run from anywhere:
    python3 -P tools/lib/lib_consistency.py            # report + write ida/exports/<T>-lib-resolved.csv
    python3 -P tools/lib/lib_consistency.py --selftest

Inputs: ida/exports/PLAYER-lib-{lua,raknet,g3d,boost}.csv (rows for target PLAYER and STUDIO),
ida/exports/<T>-names.csv and <T>-functions.csv.

Checks, per target:
  1. two libraries claiming one address (a folded /OPT:ICF body may carry several names of one library;
     that is fine, two libraries on one address is a conflict);
  2. a library row on a function whose current name is an ios:/mac: oracle name: the qualified names
     (MSVC demangled vs Itanium demangled, class/struct keywords, spaces and template arguments
     normalised) must agree, otherwise the row is a conflict. An oracle name in RBX:: (Roblox code) on
     an identical library body is an /OPT:ICF fold with Roblox code (or a mislocation) and the address
     is not claimed for the library;
  3. a library row whose address is not an IDA function start.

Resolution (one row per address in <T>-lib-resolved.csv: address, status, library, version, name,
demangled, names (all names at the address, '|'), note):
  status identical  every row at the address is identical=yes, one library, no oracle conflict
  status located    located with a compiled counterpart, not identical (or mixed)
  status span       inside a library TU span with no compiled counterpart (empty masked_diff_bytes)
  status inside     check 3: the body matched inside a longer IDA function (dropped)
  status conflict   rejected by check 1 or 2: no name applied, no reccmp library row
A row at no IDA function at all keeps its status and the note "create it" (IDA missed the function).
"""
import collections
import csv
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EXP = os.path.join(ROOT, "ida", "exports")
LIBS = ("lua", "raknet", "g3d", "boost")
TARGETS = ("PLAYER", "STUDIO")
CC = re.compile(r"\b__(?:thiscall|cdecl|stdcall|fastcall|clrcall)\s+")


def qualname(d):
    """Qualified function name from an MSVC or Itanium demangled signature, normalised."""
    d = d.split(" [", 1)[0]
    m = CC.search(d)      # the first calling convention: a later one belongs to a function-pointer parameter
    if m:
        d = d[m.end():]
    out, depth = [], 0
    for ch in d:
        if ch == "(" and depth == 0 and out and not "".join(out).endswith("operator"):
            break
        depth += ch == "<"
        depth -= ch == ">"
        out.append(ch)
    s = "".join(out)
    s = re.sub(r"\b(?:class|struct|enum|union)\s+", "", s)
    s = re.sub(r"\s+", "", s)
    s = s.replace("std::basic_string<char,std::char_traits<char>,std::allocator<char>>", "std::string")
    s = s.replace("(anonymousnamespace)", "`anonymousnamespace'").replace("`anonymousnamespace'", "anon")
    s = s.lstrip("*&")
    m = re.match(r"^_?([A-Za-z_]\w*)(?:@\d+)?$", s)     # C names: _deflate@8, _Itoa
    if m:
        s = m.group(1)
    m = re.match(r"^(.*::)`(?:scalar|vector)deletingdtor'$", s)   # MSVC ??_G == Itanium D0
    if m:
        s = m.group(1) + "~" + strip_targs(m.group(1)[:-2]).rsplit("::", 1)[-1]
    return s


def strip_targs(s):
    while True:
        t = re.sub(r"<[^<>]*>", "", s)
        if t == s:
            return s
        s = t


def agree(a, b):
    qa, qb = qualname(a), qualname(b)
    if qa == qb:
        return "exact"
    if strip_targs(qa) == strip_targs(qb):
        return "loose"
    return ""


def load_names(target):
    names = {}
    with open(os.path.join(EXP, f"{target}-names.csv")) as f:
        for r in csv.DictReader(f):
            if r["kind"] == "func":
                names[int(r["address"], 16)] = r
    extents = {}
    with open(os.path.join(EXP, f"{target}-functions.csv")) as f:
        for r in csv.DictReader(f):
            extents[int(r["address"], 16)] = int(r["size"])
    return names, set(extents), extents


def resolve(target, rows, names, starts, extents):
    import bisect
    keys = sorted(extents)

    def inside(a):
        i = bisect.bisect_right(keys, a) - 1
        return keys[i] if i >= 0 and a < keys[i] + extents[keys[i]] else None

    by = collections.defaultdict(list)
    for r in rows:
        by[int(r["address"], 16)].append(r)
    out, rep = [], collections.Counter()
    notes = collections.defaultdict(list)
    for a in sorted(by):
        rs = by[a]
        libs = sorted({r["_lib"] for r in rs})
        claimed = [r for r in rs if r["masked_diff_bytes"] != "" or r["identical"] == "yes"]
        ident = [r for r in rs if r["identical"] == "yes"]
        note = []
        status = "identical" if ident and len(ident) == len(rs) else "located" if claimed else "span"
        if a not in starts:
            host = inside(a)
            if host is not None:
                # A compiled body matching the tail of a longer target function (an inlined callee, or
                # a /GL frame prologue our .obj lacks): not a function of its own here.
                status = "inside"
                note.append("not a function start: %d bytes into IDA function 0x%08x" % (a - host, host))
                rep["inside another IDA function (dropped)"] += 1
            else:
                note.append("not an IDA function start, in no function: create it")
                rep["no IDA function (to create)"] += 1
            notes["start"].append(a)
        if len(libs) > 1:
            if ident and len(ident) == len(rs):
                # /OPT:ICF folded byte-identical bodies of two libraries into one: library code either way.
                note.append("ICF fold of identical bodies from " + "+".join(libs))
                rep["two libraries, all identical (ICF fold, kept)"] += 1
                notes["2lib"].append(a)
            elif len({r["_lib"] for r in ident}) == 1:
                rs = [r for r in rs if r["identical"] == "yes"]
                note.append("also claimed by " + ",".join(l for l in libs if l != rs[0]["_lib"]) + " (not identical)")
                rep["two libraries, identical one kept"] += 1
                status = "identical"
            else:
                status = "conflict"
                note.append("two libraries: " + ",".join(libs))
                rep["two libraries, conflict"] += 1
                notes["2lib"].append(a)
        n = names.get(a)
        pick = (ident or claimed or rs)[0]
        if n and n["source"] in ("ios:", "mac:") and status not in ("conflict", "inside"):
            if pick["name"] == n["name"]:   # unclaimed span rows carry the current name
                if status == "span" and "RBX" in qualname(n["demangled"]):
                    status = "conflict"     # a Roblox function inside the library's address range
                    note.append(f"span row on a Roblox oracle name: {n['source']} {n['demangled'][:120]}")
                    rep["span row on a Roblox oracle name"] += 1
            else:
                hits = [agree(r["demangled"] or r["name"], n["demangled"]) for r in (ident or claimed or rs)]
                if any(hits):
                    rep["oracle name agrees (%s)" % ("exact" if "exact" in hits else "loose")] += 1
                    note.append("oracle agrees: " + n["demangled"][:120])
                else:
                    # Roblox code: anything naming RBX (also a library template instantiated for an RBX
                    # type: that body is emitted from Roblox code). Otherwise both names are library
                    # code and the byte-identical compiled body outranks the oracle's pairing.
                    roblox = "RBX" in qualname(n["demangled"])
                    kind = "roblox" if roblox else "library"
                    if status == "identical" and roblox:
                        status = "conflict"
                    note.append(f"oracle disagrees ({kind}): {n['source']} {n['demangled'][:120]}")
                    rep[f"oracle disagrees, {kind} name, status {status}"] += 1
                    notes["dis-" + kind].append((a, pick["demangled"] or pick["name"], n["demangled"], status))
        rep["status " + status] += 1
        out.append({"address": "0x%08x" % a, "status": status, "library": pick["library"],
                    "version": pick["version"], "name": pick["name"], "demangled": pick["demangled"],
                    "names": "|".join(dict.fromkeys(r["name"] for r in rs)), "note": "; ".join(note)})
    return out, rep, notes


def main():
    allrows = []
    for lib in LIBS:
        with open(os.path.join(EXP, f"PLAYER-lib-{lib}.csv")) as f:
            for r in csv.DictReader(f):
                r["_lib"] = lib
                allrows.append(r)
    for t in TARGETS:
        names, starts, extents = load_names(t)
        out, rep, notes = resolve(t, [r for r in allrows if r["target"] == t], names, starts, extents)
        print(f"== {t}: {len(out)} addresses")
        for k, v in sorted(rep.items()):
            print(f"  {k}: {v}")
        for k in ("2lib", "start"):
            if notes[k]:
                print(f"  {k}:", " ".join("0x%08x" % a for a in notes[k][:40]))
        for k in ("dis-roblox", "dis-library"):
            for a, ln, on, st in notes[k]:
                print(f"  {k} 0x{a:08x} [{st}] lib={ln[:90]!r} oracle={on[:90]!r}")
        path = os.path.join(EXP, f"{t}-lib-resolved.csv")
        with open(path, "w", newline="") as f:
            w = csv.DictWriter(f, list(out[0]))
            w.writeheader()
            w.writerows(out)
        print("  wrote", os.path.relpath(path, ROOT))


def selftest():
    assert qualname("public: void __thiscall RakNet::BitStream::ResetWritePointer(void)") == "RakNet::BitStream::ResetWritePointer"
    assert qualname("RakNet::BitStream::ResetWritePointer()") == "RakNet::BitStream::ResetWritePointer"
    assert qualname("struct lua_TValue * __cdecl index2adr(struct lua_State *, int)") == "index2adr"
    assert qualname("bool __cdecl boost::system::operator==(class boost::system::error_condition const &, x)") == "boost::system::operator=="
    assert qualname("boost::exception_detail::bad_alloc_::~bad_alloc_() [D0, deleting]") == "boost::exception_detail::bad_alloc_::~bad_alloc_"
    assert qualname("_deflate@8") == "deflate" and qualname("_Itoa") == "Itoa"
    assert agree("public: virtual void * __thiscall lua_exception::`scalar deleting dtor'(unsigned int)",
                 "lua_exception::~lua_exception() [D0, deleting]") == "exact"
    assert agree("public: __thiscall G3D::Table<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >,int>::Table(void)",
                 "G3D::Table<std::string, int>::Table()") == "exact"
    print("selftest ok")


if __name__ == "__main__":
    selftest() if "--selftest" in sys.argv else main()

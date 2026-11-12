"""Generate the reccmp data sources (reccmp/<target>-*.csv) from the IDA text exports.

Run from the repo root (system python3 with pefile + capstone, as tools/vmprotect_scope.py):
    python3 -P tools/ida/gen_reccmp_csv.py            # PLAYER and STUDIO
    python3 -P tools/ida/gen_reccmp_csv.py PLAYER     # one target
    python3 -P tools/ida/gen_reccmp_csv.py --build build-x # read build-x/<exe>.map (default build/)
    python3 -P tools/ida/gen_reccmp_csv.py STUDIO --build build-studio --json S.json
                                         # also pair data by position from the functions S.json pairs
                                         # (several runs: --json A.json,B.json; their evidence is pooled)
    OGRE_NO_SIGPAIR=1 python3 -P tools/ida/gen_reccmp_csv.py ...   # without signature pairing (A/B)
    python3 -P tools/ida/gen_reccmp_csv.py --selftest

Inputs: ida/exports/<T>-functions.csv, ida/exports/<T>-names.csv, ida/exports/<T>-lib-resolved.csv
(optional, tools/lib/lib_consistency.py), the original binary (bin/...), notes/vmprotect-player.csv.

Outputs (regenerated, never hand-edited; addresses are absolute VAs, which is what reccmp's CSV loader
takes, as 8 hex digits without 0x):
  <t>-function-size.csv  address,type,size   every function extent IDA found in .text, minus:
                         unwind funclets and catch handlers (from every __CxxFrameHandler3 FuncInfo,
                         magic 0x19930520-22), adjustor thunks and import thunks (by their bytes),
                         functions VMProtect touched (notes/vmprotect-player.csv) and anything outside
                         .text (.vmp0/.vmp1). The counts are written as '#' comments at the top.
  <t>-imports.csv        address,type,size,name   PLAYER only: the linker's IAT in .rdata (0x00aa1000-) that
                         .text calls through, typed 'import'. reccmp reads VMProtect's table in .vmp1
                         instead; the 'import' type needs tools/reccmp-patches/ (see that README).
  <t>-symbols.csv        address,name        demangled ios:/mac:/prior: names, no type (stay in the denominator).
  <t>-crt.csv, -stl.csv, -atl.csv
                         address,type,size,symbol   flirt: names, type library.
  <t>-<lib>.csv          address,type,size,symbol,name   one per library (lua, raknet, g3d, zlib, libzip,
                         boost) from ida/exports/<T>-lib-resolved.csv (tools/lib/lib_consistency.py):
                         status identical rows on kept functions, type library, size = IDA's. Their
                         addresses are left out of -symbols/-crt/-stl/-atl.
                         size = max(IDA's size, the matcher's .obj length): IDA stops before an
                         in-function jump table. Pairing (reccmp a0f494c0 compare/match_msvc.py):
                         match_symbols pairs `symbol` with a recompiled PDB PUBLICS name, then
                         match_functions pairs `name` with a PDB proc name. With a build (--build DIR,
                         default build/: <exe>, .map, .pdb) every row is resolved against what reccmp
                         reads from our PDB (tools/lib/pdb_view.py): the row's symbol or a folded
                         alternative is looked up in the map (an anonymous-namespace ?A0x hash, which
                         depends on the source path, is matched by the rest of the name), and the row
                         takes the PUBLICS name reccmp keeps at that address (ICF keeps one per address),
                         or, for a static (no PUBLICS entry), an empty symbol and the PDB proc name.
                         Without a build the rows keep the matcher's symbol and statics stay unpaired.
  <t>-libdata.csv        address,type,size,symbol,name   with a build: globals, vtables, strings, floats and
                         callees the paired library functions reference, paired by position (data_rows),
                         minus addresses a source marker in Client/ owns (they pair through the marker).
  Oracle names pair by signature: an ios:/mac:/prior: name (Itanium) and our PUBLICS name (MSVC, read by
                         llvm-undname) are both reduced by norm_sig to qualified name + parameter types + cv;
                         a signature unique on both sides gives the -symbols row our PUBLICS name as `symbol`
                         (oracle_pairs). With --json (a reccmp --json run of this target on this build), every
                         function that run paired is also a position source for data_rows, when its whole
                         instruction stream (length and mnemonic per instruction) equals ours and our body ends
                         where the original's does: data is never paired from a misaligned instruction.
A generated file that reccmp-project.yml does not list is reported, so it can be added there.
"""
import bisect
import csv
import glob
import os
import re
import struct
import sys

import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))
from vmprotect_scope import Img, orig_imports  # noqa: E402  (pefile/capstone image helper, IAT scan)

TARGETS = {
    "PLAYER": ("bin/player_1023/RobloxPlayerBeta.exe", "notes/vmprotect-player.csv"),
    "STUDIO": ("bin/studio/RobloxStudioBeta.exe", None),
}
BUILD = os.path.join(ROOT, "build")
FUNCINFO_MAGIC = (0x19930520, 0x19930521, 0x19930522)
# sub ecx,imm8 / sub ecx,imm32 / sub [esp+4],imm8 / sub [esp+4],imm32, each followed by jmp rel32.
ADJUSTOR = [(re.compile(rb"\x83\xe9.\xe9", re.S), 8), (re.compile(rb"\x81\xe9.{4}\xe9", re.S), 11),
            (re.compile(rb"\x83\x6c\x24\x04.\xe9", re.S), 10), (re.compile(rb"\x81\x6c\x24\x04.{4}\xe9", re.S), 13)]


# Data our PDB names but reccmp-datacmp cannot lay out: its type record refers to a type key cvdump's
# output lacks (KeyError in reccmp/cvdump/types.py, which ends the whole datacmp run). Not paired.
DATACMP_CRASH = {"root___RBX::Reflection::ClassDescriptor::rootDescriptor"}


def LIBFILE(library):
    """reccmp/<t>-<file>.csv for a library column value of the library CSVs."""
    lib = library.lower()
    return "boost" if lib.startswith("boost") else "g3d" if lib.startswith("g3d") else lib


def read_map(path):
    """Recompiled link map: ({decorated: address}, {anonymous-namespace pattern: [decorated]},
    {addresses the map flags 'f', i.e. code})."""
    addr, code = {}, set()
    with open(path, errors="replace") as f:
        for line in f:
            parts = line.split()
            if len(parts) >= 4 and re.fullmatch(r"[0-9a-f]{4}:[0-9a-f]{8}", parts[0]) and \
                    re.fullmatch(r"[0-9a-f]{8}", parts[2]):
                addr.setdefault(parts[1], int(parts[2], 16))
                if parts[3] == "f":
                    code.add(int(parts[2], 16))
    anon = {}
    for n in addr:
        if "?A0x" in n:
            anon.setdefault(re.sub(r"\?A0x[0-9a-f]+@", "?A0x@", n), []).append(n)
    return addr, anon, code


def resolve_symbol(cands, addr, anon):
    """Recompiled address of the first candidate our link has (an anonymous-namespace hash depends on
    the source path, so ?A0x<hash> is matched by the rest of the name), else None."""
    for c in cands:
        if c in addr:
            return addr[c]
        hits = anon.get(re.sub(r"\?A0x[0-9a-f]+@", "?A0x@", c), []) if "?A0x" in c else []
        if len(hits) == 1:
            return addr[hits[0]]
    return None


def pdb_view(exe):
    """{address: (type, decorated, name)} as reccmp sees the recompiled PDB (tools/lib/pdb_view.py),
    cached beside the exe until the PDB changes."""
    import subprocess
    pdb, cache = exe[:-4] + ".pdb", exe[:-4] + ".pdbview.tsv"
    if not os.path.exists(cache) or os.path.getmtime(cache) < os.path.getmtime(pdb):
        out = subprocess.run([os.path.join(ROOT, "toolchain/venv/bin/python"), "-I",
                              os.path.join(ROOT, "tools/lib/pdb_view.py"), exe],
                             capture_output=True, text=True, check=True).stdout
        with open(cache, "w") as f:
            f.write(out)
    view = {}
    with open(cache) as f:
        for line in f:
            a, t, d, n = line.rstrip("\n").split("\t")
            view[int(a, 16)] = (t, None if d == "-" else d, None if n == "-" else n)
    return view


_OPS = sorted(["<<=", ">>=", "<<", ">>", "<=", ">=", "<", ">", "()", "->*", "->", "[]", "==", "!=", "+=", "-=",
               "*=", "/=", "%=", "&=", "|=", "^=", "&&", "||", "++", "--", "+", "-", "*", "/", "%", "&", "|", "^",
               "!", "~", "=", ","], key=len, reverse=True)
_OPRX = re.compile(r"\boperator\s*(" + "|".join(re.escape(o) for o in _OPS) + r"|new\[\]|delete\[\]|new|delete)")


def norm_sig(s):
    """One spelling for a demangled function signature from either ABI: llvm-undname of an MSVC name (run
    with --no-access-specifier --no-calling-convention --no-member-type) or c++filt of an Itanium name. Result: qualified name + parameter types + method cv, no return type, no class/struct
    keywords, no spaces around punctuation, std:: typedefs folded, long == int (size_t is unsigned long on
    the Apple targets), top-level const on by-value parameters dropped (Itanium does not encode it).
    None for what is not a plain function (thunks, guard variables, MSVC `...' special names)."""
    s = s.replace("(anonymous namespace)", "{anon}").replace("`anonymous namespace'", "{anon}")
    # the deleting destructor: Itanium D0 (names.csv marks it) is MSVC's scalar/vector deleting destructor
    s = re.sub(r"~[^()]*\(\)\s*\[D0, deleting[^\]]*\]?$", "{sdd}()", s)
    if re.fullmatch(r"[A-Za-z_]\w*", s):
        return "C:" + s  # a C function: no parameter list in either ABI
    s = re.sub(r"`(?:scalar|vector) deleting (?:destructor|dtor)'\(unsigned int\)", "{sdd}()", s)
    if "`" in s or "'" in s or " thunk " in s or s.startswith(("non-virtual thunk", "virtual thunk", "guard ",
                                                                 "typeinfo", "vtable", "VTT", "construction")):
        return None
    s = _OPRX.sub(lambda m: "operator#" + str(_OPS.index(m.group(1)) if m.group(1) in _OPS else m.group(1)), s)
    s = re.sub(r"\boperator (?=[\w:])", "operator#cv ", s)  # conversion operator: keep the space inside
    # the first '(' outside template brackets starts the parameter list
    depth, i = 0, None
    for k, ch in enumerate(s):
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
        elif ch == "(" and depth == 0:
            i = k
            break
    if i is None:
        return None
    head, tail = s[:i], s[i:]
    # an Itanium template function prints its return type first: keep what follows the last top-level space
    depth, cut = 0, 0
    for k, ch in enumerate(head):
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
        elif ch == " " and depth == 0 and not head[:k].endswith("operator#cv"):
            cut = k + 1
    s = head[cut:] + tail
    s = re.sub(r"\b(class|struct|union|enum) ", "", s)
    s = re.sub(r"\s+", " ", s)
    s = re.sub(r" ?([*&,<>()\[\]]) ?", r"\1", s)
    s = s.replace("std::__1::", "std::")
    s = re.sub(r"std::basic_(\w+)<char,std::char_traits<char>,std::allocator<char>>", r"std::\1 ", s)
    s = re.sub(r"std::basic_(\w+)<char,std::char_traits<char>>", r"std::\1 ", s)
    s = re.sub(r" ?([*&,<>()\[\]]) ?", r"\1", s).strip()
    s = s.replace("unsigned __int64", "unsigned long long").replace("__int64", "long long")
    s = re.sub(r"\blong long\b", "LL", s)
    s = re.sub(r"\blong\b", "int", s).replace("LL", "long long")
    s = s.replace("(void)", "()").replace("std::type_info", "type_info")
    s = re.sub(r"\*const(?=[,)])", "*", s)
    s = re.sub(r"(?<=\w) const(?=[,)])", "", s)
    return s


def undname(decorated):
    """{decorated: norm_sig of llvm-undname's reading} for MSVC names (one llvm-undname process). The return
    type is printed and norm_sig drops it: --no-return-type would also drop it inside function types
    (boost::function<void(int)> would print as boost::function<(int)>)."""
    import subprocess
    names = set(decorated)
    out = subprocess.run(["llvm-undname", "--no-access-specifier", "--no-calling-convention", "--no-member-type"], input="\n".join(sorted(names)) + "\n", capture_output=True,
                         text=True).stdout.split("\n")
    res, k = {}, 0
    while k + 1 < len(out):
        if out[k] in names and not out[k + 1].startswith("error:"):
            res[out[k]] = norm_sig(out[k + 1])
        k += 3 if k + 2 < len(out) and out[k + 2] == "" else 2
    return res


def qual(sig):
    """Qualified name of a norm_sig signature (what a PDB proc name of a static gives)."""
    depth = 0
    for k, ch in enumerate(sig):
        depth += ch == "<"
        depth -= ch == ">"
        if ch == "(" and depth == 0:
            return sig[:k]
    return sig


def oracle_pairs(oracle, view, cand_orig, used):
    """Pair oracle-named original functions with our build's functions by signature.
    oracle: {orig address: (demangled, mangled)} for every ios:/mac:/prior: name in names.csv.
    Our side: every FUNCTION in the PDB view, its PUBLICS name read with llvm-undname, or (a static with
    no PUBLICS entry) its proc name, which carries no parameters. Both go through norm_sig. A pair needs
    the signature to occur once among all oracle names and once among all our functions, the original to
    be in cand_orig and ours not to be in used. Statics pair on the qualified name, unique on both sides
    among all qualified names. Returns ({orig: (symbol, name)}, stats); symbol is our PUBLICS name, or
    empty for a static, whose row carries our proc name."""
    import collections
    osig = {a: norm_sig(d) for a, (d, m) in oracle.items()}
    ocount = collections.Counter(v for v in osig.values() if v)
    oqual = collections.Counter(qual(v) for v in osig.values() if v)
    funcs = {a: v for a, v in view.items() if v[0] == "FUNCTION"}
    rsig = undname([d for _, d, _ in funcs.values() if d and d.startswith("?")])
    # C names: _f (cdecl) or _f@8 (stdcall)
    rsig.update({d: "C:" + d[1:].split("@")[0] for _, d, _ in funcs.values() if d and d.startswith("_")})
    rbysig, rbyqual = collections.defaultdict(list), collections.defaultdict(list)
    for a, (_, d, nm) in funcs.items():
        if d and rsig.get(d):
            rbysig[rsig[d]].append(a)
            rbyqual[qual(rsig[d])].append(a)
        elif not d and nm:
            rbyqual[norm_sig(nm) if re.fullmatch(r"[A-Za-z_]\w*", nm) else qual(norm_sig(nm + "()") or nm)].append(a)
    out, stats = {}, collections.Counter()
    for a, sig in osig.items():
        if a not in cand_orig:
            continue
        if not sig:
            stats["oracle name not a plain function"] += 1
            continue
        hits = rbysig.get(sig, [])
        if hits:
            if ocount[sig] > 1 or len(hits) > 1:
                stats["ambiguous signature"] += 1
            elif hits[0] in used:
                stats["ours already paired by a library row"] += 1
            else:
                out[a] = (view[hits[0]][1], oracle[a][0])
                stats["paired by signature"] += 1
            continue
        q = qual(sig)
        st = [r for r in rbyqual.get(q, []) if not view[r][1]]
        if st:
            if oqual[q] > 1 or len(rbyqual[q]) > 1:
                stats["ambiguous static name"] += 1
            elif st[0] in used:
                stats["ours already paired by a library row"] += 1
            else:
                out[a] = ("", view[st[0]][2])
                stats["paired by static name"] += 1
            continue
        stats["name present, signature differs" if q in rbyqual else "not defined in our build"] += 1
    return out, stats


def obj_sizes(target):
    """{address: longest .obj function length} over the matchers' identical rows (IDA's size stops
    before an in-function jump table; reccmp must compare up to the .obj length)."""
    out = {}
    for path in glob.glob(os.path.join(ROOT, "ida", "exports", "*-lib-*.csv")):
        with open(path) as f:
            for r in csv.DictReader(f):
                if r.get("target") == target and r.get("identical") == "yes" and r.get("size"):
                    a = int(r["address"], 16)
                    out[a] = max(out.get(a, 0), int(r["size"]))
    return out




def data_rows(orig, exe, pairs, view, code, kept, skip, extra=None):
    """What the paired library functions reference, paired by position. For each (orig, recomp, length)
    whose instruction streams agree (same length and mnemonic per instruction), every base relocation of
    our image inside the recompiled function names a PDB entity, and the 4 bytes at the same offset of
    the original are its original address; every call/jmp rel32 gives a callee pair the same way, and
    every slot of a paired vftable gives a slot-function pair.
    Kept when every reference agrees on one address both ways. Function pairs only on original function
    starts (`kept`) that no other row pairs (`skip`: library rows, markers). Returns
    rows (address, type, size, symbol, name) for reccmp: data typed global/vtable/string/widechar/float,
    callees typed library unless the name says RBX (then untyped: a pairing that stays in the denominator)."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    rpe = pefile.PE(exe, fast_load=True)
    rpe.parse_data_directories([pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_BASERELOC"]])
    rbase, rimg = rpe.OPTIONAL_HEADER.ImageBase, rpe.get_memory_mapped_image()
    relocs = sorted(e.rva + rbase for blk in getattr(rpe, "DIRECTORY_ENTRY_BASERELOC", []) for e in blk.entries
                    if e.type == 3)
    # reccmp gives no type ('-') to a PUBLICS name without a proc or data record: code when the map
    # flags it 'f' (vtordisp/adjustor thunks), data otherwise (throw info __TI2?AVlua_exception@@,
    # static-local guards ??_B?1??zero@Vector3@G3D@@...).
    isfunc = lambda a: view[a][0] == "FUNCTION" or view[a][0] == "-" and view[a][1] and a in code
    starts = sorted(a for a, v in view.items() if v[0] != "-" or v[1])
    seen, anon_code = {}, set()
    kinds_data = ("DATA", "VTABLE", "STRING", "WIDECHAR", "FLOAT", "-")
    rtext = next((rbase + x.VirtualAddress, rbase + x.VirtualAddress + x.Misc_VirtualSize) for x in rpe.sections
                 if x.Name.rstrip(b"\0") == b".text")

    votes = {}

    def note(node, oa):
        seen.setdefault(node, set()).add(oa)
        votes[node] = votes.get(node, 0) + 1
        votes[node, oa] = votes.get((node, oa), 0) + 1

    for p in pairs:
        o, r, n = p[:3]
        if len(p) > 3 and p[3] != "100":  # (o, r, n, our next function): ours must end where the original's does
            if p[3] is None or p[3] < r + n or any(b not in (0xCC, 0x90) for b in rimg[r + n - rbase:p[3] - rbase]):
                continue
        ob, rb = orig.b(o, n), rimg[r - rbase:r - rbase + n]
        oins, rins = list(md.disasm(ob, 0)), list(md.disasm(rb, 0))
        if [(i.address, i.size, i.mnemonic) for i in oins] != [(i.address, i.size, i.mnemonic) for i in rins] \
                or sum(i.size for i in oins) != n:
            continue
        for i in rins:
            if i.size == 5 and rb[i.address] in (0xE8, 0xE9):
                rel = lambda b: struct.unpack_from("<i", b, i.address + 1)[0] + i.address + 5
                tr = r + rel(rb)
                if tr in view and isfunc(tr):
                    note(tr, o + rel(ob))
        for k in relocs[bisect.bisect_left(relocs, r):bisect.bisect_left(relocs, r + n - 3)]:
            vr = struct.unpack_from("<I", rimg, k - rbase)[0]
            vo = struct.unpack_from("<I", ob, k - r)[0]
            if vr not in view and rtext[0] <= vr < rtext[1] and orig.in_text(vo) and vo not in kept:
                anon_code.add(vo)  # e.g. a /GS __ehhandler$ stub: code with no PDB symbol (see below)
                continue
            j = bisect.bisect_right(starts, vr) - 1
            if j < 0:
                continue
            node = starts[j]
            t, d, nm = view[node]
            off = vr - node
            if off and (t not in ("DATA", "-") or node in code) or \
                    (d or "").startswith(("__imp_", "__real@")) and t != "FLOAT":
                continue
            if t == "FUNCTION" and off == 0 or t == "-" and node in code:
                pass  # function pointer (push offset f, vftable slot)
            elif t not in kinds_data:
                continue
            note(node, vo - off)
    for node, oas in (extra or {}).items():  # aligned_refs
        for oa in oas:
            note(node, oa)
    # vftable slots (RTTI class + slot index): a vftable paired above pairs its slot functions, up to
    # the first slot that is not code in our image.
    for node, origs in list(seen.items()):
        if view[node][0] != "VTABLE" or len(origs) != 1:
            continue
        o = next(iter(origs))
        for i in range(0x400):
            sr = struct.unpack_from("<I", rimg, node - rbase + 4 * i)[0]
            if sr not in view or not isfunc(sr) or (i and node + 4 * i in view):
                break
            note(sr, orig.u32(o + 4 * i))
    claimed = {}
    for node, origs in seen.items():
        if len(origs) > 1:
            # one of ours named by several originals: a code difference at the minority's call sites
            # (e.g. ours calls logic_error(string) where the original copy-constructs). Most votes win.
            top = sorted(origs, key=lambda oa: -votes[node, oa])
            origs = top[:1] if votes[node, top[0]] > votes[node, top[1]] else origs
        if len(origs) == 1:
            claimed.setdefault(next(iter(origs)), []).append(node)
    kinds = {"DATA": "global", "VTABLE": "vtable", "STRING": "string", "WIDECHAR": "widechar", "FLOAT": "float",
             "-": "global"}
    rows = []
    for oa, nodes in sorted(claimed.items()):
        if len(nodes) > 1:
            # Several of our bodies for one original: the original's linker folded identical bodies (ICF)
            # that ours keeps apart. reccmp pairs one; take the one most references name, if any.
            nodes = sorted(nodes, key=lambda x: -votes[x])
            nodes = nodes[:1] if votes[nodes[0]] > votes[nodes[1]] else nodes
        if len(nodes) != 1 or not orig.base <= oa < orig.base + len(orig.img):
            continue
        t, d, nm = view[nodes[0]]
        if isfunc(nodes[0]):
            if oa not in kept or oa in skip:
                continue
            # A library function's callees are library code (third party, STL, CRT helpers such as
            # __ftol2_sse or atexit destructors of library statics) unless Roblox code is named.
            roblox_callee = "RBX" in (d or "") + (nm or "")
            rows.append((f"{oa:08x}", "" if roblox_callee else "library", "", d or "", "" if d else nm))
            continue
        typ = kinds[t]
        if d in DATACMP_CRASH:
            continue
        # Strings and floats are paired by value (match_strings, float analysis): address and type only;
        # reccmp reads a float only with size 4 or 8, which the __real@<hex> name gives.
        sym = d if typ in ("global", "vtable") else ""
        name = nm if typ in ("global", "vtable") and not d else ""
        size = len(d) // 2 - 3 if typ == "float" and d and d.startswith("__real@") else ""
        if typ == "vtable":
            # reccmp sizes our vftable by the gap to our next symbol and reads the original with that size
            # unless the row gives one, so a padding null of ours reads the original's next object (the
            # next vftable's Complete Object Locator) as a slot (G3D::Plane: 1 slot, 50%). The size is the
            # original's run of code pointers, capped at our gap: past our gap the original may hold other
            # code pointers (boost::function's static manager/invoker pairs follow sp_counted_impl_pd's
            # vftable directly). ponytail: the cap hides a trailing virtual ours lacks, as before.
            gap = (starts[bisect.bisect_right(starts, nodes[0])] - nodes[0]) // 4 \
                if bisect.bisect_right(starts, nodes[0]) < len(starts) else 0x400
            n = 0
            while n < min(gap, 0x400) and orig.in_text(orig.u32(oa + 4 * n)):
                n += 1
            size = 4 * n if n < gap else ""
        rows.append((f"{oa:08x}", typ, size, sym, name))
    # Code our image references through a relocation but the PDB does not name (the __ehhandler$ stub a
    # /GS function pushes in its prologue: reccmp's find_eh_handlers labels only the `mov eax, FuncInfo`
    # inside it). Ours prints as an <OFFSETn> placeholder; a nameless 1-byte entity at the original
    # address makes reccmp treat the original's immediate as an address too, so both print the same
    # placeholder. Typed global so it never enters the function denominator.
    taken = {int(r[0], 16) for r in rows}
    rows += [(f"{a:08x}", "global", 1, "", "") for a in sorted(anon_code - taken)]
    return sorted(rows)


def aligned_refs(orig, exe, json_path, view):
    """{our node: [original address, ...]} from the reference lines a reccmp --json run shows as changed:
    in a replaced block with as many original as recompiled lines, a line pair counts only when both
    instructions sit at the same offset from their function's start and decode to the same mnemonic and
    length (an aligned instruction; nothing is taken from a misaligned one). Each absolute operand
    (call/jmp target, immediate, or base-less memory displacement) of ours that lands on a PDB entity
    names the original's operand at the same position."""
    import capstone
    import json
    from capstone import x86
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    rpe = pefile.PE(exe, fast_load=True)
    rbase, rimg = rpe.OPTIONAL_HEADER.ImageBase, rpe.get_memory_mapped_image()
    starts = sorted(a for a, v in view.items() if v[0] != "-" or v[1])

    def absops(ins):
        out = []
        for op in ins.operands:
            if op.type == x86.X86_OP_IMM and op.imm >= 0x400000:
                out.append(op.imm & 0xFFFFFFFF)
            elif op.type == x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0 and op.mem.disp >= 0x400000:
                out.append(op.mem.disp & 0xFFFFFFFF)
            else:
                out.append(None)
        return out

    refs = {}
    with open(json_path) as f:
        data = json.load(f)["data"]
    for e in data:
        if not e.get("diff") or not e.get("recomp") or e.get("type", 1) != 1:
            continue
        o0, r0 = int(e["address"], 16), int(e["recomp"], 16)
        for _, blocks in e["diff"]:
            for b in blocks:
                bo, br = b.get("orig") or [], b.get("recomp") or []
                if not bo or len(bo) != len(br):
                    continue
                for x, y in zip(bo, br):
                    ox, ry = int(x[0], 16), int(y[0], 16)
                    if ox - o0 != ry - r0:
                        continue
                    oi = next(md.disasm(orig.b(ox, 16), ox), None)
                    ri = next(md.disasm(rimg[ry - rbase:ry - rbase + 16], ry), None)
                    if not oi or not ri or oi.mnemonic != ri.mnemonic or oi.size != ri.size:
                        continue
                    for vo, vr in zip(absops(oi), absops(ri)):
                        if vo is None or vr is None or vo == vr:
                            continue
                        j = bisect.bisect_right(starts, vr) - 1
                        if j < 0:
                            continue
                        node = starts[j]
                        t, d, _ = view[node]
                        off = vr - node
                        if off and t not in ("DATA", "-") or (d or "").startswith(("__imp_", "__real@")):
                            continue
                        refs.setdefault(node, []).append(vo - off)
    return refs


def funcinfo_targets(im):
    """Unwind actions and catch handlers named by every FuncInfo in the non-code sections."""
    lo_t, hi_t = im.text
    ok = lambda a: lo_t <= a < hi_t
    inimg = lambda a: im.base <= a < im.base + len(im.img) - 16
    out = set()
    count = 0
    for name, lo, hi in im.sec:
        if name == ".text" or name.startswith(".vmp"):
            continue
        for a in range(lo, hi - 20, 4):
            magic, nstate, punwind, ntry, ptry = struct.unpack_from("<5I", im.img, a - im.base)
            if magic not in FUNCINFO_MAGIC or nstate > 0x10000 or ntry > 0x1000:
                continue
            if (nstate and not inimg(punwind + 8 * nstate)) or (ntry and not inimg(ptry + 20 * ntry)):
                continue
            acts = [im.u32(punwind + 8 * i + 4) for i in range(nstate)]
            handlers = []
            for t in range(ntry):
                ncatch, parr = struct.unpack_from("<2I", im.img, ptry + 20 * t + 12 - im.base)
                if ncatch > 0x100 or not inimg(parr + 16 * ncatch):
                    break
                handlers += [im.u32(parr + 16 * c + 12) for c in range(ncatch)]
            targets = [x for x in acts + handlers if x]
            if all(ok(x) for x in targets):
                count += 1
                out.update(targets)
    return out, count


def adjustor_thunk(body):
    return any(len(body) == n and rx.match(body) for rx, n in ADJUSTOR)


def vmp_functions(starts, sizes, path):
    """Functions containing a VMProtect region (notes/vmprotect-player.csv addresses)."""
    hit = set()
    with open(os.path.join(ROOT, path)) as f:
        for r in csv.DictReader(f):
            a = int(r["address"], 16)
            i = bisect.bisect_right(starts, a) - 1
            if i >= 0 and a < starts[i] + max(sizes[i], 1):
                hit.add(starts[i])
    return hit


def marked(target):
    """Original addresses a source marker owns (Client/, including the generated link stubs): those
    pair through the marker, so data_rows must not add a library row for them."""
    mk = re.compile(r"//\s*(?:FUNCTION|STUB|TEMPLATE|SYNTHETIC|LIBRARY):\s*" + target + r"\s+0x([0-9a-fA-F]+)")
    out = set()
    for dirpath, _, files in os.walk(os.path.join(ROOT, "Client")):
        for fn in files:
            if fn.endswith((".cpp", ".c", ".h", ".hpp", ".inl")):
                with open(os.path.join(dirpath, fn), errors="replace") as f:
                    out.update(int(m.group(1), 16) for m in mk.finditer(f.read()))
    return out


def writer(path, header, comments=()):
    f = open(path, "w", newline="")
    for c in comments:
        f.write(f"# {c}\n")
    # reccmp's reader: delimiter sniffed from the header, backslash escapes, no doubled quotes.
    w = csv.writer(f, lineterminator="\n", doublequote=False, escapechar="\\")
    w.writerow(header)
    return f, w


def gen(target, build=BUILD, json_path=None):
    exe, vmp_csv = TARGETS[target]
    t = target.lower()
    exports = os.path.join(ROOT, "ida", "exports")
    im = Img(os.path.join(ROOT, exe), os.path.join(exports, f"{target}-functions.csv"))
    with open(os.path.join(exports, f"{target}-functions.csv")) as f:
        funcs = sorted((int(r["address"], 16), int(r["size"]), r["name"]) for r in csv.DictReader(f))
    starts = [a for a, _, _ in funcs]
    sizes = [s for _, s, _ in funcs]

    slots = orig_imports(im)[0] if target == "PLAYER" else {}
    im.pe.parse_data_directories([pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
    iat = set(slots) | {imp.address for d in getattr(im.pe, "DIRECTORY_ENTRY_IMPORT", []) for imp in d.imports}
    eh, nfuncinfo = funcinfo_targets(im)
    vmp = vmp_functions(starts, sizes, vmp_csv) if vmp_csv else set()

    drop = {"outside .text": 0, "VMProtect-touched": 0, "unwind/catch funclet": 0, "import thunk": 0,
            "adjustor thunk": 0}
    keep = []
    for a, size, _ in funcs:
        body = im.b(a, min(size, 16))
        if not im.in_text(a):
            drop["outside .text"] += 1
        elif a in vmp:
            drop["VMProtect-touched"] += 1
        elif a in eh:
            drop["unwind/catch funclet"] += 1
        elif size == 6 and body[:2] == b"\xff\x25" and struct.unpack_from("<I", body, 2)[0] in iat:
            drop["import thunk"] += 1
        elif adjustor_thunk(body[:size]):
            drop["adjustor thunk"] += 1
        else:
            keep.append((a, size))
    out = {}
    path = os.path.join(ROOT, "reccmp", f"{t}-function-size.csv")
    notes = [f"Generated by tools/ida/gen_reccmp_csv.py from ida/exports/{target}-functions.csv. Do not edit.",
             f"{len(funcs)} IDA functions, {len(keep)} kept. Dropped: "
             + ", ".join(f"{k} {v}" for k, v in drop.items()) + ".",
             f"FuncInfo records parsed: {nfuncinfo} ({len(eh)} unwind/catch targets)."]
    f, w = writer(path, ["address", "type", "size"], notes)
    with f:
        w.writerows((f"{a:08x}", "function", size) for a, size in keep)
    out[path] = len(keep)

    if slots:
        path = os.path.join(ROOT, "reccmp", f"{t}-imports.csv")
        f, w = writer(path, ["address", "type", "size", "name"],
                      ["The linker's IAT that .text calls through (tools/vmprotect_scope.py orig_imports).",
                       "Type 'import' needs tools/reccmp-patches/ applied to the venv."])
        with f:
            for a, (dll, name) in sorted(slots.items()):
                name = f"Ordinal_{name[1:]}" if name.startswith("#") else name
                w.writerow((f"{a:08x}", "import", 4, f"{dll}::{name}"))
        out[path] = len(slots)

    kept = {a for a, _ in keep}
    size_of = dict((a, s) for a, s, _ in funcs)
    # Third-party library matches, one row per address (tools/lib/lib_consistency.py resolved the
    # library CSVs: status identical = byte-identical compiled body, no conflict). They win over an
    # oracle or FLIRT name at the same address.
    libs, nolib = {}, {"not identical": 0, "not a kept function": 0}
    recomp = os.path.join(build, os.path.basename(exe))
    mapfile = recomp[:-4] + ".map"
    have_build = os.path.exists(mapfile) and os.path.exists(recomp[:-4] + ".pdb")
    addr, anon, code = read_map(mapfile) if have_build else ({}, {}, set())
    view = pdb_view(recomp) if have_build else {}
    objlen = obj_sizes(target)
    pairing = {"public symbol": 0, "static name": 0, "not in our link": 0, "folded in our link": 0}
    pairs, used = [], set()
    resolved = os.path.join(exports, f"{target}-lib-resolved.csv")
    if os.path.exists(resolved):
        with open(resolved) as f:
            rs = list(csv.DictReader(f))
        for r in rs:
            a = int(r["address"], 16)
            if r["status"] != "identical":
                nolib["not identical"] += 1
                continue
            if a not in kept:
                nolib["not a kept function"] += 1
                continue
            sym, name = r["name"], r["demangled"] or r["name"]
            size = max(size_of[a], objlen.get(a, 0))
            if have_build:
                ra = resolve_symbol([sym] + [c for c in r["names"].split("|") if c and c != sym], addr, anon)
                vt, d, nm = view.get(ra, (None, None, None)) if ra else (None, None, None)
                # vtordisp/adjustor thunks ($4PPPPPPPM@, $0PPPPPPPM@) have a PUBLICS name but no proc
                # record, so reccmp gives them no type; match_symbols pairs them all the same.
                if ra is None or not (vt == "FUNCTION" or vt == "-" and d):
                    pairing["not in our link"] += 1
                elif ra in used:
                    pairing["folded in our link"] += 1  # another row already pairs this recompiled address
                else:
                    used.add(ra)
                    pairs.append((a, ra, objlen.get(a, 0)))
                    if d:
                        if d != sym:
                            sym, name = d, d  # the name our linker kept for a folded body
                        pairing["public symbol"] += 1
                    else:
                        sym, name = "", nm  # a static: no PUBLICS entry, paired by its PDB proc name
                        pairing["static name"] += 1
            libs.setdefault(LIBFILE(r["library"]), []).append((f"{a:08x}", "library", size, sym, name))
    claimed = {int(row[0], 16) for rows in libs.values() for row in rows}
    names = {"symbols": [], "crt": [], "stl": [], "atl": []}
    oracle = {}
    with open(os.path.join(exports, f"{target}-names.csv")) as f:
        for r in csv.DictReader(f):
            a = int(r["address"], 16)
            if r["kind"] == "func" and r["source"] in ("ios:", "mac:", "prior:"):
                oracle[a] = (r["demangled"] or r["name"], r["name"])
            if r["kind"] != "func" or a not in kept or a in claimed:
                continue
            if r["source"] in ("ios:", "mac:", "prior:"):
                names["symbols"].append((f"{a:08x}", r["demangled"] or r["name"]))
            elif r["source"] == "flirt:":
                n = r["name"]
                lib = "atl" if "ATL@@" in n else "stl" if "@std@@" in n else "crt"
                names[lib].append((f"{a:08x}", "library", size_of[a], n))
    mk = marked(target)
    named, nstats = {}, {}
    if have_build and not os.environ.get("OGRE_NO_SIGPAIR"):
        named, nstats = oracle_pairs(oracle, view, kept - claimed - mk, used)
    datarows = []
    if pairs or json_path:
        # Pairs for the position method: the library rows, plus (with --json) every function the last
        # reccmp run paired whose instruction stream agrees with ours over the whole body, our body ending
        # where the original's does (only alignment padding up to our next function).
        jpairs = []
        if json_path:
            import json
            nxt = sorted(code | {a for a, v in view.items() if v[0] == "FUNCTION"})
            got = {}
            for jp in json_path.split(","):
                with open(jp) as f:
                    for e in json.load(f)["data"]:
                        if e.get("recomp") and e.get("type", 1) == 1 and int(e["address"], 16) in size_of:
                            o, r = int(e["address"], 16), int(e["recomp"], 16)
                            # a 100% function needs no end check: reccmp compared the whole body
                            full = e["matching"] >= 1.0 or e.get("effective")
                            if full or (o, r) not in got:
                                j = bisect.bisect_right(nxt, r)
                                got[o, r] = "100" if full else nxt[j] if j < len(nxt) else None
            jpairs = [(o, r, size_of[o], end) for (o, r), end in sorted(got.items())]
        skip = mk | {int(row[0], 16) for rows in libs.values() for row in rows}
        if os.environ.get("OGRE_NO_SIGPAIR"):  # the method before signature pairing, for A/B scoring
            skip |= {a for a, (d, _) in oracle.items() if "RBX" in d}
        datarows = data_rows(im, recomp, [p for p in pairs if p[2]], view, code, kept, skip)
        if jpairs:
            # The library pairs (byte-identical bodies) decide first; the --json pairs only add addresses
            # and symbols the library pass left open. An original the linker folded (ICF) is claimed by
            # several of our bodies once more callers vote, and would otherwise lose its library row.
            more = data_rows(im, recomp, [p for p in pairs if p[2]] + jpairs, view, code, kept, skip)
            have = {r[0] for r in datarows} | {r[3] for r in datarows if r[3]}
            datarows = sorted(datarows + [r for r in more if r[0] not in have and not (r[3] and r[3] in have)])
            # last, the aligned reference lines of the functions that pass did not take whole
            refs = {}
            for jp in json_path.split(","):
                for node, oas in aligned_refs(im, recomp, jp, view).items():
                    refs.setdefault(node, []).extend(oas)
            more = data_rows(im, recomp, [p for p in pairs if p[2]] + jpairs, view, code, kept, skip, refs)
            have = {r[0] for r in datarows} | {r[3] for r in datarows if r[3]}
            datarows = sorted(datarows + [r for r in more if r[0] not in have and not (r[3] and r[3] in have)])
        # A signature pair that a position pair contradicts (same original, other symbol, or the same
        # symbol on another original) is dropped and the position pair kept: the original's linker folded
        # identical bodies (ICF) and the oracle names one of them, while the position pair names the body
        # our code calls (e.g. boost::unique_lock<mutex>::lock vs <recursive_mutex>::lock).
        pos = {int(r[0], 16): r[3] for r in datarows if r[1] in ("library", "") and r[3]}
        bysym = {}
        for a, sym in pos.items():
            bysym.setdefault(sym, set()).add(a)
        bad = {a for a, (sym, _) in named.items() if sym and (a in pos and pos[a] != sym or
                                                               bysym.get(sym, {a}) != {a})}
        nstats["dropped: a position pair contradicts it"] = len(bad)
        for a in sorted(bad):
            print(f"  {target} {a:08x}: signature pairs {named[a][0]}, position says "
                  f"{pos.get(a) or sorted(f'{x:08x}' for x in bysym.get(named[a][0], ()))}")
        named = {a: v for a, v in named.items() if a not in bad}
    names["symbols"] = [(a, n, named[int(a, 16)][0] if int(a, 16) in named else "") for a, n in names["symbols"]]
    # a static pairs by our proc name (match_functions): the row's name is ours
    names["symbols"] = [(a, named[int(a, 16)][1] if int(a, 16) in named and not sym else n, sym)
                        for a, n, sym in names["symbols"]]
    for lib, rows in names.items():
        path = os.path.join(ROOT, "reccmp", f"{t}-{lib}.csv")
        hdr = ["address", "name", "symbol"] if lib == "symbols" else ["address", "type", "size", "symbol"]
        notes2 = [f"Generated from ida/exports/{target}-names.csv. Do not edit."]
        if lib == "symbols" and nstats:
            notes2.append("symbol: our PUBLICS name for an oracle name whose signature (norm_sig) is unique on "
                          "both sides; a static carries our proc name instead.")
            notes2.append("Signature pairing: " + ", ".join(f"{k} {v}" for k, v in sorted(nstats.items())) + ".")
        f, w = writer(path, hdr, notes2)
        with f:
            w.writerows(rows)
        out[path] = len(rows)
    if nstats:
        notes.append("Signature pairing: " + ", ".join(f"{k} {v}" for k, v in sorted(nstats.items())) + ".")

    if datarows:
        rows = datarows
        path = os.path.join(ROOT, "reccmp", f"{t}-libdata.csv")
        f, w = writer(path, ["address", "type", "size", "symbol", "name"],
                      ["Generated by tools/ida/gen_reccmp_csv.py: data the paired functions reference,",
                       f"paired by relocation position against {os.path.relpath(recomp, ROOT)}. Do not edit.",
                       "Position sources: library rows" + (" + --json " + ",".join(
                           os.path.relpath(j, ROOT) for j in json_path.split(",")) if json_path else "") + "."])
        with f:
            w.writerows(r for r in rows if r[1] in ("library", "") or int(r[0], 16) not in kept)
        out[path] = len(rows)

    for lib, rows in sorted(libs.items()):
        path = os.path.join(ROOT, "reccmp", f"{t}-{lib}.csv")
        f, w = writer(path, ["address", "type", "size", "symbol", "name"],
                      [f"Generated from ida/exports/{target}-lib-resolved.csv (status identical). Do not edit.",
                       f"Library rows skipped for all libraries: " + ", ".join(f"{k} {v}" for k, v in nolib.items()) + ".",
                       f"Resolved against {os.path.relpath(recomp, ROOT)} (map + PDB) for all libraries: "
                       + (", ".join(f"{k} {v}" for k, v in pairing.items()) if have_build else "no build") + "."])
        with f:
            w.writerows(rows)
        out[path] = len(rows)
    return notes, out


def listed_sources():
    with open(os.path.join(ROOT, "reccmp-project.yml")) as f:
        return set(re.findall(r"^\s*-\s*(reccmp/\S+\.csv)\s*$", f.read(), flags=re.M))


def selftest():
    assert adjustor_thunk(b"\x83\xe9\x04\xe9\x00\x00\x00\x00")
    assert adjustor_thunk(b"\x83\x6c\x24\x04\x08\xe9\x00\x00\x00\x00")
    assert not adjustor_thunk(b"\x83\xe9\x04\xe9\x00\x00\x00\x00\xcc")
    assert not adjustor_thunk(b"\x8b\xff\x55\x8b\xec\x00\x00\x00")
    addr, anon = {"?g@?A0x12@@YAXXZ": 0x401000}, {"?g@?A0x@@YAXXZ": ["?g@?A0x12@@YAXXZ"]}  # noqa
    assert resolve_symbol(["?h@@YAXXZ", "?g@?A0x99@@YAXXZ"], addr, anon) == 0x401000
    a = norm_sig("RBX::Face::minMax(class G3D::Vector3 const &, class G3D::Vector3 const &, float &, float &) const")
    assert a == norm_sig("RBX::Face::minMax(G3D::Vector3 const&, G3D::Vector3 const&, float&, float&) const")
    assert norm_sig("RBX::VerbContainer::getVerb(class std::basic_string<char, struct std::char_traits<char>, "
                    "class std::allocator<char>> const &)") == norm_sig("RBX::VerbContainer::getVerb(std::string const&)")
    assert norm_sig("RBX::Name::doDeclare<class RBX::sMotor6D>(void)") == \
        norm_sig("RBX::Name const& RBX::Name::doDeclare<RBX::sMotor6D>()")
    assert norm_sig("RakNet::BitStream & RBX::operator>><unsigned long>(RakNet::BitStream&, unsigned long&)") == \
        norm_sig("RBX::operator>><unsigned int>(class RakNet::BitStream &, unsigned int &)")
    assert norm_sig("{anon}::f(char const *const)") == norm_sig("(anonymous namespace)::f(char const*)")
    assert norm_sig("RBX::Foo::operator<(int)") != norm_sig("RBX::Foo::operator<<(int)")
    assert norm_sig("RBX::Foo::`vftable'") is None
    assert norm_sig("gpc_polygon_clip") == "C:gpc_polygon_clip"
    assert norm_sig("RBX::X::~X() [D0, deleting: the MSVC scalar deleting destructor]") == "RBX::X::{sdd}()"
    assert qual("RBX::A<RBX::B>::f(int)const") == "RBX::A<RBX::B>::f"
    assert norm_sig("void * RBX::FWBase::`scalar deleting dtor'(unsigned int)") == \
        norm_sig("RBX::FWBase::~FWBase()[D0, deleting: the MSVC scalar deleting destructor]")
    assert norm_sig("void RBX::Instance::writeXml(class boost::function<bool (class RBX::Instance *)> const &)") \
        == norm_sig("RBX::Instance::writeXml(boost::function<bool (RBX::Instance*)> const&)")
    print("selftest ok")


def main(argv):
    if "--selftest" in argv:
        return selftest()
    listed = listed_sources()
    build = BUILD
    if "--build" in argv:
        i = argv.index("--build")
        build = os.path.abspath(argv[i + 1])
        argv = argv[:i] + argv[i + 2:]
    json_path = None
    if "--json" in argv:
        i = argv.index("--json")
        json_path = ",".join(os.path.abspath(p) for p in argv[i + 1].split(","))
        argv = argv[:i] + argv[i + 2:]
        assert len(argv) == 1, "--json goes with one target"
    for target in argv or list(TARGETS):
        notes, out = gen(target, build, json_path)
        print(target + ": " + " ".join(notes[1:]))
        for path, n in out.items():
            rel = os.path.relpath(path, ROOT)
            flag = "" if rel in listed else "   <- not listed in reccmp-project.yml"
            print(f"  {rel}: {n} rows{flag}")


if __name__ == "__main__":
    main(sys.argv[1:])

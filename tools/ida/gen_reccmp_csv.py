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
  <t>-function-size.csv  address,type,size   every function extent IDA found in .text (true_size extends it over
                         an in-function jump table or a tail IDA cut at a call it took for noreturn), minus:
                         unwind funclets and catch handlers (from every __CxxFrameHandler3 FuncInfo,
                         magic 0x19930520-22), adjustor thunks and import thunks (by their bytes),
                         functions VMProtect touched (notes/vmprotect-player.csv) and anything outside
                         .text (.vmp0/.vmp1). The counts are written as '#' comments at the top.
  <t>-imports.csv        address,type,size,name   PLAYER only: the linker's IAT in .rdata (0x00aa1000-) that
                         .text calls through, typed 'import'. reccmp reads VMProtect's table in .vmp1
                         instead; the 'import' type needs tools/reccmp-patches/ (see that README).
  <t>-symbols.csv        address,name,symbol,type   demangled ios:/mac:/prior: names, no type (stay in the denominator).
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
Rules that keep a regeneration from undoing the last one (2026-11-12):
  - Twins: a reccmp --json body pair at 100% counts as 1000 votes; ties go to a signature or match: name,
    then to the lowest address. A match: name (a pairing a reccmp run proved at 100%) beats a position or
    signature pair that contradicts it, unless that pair names an identical twin of the proven body.
  - match:/lib:/rtti:/str: names (our own MSVC names) pair through <t>-symbols.csv (symbol column, or the
    name alone for a C function or static); <t>-symbols.csv has a type column for them.
  - <t>-libdata.csv keeps every row of the previous file that still resolves and nothing contradicts.
  - Vtable rows carry the original's slot count (vt_slots), also for `// VTABLE:` markers.
  - Every vftable a --json run paired votes its slot functions (vt_slot_refs, one vote per slot).
  - A float our linker ICF-folded with a named constant takes its size from the map's __real@ alias.
  - A library row whose size runs over an in-function jump table is read up to the first instruction
    that differs (the table), and its own table is never a nameless code row.
  - Pool only --json runs of the build being regenerated against: a run from an earlier link pairs
    stale recompiled addresses (2026-11-13: VerbContainer D0 0x006f9250 got Assembly::onLowersChanged).
  - A non-STUB marker reccmp could not pair (its log <json>.txt says 'Debug data out of sync') does not
    keep data_rows off its address.
  - Typing: an oracle-named original is library only in a third-party namespace (third_party); otherwise
    library unless the name mentions rbx/RBX.
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


# ClassDescriptor-typed statics (ClassDescriptor::rootDescriptor's `root`, the `describedClassDescriptor`
# static of each Described<...>::classDescriptor()) make reccmp-datacmp die (KeyError in
# reccmp/cvdump/types.py). They stay paired, since the classDescriptor() functions and their atexit
# destructors need them at 100%, and reccmp-project.yml lists their names under report:
# ignore-variables, which datacmp skips.


THIRD_PARTY = ("std::", "stdext::", "boost::", "G3D::", "RakNet::", "DataStructures::", "ATL::", "C:gpc_")


def third_party(demangled):
    """An oracle-named function is library code when its qualified name is in a third-party namespace
    (std::, boost::, G3D::, RakNet::, ...) and no Roblox type is involved."""
    return not roblox(demangled) and qual(norm_sig(demangled) or demangled).startswith(THIRD_PARTY)


def roblox(name):
    """Roblox code (RBX:: and the lowercase rbx:: namespace) is never typed library."""
    return "rbx" in (name or "").lower()


def code_imm(text, value):
    """Whether .text holds `value` as the imm32 of push / mov reg / mov r/m32 (opcode 68, B8-BF, C7 with
    a modrm and no, 1 or 4 displacement bytes): a byte search alone also hits instruction tails."""
    pat, k = struct.pack("<I", value), -1
    while (k := text.find(pat, k + 1)) >= 0:
        if k and (text[k - 1] == 0x68 or 0xB8 <= text[k - 1] <= 0xBF) or \
                any(k >= d and text[k - d] == 0xC7 for d in (2, 3, 6)):
            return True
    return False


def vt_slots(orig, oa, labels):
    """The original vftable's slot count: its run of .text pointers, ending before the next labelled
    original object (labels: sorted addresses of the IDA data labels and of every data row; the next
    vftable's Complete Object Locator is one) or the first one code addresses, at most 0x400."""
    j = bisect.bisect_right(labels, oa)
    lim = labels[j] if j < len(labels) else oa + 0x1000
    if not hasattr(orig, "text_bytes"):
        orig.text_bytes = bytes(orig.img[orig.text[0] - orig.base:orig.text[1] - orig.base])
    n = 0
    while n < 0x400 and oa + 4 * n < lim and orig.in_text(orig.u32(oa + 4 * n)):
        # a slot is never addressed by code; an object after the vftable that has no label may be
        # (boost::function's stored_vtable {manager, invoker}, stored as `address | 1`)
        if n and any(code_imm(orig.text_bytes, oa + 4 * n + t) for t in (0, 1)):
            break
        n += 1
    return n


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




def data_rows(orig, exe, pairs, view, code, kept, skip, extra=None, labels=(), prefer=None, oracle_lib=None):
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
    reals = {}
    for n, ra in read_map(exe[:-4] + ".map")[0].items():
        if n.startswith("__real@"):
            reals[ra] = len(n) // 2 - 3

    def note(node, oa, weight=1):
        seen.setdefault(node, set()).add(oa)
        votes[node] = votes.get(node, 0) + weight
        votes[node, oa] = votes.get((node, oa), 0) + weight

    for p in pairs:
        o, r, n = p[:3]
        if len(p) > 3 and p[3] == "100" and r in view and isfunc(r):
            # the reccmp run paired these two whole bodies at 100%: stronger than any number of call sites
            note(r, o, 1000)
        if len(p) > 3 and p[3] != "100":  # (o, r, n, our next function): ours must end where the original's does
            if p[3] is None or p[3] < r + n or any(b not in (0xCC, 0x90) for b in rimg[r + n - rbase:p[3] - rbase]):
                continue
        ob, rb = orig.b(o, n), rimg[r - rbase:r - rbase + n]
        oins, rins = list(md.disasm(ob, 0)), list(md.disasm(rb, 0))
        ok, ek = [(i.address, i.size, i.mnemonic) for i in oins], [(i.address, i.size, i.mnemonic) for i in rins]
        if len(p) == 3 and ok != ek:
            # a byte-identical library body (matcher row) whose size runs over an in-function jump table:
            # the tables' dwords differ, so read only the code before the first disagreeing instruction
            # (inflate 0x009e22e0 and its static `order` table)
            k = next(k for k, (x, y) in enumerate(zip(ok + [None], ek + [None])) if x != y)
            n = ok[k - 1][0] + ok[k - 1][1] if k else 0
            oins, rins, ok, ek = oins[:k], rins[:k], ok[:k], ek[:k]
        if ok != ek or sum(i.size for i in oins) != n or not n:
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
                if len(p) > 3 or not o <= vo < o + p[2]:  # not a library function's own jump table (`jmp [table+eax*4]`)
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
            # Ties go to the lowest original address, so a regeneration from the same evidence always
            # makes the same choice (twin bodies the original kept apart).
            # A signature or match: name for one of them (prefer: {our symbol: original}) breaks a tie first.
            pref = (prefer or {}).get(view[node][1])
            origs = sorted(origs, key=lambda oa: (-votes[node, oa], oa != pref, oa))[:1]
        if len(origs) == 1:
            claimed.setdefault(next(iter(origs)), []).append(node)
    kinds = {"DATA": "global", "VTABLE": "vtable", "STRING": "string", "WIDECHAR": "widechar", "FLOAT": "float",
             "-": "global"}
    rows = []
    for oa, nodes in sorted(claimed.items()):
        if len(nodes) > 1:
            # Several of our bodies for one original: the original's linker folded identical bodies (ICF)
            # that ours keeps apart. reccmp pairs one; take the one most references name, if any.
            # A tie goes to our lowest address: deterministic for one build.
            nodes = sorted(nodes, key=lambda x: (-votes[x], (prefer or {}).get(view[x][1]) != oa, x))[:1]
        if len(nodes) != 1 or not orig.base <= oa < orig.base + len(orig.img):
            continue
        t, d, nm = view[nodes[0]]
        if isfunc(nodes[0]):
            if oa not in kept or oa in skip:
                continue
            # A library function's callees are library code (third party, STL, CRT helpers such as
            # __ftol2_sse or atexit destructors of library statics) unless Roblox code is named.
            # An original with an ios:/mac:/prior: name is typed by that name (oracle_lib: {orig: third party})
            lib = oracle_lib[oa] if oa in (oracle_lib or {}) else not roblox((d or "") + (nm or ""))
            rows.append((f"{oa:08x}", "library" if lib else "", "", d or "", "" if d else nm))
            continue
        typ = kinds[t]
        # Strings and floats are paired by value (match_strings, float analysis): address and type only;
        # reccmp reads a float only with size 4 or 8, which the __real@<hex> name gives.
        sym = d if typ in ("global", "vtable") else ""
        name = nm if typ in ("global", "vtable") and not d else ""
        size = len(d) // 2 - 3 if typ == "float" and d and d.startswith("__real@") else ""
        if typ == "float" and not size:  # ICF folded __real@ with a named constant: the map keeps the alias
            size = reals.get(nodes[0], "")
        if typ == "vtable":
            size = "vt"  # the original's slot count, set below once every data row is known
        rows.append((f"{oa:08x}", typ, size, sym, name))
    # Code our image references through a relocation but the PDB does not name (the __ehhandler$ stub a
    # /GS function pushes in its prologue: reccmp's find_eh_handlers labels only the `mov eax, FuncInfo`
    # inside it). Ours prints as an <OFFSETn> placeholder; a nameless 1-byte entity at the original
    # address makes reccmp treat the original's immediate as an address too, so both print the same
    # placeholder. Typed global so it never enters the function denominator.
    taken = {int(r[0], 16) for r in rows}
    rows += [(f"{a:08x}", "global", 1, "", "") for a in sorted(anon_code - taken)]
    # reccmp sizes our vftable by the gap to our next symbol and reads the original with the row's size
    # (else with ours): the original's own slot count shows a slot ours lacks or adds at the end.
    bounds = sorted(set(labels) | {int(r[0], 16) for r in rows})
    rows = [r[:2] + (4 * vt_slots(orig, int(r[0], 16), bounds) or "",) + r[3:] if r[2] == "vt" else r
            for r in rows]
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


def true_size(im, a, size, nxt):
    """IDA's extent of the function at a, extended to what the original's .obj holds (bounded by nxt, the
    next IDA function start): (1) over the in-function jump table right after it (4-aligned dwords that
    point into the function; IDA stops before the table, e.g. Math::matrixTiltQuadrant 0x00758320 245 ->
    264); (2) over the fall-through tail IDA cut off after a call it took for noreturn (the bytes up to
    nxt, minus int3 padding, decode to code ending in ret; e.g. Bridge<T>::on_newindex 0x00670170, cut
    at its call to the thrower, really 0x30)."""
    import capstone
    end = a + size
    p = (end + 3) & ~3
    q = p
    while q + 4 <= nxt and a <= im.u32(q) < end:
        q += 4
    if q > p:
        return q - a
    tail = im.b(end, max(nxt - end, 0)).rstrip(b"\xcc")
    if a & 15 or not tail or len(tail) > 64 or b"\xcc" in tail:  # int3 inside: padding before code IDA did not define
        return size
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    body = list(md.disasm(im.b(a, size), a))
    ins = list(md.disasm(tail, end))
    if not body or body[-1].mnemonic != "call" or sum(i.size for i in body) != size \
            or sum(i.size for i in ins) != len(tail) or ins[-1].mnemonic != "ret":
        return size
    return end + len(tail) - a


def adjustor_thunk(body):
    return any(len(body) == n and rx.match(body) for rx, n in ADJUSTOR)


def same_body(orig, a, b, size_of):
    """Two original functions with the same instructions (call/jmp rel32 compared by target)."""
    import capstone
    n = size_of.get(a)
    if not n or n != size_of.get(b):
        return False
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    # a branch inside the body (a recursive call included) compares by its offset from the start

    def key(x):
        out = []
        for i in md.disasm(orig.b(x, n), x):
            t = int(i.op_str, 16) if re.fullmatch(r"0x[0-9a-f]+", i.op_str) else None
            out.append((i.mnemonic, "+%x" % (t - x) if t is not None and x <= t < x + n else i.op_str))
        return out
    return key(a) == key(b)


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


def marked(target, kinds=("FUNCTION", "STUB", "TEMPLATE", "SYNTHETIC", "LIBRARY")):
    """Original addresses a source marker owns (Client/, including the generated link stubs): those
    pair through the marker, so data_rows must not add a library row for them."""
    mk = re.compile(r"//\s*(?:" + "|".join(kinds) + r"):\s*" + target + r"\s+0x([0-9a-fA-F]+)")
    out = set()
    for dirpath, _, files in os.walk(os.path.join(ROOT, "Client")):
        for fn in files:
            if fn.endswith((".cpp", ".c", ".h", ".hpp", ".inl")):
                with open(os.path.join(dirpath, fn), errors="replace") as f:
                    out.update(int(m.group(1), 16) for m in mk.finditer(f.read()))
    return out


def vt_slot_refs(orig, exe, json_path, view, code):
    """{our function node: [original address, ...]} from the vftables a reccmp --json run paired (type 5
    rows: a VTABLE marker or a vtable CSV row, i.e. RTTI class): slot i of ours names slot i of the
    original, up to the first slot that is not code in our image or the next symbol of ours. Weighs like
    one call site per slot, so it fills slot functions nothing else pairs (ICF-shared EnumDesc<T> bodies)
    but never outvotes a body reccmp proved at 100%."""
    import json
    rpe = pefile.PE(exe, fast_load=True)
    rbase, rimg = rpe.OPTIONAL_HEADER.ImageBase, rpe.get_memory_mapped_image()
    isfunc = lambda a: view[a][0] == "FUNCTION" or view[a][0] == "-" and view[a][1] and a in code
    refs = {}
    with open(json_path) as f:
        data = json.load(f)["data"]
    for e in data:
        if e.get("type") != 5 or not e.get("recomp"):
            continue
        o, r = int(e["address"], 16), int(e["recomp"], 16)
        if view.get(r, ("",))[0] != "VTABLE":
            continue
        for i in range(0x400):
            if r + 4 * i + 4 - rbase > len(rimg) or (i and r + 4 * i in view):
                break
            sr = struct.unpack_from("<I", rimg, r - rbase + 4 * i)[0]
            if sr not in view or not isfunc(sr) or not orig.in_text(orig.u32(o + 4 * i)):
                break
            refs.setdefault(sr, []).append(orig.u32(o + 4 * i))
    return refs


def markers_above(path, line, target):
    """Original addresses of the markers in the comment block right above `line` (1-based). STUB ones too:
    a STUB on a template line reccmp cannot pair (Type.h:82, Type::Type<void> 0x0079fc70) would otherwise
    take its address away from the position row that paired it."""
    try:
        with open(path, errors="replace") as f:
            lines = f.read().split("\n")
    except OSError:
        return set()
    out, i = set(), line - 2
    while i >= 0 and lines[i].strip().startswith("//"):
        m = re.match(r"\s*//\s*(FUNCTION|STUB|TEMPLATE|SYNTHETIC|LIBRARY):\s*" + target + r"\s+0x([0-9a-fA-F]+)", lines[i])
        if m:
            out.add(int(m.group(2), 16))
        i -= 1
    return out


def sticky(path, new, view, kept, held, used_sym, orig, labels):
    """new + the rows of the previous <t>-libdata.csv that still hold: its symbol (or a static's proc
    name) is still in our PDB, no new row or other source takes its address or its symbol, and a
    function row still starts a kept original function. Pairings then only change when evidence
    contradicts them, not when one reccmp run happens not to vote for them. Vtable sizes are redone."""
    if not os.path.exists(path):
        return list(new), 0
    syms = {d for _, d, _ in view.values() if d}
    procs = {n for t, d, n in view.values() if n and not d}
    have = {int(r[0], 16) for r in new} | held
    taken = {r[3] for r in new if r[3]} | {r[4] for r in new if r[4]} | used_sym
    with open(path) as f:
        lines = [x for x in f if not x.startswith("#")]
    out, n = list(new), 0
    for r in list(csv.reader(lines, escapechar="\\", doublequote=False))[1:]:
        a, typ, size, sym, name = (r + [""] * 5)[:5]
        oa = int(a, 16)
        if oa in have or (sym or name) in taken or typ in ("library", "") and oa not in kept or \
                sym and sym not in syms or not sym and name and name not in procs:
            continue
        have.add(oa)
        taken.add(sym or name)
        if typ == "vtable":
            size = 4 * vt_slots(orig, oa, sorted(set(labels) | have)) or ""
        out.append((a, typ, size, sym, name))
        n += 1
    return sorted(out), n


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
    extended = 0
    for k, (a, size, _) in enumerate(funcs):
        if im.in_text(a) and k + 1 < len(funcs):
            n = true_size(im, a, size, funcs[k + 1][0])
            extended += n != size
            size = n
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
             f"FuncInfo records parsed: {nfuncinfo} ({len(eh)} unwind/catch targets).",
             f"{extended} sizes extended past IDA's end (true_size: in-function jump table or noreturn tail)."]
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
    oracle, labels, decorated = {}, set(), {}
    with open(os.path.join(exports, f"{target}-names.csv")) as f:
        for r in csv.DictReader(f):
            a = int(r["address"], 16)
            if r["kind"] != "func":
                labels.add(a)
            # MSVC names: our own PUBLICS names (match: = reccmp proved the body at 100%; lib:, rtti:, str:)
            if r["kind"] == "func" and r["source"] in ("match:", "lib:", "rtti:", "str:") and \
                    a in kept and a not in claimed:
                decorated[a] = (r["name"], r["demangled"] or r["name"], r["source"])
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
    labels = sorted(labels)
    mk = marked(target)
    # A marker reccmp could not pair ('Debug data out of sync with function near: <file>:<line>' in the
    # run's log, <json minus .json>.txt: one template line whose instantiations carry two __FILE__
    # spellings) leaves its address to a position or vftable-slot row.
    failed = set()
    for jp in (json_path or "").split(",") if json_path else []:
        log = jp[:-5] + ".txt"
        if os.path.exists(log):
            with open(log, errors="replace") as f:
                for m in re.finditer(r"Debug data out of sync with function near: (.+?):(\d+)\.", f.read()):
                    failed |= markers_above(m.group(1), int(m.group(2)), target)
    named, nstats = {}, {}
    if have_build and not os.environ.get("OGRE_NO_SIGPAIR"):
        named, nstats = oracle_pairs(oracle, view, kept - claimed - mk, used)
    # our PUBLICS name for each MSVC-named original; with the signature pairs, the tie-break for twins
    dres = {}
    for a, (nm, _, _) in sorted(decorated.items()):
        ra = resolve_symbol([nm], addr, anon) if have_build and nm.startswith(("?", "_")) else None
        vt, d, _ = view.get(ra, (None, None, None)) if ra else (None, None, None)
        dres[a] = d if d and vt in ("FUNCTION", "-") else None
    oracle_lib = {a: third_party(dem) for a, (dem, _) in oracle.items()}
    prefer = {}
    for a, v in sorted(named.items()):
        if v[0]:
            prefer.setdefault(v[0], a)
    for a, d in sorted(dres.items()):
        if d:
            prefer.setdefault(d, a)
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
        skip -= failed
        if os.environ.get("OGRE_NO_SIGPAIR"):  # the method before signature pairing, for A/B scoring
            skip |= {a for a, (d, _) in oracle.items() if "RBX" in d}
        datarows = data_rows(im, recomp, [p for p in pairs if p[2]], view, code, kept, skip, labels=labels, prefer=prefer, oracle_lib=oracle_lib)
        if jpairs:
            # The library pairs (byte-identical bodies) decide first; the --json pairs only add addresses
            # and symbols the library pass left open. An original the linker folded (ICF) is claimed by
            # several of our bodies once more callers vote, and would otherwise lose its library row.
            more = data_rows(im, recomp, [p for p in pairs if p[2]] + jpairs, view, code, kept, skip,
                             labels=labels, prefer=prefer, oracle_lib=oracle_lib)
            have = {r[0] for r in datarows} | {r[3] for r in datarows if r[3]}
            datarows = sorted(datarows + [r for r in more if r[0] not in have and not (r[3] and r[3] in have)])
            # last, the aligned reference lines of the functions that pass did not take whole
            refs = {}
            for jp in json_path.split(","):
                for node, oas in aligned_refs(im, recomp, jp, view).items():
                    refs.setdefault(node, []).extend(oas)
                for node, oas in vt_slot_refs(im, recomp, jp, view, code).items():
                    refs.setdefault(node, []).extend(oas)
            more = data_rows(im, recomp, [p for p in pairs if p[2]] + jpairs, view, code, kept, skip, refs,
                             labels, prefer, oracle_lib)
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
    # A match: name is a pairing a reccmp run proved at 100%; it beats a position or signature pair that
    # contradicts it (twin bodies: a position vote from an aligned reference line in a function that does
    # not match is weaker than a whole body that does).
    proven = {}
    for a, d in sorted(dres.items()):
        if d and decorated[a][2] == "match:" and a not in mk:
            proven.setdefault(d, a)
    proven_at = {a: d for d, a in proven.items()}
    # ... unless the position pair names an identical twin of the proven body: then the callers' votes decide.
    beaten = lambda oa, sym: sym in proven and proven[sym] != oa and not same_body(im, oa, proven[sym], size_of) \
        or oa in proven_at and sym != proven_at[oa]
    datarows = [r for r in datarows if r[1] not in ("library", "") or not beaten(int(r[0], 16), r[3])]
    named = {a: v for a, v in named.items() if not beaten(a, v[0])}
    names["symbols"] = [(a, n, named[int(a, 16)][0] if int(a, 16) in named else "") for a, n in names["symbols"]]
    # a static pairs by our proc name (match_functions): the row's name is ours
    names["symbols"] = [(a, named[int(a, 16)][1] if int(a, 16) in named and not sym else n, sym, "")
                        for a, n, sym in names["symbols"]]
    # MSVC-named originals (match:/lib:/rtti:/str:) pair by the PUBLICS name our link keeps at that body,
    # unless a marker, a library row, a position row or a signature pair already holds the address or
    # the symbol. This keeps a pairing a reccmp run proved (match:) from depending on the next run.
    held = {int(r[0], 16) for r in datarows if r[1] in ("library", "")} | mk
    used_sym = {r[3] for r in datarows if r[3]} | {v[0] for v in named.values() if v[0]} | \
        {row[3] for rows in libs.values() for row in rows if row[3]}
    dstats = {}
    used_name = {r[1] for r in names["symbols"]}
    for a, (nm, dem, src) in sorted(decorated.items()):
        d = dres[a]
        c_name = not nm.startswith(("?", "_"))  # a C function or a static: pairs by our PDB proc name
        why = "held by another row" if a in held else \
            ("name paired elsewhere" if nm in used_name else None) if c_name else \
            "not in our link" if not d else "symbol paired elsewhere" if d in used_sym else None
        if why:
            dstats[why] = dstats.get(why, 0) + 1
            continue
        if c_name:
            used_name.add(nm)
        else:
            used_sym.add(d)
        dstats["paired"] = dstats.get("paired", 0) + 1
        typ = "library" if src == "lib:" or not roblox(nm) else ""
        names["symbols"].append((f"{a:08x}", nm if c_name else dem, "" if c_name else d, typ))
    # An oracle label (no symbol) at an address a marker owns would rename the original under the marker:
    # a SYNTHETIC/TEMPLATE nameref then looks up a name the original no longer carries and never pairs
    # (AssemblyStage's deleting destructor 0x008ea3c0 under "~AssemblyStage() [D0, ...]"). The marker names it.
    names["symbols"] = [r for r in names["symbols"] if r[2] or int(r[0], 16) not in mk]
    names["symbols"].sort()
    for lib, rows in names.items():
        path = os.path.join(ROOT, "reccmp", f"{t}-{lib}.csv")
        hdr = ["address", "name", "symbol", "type"] if lib == "symbols" else ["address", "type", "size", "symbol"]
        notes2 = [f"Generated from ida/exports/{target}-names.csv. Do not edit."]
        if lib == "symbols" and nstats:
            notes2.append("symbol: our PUBLICS name for an oracle name whose signature (norm_sig) is unique on "
                          "both sides; a static carries our proc name instead.")
            notes2.append("Signature pairing: " + ", ".join(f"{k} {v}" for k, v in sorted(nstats.items())) + ".")
        if lib == "symbols" and dstats:
            notes2.append("MSVC-named rows (match:/lib:/rtti:/str:): " + ", ".join(
                f"{k} {v}" for k, v in sorted(dstats.items())) + ".")
        f, w = writer(path, hdr, notes2)
        with f:
            w.writerows(rows)
        out[path] = len(rows)
    if nstats:
        notes.append("Signature pairing: " + ", ".join(f"{k} {v}" for k, v in sorted(nstats.items())) + ".")

    if datarows:
        path = os.path.join(ROOT, "reccmp", f"{t}-libdata.csv")
        rows, nsticky = sticky(path, datarows, view, kept, (mk - failed) | claimed | {int(r[0], 16) for r in names["symbols"]},
                               {r[2] for r in names["symbols"] if r[2]} | used_sym, im, labels)
        # a VTABLE marker gets the original's slot count the same way
        have = {int(r[0], 16) for r in rows}
        bounds = sorted(set(labels) | have)
        rows += [(f"{a:08x}", "vtable", 4 * vt_slots(im, a, bounds) or "", "", "")
                 for a in sorted(marked(target, ("VTABLE",)) - have)]
        rows.sort()
        f, w = writer(path, ["address", "type", "size", "symbol", "name"],
                      ["Generated by tools/ida/gen_reccmp_csv.py: data the paired functions reference,",
                       f"paired by relocation position against {os.path.relpath(recomp, ROOT)}. Do not edit.",
                       "Position sources: library rows" + (" + --json " + ",".join(
                           os.path.relpath(j, ROOT) for j in json_path.split(",")) if json_path else "") + ".",
                       f"Kept from the previous file (still resolvable, not contradicted): {nsticky} rows."])
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
    assert code_imm(b"\x90\x68\x75\x05\xb8\x00", 0xb80575) and not code_imm(b"\x00\x75\x05\xb8\x00", 0xb80575)
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
    assert third_party("boost::bad_weak_ptr::what() const") and third_party("gpc_polygon_clip")
    assert not third_party("XmlNameValuePair::clearValue()") and not third_party("std::vector<RBX::Instance*>::size() const")
    assert norm_sig("void * RBX::FWBase::`scalar deleting dtor'(unsigned int)") == \
        norm_sig("RBX::FWBase::~FWBase()[D0, deleting: the MSVC scalar deleting destructor]")
    assert norm_sig("void RBX::Instance::writeXml(class boost::function<bool (class RBX::Instance *)> const &)") \
        == norm_sig("RBX::Instance::writeXml(boost::function<bool (RBX::Instance*)> const&)")
    class FakeImg:  # true_size over a byte string mapped at 0x1000
        def __init__(self, data): self.d = data
        def b(self, a, n): return self.d[a - 0x1000:a - 0x1000 + n]
        def u32(self, a): return struct.unpack_from("<I", self.d, a - 0x1000)[0]
    # 6 bytes of code (jmp [eax*4+table]), 2 bytes padding, a 2-entry table pointing into the function
    jt = FakeImg(b"\xff\x24\x85\x08\x10\x00\x8d\x49" + struct.pack("<II", 0x1000, 0x1004) + b"\xcc" * 4)
    assert true_size(jt, 0x1000, 6, 0x1020) == 16
    # call cut off by IDA's noreturn guess, then `add esp,4; ret`, then int3 padding
    nr = FakeImg(b"\xe8\x00\x00\x00\x00\x83\xc4\x04\xc3" + b"\xcc" * 7)
    assert true_size(nr, 0x1000, 5, 0x1010) == 9
    assert true_size(nr, 0x1001, 4, 0x1010) == 4  # an unaligned start is never extended over a tail
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

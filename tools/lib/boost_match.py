"""Compile boost 1.53.0's filesystem, program_options, iostreams, system and thread sources with VC9 SP1
and locate/compare every boost function they emit against PLAYER, STUDIO and the shipped boost.dll.

Run (from anywhere; paths are repo-relative):
    python3 -P tools/lib/boost_match.py                      # compile + match, write the CSVs
    python3 -P tools/lib/boost_match.py --preset bjam        # try another flag preset (see PRESETS)
    python3 -P tools/lib/boost_match.py --no-compile         # reuse the objects of the last compile
    python3 -P tools/lib/boost_match.py --no-write -v        # print per-function results only
    python3 -P tools/lib/boost_match.py --no-compile --check # assert the flag-deciding anchors below

Flags (default preset "roblox", measured 2026-11-12):
    exe-side static libs (filesystem, iostreams, program_options):
        /O2 /Ob2 /Oi /Oy /arch:SSE /fp:fast /GR /EHsc /MD  (the App flags without /GS-)
        /GS    zlib_error::check 0x006130e0 has the __security_cookie prologue (absent under /GS-)
        /EHsc  codecvt_error_category 0x009c0f20: /EHs (boost.build's default) wraps the local
               static's atexit registration in an EH frame; the target has none
    boost.dll (thread + system): the App flags /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD
        /GS-   47/47 out-of-line thread+system functions identical; /GS gives 35/44
        plus BOOST_SYSTEM_SOURCE and BOOST_CHRONO_HEADER_ONLY (see DLL_EXTRA)
    program_options is /GL (LTCG) in PLAYER: untyped_value::xparse 0x00478330 calls the
    multiple_occurrences ctor 0x00478060 with `this` pushed on the stack instead of in ecx, and keeps
    an `and esp,-8` aligned frame; only whole-program code generation produces that, so its
    non-virtual internals cannot match an .obj compile.

Inputs:
    staging/boost_1_53_0/libs/<lib>/src     pristine upstream .cpp (boost_1_53_0.tar.bz2, sourceforge,
                                            md5 a00d22605d5dbcfb4c9936a9b35bc4c2 as published there)
    3rdparty/_downloads/boost_1_53_0/boost_1_53_0     the boost headers (-I)
    3rdparty/_downloads/zlib-1.2.3/zlib-1.2.3         zlib.h/zconf.h (PLAYER: ZLIB_VERSION "1.2.3",
                                                      deflateInit2_ 0x009e1e80 is stdcall: ZLIB_WINAPI)
Outputs:
    --objdir (default /tmp/claude-1000/ogre-boost-obj/<preset>)/{exe,dll}/*.obj; not under build/,
                                       which the CMake skeleton owns and regenerates
    ida/exports/PLAYER-lib-boost.csv   rows for target PLAYER and STUDIO
    ida/exports/BOOSTDLL-lib-boost.csv rows for bin/player/boost.dll (BOOSTDLL_1015) and
                                       bin/studio/boost.dll (BOOSTDLL_STUDIO), thread+system DLL build

How a function is located (evidence column), strongest first:
    body        masked bytes equal at exactly one target function start (or a unique .text hit)
    body+xref   several equal starts, the one a located caller's relocation points at wins
    xref        not equal anywhere; a located, identical function's relocation names its address
    name        unique qualified-name match against <TARGET>-names.csv (non-template names only)
    export      boost.dll export of the same decorated name (incremental-link thunk followed)
identical=yes only when every unmasked byte over the whole .obj function length is equal
(relocation bytes masked, trailing int3/nop trimmed, as tools/probe_compare.py does).
masked_diff_bytes = unmasked differing bytes over the common length + the length difference.
Functions not in namespace boost (std:: instantiations, CRT helpers) are left out of the CSV.
"""
import argparse
import bisect
import collections
import concurrent.futures
import csv
import os
import re
import struct
import subprocess
import sys

import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "tools/lib"))
from probe_compare import read_obj  # noqa: E402
import lua_match as LM  # noqa: E402  (const_diff: referenced constants compared by value)

CMAP = {}  # LM.consts() of every compiled object, keyed by name or (obj path, name) for statics

SRC = os.path.join(ROOT, "staging/boost_1_53_0/libs")
INC = os.path.join(ROOT, "3rdparty/_downloads/boost_1_53_0/boost_1_53_0")
ZINC = os.path.join(ROOT, "3rdparty/_downloads/zlib-1.2.3/zlib-1.2.3")
CL = os.path.join(ROOT, "tools/msvc9/cl")
VERSION = "1.53.0"
REL32, DIR32 = 0x14, 0x06

EXE_SRC = {  # statically linked into the exe (boost.static.vcxproj + program_options)
    "filesystem": ["codecvt_error_category", "operations", "path", "path_traits", "portability",
                   "unique_path", "utf8_codecvt_facet", "windows_file_codecvt"],
    "program_options": ["cmdline", "config_file", "convert", "options_description", "parsers",
                        "positional_options", "split", "utf8_codecvt_facet", "value_semantic",
                        "variables_map", "winmain"],
    "iostreams": ["file_descriptor", "gzip", "mapped_file", "zlib"],
}
DLL_SRC = {  # boost.dll (Client/boostlibs): exports Boost.Thread + Boost.System only
    "system": ["error_code"],
    "thread": ["win32/thread", "win32/tss_dll", "win32/tss_pe", "tss_null", "future"],
}
# Common to every preset. _SECURE_SCL=0 and the WIN32/ZLIB_WINAPI/_WIN32_WINNT set are from the
# 2016 boost.static.vcxproj (Release|Win32); BOOST_ALL_NO_LIB only drops #pragma comment(lib).
DEFS = ("/DWIN32 /DNDEBUG /D_LIB /D_SECURE_SCL=0 /DZLIB_WINAPI /D_CRT_SECURE_NO_WARNINGS "
        "/D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010100 /DBOOST_ALL_NO_LIB").split()
# preset -> (flags for the exe-side static libs, flags for boost.dll's thread+system)
_GS = "/O2 /Ob2 /Oi /Oy /arch:SSE /fp:fast /GR /EHsc /MD"   # App flags minus /GS-
_NOGS = _GS + " /GS-"                                      # the App flags (Lua probe)
PRESETS = {
    # Default, the measured best. Exe side keeps /GS: zlib_error::check 0x006130e0 has the
    # __security_cookie prologue; /EHsc, not /EHs: codecvt_error_category 0x009c0f20 has no EH frame
    # around its function-local static. boost.dll is /GS- (47/47 out-of-line thread+system).
    "roblox": (_GS, _NOGS),
    "roblox_nogs": (_NOGS, _NOGS),
    "roblox_gs": (_GS, _GS),
    # 2016 boost.static.vcxproj Release|Win32 literally (/arch:SSE2, /GS-).
    "vcxproj2016": ("/O2 /Ob2 /Oi /GS- /arch:SSE2 /fp:fast /GR /EHsc /MD",) * 2,
    # boost.build msvc-9.0 variant=release runtime-link=shared defaults.
    "bjam": ("/O2 /Ob2 /GR /EHs /MD /Zc:forScope /Zc:wchar_t",) * 2,
}
EXE_EXTRA = "/DBOOST_SYSTEM_DYN_LINK /DBOOST_THREAD_DYN_LINK".split()  # system/thread live in boost.dll
# BOOST_SYSTEM_SOURCE in every DLL TU: boost.dll's thread code calls system_category directly
# (boost.dll 0x10011751 call rel32), not through an import, so thread.cpp saw system as dllexport.
# BOOST_CHRONO_HEADER_ONLY: try_join_until (boost.dll 0x10014880) inlines system_clock::now()
# (GetSystemTimeAsFileTime - 0x019db1ded53e8000), and boost.dll exports no chrono symbol.
DLL_EXTRA = ("/DBOOST_SYSTEM_DYN_LINK /DBOOST_SYSTEM_SOURCE /DBOOST_THREAD_BUILD_DLL /DBOOST_CHRONO_HEADER_ONLY "
             "/D_USRDLL /D_WINDLL").split()

TARGETS = {
    "PLAYER": ("bin/player_1023/RobloxPlayerBeta.exe", "ida/exports/PLAYER-functions.csv",
               "ida/exports/PLAYER-names.csv"),
    "STUDIO": ("bin/studio/RobloxStudioBeta.exe", "ida/exports/STUDIO-functions.csv",
               "ida/exports/STUDIO-names.csv"),
}
DLLS = {"BOOSTDLL_1015": "bin/player/boost.dll", "BOOSTDLL_STUDIO": "bin/studio/boost.dll"}


# ---------------------------------------------------------------- compile

def compile_all(preset, outdir, flags_override=None):
    jobs = []
    for i, (kind, table, extra) in enumerate((("exe", EXE_SRC, EXE_EXTRA), ("dll", DLL_SRC, DLL_EXTRA))):
        flags = (flags_override or PRESETS[preset][i]).split()
        d = os.path.join(outdir, kind)
        os.makedirs(d, exist_ok=True)
        for lib, files in table.items():
            for f in files:
                src = os.path.join(SRC, lib, "src", f + ".cpp")
                obj = os.path.join(d, f"{lib}__{os.path.basename(f)}.obj")
                cmd = [CL, "/nologo", "/c", "/W0", *flags, *DEFS, *extra, "/I" + INC, "/I" + ZINC,
                       "/Fo" + obj, src]
                jobs.append((obj, cmd))
    def run(job):
        obj, cmd = job
        p = subprocess.run(cmd, capture_output=True, text=True)
        return obj, p.returncode, p.stdout + p.stderr
    with concurrent.futures.ThreadPoolExecutor(4) as ex:
        for obj, rc, out in ex.map(run, jobs):
            if rc:
                sys.exit(f"compile failed: {obj}\n{out}")


# ---------------------------------------------------------------- objects

def comdat_selection(path):
    """{section number: COMDAT selection} from the section symbols' aux records (2 = SELECT_ANY)."""
    d = open(path, "rb").read()
    _, _, _, symptr, nsym, _, _ = struct.unpack_from("<HHIIIHH", d, 0)
    sel, i = {}, 0
    while i < nsym:
        o = symptr + i * 18
        value, secno, typ, cls, naux = struct.unpack_from("<IhHBB", d, o + 8)
        if cls == 3 and value == 0 and naux == 1 and secno > 0 and secno not in sel:
            sel[secno] = d[o + 18 + 14]
        i += 1 + naux
    return sel


def obj_functions(path):
    """Yield (name, code, relocs[(off, sym, type, addend)], selection) for every function in a COFF
    object. selection 2 (SELECT_ANY) marks an inline/template body any TU may have supplied."""
    secs, syms = read_obj(path)
    sel = comdat_selection(path)
    bysec = collections.defaultdict(list)
    for nm, value, secno, typ, cls in syms.values():
        if secno > 0 and typ & 0x20 and cls in (2, 3):
            bysec[secno].append((value, nm, cls))
    for secno, fl in bysec.items():
        fl.sort()
        sec = secs[secno - 1]
        for i, (start, nm, cls) in enumerate(fl):
            end = fl[i + 1][0] if i + 1 < len(fl) else len(sec["data"])
            code = sec["data"][start:end].rstrip(b"\xcc\x90")
            relocs = []
            for off, si, t in sec["relocs"]:
                if start <= off < start + len(code):
                    o = off - start
                    relocs.append((o, syms[si][0], t, struct.unpack_from("<i", code, o)[0]))
            yield nm, code, relocs, sel.get(secno, 0), cls == 3


def obj_data(path):
    """Yield (name, static, bytes, mask) for every data symbol (strings, vftables, constants) in a
    COFF object, up to 64 bytes each. Used to check a located function's data relocations."""
    secs, syms = read_obj(path)
    bysec = collections.defaultdict(list)
    for nm, value, secno, typ, cls in syms.values():
        if secno > 0 and cls in (2, 3) and not typ & 0x20:
            bysec[secno].append((value, nm, cls))
    for secno, dl in bysec.items():
        sec = secs[secno - 1]
        if sec["flags"] & 0x20 or not sec["data"]:  # code, or uninitialised
            continue
        dl.sort()
        for i, (start, nm, cls) in enumerate(dl):
            if nm.startswith(".") or nm.startswith("$"):
                continue
            end = min(dl[i + 1][0] if i + 1 < len(dl) else len(sec["data"]), start + 256)
            if end <= start:
                continue
            rel = [(off - start, syms[si][0]) for off, si, _ in sec["relocs"] if start <= off < end]
            mask = {o + k for o, _ in rel for k in range(4)}
            yield nm, cls == 3, sec["data"][start:end], mask, rel


def demangle(names):
    # llvm-undname prints "<input>\n<demangled or error>\n\n" per input line.
    p = subprocess.run(["llvm-undname"], input="\n".join(names) + "\n", capture_output=True, text=True)
    blocks = [b.split("\n") for b in p.stdout.split("\n\n") if b.strip()]
    return {b[0]: ("" if len(b) < 2 or b[1].startswith("error") else b[1]) for b in blocks}


def qualname(dem):
    """'public: void __thiscall boost::a::b(int)' -> 'boost::a::b' (None when templated/odd)."""
    m = re.search(r"(?:__thiscall|__cdecl|__stdcall|__fastcall) (.+?)\(", dem) or re.match(r"([\w:~]+)\(", dem)
    if not m:
        return None
    q = m.group(1).strip()
    return None if "<" in q or "`" in q else q


# ---------------------------------------------------------------- target images

class Image:
    def __init__(self, path, funcs_csv=None, names_csv=None, dll=False):
        pe = pefile.PE(os.path.join(ROOT, path), fast_load=not dll)
        self.pe, self.base = pe, pe.OPTIONAL_HEADER.ImageBase
        self.img = pe.get_memory_mapped_image()
        self.sections = [(s.Name.rstrip(b"\0"), self.base + s.VirtualAddress,
                          self.base + s.VirtualAddress + s.Misc_VirtualSize) for s in pe.sections]
        text = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
        self.text = (text.VirtualAddress, text.VirtualAddress + text.Misc_VirtualSize)
        if funcs_csv:
            self.starts = sorted(int(r["address"], 16) for r in csv.DictReader(open(os.path.join(ROOT, funcs_csv))))
        else:
            self.starts = []
        self.byname = collections.defaultdict(list)  # qualified name -> addresses (function names)
        self.known = {}  # MSVC-decorated or C name -> address (imports, RTTI, vftables, FLIRT)
        self.rtti_at = {}  # address -> vftable/RTTI record name
        self.oracle = {}  # address -> iOS/Mac demangled function name
        if names_csv:
            for r in csv.DictReader(open(os.path.join(ROOT, names_csv))):
                a = int(r["address"], 16)
                # Skip names IDA put on VMProtect's loader import table (.vmp1): PLAYER's .text calls
                # imports through the linker IAT in .rdata, so only names in the linker's sections count.
                if (r["name"].startswith(("?", "__imp_", "_")) and not r["name"].startswith("_Z")
                        and self.section_of(a) in (b".text", b".rdata", b".data")):
                    self.known[anon(r["name"])] = a
                    if r["name"].startswith(("??_7", "??_R")):
                        self.rtti_at[a] = anon(r["name"])
                if r["kind"] == "func" and r["source"] in ("ios:", "mac:"):
                    self.oracle[a] = r["demangled"]
                if r["kind"] != "func" or "[D" in r["demangled"]:  # Itanium D0/D1 != MSVC dtor
                    continue
                q = r["demangled"].split("(")[0]
                if "<" not in q:
                    self.byname[q].append(a)
        self.exports = {}
        self.ilt = dll  # boost.dll was linked /INCREMENTAL: exports and calls go through jmp thunks
        if dll:
            for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
                a = self.base + e.address
                b = self.img[e.address]
                if b == 0xE9:  # incremental-link thunk
                    a = a + 5 + struct.unpack_from("<i", self.img, e.address + 1)[0]
                self.exports[e.name.decode()] = a
        self.index = collections.defaultdict(list)
        for a in self.starts:
            o = a - self.base
            self.index[bytes(self.img[o:o + 2])].append(a)
            self.index[bytes(self.img[o:o + 1])].append(a)

    def section_of(self, addr):
        return next((n for n, lo, hi in self.sections if lo <= addr < hi), None)

    def bytes_at(self, addr, n):
        o = addr - self.base
        return bytes(self.img[o:o + n])

    def in_image(self, addr, n):
        return 0 <= addr - self.base and addr - self.base + n <= len(self.img)

    def in_text(self, addr):
        return self.text[0] <= addr - self.base < self.text[1]


def compare(img, addr, code, mask, f=None):
    """(identical, masked_diff_bytes) of code vs the target at addr over the obj function length.
    With f (a function dict), the constants it references (strings, floats) must hold the same values."""
    theirs = img.bytes_at(addr, len(code))
    # The target's own end: trailing padding means the target body is shorter.
    tl = len(theirs)
    diffs = sum(1 for i in range(min(len(code), tl)) if i not in mask and code[i] != theirs[i])
    if f is not None and diffs == 0:
        diffs = LM.const_diff(f, theirs, lambda a, n: img.bytes_at(a, n) if img.in_image(a, 1) else b"", CMAP)
    return diffs == 0 and tl == len(code), diffs + abs(len(code) - tl)


def body_candidates(img, code, mask):
    if not code:
        return []
    key = code[:2] if 0 not in mask and 1 not in mask else code[:1] if 0 not in mask else None
    if key is None:
        cands = img.starts
    else:
        cands = img.index.get(key, [])
    return [a for a in cands if compare(img, a, code, mask)[0]]


def text_scan(img, code, mask):
    pat = b"".join(b"." if i in mask else re.escape(code[i:i + 1]) for i in range(len(code)))
    lo, hi = img.text
    blob = bytes(img.img[lo:hi])
    return [img.base + lo + m.start() for m in re.finditer(pat, blob, re.S)][:3]


# ---------------------------------------------------------------- matching

MIN_REAL = 16    # unmasked bytes a relocation-free function needs before its body alone places it
STRONG_REAL = 32  # unmasked bytes that let an unverified, uniquely placed function vouch for its callees


def anon(name):
    """Drop the per-build hash of anonymous namespaces (?A0x1234abcd) so names compare across builds."""
    return re.sub(r"\?A0x[0-9a-f]{8}", "?A0x", name)


def reloc_target(img, a, off, t, add):
    v = struct.unpack("<I", img.bytes_at(a + off, 4))[0]
    tgt = ((a + off + 4 + v if t == REL32 else v) - add) & 0xFFFFFFFF
    if img.ilt and img.in_text(tgt) and img.bytes_at(tgt, 1) == b"\xe9":  # incremental-link thunk
        tgt = (tgt + 5 + struct.unpack("<i", img.bytes_at(tgt + 1, 4))[0]) & 0xFFFFFFFF
    return tgt


def check_relocs(img, f, a, known, data):
    """(verified, contradicted) relocations of f placed at a. A relocation is verified when its
    symbol has a known target address equal to where the target's operand points, or when it names
    an .obj data symbol (string, constant) whose unmasked bytes equal the target's bytes there."""
    ver = con = 0
    for off, sym, t, add in f["relocs"]:
        tgt = reloc_target(img, a, off, t, add)
        sk = anon(f["obj"] + "!" + sym)
        k = sk if (sk in known or sk in data) else anon(sym)
        if k not in known and k.startswith(("??_7", "??_R")) and tgt in img.rtti_at:
            con += 1  # the target's vftable/RTTI record there has another class's name
        elif k in known:
            if known[k] == tgt:
                ver += 1
            else:
                con += 1
        elif k in data:
            b, m = data[k]
            real = [i for i in range(len(b)) if i not in m]
            if len(real) < 4 or not img.in_image(tgt, len(b)):
                continue
            theirs = img.bytes_at(tgt, len(b))
            if all(b[i] == theirs[i] for i in real):
                ver += 1
            else:
                con += 1
    return ver, con


def match(img, funcs, data):
    """funcs/data: from load_funcs. Returns {key: (addr, evidence)}.

    1. Body: masked-equal function starts (or .text hits) per function.
    2. Accept a placement when its relocations contradict nothing already known and either some
       are verified or it is the only candidate with >= MIN_REAL real bytes and no other function
       with a different relocation set claims the address (folding lookalikes).
    3. Every accepted, identical placement makes its relocation targets known; an obj function
       that is a reloc target and not body-placed is placed there ("xref").
    4. Repeat to a fixpoint; then try unique qualified-name matches for the rest.
    """
    real = lambda f: len(f["code"]) - len(f["mask"])
    bykey = {f["key"]: f for f in funcs}
    cands = {}
    for f in funcs:
        if real(f) < 6:
            continue
        c = body_candidates(img, f["code"], f["mask"])
        if not c and real(f) >= 24:
            c = text_scan(img, f["code"], f["mask"])
        if c:
            cands[f["key"]] = c
    claims = collections.defaultdict(set)  # address -> distinct relocation-symbol tuples
    for k, c in cands.items():
        if len(c) == 1:
            claims[c[0]].add(tuple(r[1] for r in bykey[k]["relocs"]))
    known = dict(img.known)
    loc = {}
    if img.exports:
        for f in funcs:
            if f["name"] in img.exports:
                loc[f["key"]] = (img.exports[f["name"]], "export")
                known[anon(f["key"])] = img.exports[f["name"]]
    # vftables: a .obj vftable whose name the target knows (RTTI) places each slot's function.
    for vname, (vobj, rel) in data.get("__vftables__", {}).items():
        if vname not in img.known:
            continue
        va = img.known[vname]
        for off, sym in rel:
            sk = vobj + "!" + sym
            sk = sk if sk in bykey else sym
            if sk not in bykey or sk in loc:
                continue
            tgt = struct.unpack("<I", img.bytes_at(va + off, 4))[0]
            if img.in_text(tgt):
                loc[sk] = (tgt, f"vftable 0x{va:08x}+0x{off:x}")
                known.setdefault(anon(sk), tgt)
    used = set()
    changed = True
    while changed:
        changed = False
        for k, c in cands.items():
            if k in loc:
                continue
            f = bykey[k]
            scored = [(a, *check_relocs(img, f, a, known, data)) for a in c]
            good = [a for a, v, x in scored if x == 0 and v > 0]
            if len(good) == 1:
                loc[k] = (good[0], "body+verified" if len(c) == 1 else "body+xref")
            elif len(c) == 1 and scored[0][2] == 0 and real(f) >= MIN_REAL and len(claims[c[0]]) == 1:
                loc[k] = (c[0], "body")
            else:
                continue
            known[anon(k)] = loc[k][0]
            changed = True
        for k, (a, ev) in list(loc.items()):
            f = bykey[k]
            if k in used or not compare(img, a, f["code"], f["mask"], f)[0]:
                continue
            if ev == "body" and real(f) < STRONG_REAL:
                continue
            used.add(k)
            for off, sym, t, add in f["relocs"]:
                tgt = reloc_target(img, a, off, t, add)
                sk = f["obj"] + "!" + sym
                sk = sk if sk in bykey else sym
                if sk not in bykey:
                    continue
                if anon(sk) not in known:
                    known[anon(sk)] = tgt
                    changed = True
                if sk not in loc and known[anon(sk)] == tgt and img.in_text(tgt):
                    loc[sk] = (tgt, f"xref from 0x{a:08x}")
                    changed = True
    for k, f in bykey.items():
        if k in loc or not f["qual"]:
            continue
        hits = img.byname.get(f["qual"], [])
        same = [g for g in funcs if g["qual"] == f["qual"]]
        if len(hits) == 1 and len(same) == 1:
            loc[k] = (hits[0], "name")
    return loc


def library_of(objpath):
    return os.path.basename(objpath).split("__")[0]


def load_funcs(objs):
    funcs, seen, data, vt = [], set(), {}, {}
    for obj in objs:
        for nm, static, b, m, rel in obj_data(obj):
            data.setdefault(anon(obj + "!" + nm if static else nm), (b, m))
            if nm.startswith("??_7"):
                vt.setdefault(anon(nm), (obj, rel))
        for nm, code, relocs, sel, static in obj_functions(obj):
            key = obj + "!" + nm if static else nm  # statics are per object
            if key in seen:  # COMDAT duplicate across objects: keep the first
                continue
            seen.add(key)
            mask = {o + k for o, _, _, _ in relocs for k in range(4)}
            funcs.append(dict(key=key, name=nm, code=code, relocs=relocs, mask=mask, obj=obj,
                              inline=sel == 2))
    dem = demangle([f["name"] for f in funcs])
    for f in funcs:
        f["dem"] = dem.get(f["name"], "") or f["name"]
        f["qual"] = qualname(f["dem"])
    data["__vftables__"] = vt
    return funcs, data


def is_boost(f):
    """Boost code: every out-of-line function of a boost .cpp, every anonymous-namespace function
    (TU-private, so it can only come from that .cpp), and inline/template bodies with a boost type
    in the name. Not EH funclets."""
    if f["name"].startswith(("__ehhandler$", "__unwindfunclet$", "__catch$", "__tryend$")):
        return False
    return not f["inline"] or "boost@@" in f["name"] or "?A0x" in f["name"]


def rows_for(target, img, funcs, loc):
    rows = []
    for f in funcs:
        if f["key"] not in loc or not is_boost(f):
            continue
        a, ev = loc[f["key"]]
        ident, nd = compare(img, a, f["code"], f["mask"], f)
        rows.append(dict(target=target, address=f"0x{a:08x}", size=len(f["code"]), name=f["name"],
                         demangled=f["dem"], library="boost-" + library_of(f["obj"]), version=VERSION,
                         identical="yes" if ident else "no", masked_diff_bytes=nd,
                         obj=os.path.relpath(f["obj"], os.path.dirname(os.path.dirname(f["obj"]))),
                         evidence=ev + (";header-inline" if f["inline"] else ";out-of-line")))
    # One row per target address. Several .obj functions on one address are the linker's ICF
    # folding (or lookalikes the evidence could not split): keep the best-evidenced one as the row
    # and list the other decorated names in the evidence column.
    rank = lambda r: (r["identical"] != "yes", "header-inline" in r["evidence"], "boost@@" not in r["name"])
    by = collections.defaultdict(list)
    for r in rows:
        by[(r["target"], r["address"])].append(r)
    out = []
    for k in sorted(by):
        grp = sorted(by[k], key=rank)
        r = grp[0]
        if len(grp) > 1:
            r["evidence"] += ";folded-with:" + "|".join(g["name"] for g in grp[1:])
        # A trivial body the linker folded with Roblox code: say so, so a library row does not
        # silently claim a Roblox function's address.
        o = img.oracle.get(int(r["address"], 16), "")
        if o and "boost::" not in o and not o.startswith("std::"):
            r["evidence"] += ";icf-shared-with:" + o
        out.append(r)
    rows = out
    return rows


CHECKS = [("PLAYER", "0x006130e0"), ("PLAYER", "0x009c0f20"), ("PLAYER", "0x00613170"),
          ("PLAYER", "0x009beba0"), ("BOOSTDLL_1015", "0x10011730"), ("BOOSTDLL_1015", "0x10014880")]

COLS = "target,address,size,name,demangled,library,version,identical,masked_diff_bytes,obj,evidence".split(",")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--preset", default="roblox", choices=sorted(PRESETS))
    ap.add_argument("--flags", help="codegen flags for both exe and dll objects instead of a preset")
    ap.add_argument("--objdir", default="/tmp/claude-1000/ogre-boost-obj")
    ap.add_argument("--no-compile", action="store_true")
    ap.add_argument("--no-write", action="store_true")
    ap.add_argument("-v", action="store_true")
    ap.add_argument("--check", action="store_true", help="assert the anchor functions are identical")
    a = ap.parse_args()
    outdir = os.path.join(a.objdir, a.preset if not a.flags else "custom")
    if not a.no_compile:
        compile_all(a.preset, outdir, a.flags)
    lst = lambda k: sorted(os.path.join(outdir, k, f) for f in os.listdir(os.path.join(outdir, k)) if f.endswith(".obj"))
    (exe_funcs, exe_data), (dll_funcs, dll_data) = load_funcs(lst("exe")), load_funcs(lst("dll"))
    for o in lst("exe") + lst("dll"):
        CMAP.update(LM.consts(LM.obj_data(o, o)))

    exe_keys = {f["key"] for f in exe_funcs}
    both = exe_funcs + [f for f in dll_funcs if f["key"] not in exe_keys]  # a COMDAT once
    out = {"exe": [], "dll": []}
    for t, (exe, fcsv, ncsv) in TARGETS.items():
        img = Image(exe, fcsv, ncsv)
        # dll objects too: shows which thread/system header code is also compiled into the exe
        loc = match(img, both, {**dll_data, **exe_data,
                                "__vftables__": {**dll_data["__vftables__"], **exe_data["__vftables__"]}})
        out["exe"] += rows_for(t, img, both, loc)
    for t, path in DLLS.items():
        img = Image(path, dll=True)
        img.starts = sorted(set(img.exports.values()))
        for s in img.starts:
            o = s - img.base
            img.index[bytes(img.img[o:o + 2])].append(s)
            img.index[bytes(img.img[o:o + 1])].append(s)
        loc = match(img, dll_funcs, dll_data)
        out["dll"] += rows_for(t, img, dll_funcs, loc)

    for kind, rows in out.items():
        by = collections.defaultdict(lambda: [0, 0])
        for r in rows:
            k = (r["target"], r["library"], "inline" if "header-inline" in r["evidence"] else "own")
            by[k][0] += 1
            by[k][1] += r["identical"] == "yes"
        for (t, lib, kd), (n, y) in sorted(by.items()):
            print(f"{t:<16} {lib:<24} {kd:<7} located {n:4d}  identical {y:4d}")
        if a.v:
            for r in rows:
                print(f"  {r['target']:<8} {r['address']} {r['identical']:<3} {r['masked_diff_bytes']:>5} "
                      f"{r['obj']:<36} {r['evidence']:<10} {r['demangled'][:110]}")
    if a.check:
        got = {(r["target"], r["address"]): r["identical"] for rows in out.values() for r in rows}
        for anchor in CHECKS:
            assert got.get(anchor) == "yes", f"anchor {anchor} not identical: {got.get(anchor)}"
        print("check: all", len(CHECKS), "anchors identical")
    nboost = collections.Counter((library_of(f["obj"]), "inline" if f["inline"] else "own")
                                 for f in exe_funcs + dll_funcs if is_boost(f))
    print("boost functions emitted per library:", dict(nboost))
    if not a.no_write:
        for kind, fn in (("exe", "ida/exports/PLAYER-lib-boost.csv"), ("dll", "ida/exports/BOOSTDLL-lib-boost.csv")):
            with open(os.path.join(ROOT, fn), "w", newline="") as fh:
                w = csv.DictWriter(fh, COLS)
                w.writeheader()
                w.writerows(out[kind])
            print("wrote", fn, len(out[kind]), "rows")


if __name__ == "__main__":
    main()

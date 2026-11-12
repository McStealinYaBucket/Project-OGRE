"""Compile G3D 8.00 (Roblox fork) and its bundled zlib 1.2.3, libzip 0.9 and IJG 6b with VC9 SP1,
then locate and byte-match every function in PLAYER and STUDIO.

Run (from anywhere, a few minutes):
    python3 -P tools/lib/g3d_match.py                    # build, match, write ida/exports/PLAYER-lib-g3d.csv
    python3 -P tools/lib/g3d_match.py --only zlib jpeg   # subset of components (CSV still written)
    python3 -P tools/lib/g3d_match.py --flags "/O2 /Ob2 /Oi /Oy- /GS- /arch:SSE2 /fp:fast"   # flag probe
    python3 -P tools/lib/g3d_match.py --tp               # compile the C libraries as C++ (probe)
    python3 -P tools/lib/g3d_match.py -v inflate         # also print masked diffs (PLAYER) for these names
    python3 -P tools/lib/g3d_match.py --keep DIR         # build in DIR and keep it (default: a mktemp dir)

Sources (staging/g3d-8.00/<component>/upstream, unmodified, plus patches/*.patch applied in name order
with patch -p1 inside a scratch copy; each patch says which PLAYER bytes or which build need justify it):
  zlib    zlib.net/fossils/zlib-1.2.3.tar.gz   sha256 1795c7d0...4b05e (matches zlib.net's published hash)
  jpeg    ijg.org/files/jpegsrc.v6b.tar.gz     sha256 75c3ec24...35d
  libzip  libzip.org/download/libzip-0.9.tar.gz sha256 b0d0a768...a05f (no upstream hash published)
  G3D     no pristine G3D 8.00 archive is obtainable (sourceforge deleted g3d-cpp/8.00 between 2024 and
          2026; the g3d SVN starts at G3D 9). The base is the existing tree's vendored copy,
          "2013 55% App/Rendering/g3d" (G3D_VER 80000, Roblox's fork as of 2016, read in place, never
          copied into staging), plus staging/g3d-8.00/G3D/patches. Upstream G3D text for the patches:
          3rdparty/_downloads/g3d-svn-r30-full (svn.code.sf.net/p/g3d/code G3D9/G3D.lib r30, G3D 9.00.01,
          the oldest revision there), each patch justified by PLAYER bytes in its header.
G3D 8.00 bundled exactly these library versions with small MSVC edits; those edits are taken from G3D's
own SVN copy (svn.code.sf.net/p/g3d/code r100, zip.lib/jpeg.lib), see the patch headers.

Flags (FLAGS below) are the App flags proven on Lua, re-proven here (2026-11-12, PLAYER): flipping one
flag loses /Oy- 233, /arch:SSE2 65, no /arch 79, /fp:precise 53, /GS 21 identical functions; deciding
functions _zcfree 0x009eb660 (/Oy), _crc32@12 0x009e0700 (/arch:SSE, not SSE2), _flush_pending 0x009e0750
(SSE, not x87), G3D::fnan 0x009d7e50 (/fp:fast), _gen_codes 0x009eae50 (/GS-). The C libraries compile as
C (/TC; libzip as C++ loses 11) with ZLIB_WINAPI (PLAYER's deflate ends in `ret 8`). /Gy: deflate.c's
unreferenced functions (deflateInit_, deflateParams, ...) are absent while deflateInit2_ is linked.

Rows: functions defined in each .obj (COMDAT selection other than select-any); inline/template instances
(select-any) only help locating, and get a row only when they sit inside their own TU's span. Target
functions inside the library block that nothing claims get rows with empty masked_diff_bytes, attributed to
a TU only when both claimed neighbours come from it. `size` is the .obj length for located rows, IDA's size
for unclaimed ones. Location and the identical/masked_diff_bytes columns use tools/lib/lua_match.py's engine
(search, reloc, xref, order, ios/mac name; see its docstring), imported, so a change there changes results.
identical also requires every referenced read-only constant (strings, floats, doubles) to hold the same
value (lua_match.const_diff): masking alone called Color4::one identical to a 0.6f loader at 0x007681b0.
tools/lib/g3d_calls.py lists which G3D functions Roblox code calls out of line (the header patches).
"""
import argparse
import csv
import difflib
import glob
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools/lib"))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import lua_match as LM  # noqa: E402
from probe_compare import masked_text  # noqa: E402

STAGE = os.path.join(ROOT, "staging/g3d-8.00")
APP = os.path.join(ROOT, "2013 55% App")
CL = os.path.join(ROOT, "tools/msvc9/cl")
FLAGS = "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast"
COMMON = "/nologo /c /W0 /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_CRT_SECURE_NO_WARNINGS".split()

JPEG = ("jcapimin jcapistd jctrans jcparam jdatadst jcinit jcmaster jcmarker jcmainct jcprepct jccoefct "
        "jccolor jcsample jchuff jcphuff jcdctmgr jfdctfst jfdctflt jfdctint jdapimin jdapistd jdtrans "
        "jdatasrc jdmaster jdinput jdmarker jdhuff jdphuff jdmainct jdcoefct jdpostct jddctmgr jidctfst "
        "jidctflt jidctint jidctred jdsample jdcolor jquant1 jquant2 jdmerge jcomapi jutils jerror jmemmgr "
        "jmemansi").split()  # makefile.vc LIBOBJECTS; G3D's bundled copy uses jmemansi
COMPONENTS = {
    # name: (version, staged dir, source subdir, sources (None = every .c/.cpp), extra cl args)
    "zlib": ("1.2.3", "zlib-1.2.3", ".", "adler32 compress crc32 deflate gzio infback inffast inflate "
             "inftrees trees uncompr zutil".split(), ["/DZLIB_WINAPI"]),
    "jpeg": ("6b", "jpeg-6b", ".", JPEG, []),
    "libzip": ("0.9", "libzip-0.9", "lib", None, ["/DZLIB_WINAPI", "/I{zlib}"]),
    "G3D": ("8.00-roblox", "G3D", "G3Dcpp", None,
            ["/TP", "/DZLIB_WINAPI", "/I{compat}", "/I{G3D}/include", "/I{zlib}", "/I{jpeg}",
             "/I" + os.path.join(APP, "Rendering/RbxG3D/include"), "/I" + os.path.join(APP, "Log/include"),
             "/I" + os.path.join(APP, "Rendering")]),
}
G3D_SKIP = {"GImage_png.cpp"}  # PLAYER links no libpng (no png_* strings, no libpng version)

# Build shims for VC9 (not source edits): the 2016 G3D headers include <stdint.h> and GImage.cpp <png.h>.
COMPAT = {
    "stdint.h": "#pragma once\ntypedef signed char int8_t; typedef unsigned char uint8_t; typedef short int16_t;\n"
                "typedef unsigned short uint16_t; typedef int int32_t; typedef unsigned int uint32_t;\n"
                "typedef __int64 int64_t; typedef unsigned __int64 uint64_t;\n",
    "png.h": "#pragma once\ntypedef unsigned char *png_bytep; typedef unsigned int png_size_t;\n"
             "extern \"C\" int png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num_to_check);\n",
}


def build(d, comps, flags, tp, use_patches):
    """Returns {component: (version, [tu names], obj dir)}."""
    dirs = {"compat": os.path.join(d, "compat")}
    os.makedirs(dirs["compat"], exist_ok=True)
    for n, s in COMPAT.items():
        open(os.path.join(dirs["compat"], n), "w").write(s)
    for comp in COMPONENTS:  # every source tree is prepared: libzip and G3D include zlib/jpeg headers
        ver, staged, sub, _, _ = COMPONENTS[comp]
        dst = os.path.join(d, comp)
        shutil.rmtree(dst, ignore_errors=True)
        shutil.rmtree(os.path.join(d, "obj", comp), ignore_errors=True)
        if comp == "G3D":
            os.makedirs(dst, exist_ok=True)
            for s in ("G3Dcpp", "include"):
                shutil.copytree(os.path.join(APP, "Rendering/g3d", s), os.path.join(dst, s), dirs_exist_ok=True)
        else:
            shutil.copytree(os.path.join(STAGE, staged, "upstream"), dst, dirs_exist_ok=True)
        for p in sorted(glob.glob(os.path.join(STAGE, staged, "patches/*.patch"))) if use_patches else []:
            r = subprocess.run(["patch", "-p1", "-s", "-d", dst, "-i", p], capture_output=True, text=True)
            if r.returncode:
                sys.exit(f"{p}: {r.stdout}{r.stderr}")
        dirs[comp] = dst
    out = {}
    for comp in comps:
        ver, staged, sub, srcs, extra = COMPONENTS[comp]
        src = os.path.join(dirs[comp], sub)
        if srcs is None:
            srcs = sorted(f.rsplit(".", 1)[0] for f in os.listdir(src)
                          if f.endswith((".c", ".cpp")) and f not in G3D_SKIP)
        files = [os.path.join(src, f) for f in os.listdir(src) if f.rsplit(".", 1)[0] in srcs
                 and f.endswith((".c", ".cpp"))]
        objd = os.path.join(d, "obj", comp)
        os.makedirs(objd, exist_ok=True)
        lang = [] if comp == "G3D" else (["/TP"] if tp else ["/TC"])
        args = [a.format(**dirs) for a in extra]
        r = subprocess.run([CL] + COMMON + flags.split() + lang + args + ["/I" + src, "/Fo" + objd + "/"] + files,
                           capture_output=True, text=True, timeout=1200)
        errs = [l for l in r.stdout.splitlines() if " error " in l or "fatal error" in l]
        if errs:
            print(f"{comp}: {len(errs)} compile errors (those TUs are skipped):\n  " + "\n  ".join(errs[:20]))
        tus = sorted(f[:-4] for f in os.listdir(objd) if f.endswith(".obj"))
        out[comp] = (ver, tus, objd)
    return out


def selectany(path):
    """Names of functions whose COMDAT selection is IMAGE_COMDAT_SELECT_ANY (2): inline and template
    instances every including TU emits. The linker keeps one copy; they are not this library's code."""
    d = open(path, "rb").read()
    _, nsec, _, symptr, nsym, opt, _ = struct.unpack_from("<HHIIIHH", d, 0)
    strtab = symptr + nsym * 18
    sel, funcs, i = {}, [], 0
    while i < nsym:
        o = symptr + i * 18
        raw = d[o:o + 8]
        nm = (d[strtab + struct.unpack_from("<I", raw, 4)[0]:].split(b"\0", 1)[0] if raw[:4] == b"\0\0\0\0"
              else raw.rstrip(b"\0")).decode("latin1")
        value, secno, typ, cls, naux = struct.unpack_from("<IhHBB", d, o + 8)
        if cls == 3 and value == 0 and naux == 1 and nm.startswith("."):
            sel.setdefault(secno, d[o + 18 + 14])
        elif secno > 0 and typ & 0x20:
            funcs.append((nm, secno))
        i += 1 + naux
    return {nm for nm, sec in funcs if sel.get(sec) == 2}


def qualified(name, dem):
    """C name for C symbols (_f, _f@8), qualified C++ name (G3D::Vector3::unitize) for decorated ones."""
    if not name.startswith("?"):
        return re.sub(r"@\d+$", "", name[1:] if name.startswith("_") else name)
    m = re.search(r"__(?:cdecl|stdcall|thiscall|fastcall) (.+?)\((?!\))", dem.get(name, ""))
    if not m:
        m = re.search(r"__(?:cdecl|stdcall|thiscall|fastcall) (.+?)\(", dem.get(name, ""))
    return m.group(1) if m else name


def filter_names(tgt, cnames):
    """Restrict tgt.names to ios:/mac:/match: names whose base name is one of ours and occurs at one address only;
    rewrite them to 'base(' so lua_match.locate's name step can match C and qualified C++ names."""
    full = tgt.names
    by = defaultdict(list)
    for a, (n, dm, s) in full.items():
        if s in ("ios:", "mac:") and dm:
            base = dm.split("(")[0].strip()
            if base in cnames:
                by[base].append((a, n, dm, s))
        elif s == "match:" and n.startswith("?"):  # earlier byte match, MSVC-demangled: qualify it
            base = qualified(n, {n: dm})
            if base in cnames:
                by[base].append((a, n, base + "(" + dm.partition("(")[2], s))
    tgt.names = {v[0][0]: (v[0][1], k + "(" + v[0][2].partition("(")[2], v[0][3])
                 for k, v in by.items() if len(v) == 1}
    return full


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--only", nargs="*", default=list(COMPONENTS))
    ap.add_argument("--flags", default=FLAGS)
    ap.add_argument("--tp", action="store_true", help="compile zlib/jpeg/libzip as C++")
    ap.add_argument("--no-patches", action="store_true")
    ap.add_argument("--targets", nargs="*", default=["PLAYER", "STUDIO"])
    ap.add_argument("--keep")
    ap.add_argument("--out", default=os.path.join(ROOT, "ida/exports/PLAYER-lib-g3d.csv"))
    ap.add_argument("-v", nargs="*", default=[], help="names to print masked diffs for (PLAYER)")
    a = ap.parse_args()
    os.makedirs("/tmp/claude-1000", exist_ok=True)
    d = a.keep or tempfile.mkdtemp(prefix="ogre-g3d-", dir="/tmp/claude-1000")
    os.makedirs(d, exist_ok=True)
    built = build(d, a.only, a.flags, a.tp, not a.no_patches)
    funcs, data, libof = [], [], {}
    for comp, (ver, tus, objd) in built.items():
        for tu in tus:
            key = f"{comp}/{tu}"
            libof[key] = (comp, ver, os.path.join(objd, tu + ".obj"))
            sa = selectany(os.path.join(objd, tu + ".obj"))
            for f in LM.obj_funcs(os.path.join(objd, tu + ".obj"), key):
                f["inline"] = f["name"] in sa
                funcs.append(f)
            data += LM.obj_data(os.path.join(objd, tu + ".obj"), key)
    dem = LM.undname([f["name"] for f in funcs])
    for f in funcs:
        f["dem"], f["cname"] = dem.get(f["name"], f["name"]), qualified(f["name"], dem)
    cnames = {f["cname"] for f in funcs}
    rows, log, summary = [], [], []
    for tname in a.targets:
        tgt = LM.Target(tname)
        tgt.strlit = LM.consts(data)  # compare() then checks referenced strings/floats/doubles by value
        full = filter_names(tgt, cnames)
        loc = LM.locate(funcs, data, tgt, log)
        tgt.names = full
        claimed = defaultdict(list)
        stats = defaultdict(lambda: [0, 0, 0])
        spans = {}
        for tu in libof:  # address span of each TU's own (non-inline) located functions
            addrs = LM.cluster(sorted(loc[(f["tu"], f["name"])][0] for f in funcs
                                      if f["tu"] == tu and not f["inline"] and (f["tu"], f["name"]) in loc))
            if addrs and addrs[0] is not None:
                spans[tu] = (addrs[0], addrs[-1] + tgt.size.get(addrs[-1], 1))

        def row(f, addr, evk):
            comp, ver, obj = libof[f["tu"]]
            diff, same = tgt.compare(f, addr)
            if not same and evk.split(":")[0] in ("xref", "order") and LM.similarity(f, tgt, addr) < 0.5:
                log.append(f"{tname} {f['cname']}: 0x{addr:08x} ({evk}) dropped, similarity < 0.5")
                return None
            claimed[addr].append(f["cname"])
            rows.append(dict(target=tname, address=f"0x{addr:08x}", size=len(f["code"]), name=f["name"],
                             demangled=f["dem"], library=comp, version=ver, identical="yes" if same else "no",
                             masked_diff_bytes=diff, obj=os.path.relpath(obj, d), evidence=evk,
                             _unmasked=len(f["code"]) - len(f["mask"]), _rel32=any(r[2] == LM.REL32 for r in f["relocs"])))
            if tname == "PLAYER" and (f["cname"] in a.v or f["name"] in a.v):
                n = max(len(f["code"]), tgt.size.get(addr, 0))
                print(f"== {f['dem']} @ 0x{addr:08x}")
                for l in difflib.unified_diff(masked_text(LM.MD, f["code"], f["mask"], 0),
                                              masked_text(LM.MD, tgt.bytes(addr, n), f["mask"], 0),
                                              "obj", "target", lineterm="", n=2):
                    print("    " + l)
            return same

        for f in funcs:
            k = (f["tu"], f["name"])
            if f["inline"] or k not in loc:
                stats[libof[f["tu"]][0]][0] += not f["inline"]
                continue
            stats[libof[f["tu"]][0]][0] += 1
            same = row(f, loc[k][0], loc[k][2])
            if same is not None:
                stats[libof[f["tu"]][0]][1] += 1
                stats[libof[f["tu"]][0]][2] += same
        n_inline = 0
        for f in funcs:  # inline/template instances the linker kept from this TU: inside its span
            k = (f["tu"], f["name"])
            if f["inline"] and k in loc and f["tu"] in spans and loc[k][0] not in claimed:
                lo, hi = spans[f["tu"]]
                if lo <= loc[k][0] < hi:
                    n_inline += row(f, loc[k][0], "inline/template COMDAT (select-any) inside its TU span; "
                                    + loc[k][2]) is not None
        for addr, who in claimed.items():
            if len(who) > 1:
                log.append(f"{tname} 0x{addr:08x} claimed by {who} (folded by /OPT:ICF or a mislocation)")
        # Unclaimed target functions inside the library block: attributed to a TU only when the nearest
        # claimed functions on both sides come from that same TU (the linker keeps each .obj contiguous;
        # /OPT:ICF survivors can sit in another TU's run, so a lone outlier never widens a span).
        extra = 0
        mine = sorted((int(r["address"], 16), r["obj"], r["library"], r["version"]) for r in rows
                      if r["target"] == tname and r["masked_diff_bytes"] != "")
        block = LM.cluster([m[0] for m in mine], gap=0x8000) if mine else []
        inb = [m for m in mine if block and block[0] <= m[0] <= block[-1]]
        # Outside the block only identical bodies stay (/OPT:ICF can keep another TU's identical copy, and
        # vftables then point at it); short forwarders whose bytes are mostly a masked call are dropped.
        for r in [r for r in rows if r["target"] == tname and r["masked_diff_bytes"] != ""]:
            if block and block[0] <= int(r["address"], 16) <= block[-1] + 0x1000:
                continue
            if r["identical"] == "yes" and (r["_unmasked"] >= 24 or not r["_rel32"] or r["evidence"].startswith("data:")):
                r["evidence"] += "; outside the library block (identical copy kept elsewhere, /OPT:ICF)"
            else:
                log.append(f"{tname} {r['demangled']}: 0x{r['address'][2:]} ({r['evidence']}) dropped, outside "
                           f"the library block and {'a short forwarder' if r['identical'] == 'yes' else 'not identical'}")
                rows.remove(r)
        for p, q in zip(inb, inb[1:]):
            for s in tgt.starts:
                if p[0] < s < q[0] and s not in claimed and tgt.size[s] > 0:
                    same = p[1] == q[1]
                    nm = tgt.names.get(s, ("sub_%X" % s, "", ""))
                    extra += 1
                    claimed[s].append("?")
                    rows.append(dict(target=tname, address=f"0x{s:08x}", size=tgt.size[s], name=nm[0],
                                     demangled=nm[1], library=p[2] if p[2] == q[2] else "G3D-block",
                                     version=p[3] if p[2] == q[2] else "", identical="no", masked_diff_bytes="",
                                     obj=p[1] if same else "",
                                     evidence=f"no compiled counterpart; between {os.path.basename(p[1])}@0x{p[0]:08x} "
                                              f"and {os.path.basename(q[1])}@0x{q[0]:08x}"
                                              + (f"; {nm[2]} name" if nm[2] else "")))
        if block:
            # Extend the block over uncompiled G3D files (Log, debugAssert, FileSystem, prompt ...): walk
            # outwards while target functions are unnamed or named G3D::..., and keep up to the last G3D name.
            i0, i1 = tgt.starts.index(block[0]), tgt.starts.index(block[-1])
            for step, idx in ((-1, i0), (1, i1)):
                ext, j = [], idx + step
                while 0 <= j < len(tgt.starts):
                    nm = tgt.names.get(tgt.starts[j], ("", "", ""))
                    if nm[1] and not nm[1].startswith("G3D::"):
                        break
                    ext.append(tgt.starts[j])
                    j += step
                named = [k for k, x in enumerate(ext) if tgt.names.get(x, ("", ""))[1]]
                for x in ext[:named[-1] + 1] if named else []:
                    if x not in claimed and tgt.size[x] > 0:
                        nm = tgt.names.get(x, ("sub_%X" % x, "", ""))
                        extra += 1
                        claimed[x].append("?")
                        rows.append(dict(target=tname, address=f"0x{x:08x}", size=tgt.size[x], name=nm[0],
                                         demangled=nm[1], library="G3D", version="8.00-roblox", identical="no",
                                         masked_diff_bytes="", obj="", evidence="no compiled counterpart (file not "
                                         "in the fork); G3D block run up to the last G3D:: name"
                                         + (f"; {nm[2]} name" if nm[2] else "")))
            lo = min(int(r["address"], 16) for r in rows if r["target"] == tname and r["library"] in ("G3D", "G3D-block")
                     and block[0] - 0x3000 < int(r["address"], 16))
            summary.append(f"{tname}: library block 0x{lo:08x}-0x{block[-1]:08x} (first G3D function to last "
                           f"claimed function)")
        summary.append(f"{tname}: {n_inline} inline/template instances inside TU spans")
        for comp, (n, l, i) in stats.items():
            summary.append(f"{tname} {comp:<7} {n:4d} compiled functions, {l:4d} located, {i:4d} identical")
        summary.append(f"{tname}: {extra} unclaimed target functions inside located TU spans")
    rows.sort(key=lambda r: (r["target"] != "PLAYER", r["target"], r["address"]))
    if rows:
        with open(a.out, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=[k for k in rows[0] if not k.startswith("_")], extrasaction="ignore")
            w.writeheader()
            w.writerows(rows)
    for l in log:
        print("note:", l)
    print("\n".join(summary))
    print(f"flags: {a.flags}{' (C as C++)' if a.tp else ''}  wrote {a.out}  build dir {d}")


if __name__ == "__main__":
    main()

"""Build the VC9 SP1 FLIRT signatures (toolchain/sig/vc9sp1-*.sig) from the toolchain's own libs.

Run from the repo root (FLAIR from ~/ida-pro-9.4/tools/flair, override with FLAIR=...):
    python3 -P tools/ida/build_flirt.py
Writes toolchain/sig/vc9sp1-<lib>.sig plus the .pat/.exc/.log it was made from. Fails if sigmake
reports a collision that the RESOLVE table below has not decided, so a new lib or a changed lib
never gets its collisions accepted blindly. Apply them with tools/ida/apply_flirt.py.

Libraries (the static code a /MD VS2008 SP1 build can pull in; everything else is import stubs or
data only):
  msvcrt    VC/lib/msvcrt.lib static members (startup, EH prologs, security cookie, 64-bit
            arithmetic helpers, chkstk, RTC); 1720 of 1806 members are import stubs, skipped by pcf
  msvcprt   VC/lib/msvcprt.lib static members (locale facet registration, 8 functions)
  comsuppw  VC/lib/comsuppw.lib (_com_error, _bstr_t helpers). Not comsupp.lib: the two are
            byte-identical and differ only in wchar_t mangling, and both Windows targets import
            native-wchar_t names (`basic_string@_W...` from MSVCP90), so /Zc:wchar_t, comsuppw
  atls      VC/atlmfc/lib/atls.lib (static ATL; PLAYER has 33 ATL RTTI classes)
Left out on measurement: delayimp.lib (neither target has a delay-import directory, data dir 13
is 0), uuid/strmiids/dxguid (data only, pcf finds no code) and every SDK v6.0A static lib with
code (28 libs: strsafe, MiniDump, Fci/Fdi, unicows, ...). Measured on scratch copies: 26 of them hit
nothing in PLAYER or STUDIO, and the only hits were false positives on trivial Roblox accessors
(clfsmgmt.lib CClfsLogCcb::GetManagementContext at PLAYER 0x004b0330; format.lib's
tagCOR_ILMETHOD_SECT_EH_CLAUSE_FAT setters at PLAYER 0x005c30c0/0x00745a20, STUDIO 0x0065e3d0/
0x008a8950, and 2 unknown_libname each). Measured 2026-11-12.
"""
import os
import re
import subprocess
import sys

FLAIR = os.environ.get("FLAIR", os.path.expanduser("~/ida-pro-9.4/tools/flair"))
OUT = "toolchain/sig"
LIBS = {
    "msvcrt": "toolchain/msvc9/VC/lib/msvcrt.lib",
    "msvcprt": "toolchain/msvc9/VC/lib/msvcprt.lib",
    "comsuppw": "toolchain/msvc9/VC/lib/comsuppw.lib",
    "atls": "toolchain/msvc9/VC/atlmfc/lib/atls.lib",
}

# Every collision group sigmake reports, keyed by the sorted tuple of its names. The value is the
# name to keep, or None to exclude the whole group. A group is only resolved to a name when the
# binaries say which one it is; byte-identical aliases with no such evidence stay unnamed
# (CLAUDE.md: competing plausible names mean the placeholder stays).
RESOLVE = {
    # dowildcard setters. Both targets import __getmainargs/_acmdln and not __wgetmainargs.
    ("___setargv", "___wsetargv"): "___setargv",
    # `call __security_init_cookie / jmp __tmainCRTStartup`. Both targets are subsystem 2 (GUI)
    # and use the ANSI command line (_acmdln).
    ("_WinMainCRTStartup", "_mainCRTStartup", "_wWinMainCRTStartup", "_wmainCRTStartup"):
        "_WinMainCRTStartup",
}
# atls.lib: 14 groups, all byte-identical template instances or aliases (CA2WEX/CW2AEX dtors,
# CSimpleArray<G/K/HINSTANCE/CModuleInfo> members, AtlAdd/AtlMultiply<unsigned int/unsigned long>,
# AtlConvFreeMemory<char/wchar_t>, ::/ATL:: __AllocStdCallThunk and __FreeStdCallThunk,
# CComBSTR vs CComTypeInfoHolder::stringdispid dtors, operator new(size_t, void*) vs
# CSimpleArray::Wrapper placement new). Nothing in either binary picks one: excluded.
EXCLUDE_LIBS_DEFAULT = {"atls"}

# Modules removed from the .pat before sigmake: bodies with no fixups that are generic enough to
# match Roblox code. Measured on scratch copies of PLAYER and STUDIO, 2026-11-12.
DROP = {
    # mov eax,ecx / six zeroing stores / ret. PLAYER 0x0042d480 matched: a Roblox-compiled COMDAT
    # copy of CComCriticalSection::CComCriticalSection (no /hotpatch prefix, memset inlined),
    # called from ??0_ATL_BASE_MODULE70; atls's own copy calls memset. CTraceSnapshot is the
    # debug-trace snapshot and has no business in a release build.
    "??0CTraceSnapshot@@QAE@XZ",
}


def run(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True)


def groups(exc_text):
    out, cur = [], []
    for line in exc_text.splitlines():
        if line.startswith(";"):
            continue
        if not line.strip():
            if cur:
                out.append(cur)
            cur = []
            continue
        cur.append(re.split(r"\s", line.lstrip("+-"), maxsplit=1)[0])
    if cur:
        out.append(cur)
    return out


def build(lib, src):
    pat, sig, exc = (os.path.join(OUT, "vc9sp1-%s.%s" % (lib, e)) for e in ("pat", "sig", "exc"))
    r = run(os.path.join(FLAIR, "pcf"), src, pat)
    if r.returncode:
        sys.exit("pcf %s: %s" % (src, r.stdout + r.stderr))
    lines = open(pat, errors="surrogateescape").read().splitlines(True)
    keep = [l for l in lines if not (len(l.split()) > 5 and l.split()[5] in DROP)]
    open(pat, "w", errors="surrogateescape").write("".join(keep))
    if len(keep) != len(lines):
        print("%s: dropped %d module(s) listed in DROP" % (lib, len(lines) - len(keep)))
    for f in (sig, exc):
        if os.path.exists(f):
            os.remove(f)
    name = "VS2008 SP1 (15.00.30729.01) %s" % os.path.basename(src)
    r = run(os.path.join(FLAIR, "sigmake"), "-n" + name, pat, sig)
    if os.path.exists(exc):  # collisions: decide each group, then rerun
        lines = []
        for g in groups(open(exc).read()):
            key = tuple(sorted(g))
            if key in RESOLVE:
                keep = RESOLVE[key]
            elif lib in EXCLUDE_LIBS_DEFAULT:
                keep = None
            else:
                sys.exit("%s: undecided collision %s; add it to RESOLVE" % (lib, key))
            raw = [l for l in open(exc).read().splitlines() if l and not l.startswith(";")]
            for l in raw:
                n = re.split(r"\s", l, maxsplit=1)[0]
                if n in g:
                    lines.append(("+" if n == keep else "") + l)
            lines.append("")
        open(exc, "w").write("\n".join(lines) + "\n")
        r = run(os.path.join(FLAIR, "sigmake"), "-n" + name, pat, sig)
    open(os.path.join(OUT, "vc9sp1-%s.log" % lib), "w").write(r.stdout + r.stderr)
    if not os.path.exists(sig) or "COLLISIONS" in r.stdout + r.stderr:
        sys.exit("sigmake %s failed:\n%s" % (lib, r.stdout + r.stderr))
    npat = sum(1 for l in open(pat) if not l.startswith("---"))
    print("%-9s %5d patterns -> %s" % (lib, npat, sig))


def selfcheck():
    t = ";--- header\n; x\n\n_a   \t00 0000 C3\n_b\t00 0000 C3\n\n+_c x\n_d x\n"
    assert groups(t) == [["_a", "_b"], ["_c", "_d"]], groups(t)


if __name__ == "__main__":
    selfcheck()
    os.makedirs(OUT, exist_ok=True)
    for lib, src in LIBS.items():
        build(lib, src)

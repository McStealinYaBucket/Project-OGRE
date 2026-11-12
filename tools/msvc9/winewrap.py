"""Run a VC9 SP1 tool from toolchain/msvc9 under wine with Unix-path translation.

Run: python3 -P tools/msvc9/winewrap.py <tool> [args...]   (tool = cl, link, lib, rc, mt, ml, ...)
Normally invoked through the shell wrappers tools/msvc9/{cl,link,lib,rc,mt,ml}.

- WINEPREFIX = toolchain/wineprefix, where drive O: is the repo root (the repo path has a space).
- INCLUDE / LIB point into toolchain/msvc9 (VC, ATL/MFC, Windows SDK v6.0A); WINEPATH adds VC/bin
  and SDK/bin. WINEDEBUG=-all.
- Absolute Unix paths in arguments ("/home/x/a.cpp", "/Fo/home/x/a.obj", "/OUT:/home/x/a.exe",
  "@/home/x/a.rsp") become O:\\... (inside the repo) or Z:\\... (elsewhere). Response files are
  copied with the same translation applied. A bare "/word" is left alone, so "/lib" stays a switch.
- OGRE_TARGET=PLAYER or STUDIO (set by cmake/msvc9-wine.cmake through the compiler launcher) makes
  the repo C:\\BuildAgent\\work\\<that build's checkout>\\ instead of O:\\, and the boost headers the
  2013 Contrib2 include directory, so __FILE__ in asserts is the 2013 string. drive_c gets a symlink
  for each (made on first use). A source file named in tools/msvc9/paths-<TARGET>.txt (gen_paths.py)
  is passed in that spelling: wine resolves case-insensitively. What VC9 does (measured under wine,
  same as the 2013 strings): a main file's __FILE__ is its command-line path; a header found on an
  /I path is that /I path + "\\" + the #include spelling (C:\\...\\Client\\App\\include\\Util/Extents.h);
  a header found next to its includer is the includer's directory lowercased + "\\" + the #include
  spelling (c:\\buildagent\\...\\client\\network\\raknet\\source\\DS_List.h). So per-library /I
  spellings (Client/App/Include for RbxOgre) are spelled that way in CMakeLists.txt.
- The tool's output has O:\\, Z:\\ and those 2013 paths turned back into Unix paths, with the case found
  on disk, so Ninja's msvc deps parser ("Note: including file:") and editors see real host paths.
- The tool's exit code is returned unchanged.
- tools/msvc9/ntlm-fake goes first on PATH (see the ntlm_auth there: without it mspdb80.dll cannot
  reach mspdbsrv.exe and link /DEBUG fails with LNK1101), and mspdbsrv.exe is started detached
  (-start -spawn -shutdowntime -1) before the tool runs. Otherwise cl /Zi or link /DEBUG spawn it
  themselves, it inherits the captured stdout, and the wrapper hangs until it exits.
- wineserver -p is started detached on /dev/null before anything else (a no-op when one is already
  running for the prefix), and the tool's output is captured through a temp file, not a pipe, so
  no daemon wine leaves behind can hold the wrapper open.
  Stop it with: WINEPREFIX=toolchain/wineprefix /usr/lib/x86_64-linux-gnu/wine/wineserver -k
"""
import os
import re
import shlex
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TC = os.path.join(ROOT, "toolchain")
DRIVE = "O:"
WIN_TC = DRIVE + r"\toolchain\msvc9"
EXE = {"cl": "VC/bin/cl.exe", "link": "VC/bin/link.exe", "lib": "VC/bin/lib.exe",
       "ml": "VC/bin/ml.exe", "nmake": "VC/bin/nmake.exe", "dumpbin": "VC/bin/dumpbin.exe",
       "cvtres": "VC/bin/cvtres.exe", "mspdbsrv": "VC/bin/mspdbsrv.exe",
       "rc": "SDK/bin/RC.Exe", "mt": "SDK/bin/mt.exe"}
WINESERVER = "/usr/lib/x86_64-linux-gnu/wine/wineserver"
PREFIX = re.compile(r"^(@|[-/][A-Za-z]+:?)?(/.+)$")
DRIVE_C = os.path.join(TC, "wineprefix", "drive_c")
# The 2013 build machines: checkout (assert strings) and boost include directory, per target.
CHECKOUT = {"PLAYER": "407e17641b466b38", "STUDIO": "fa0a2141fe3d2f4d"}
BOOST_WIN = {"PLAYER": r"C:\Users\build\build_SM-UTIL14\build_SM-UTIL14\Contrib2\boost_1_53_0\include",
             "STUDIO": r"C:\Users\build\build_SM-UTIL13\Contrib2\boost_1_53_0\include"}
BOOST_HOST = os.path.join(ROOT, "3rdparty", "_downloads", "boost_1_53_0", "boost_1_53_0")
TARGET = os.environ.get("OGRE_TARGET", "")
if TARGET and TARGET not in CHECKOUT:
    sys.exit(f"winewrap: OGRE_TARGET={TARGET!r} is not one of {sorted(CHECKOUT)}")
# (host dir, Windows dir), most specific first. O: stays for the toolchain (INCLUDE, LIB) and output.
MAPS = ([(BOOST_HOST, BOOST_WIN[TARGET]), (ROOT, "C:\\BuildAgent\\work\\" + CHECKOUT[TARGET])] if TARGET else []) \
    + [(ROOT, DRIVE)]


def spellings():
    """lowercase checkout-relative path -> the 2013 spelling of that source file."""
    if not TARGET:
        return {}
    with open(os.path.join(ROOT, "tools", "msvc9", f"paths-{TARGET}.txt")) as f:
        return {p.lower(): p for p in f.read().split("\n") if p}


SPELL = spellings()


def real_case(path):
    """path with each component spelled as on disk (case-insensitive match); unmatched tail kept."""
    if os.path.lexists(path):
        return path
    parts = path.strip("/").split("/")
    cur = "/"
    for i, c in enumerate(parts):
        nxt = os.path.join(cur, c)
        if not os.path.lexists(nxt):
            try:
                hit = [n for n in os.listdir(cur) if n.lower() == c.lower()]
            except OSError:
                hit = []
            if not hit:
                return os.path.join(cur, *parts[i:])
            nxt = os.path.join(cur, hit[0])
        cur = nxt
    return cur


def link_drive_c():
    """Make each mapped C:\\ directory a symlink to its host directory inside the wine prefix."""
    for host, win in MAPS:
        if not win.startswith("C:"):
            continue
        link = real_case(os.path.join(DRIVE_C, *win[3:].split("\\")))
        if not os.path.lexists(link):
            os.makedirs(os.path.dirname(link), exist_ok=True)
            try:
                os.symlink(host, link)
            except FileExistsError:  # a parallel compile made it first
                pass


def to_win(path):
    tail = "\\" if path.endswith("/") else ""
    path = os.path.normpath(path)
    for host, win in MAPS:
        if path == host or path.startswith(host + "/"):
            rel = path[len(host) + 1:].replace("/", "\\")
            if host == ROOT:
                rel = SPELL.get(rel.lower(), rel)
            return (win + "\\" + rel if rel else win) + tail
    return "Z:" + path.replace("/", "\\") + tail


def is_host_path(p):
    parts = p.strip("/").split("/")
    return len(parts) >= 2 and os.path.exists("/" + parts[0])


def translate(arg, tmpdir):
    if is_host_path(arg):
        prefix, path = "", arg
    else:
        m = PREFIX.match(arg)
        if not m or not is_host_path(m.group(2)):
            return arg
        prefix, path = m.group(1) or "", m.group(2)
    if prefix == "@":
        path = translate_rsp(path, tmpdir)
    return prefix + to_win(path)


def translate_rsp(path, tmpdir):
    with open(path, encoding="utf-8", errors="surrogateescape") as f:
        tokens = shlex.split(f.read(), posix=False)
    out = os.path.join(tmpdir, os.path.basename(path))
    with open(out, "w", encoding="utf-8", errors="surrogateescape") as f:
        for t in tokens:
            q = len(t) > 1 and t[0] == t[-1] == '"'
            t = translate(t[1:-1] if q else t, tmpdir)
            f.write(f'"{t}"\n' if q or " " in t else t + "\n")
    return out


BACK_ROOTS = sorted({w: h for h, w in MAPS}.items(), key=lambda x: -len(x[0])) + [("Z:", "")]
BACK = re.compile("(" + "|".join(re.escape(w) for w, _ in BACK_ROOTS) + r")((?:\\[^\\\r\n:*?\"<>|]+)+)",
                  re.IGNORECASE)


def to_unix(m):
    base = next(h for w, h in BACK_ROOTS if w.lower() == m.group(1).lower())
    path = base + m.group(2).replace("\\", "/")
    return real_case(os.path.normpath(path)) if base else path


def main(argv):
    tool = argv[0]
    env = dict(os.environ)
    env.update(
        WINEPREFIX=os.path.join(TC, "wineprefix"), WINEDEBUG="-all",
        INCLUDE=";".join(WIN_TC + s for s in (r"\VC\include", r"\VC\atlmfc\include", r"\SDK\Include")),
        LIB=";".join(WIN_TC + s for s in (r"\VC\lib", r"\VC\atlmfc\lib", r"\SDK\Lib")),
        WINEPATH=WIN_TC + r"\VC\bin;" + WIN_TC + r"\SDK\bin",
    )
    env["PATH"] = os.path.join(ROOT, "tools", "msvc9", "ntlm-fake") + os.pathsep + env.get("PATH", "")
    # A wine process that finds no wineserver for the prefix starts one, and that server (and the
    # winedevice/services processes it brings up) inherits the caller's stdout. If that is our
    # capture pipe, the wrapper never sees EOF and hangs. So start the server first, detached and
    # on /dev/null; "-p" keeps it alive. If one is running it exits at once with status 2.
    subprocess.run([WINESERVER, "-p"], env=env, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL, start_new_session=True)
    if subprocess.run(["pgrep", "-x", "mspdbsrv.exe"], stdout=subprocess.DEVNULL).returncode:
        subprocess.Popen(["wine", os.path.join(TC, "msvc9", EXE["mspdbsrv"]), "-start", "-spawn",
                          "-shutdowntime", "-1"], env=env, stdin=subprocess.DEVNULL,
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
    link_drive_c()
    with tempfile.TemporaryDirectory(prefix="msvc9-") as tmpdir:
        args = [translate(a, tmpdir) for a in argv[1:]]
        cmd = ["wine", os.path.join(TC, "msvc9", EXE[tool])] + args
        # Output goes to a file, not a pipe: run() then waits for the tool to exit, not for every
        # process that inherited the descriptor to close it.
        with tempfile.TemporaryFile(dir=tmpdir) as log:
            p = subprocess.run(cmd, env=env, stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT)
            log.seek(0)
            out = log.read()
    out = out.decode("cp1252", errors="replace").replace("\r\n", "\n")
    sys.stdout.write(BACK.sub(to_unix, out))
    return p.returncode


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

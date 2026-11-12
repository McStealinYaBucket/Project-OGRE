"""Compile census of the imported prior App decomp under VC9: which TUs compile, and why the rest fail.

Run from the repo root:
    cmake -G Ninja -S . -B build-census -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake \
          -DOGRE_PRIOR_APP=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    python3 -P tools/app_compile_census.py [--build build-census] [--log ninja.log] [--no-build]

Builds every object of the imported TUs with `ninja -k 0` (objects only, no link), keeps the output in
<build>/census-ninja.log, and writes notes/app-compile-census.csv:
    tu, library, vendored, status (ok/error), error_count, first_error_code, first_error_message,
    first_error_location
The TU set is every compile_commands.json entry whose source is in notes/app-import-manifest.csv.
--no-build parses an existing log instead (--log names it).
"""
import collections, csv, json, os, re, subprocess, sys

ERR = re.compile(r"^(?P<loc>.*?)\s*:\s*(?:fatal )?error (?P<code>C\d{4})\s*:\s*(?P<msg>.*)$")
FAILED = re.compile(r"^FAILED: (?:\[code=\d+\] )?(?P<obj>\S+)")
ROOT = os.getcwd()


def opt(name, default=None):
    return sys.argv[sys.argv.index(name) + 1] if name in sys.argv else default


def short(loc):
    """Repo-relative file(line) of an error location, as cl printed it."""
    loc = loc.replace("\\", "/")
    for pre in (ROOT + "/", "O:/"):
        if loc.lower().startswith(pre.lower()):
            return loc[len(pre):]
    return loc


def parse(log):
    """obj -> list of (code, msg, loc) for every FAILED block."""
    out, cur = {}, None
    for line in log.splitlines():
        m = FAILED.match(line)
        if m:
            cur = out.setdefault(m.group("obj"), [])
            continue
        if line.startswith("[") and "] " in line[:20]:
            cur = None
            continue
        if cur is not None and (e := ERR.match(line)):
            cur.append((e.group("code"), e.group("msg").strip(), short(e.group("loc"))))
    return out


def main():
    build = opt("--build", "build-census")
    log_path = opt("--log", os.path.join(build, "census-ninja.log"))
    manifest = {r["new"]: r["kind"] for r in csv.DictReader(open("notes/app-import-manifest.csv")) if r["new"]}
    tus = []
    for e in json.load(open(os.path.join(build, "compile_commands.json"))):
        rel = os.path.relpath(e["file"], ROOT)
        if rel in manifest:
            obj = re.search(r"/Fo(\S+)", e["command"]).group(1)
            lib = re.match(r"CMakeFiles/([^/]+)\.dir/", obj).group(1)
            tus.append((rel, lib, manifest[rel], obj))
    if "--no-build" not in sys.argv:
        r = subprocess.run(["ninja", "-C", build, "-k", "0"] + [t[3] for t in tus],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace")
        open(log_path, "w").write(r.stdout)
    failed = parse(open(log_path, errors="replace").read())
    rows, by_code, by_loc, all_codes = [], collections.Counter(), collections.Counter(), collections.Counter()
    status = collections.Counter()
    for rel, lib, kind, obj in sorted(tus):
        errs = failed.get(obj)
        if errs is None:
            st = "ok" if os.path.exists(os.path.join(build, obj)) else "not built"
        else:
            st = "error"
        first = errs[0] if errs else ("", "", "")
        rows.append([rel, lib, "yes" if kind == "vendored" else "no", st, len(errs or []), *first])
        status[lib, kind, st] += 1
        if errs:
            by_code[first[0], first[1][:90]] += 1
            by_loc[re.sub(r"\(\d+\)$", "", first[2])] += 1
            for c, _, _ in errs:
                all_codes[c] += 1
    with open("notes/app-compile-census.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["tu", "library", "vendored", "status", "error_count", "first_error_code",
                    "first_error_message", "first_error_location"])
        w.writerows(rows)
    print("TUs: %d" % len(rows))
    for (lib, kind, st), n in sorted(status.items()):
        print("  %-8s %-9s %-6s %4d" % (lib, kind, st, n))
    print("\nfirst error, by code and message:")
    for (c, m), n in by_code.most_common(25):
        print("  %4d  %s %s" % (n, c, m))
    print("\nfirst error, by file it is reported in:")
    for loc, n in by_loc.most_common(20):
        print("  %4d  %s" % (n, loc))
    print("\nall errors by code:", ", ".join("%s %d" % kv for kv in all_codes.most_common(15)))


if __name__ == "__main__":
    main()

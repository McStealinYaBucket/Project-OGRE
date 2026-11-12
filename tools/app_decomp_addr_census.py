"""Census of the `// 0xADDRESS` evidence comments in the prior App decomp, scored against
each target's function list, to find which binary those addresses belong to.

Run from the repo root:
    python3 -P tools/app_decomp_addr_census.py ["2013 55% App"] [--json out.json] [--macho path/to/RobloxStudio]

An "address comment" is a 0x token of 5..8 hex digits inside a // or /* */ comment.
Each one is classified against ida/exports/<TARGET>-functions.csv as start / interior /
miss. Also checks the tree's own "(NNNB" / "(NNN bytes)" size notes against the
function size, which no wrong binary can pass by accident.
--macho names the Mac Studio Mach-O so the MACSTUDIO misses are split by section
(data address vs. not an address at all).
--json writes the per-comment records (file, line, address, text) for the string test.
"""
import bisect, csv, json, os, re, struct, sys
from collections import Counter, defaultdict

TARGETS = ["MACSTUDIO", "PLAYER", "PLAYER_1015", "STUDIO", "IOS_1029", "IOS_1021", "IOS_1014"]
EXTS = (".cpp", ".h", ".hpp", ".inl", ".c", ".cc", ".mm")
ADDR = re.compile(r"\b0x([0-9A-Fa-f]{5,8})\b")
SIZE = re.compile(r"\b0x([0-9A-Fa-f]{5,8})\s*\((\d+)\s*(?:B\b|bytes)")
# ponytail: line-based comment detection (// to end of line, /* */ tracked across lines);
# a "//" inside a string literal counts as a comment. Fine for a census.
BLOCK = re.compile(r"/\*.*?\*/", re.S)


def comments(text):
    """Yield (lineno, comment_text) for every comment fragment."""
    for m in BLOCK.finditer(text):
        line = text.count("\n", 0, m.start()) + 1
        for i, part in enumerate(m.group().split("\n")):
            yield line + i, part
    for i, ln in enumerate(BLOCK.sub(lambda m: "\n" * m.group().count("\n"), text).split("\n"), 1):
        p = ln.find("//")
        if p >= 0:
            yield i, ln[p:]


def macho_sections(path):
    """[(start, end, 'SEG,sect')] of a thin 32-bit Mach-O."""
    d = open(path, "rb").read(0x10000)
    ncmds, o, out = struct.unpack_from("<I", d, 16)[0], 28, []
    for _ in range(ncmds):
        cmd, size = struct.unpack_from("<II", d, o)
        if cmd == 1:  # LC_SEGMENT
            seg = d[o + 8:o + 24].rstrip(b"\0").decode()
            for k in range(struct.unpack_from("<I", d, o + 48)[0]):
                so = o + 56 + k * 68
                a, n = struct.unpack_from("<II", d, so + 32)
                out.append((a, a + n, seg + "," + d[so:so + 16].rstrip(b"\0").decode()))
        o += size
    return out


def load_funcs(target):
    path = os.path.join("ida", "exports", target + "-functions.csv")
    if not os.path.exists(path):
        return None
    rows = sorted((int(r["address"], 16), int(r["size"]), r["name"]) for r in csv.DictReader(open(path)))
    return [r[0] for r in rows], rows


def classify(funcs, a):
    starts, rows = funcs
    i = bisect.bisect_right(starts, a) - 1
    if i >= 0 and rows[i][0] <= a < rows[i][0] + rows[i][1]:
        return ("start" if rows[i][0] == a else "interior"), rows[i]
    return "miss", None


def main():
    opt = lambda k: sys.argv[sys.argv.index(k) + 1] if k in sys.argv else None
    out, macho = opt("--json"), opt("--macho")
    args = [a for a in sys.argv[1:] if not a.startswith("--") and a not in (out, macho)]
    root = args[0] if args else "2013 55% App"
    recs, sizes = [], []
    for d, _, files in os.walk(root):
        for fn in files:
            if not fn.endswith(EXTS):
                continue
            p = os.path.join(d, fn)
            text = open(p, encoding="utf-8", errors="replace").read()
            for line, c in comments(text):
                for m in ADDR.finditer(c):
                    recs.append({"file": os.path.relpath(p, root), "line": line, "addr": int(m.group(1), 16), "text": c.strip()})
                for m in SIZE.finditer(c):
                    sizes.append((int(m.group(1), 16), int(m.group(2))))
    print("address comments: %d   distinct addresses: %d   files with any: %d"
          % (len(recs), len({r["addr"] for r in recs}), len({r["file"] for r in recs})))

    funcs = {t: f for t in TARGETS if (f := load_funcs(t))}
    print("\n%-12s %8s %8s %8s %8s   %s" % ("target", "start", "interior", "miss", "start%", "distinct functions hit"))
    for t, f in funcs.items():
        c, hit = Counter(), set()
        for r in recs:
            k, row = classify(f, r["addr"])
            c[k] += 1
            if row:
                hit.add(row[0])
            if t == "MACSTUDIO":
                r["mac"] = k
                r["mac_func"] = row and "0x%08x %s" % (row[0], row[2])
        print("%-12s %8d %8d %8d %7.1f%%   %d" % (t, c["start"], c["interior"], c["miss"], 100.0 * c["start"] / len(recs), len(hit)))

    print("\nsize notes '0xADDR (NNNB': %d" % len(sizes))
    for t, f in funcs.items():
        ok = sum(1 for a, n in sizes if (k := classify(f, a))[0] == "start" and k[1][1] == n)
        print("  %-12s address is a function start of exactly that size: %d (%.1f%%)" % (t, ok, 100.0 * ok / max(1, len(sizes))))

    print("\nper directory (comments / distinct addresses / MACSTUDIO start, interior, miss):")
    by = defaultdict(list)
    for r in recs:
        parts = r["file"].split(os.sep)
        by[os.sep.join(parts[:2]) if len(parts) > 2 else parts[0]].append(r)
    for k in sorted(by):
        c = Counter(r.get("mac", "?") for r in by[k])
        print("  %-34s %5d %5d   %5d %5d %5d" % (k, len(by[k]), len({r["addr"] for r in by[k]}), c["start"], c["interior"], c["miss"]))
    if macho:
        secs = macho_sections(macho)
        where = Counter(next((n for a, e, n in secs if a <= r["addr"] < e), "outside every section (a constant, not an address)")
                        for r in recs if r.get("mac") == "miss")
        print("\nMACSTUDIO misses by Mach-O section:")
        for n, c in where.most_common():
            print("  %5d  %s" % (c, n))
    if out:
        json.dump(recs, open(out, "w"), indent=0)


if __name__ == "__main__":
    main()

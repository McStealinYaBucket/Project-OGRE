"""Marker pass over the imported prior App decomp (Phase 2 step 7): add reccmp STUB markers.

Run from the repo root, after tools/import_prior_app.py (safe to run again):
    python3 -P tools/convert_app_markers.py [--dry-run] [--selftest]

For each row of ida/exports/PLAYER-app-decomp-map.csv with a PLAYER address, find the C++ function
definition in Client/ it belongs to and put "// STUB: PLAYER 0x%08x" directly above it, plus
"// STUB: STUDIO 0x%08x" when ida/exports/STUDIO-app-decomp-map.csv pairs the same Mac function.
How a row finds its function ("basis" in notes/app-marker-placements.csv):
  name      the demangled Mac name matches the definition (last component, and the class/namespace
            before it when both sides have one), searched first in the files the map lists for the
            row (mapped through notes/app-import-manifest.csv), then in the whole imported tree.
            Several overloads: parameter count, then a comment citing the row's Mac function.
  comment   no definition carries the Mac name (or the Mac function is unnamed), but the comment
            block directly above exactly one definition starts with an address inside that Mac
            function. Weaker: the prior tree may name the function differently, or the comment may
            describe a callee.
A definition gets at most one marker per target. Rows that find nothing, or more than one candidate,
are counted and listed, never guessed.

Comments that start with a Mac address ("// 0x5451F8: ..." or "/* 0x5451F8 ...") are rewritten to
"// mac: 0x5451F8: ..." so they cannot be read as markers; so are prose comments that fit reccmp's
marker pattern ("// dword_FADA34: InitFunc_123 0x1AA72B ..." -> "// mac: dword_FADA34: ..."). Existing "// STUB: PLAYER|STUDIO" lines are
removed and placed again on every run; a function that already has a FUNCTION (or other) marker for a
target keeps it and gets no STUB for that target. Vendored trees are not touched.
Writes notes/app-marker-placements.csv and prints the counts, the unplaced rows by reason, and every
file whose markers are not in ascending address order.
"""
import bisect, collections, csv, os, re, subprocess, sys

MAPS = {"PLAYER": "ida/exports/PLAYER-app-decomp-map.csv", "STUDIO": "ida/exports/STUDIO-app-decomp-map.csv"}
EXTS = (".cpp", ".h", ".hpp", ".inl", ".c")
CONTROL = {"if", "for", "while", "switch", "catch", "return", "sizeof", "else", "do", "try"}
ADDR = re.compile(r"\b0x([0-9A-Fa-f]{5,8})\b")
LEAD = re.compile(r"^(\s*(?://+|/\*+)\s*)(0x[0-9A-Fa-f]{5,8}\b)")
STUB_LINE = re.compile(r"^\s*// STUB: (PLAYER|STUDIO) 0x[0-9a-f]{8}\s*$")
# reccmp's own marker pattern ("// word: word 0x..."); prior prose that fits it gets "mac: " too.
PROSE_MARKER = re.compile(r"^(\s*//+\s*)\w+:\s*(\w+)\s+0x[a-f0-9]+", re.I)
ANY_MARKER = re.compile(r"^\s*// [A-Z]+: (PLAYER|STUDIO) 0x[0-9a-f]+")


# --- C++ scanning -------------------------------------------------------------------------------------

def blank(text):
    """Comments, string/char literal contents and preprocessor lines -> spaces (newlines kept)."""
    out, i, n = list(text), 0, len(text)
    line_start = True
    while i < n:
        c = text[i]
        if line_start and c == "#":
            j = i
            while j < n and not (text[j] == "\n" and text[j - 1] != "\\"):
                j += 1
            for k in range(i, j):
                if out[k] != "\n":
                    out[k] = " "
            i = j
            continue
        if c == "\n":
            line_start = True
            i += 1
            continue
        if not c.isspace():
            line_start = False
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                out[k] = " "
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if out[k] != "\n":
                    out[k] = " "
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            for k in range(i + 1, min(j, n)):
                out[k] = " "
            i = j + 1
        else:
            i += 1
    return "".join(out)


def strip_angles(s):
    prev = None
    while prev != s:
        prev, s = s, re.sub(r"<[^<>]*>", "", s)
    return s


def top_groups(s):
    """[(open, close)] of the top-level () groups in s."""
    out, depth, start = [], 0, 0
    for i, c in enumerate(s):
        if c == "(":
            if depth == 0:
                start = i
            depth += 1
        elif c == ")" and depth:
            depth -= 1
            if depth == 0:
                out.append((start, i))
    return out


def count_params(s):
    s = strip_angles(s).strip()
    if s in ("", "void"):
        return 0
    depth, n = 0, 1
    for c in s:
        depth += c in "(["
        depth -= c in ")]"
        n += c == "," and depth == 0
    return n


def split_name(name):
    """'A<x>::B::operator<' -> ['A', 'B', 'operator<'] (template arguments dropped)."""
    name = re.sub(r"\s+", "", name)
    op = name.find("operator")
    tail = None
    if op >= 0 and (op == 0 or not (name[op - 1].isalnum() or name[op - 1] == "_")):
        name, tail = name[:op], name[op:]
    parts = [p for p in strip_angles(name).split("::") if p]
    return parts + [tail] if tail else parts


def function_header(header):
    """(decl offset in header, name parts, param count) if header is a function definition's."""
    hs = header.rstrip()
    groups = top_groups(hs)
    for gi, (s, e) in enumerate(groups):
        rem = hs[e + 1:]
        if rem.lstrip().startswith("::") or not re.fullmatch(
                r"\s*(?:const\b\s*)?(?:throw\s*\([^)]*\)\s*)?(?::.*)?", rem, re.S):
            continue
        full = hs[:s]
        m = re.search(r"((?:\w+\s*(?:<[^;{}()]*>)?\s*::\s*)*operator\b(?:\s*\(\s*\)|[^()]*))$", full, re.S)
        if m:
            name, start = m.group(1), m.start(1)
        else:
            m = re.search(r"((?:\w+\s*::\s*)*~?\s*\w+)\s*$", strip_angles(full))
            if not m:
                return None
            name, start = m.group(1), s
        # the declaration starts after the last top-level group before the name (a macro call)
        prev = max([e2 + 1 for s2, e2 in groups if e2 < start] or [0])
        pre = hs[prev:s]
        parts = split_name(name)
        if not parts or parts[-1] in CONTROL:
            return None
        off = prev + (len(pre) - len(pre.lstrip()))
        return off, parts, count_params(hs[s + 1:e])
    return None


def scan(text):
    """Function definitions: dicts with parts (qualified), params, decl_line, body_end_line (1-based)."""
    code = blank(text)
    line_of = lambda pos: code.count("\n", 0, pos) + 1
    funcs, stack, hstart = [], [("ns", "")], 0
    for pos, c in enumerate(code):
        kind = stack[-1][0]
        if c == "{":
            if kind in ("func", "other"):
                stack.append(("other", ""))
                continue
            header = code[hstart:pos]
            lab = re.match(r"(\s*(?:(?:public|private|protected)\s*:\s*)+)", header)
            skip = len(lab.group(1)) if lab else 0
            h = header[skip:]
            hn = strip_angles(re.sub(r"^\s*template\s*", "", h))
            fh = function_header(h) if hn.rstrip().endswith(")") or re.search(r"\)\s*(const|throw\s*\(\s*\))\s*$", hn) else None
            ns = re.search(r"\bnamespace\s*(\w*)\s*$", h)
            if ns:
                stack.append(("ns", ns.group(1)))
            elif re.search(r"\bextern\s*\"[^\"]*\"\s*$", h):
                stack.append(("ns", ""))
            elif fh:
                off, parts, np = fh
                scope = [n for k, n in stack if k in ("ns", "class") and n]
                decl = hstart + skip + off
                while decl < pos and code[decl].isspace():
                    decl += 1
                funcs.append({"parts": scope + parts, "params": np, "decl_line": line_of(decl), "open": pos})
                stack.append(("func", len(funcs) - 1))
            elif re.search(r"\b(?:class|struct|union)\b", hn):
                m = re.search(r"\b(?:class|struct|union)\s+(?:\w+\s+)*?(\w+)\s*(?::[^;{}]*)?$", hn.strip())
                stack.append(("class", m.group(1) if m else ""))
            else:
                stack.append(("other", ""))
            hstart = pos + 1
        elif c == "}":
            if len(stack) > 1:
                k, v = stack.pop()
                if k == "func":
                    funcs[v]["end_line"] = line_of(pos)
            if stack[-1][0] in ("ns", "class"):
                hstart = pos + 1
        elif c == ";" and kind in ("ns", "class"):
            hstart = pos + 1
    for f in funcs:
        f.setdefault("end_line", f["decl_line"])
    return funcs


# --- Mac side -----------------------------------------------------------------------------------------

def demangle(names):
    raw = [re.sub(r"^j_", "", n) for n in names]
    inp = "\n".join(r[1:] if r.startswith("__Z") else r for r in raw) + "\n"
    out = subprocess.run(["llvm-cxxfilt"], input=inp, capture_output=True, text=True).stdout.splitlines()
    return dict(zip(names, out))


def mac_parts(dem):
    """Demangled name -> (parts, param count) or None for an unnamed function."""
    if re.fullmatch(r"sub_[0-9A-Fa-f]+", dem):
        return None
    s = re.sub(r"(\s*(const|volatile|&|&&))+$", "", dem.strip())
    if not s.endswith(")"):
        return [s.lstrip("_")], None
    depth = 0
    for i in range(len(s) - 1, -1, -1):
        depth += s[i] == ")"
        depth -= s[i] == "("
        if depth == 0:
            break
    qual, params = s[:i], s[i + 1:-1]
    # drop a leading return type (function template instances): last top-level space-separated token
    d, cut = 0, 0
    for j, ch in enumerate(qual):
        d += ch in "<("
        d -= ch in ">)"
        if ch == " " and d == 0 and "operator" not in qual[max(0, j - 9):j + 1]:
            cut = j + 1
    qual = qual[cut:]
    return [p for p in split_name(qual.replace("(anonymous namespace)", "")) if p], count_params(params)


def name_match(src, mac):
    if src[-1] != mac[-1]:
        return False
    if len(src) >= 2 and len(mac) >= 2:
        return src[-2] == mac[-2]
    return len(src) == len(mac) == 1


# --- main ---------------------------------------------------------------------------------------------

def load_mac_funcs():
    rows = sorted((int(r["address"], 16), int(r["size"])) for r in csv.DictReader(open("ida/exports/MACSTUDIO-functions.csv")))
    return [r[0] for r in rows], rows


def mac_func_of(funcs, a):
    starts, rows = funcs
    i = bisect.bisect_right(starts, a) - 1
    if i >= 0 and rows[i][0] <= a < rows[i][0] + rows[i][1]:
        return rows[i][0]
    return None


def comment_part(line):
    """The comment text of a line (after // or /*, or a block-comment continuation line)."""
    for tok in ("//", "/*"):
        if tok in line:
            return line[line.index(tok):]
    return line if line.lstrip().startswith("*") else ""


def comment_block_above(lines, decl_line):
    """Line indexes (0-based) of the comment lines directly above decl_line (1-based)."""
    out, i, in_block = [], decl_line - 2, False
    while i >= 0:
        s = lines[i].strip()
        if s.endswith("*/") or in_block:
            out.append(i)
            in_block = "/*" not in s
            i -= 1
            continue
        if s.startswith("//") and not ANY_MARKER.match(lines[i]):
            out.append(i)
            i -= 1
            continue
        if ANY_MARKER.match(lines[i]):
            i -= 1
            continue
        break
    return out


def main():
    dry = "--dry-run" in sys.argv
    manifest = {r["old"]: r["new"] for r in csv.DictReader(open("notes/app-import-manifest.csv")) if r["new"]}
    kinds = {r["new"]: r["kind"] for r in csv.DictReader(open("notes/app-import-manifest.csv")) if r["new"]}
    files = sorted(n for n, k in kinds.items() if k == "code" and n.endswith(EXTS) and os.path.getsize(n))
    macf = load_mac_funcs()

    # Read every file, drop old STUB lines, rewrite leading Mac comments, scan definitions.
    texts, funcs, cited = {}, [], collections.defaultdict(set)  # cited: (file, func idx) -> mac starts
    prefixed = 0
    for path in files:
        raw = open(path, encoding="utf-8", errors="surrogateescape", newline="").read()
        nl = "\r\n" if "\r\n" in raw else "\n"
        lines = [l for l in raw.split(nl) if not STUB_LINE.match(l)]
        for i, l in enumerate(lines):
            m = LEAD.match(l)
            if m and mac_func_of(macf, int(m.group(2), 16)) is not None:
                lines[i] = m.group(1) + "mac: " + l[m.end(1):]
                prefixed += 1
            elif (m := PROSE_MARKER.match(l)) and m.group(2) not in ("PLAYER", "STUDIO"):
                lines[i] = m.group(1) + "mac: " + l[m.end(1):]  # prose reccmp would parse as a marker
                prefixed += 1
        texts[path] = (lines, nl)
        for f in scan("\n".join(lines)):
            f["file"] = path
            above = comment_block_above(lines, f["decl_line"])
            f["above"] = above
            first = None
            for i in sorted(above):
                m = ADDR.search(lines[i])
                if m:
                    first = mac_func_of(macf, int(m.group(1), 16))
                    break
            f["first_above"] = first
            for i in above + list(range(f["decl_line"] - 1, f["end_line"])):
                for m in ADDR.finditer(comment_part(lines[i])):
                    a = mac_func_of(macf, int(m.group(1), 16))
                    if a is not None:
                        cited[id(f)].add(a)
            funcs.append(f)
    by_file = collections.defaultdict(list)
    by_last = collections.defaultdict(list)
    for f in funcs:
        by_file[f["file"]].append(f)
        by_last[f["parts"][-1]].append(f)

    rows = {t: {int(r["mac_address"], 16): r for r in csv.DictReader(open(p))} for t, p in MAPS.items()}
    player = {a: r for a, r in rows["PLAYER"].items() if r["player_address"]}
    dem = demangle([r["mac_name"] for r in player.values()])

    placed, unplaced, taken = {}, collections.defaultdict(list), {}
    for mac, r in sorted(player.items()):
        olds = [x for x in r["files"].split(";") if x]
        news = [manifest[x] for x in olds if x in manifest and kinds[manifest[x]] == "code"]
        if not news:
            unplaced["files not imported (Lua, Bullet, vendored)"].append(r)
            continue
        mp = mac_parts(dem[r["mac_name"]])
        cands, basis = [], "name"
        if mp:
            parts, np = mp
            pool = [f for n in news for f in by_file.get(n, [])]
            cands = [f for f in pool if name_match(f["parts"], parts)]
            if not cands:
                cands = [f for f in by_last.get(parts[-1], []) if name_match(f["parts"], parts)]
            if len(cands) > 1 and np is not None:
                cands = [f for f in cands if f["params"] == np] or cands
            if len(cands) > 1:
                cands = [f for f in cands if mac in cited[id(f)]] or cands
        if not cands:
            basis = "comment"
            cands = [f for n in news for f in by_file.get(n, []) if f["first_above"] == mac]
        if len(cands) != 1:
            unplaced["no definition found" if not cands else "ambiguous (%s)" % basis].append(r)
            continue
        f = cands[0]
        key = (f["file"], f["decl_line"])
        if key in taken:
            if placed[key]["PLAYER"] == int(r["player_address"], 16):
                unplaced["same definition and PLAYER address as a placed row (C1/C2, D0/D1/D2)"].append(r)
            else:
                unplaced["definition already has another PLAYER address (template instances, overloads)"].append(r)
            continue
        taken[key] = mac
        studio = rows["STUDIO"].get(mac, {}).get("player_address") or ""
        placed[key] = {"func": f, "mac": mac, "PLAYER": int(r["player_address"], 16),
                       "STUDIO": int(studio, 16) if studio else None, "basis": basis, "row": r}

    # Insert markers bottom-up per file; check ascending order.
    count = collections.Counter()
    out_rows, not_ascending = [], []
    for path, (lines, nl) in texts.items():
        items = sorted((v for (p, _), v in placed.items() if p == path), key=lambda v: v["func"]["decl_line"])
        for t in ("PLAYER", "STUDIO"):
            seq = [v[t] for v in items if v[t] is not None]
            bad = sum(1 for a, b in zip(seq, seq[1:]) if b < a)
            if bad:
                not_ascending.append((path, t, bad, len(seq)))
        for v in reversed(items):
            i = v["func"]["decl_line"] - 1
            indent = re.match(r"\s*", lines[i]).group(0)
            have, j = set(), i - 1
            while j >= 0 and ANY_MARKER.match(lines[j]):
                have.add(ANY_MARKER.match(lines[j]).group(1))
                j -= 1
            new = ["%s// STUB: %s 0x%08x" % (indent, t, v[t]) for t in ("PLAYER", "STUDIO")
                   if v[t] is not None and t not in have]
            lines[i:i] = new
            for t in ("PLAYER", "STUDIO"):
                count[t] += v[t] is not None and t not in have
            count["basis " + v["basis"]] += 1
            out_rows.append([path, v["func"]["decl_line"], "::".join(v["func"]["parts"]), "0x%08x" % v["PLAYER"],
                             "0x%08x" % v["STUDIO"] if v["STUDIO"] is not None else "", "0x%08x" % v["mac"],
                             v["row"]["mac_name"], v["row"]["evidence_class"], v["basis"],
                             "yes" if "<" in dem[v["row"]["mac_name"]].split("(")[0] else "no"])
        if not dry:
            open(path, "w", encoding="utf-8", errors="surrogateescape", newline="").write(nl.join(lines))
    if not dry:
        with open("notes/app-marker-placements.csv", "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["file", "decl_line_before_markers", "function", "player", "studio", "mac", "mac_name",
                        "evidence_class", "basis", "template"])
            w.writerows(sorted(out_rows))
    print("files scanned %d, definitions found %d, Mac comments prefixed %d" % (len(files), len(funcs), prefixed))
    print("map rows with a PLAYER address: %d" % len(player))
    print("placed: %d  (STUB PLAYER %d, STUB STUDIO %d; by name %d, by comment %d)" % (
        len(placed), count["PLAYER"], count["STUDIO"], count["basis name"], count["basis comment"]))
    for k, v in sorted(unplaced.items()):
        print("unplaced, %s: %d" % (k, len(v)))
        for r in v[:8]:
            print("    %s %s %s" % (r["mac_address"], r["mac_name"][:70], r["files"][:60]))
    print("files not in ascending order: %d" % len(not_ascending))
    for p, t, bad, n in sorted(not_ascending):
        print("    %-60s %s: %d descents in %d markers" % (p, t, bad, n))


def selftest():
    src = """namespace RBX {
class Foo : public Bar<int> {
public:
    int get() const { return x; }
    Foo(int a) : x(a), y(f(a)) {}
};
FASTFLAGVARIABLE(Thing, false)
template<class T> void Baz<T>::run(T a, int b)
{
    if (a) { b++; }
}
bool Foo::operator()(int a) const { return a; }
extern "C" { int c_func(void) { return 0; } }
}
"""
    got = [("::".join(f["parts"]), f["params"], f["decl_line"]) for f in scan(src)]
    want = [("RBX::Foo::get", 0, 4), ("RBX::Foo::Foo", 1, 5), ("RBX::Baz::run", 2, 8),
            ("RBX::Foo::operator()", 1, 12), ("RBX::c_func", 0, 13)]
    assert got == want, got
    assert mac_parts("RBX::Instance::setName(std::string const&)") == (["RBX", "Instance", "setName"], 1)
    assert mac_parts("void RBX::Foo<int>::bar<char>(int, char) const") == (["RBX", "Foo", "bar"], 2)
    assert mac_parts("sub_FEA4") is None
    assert name_match(["RBX", "Instance", "setName"], ["RBX", "Instance", "setName"])
    assert not name_match(["setName"], ["RBX", "Instance", "setName"])
    print("selftest ok")


if __name__ == "__main__":
    selftest() if "--selftest" in sys.argv else main()

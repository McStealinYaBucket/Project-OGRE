"""Line up a source file's asserts with the 2013 assert strings in the original binaries.

    python3 -P tools/assert_lines.py Client/App/util/Math.cpp [more files...]
    python3 -P tools/assert_lines.py --apply [--ignore 555,556] Client/App/util/Math.cpp
    python3 -P tools/assert_lines.py --selftest

The originals' assert messages are "EXPR file: PATH line: N". For each source file this lists the
RBXASSERT calls in it (line of the closing parenthesis: VC9's __LINE__ for a macro call spread over
several lines, stringized expression) next to the 2013 asserts recorded for the same path
(case-insensitive, after "\\Client\\"), aligned in order by expression (difflib). Columns: our line,
PLAYER line, STUDIO line, delta (2013 - ours, PLAYER), expression. A row with only a 2013 side is an
assert our source lacks (or spells differently); one with only our side is an assert the 2013 build
does not have at that path. Repeated expressions ("0") can align wrongly: check before --apply.

--apply lays the file out so every anchor (an assert whose expression matches its aligned 2013
assert, at the same line in PLAYER and STUDIO; --ignore drops some of ours by line before aligning)
lands on its 2013 line: it inserts blank lines above the anchor's function (the last blank line
above it, else after the last top-level closing brace, else right above the anchor), and removes
lines by deleting blank lines, then joining runs of plain // comment lines (never a reccmp marker or
the nameref after one), latest first, between the previous anchor and this one. It stops at an
anchor it cannot reach and says so, leaving the lines it already moved.
"""
import difflib
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BINS = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
MSG = re.compile(rb"([\x20-\x7e]{1,400}?) file: ([\x20-\x7e]+?) line: (\d+)\x00")
CALL = re.compile(r"\bRBXASSERT\s*\(")  # the _SLOW/_VERY_FAST/_IF_VALIDATING forms compile to nothing


def key(path):
    p = path.replace("\\", "/").lower()
    i = p.rfind("/client/")
    return p[i + 8:] if i >= 0 else p


_cache = {}


def originals(tgt):
    if tgt not in _cache:
        data = open(os.path.join(ROOT, BINS[tgt]), "rb").read()
        d, whole = {}, set(data.split(b"\0"))
        for m in MSG.finditer(data):
            raw = m.group(1)
            # the match can start inside preceding data; the expression is also its own string
            expr = next((raw[i:] for i in range(len(raw)) if raw[i:] in whole), raw).decode()
            d.setdefault(key(m.group(2).decode()), set()).add((int(m.group(3)), expr))
        _cache[tgt] = d
    return _cache[tgt]


def ours(path):
    src = open(path, encoding="utf-8", errors="surrogateescape").read()
    out = []
    for m in CALL.finditer(src):
        if src[src.rfind("\n", 0, m.start()) + 1:m.start()].lstrip().startswith(("//", "#")):
            continue
        i, depth = m.end(), 1
        while i < len(src) and depth:
            depth += {"(": 1, ")": -1}.get(src[i], 0)
            i += 1
        # VC9's __LINE__ for a macro call spread over several lines is the line of its closing ')'
        line = src.count("\n", 0, i - 1) + 1
        expr = " ".join(src[m.end():i - 1].split())
        out.append((line, expr))
    return out


def align(mine, orig):
    """{index into mine: 2013 line}, [2013 (line, expr) with no partner]"""
    sm = difflib.SequenceMatcher(None, [e for _, e in mine], [e for _, e in orig], autojunk=False)
    got, lone = {}, []
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        for x in range(max(i2 - i1, j2 - j1)):
            if i1 + x < i2 and j1 + x < j2:
                got[i1 + x] = orig[j1 + x]
            elif j1 + x < j2:
                lone.append(orig[j1 + x])
    return got, lone


def main(paths):
    for path in paths:
        k = key("/client/" + os.path.relpath(os.path.abspath(path), os.path.join(ROOT, "Client")))
        mine = ours(path)
        orig = {t: sorted(originals(t).get(k, ())) for t in BINS}
        print(f"== {path}  ours {len(mine)}  PLAYER {len(orig['PLAYER'])}  STUDIO {len(orig['STUDIO'])}")
        (gp, lp), (gs, ls) = align(mine, orig["PLAYER"]), align(mine, orig["STUDIO"])
        for i, (line, expr) in enumerate(mine):
            p, s = gp.get(i), gs.get(i)
            d = p[0] - line if p else ""
            diff = f"   2013: {p[1]}" if p and p[1] != expr else ""
            mark = " " if p and not diff and d == 0 else "*"
            print(f"{mark} {line:>5} {p[0] if p else '':>5} {s[0] if s else '':>5} {d!s:>5}  {expr[:90]}{diff[:100]}")
        for t, lone in (("PLAYER", lp), ("STUDIO", ls)):
            for line, expr in lone:
                print(f"  {t} only-2013 {line:>5}  {expr[:120]}")


MARKER = re.compile(r"^\s*// (FUNCTION|STUB|TEMPLATE|SYNTHETIC|LIBRARY|GLOBAL|VTABLE|SIZE)\b|clang-format")


def plain_comment(lines, i):
    t = lines[i].strip()
    return (t.startswith("//") and not MARKER.match(lines[i])
            and not (i > 0 and re.match(r"^\s*// (TEMPLATE|SYNTHETIC|LIBRARY):", lines[i - 1])))


def layout(path, ignore):
    mine = [m for m in ours(path) if m[0] not in ignore]
    k = key("/client/" + os.path.relpath(os.path.abspath(path), os.path.join(ROOT, "Client")))
    orig = {t: sorted(originals(t).get(k, ())) for t in BINS}
    (gp, _), (gs, _) = align(mine, orig["PLAYER"]), align(mine, orig["STUDIO"])
    anchors = [(line, gp[i][0]) for i, (line, e) in enumerate(mine)
               if i in gp and i in gs and gp[i] == (gs[i][0], e)]
    lines = open(path, encoding="utf-8", errors="surrogateescape").read().split("\n")
    shift, prev = 0, 0  # prev: 0-based index of the previous anchor in the edited file
    for line, want in anchors:
        cur = line - 1 + shift
        need = want - 1 - cur
        if need > 0:
            at = next((j for j in range(cur - 1, prev, -1) if not lines[j].strip()), None)
            if at is None:  # else after the last closing brace line, else right above the anchor
                at = next((j + 1 for j in range(cur - 1, prev, -1) if lines[j].rstrip() in ("}", "};", "\t}", "\t};")), cur)
            lines[at:at] = [""] * need
        while need < 0:
            j = next((j for j in range(cur - 1, prev, -1) if not lines[j].strip()), None)
            if j is None:
                j = next((j for j in range(cur - 1, prev + 1, -1)
                          if plain_comment(lines, j) and plain_comment(lines, j - 1)), None)
                if j is None:
                    print(f"{path}: cannot pull our line {line} up to {want} ({-need} short)")
                    open(path, "w", encoding="utf-8", errors="surrogateescape").write("\n".join(lines))
                    return
                lines[j - 1] = lines[j - 1].rstrip() + " " + lines[j].strip()[2:].strip()
            del lines[j]
            need += 1
            cur -= 1
        shift = want - line
        prev = want - 1
        print(f"{path}: {line} -> {want}")
    open(path, "w", encoding="utf-8", errors="surrogateescape").write("\n".join(lines))


def selftest():
    import tempfile
    src = "x\n\t// RBXASSERT(skip);\n\tRBXASSERT(a  ==\n\t\tb);\n\tRBXASSERT(f(c));\n"
    with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as f:
        f.write(src)
    assert ours(f.name) == [(4, "a == b"), (5, "f(c)")], ours(f.name)
    os.unlink(f.name)
    print("ok")


if __name__ == "__main__":
    a = sys.argv[1:]
    if a == ["--selftest"]:
        selftest()
    elif a and a[0] == "--apply":
        ign = set()
        if a[1] == "--ignore":
            ign = {int(x) for x in a[2].split(",")}
            a = a[2:]
        for f in a[1:]:
            layout(f, ign)
    else:
        main(a)

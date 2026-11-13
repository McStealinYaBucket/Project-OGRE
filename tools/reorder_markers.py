"""Reorder whole marked definitions inside a .cpp so the reccmp markers ascend (decomplint
function_out_of_order), without touching function bodies.

    python3 -P tools/reorder_markers.py plan  [files...]   # per file: descents now / after, or why not
    python3 -P tools/reorder_markers.py apply [files...]   # rewrite the files the plan calls fixable
    python3 -P tools/reorder_markers.py apply --strict F   # runs split at every non-comment line (use when
                                                           # moving across unmarked items breaks the compile)

Default files: every .cpp under Client/ except Client/_generated.

A chunk is a marker block (// FUNCTION|STUB|TEMPLATE|SYNTHETIC|LIBRARY: PLAYER|STUDIO 0x...), the
comment lines directly above it, and the definition after it (to its matching `}`; a declaration or
a `// name` nameref ends at its line). A run is a sequence of chunks separated only by blank and
comment lines and complete unmarked items (gap_ok); those gaps stay where they are. Chunks are permuted inside their run only, so nothing a run does not contain (namespaces,
statics, preprocessor lines, class bodies) moves. A run is sorted by its PLAYER address (FOLDED markers
ignored, as decomplint does); it is applied only when:
  - every chunk in the run has a PLAYER marker,
  - STUDIO markers ascend after the PLAYER sort (else: targets disagree),
  - no chunk or gap whose start line would change contains an assert or other __LINE__ user (else: pinned),
  - the file's PLAYER descents go down and its STUDIO descents do not go up.
A file whose descents cross runs keeps the cross-run descents (reported as layout).
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MARK = re.compile(r"^\s*// (FUNCTION|STUB|TEMPLATE|SYNTHETIC|LIBRARY): (PLAYER|STUDIO) 0x([0-9a-f]{8})( FOLDED\b)?")
PIN = re.compile(r"RBXASSERT(?!_LINE)|RBX_CRASH_ASSERT|RBX_LOG_ASSERT|LEGACY_ASSERT|__LINE__|_FILE_AND_LINE_|RBXPROFILER"
                 r"|RBX_SIGNALS_ASSERT|STATIC_FACTORY_DEFINITIONS|RBX_JUNK|\bassert\s*\(")


def is_comment(s):
    s = s.strip()
    return s.startswith("//") or (s.startswith("/*") and s.endswith("*/"))


def scan_body(lines, i):
    """From line i, find the end line of the declaration/definition that starts there."""
    depth = 0
    in_str = None
    in_block = False
    for j in range(i, len(lines)):
        s = lines[j]
        k = 0
        while k < len(s):
            c = s[k]
            if in_block:
                if s.startswith("*/", k):
                    in_block = False
                    k += 1
            elif in_str:
                if c == "\\":
                    k += 1
                elif c == in_str:
                    in_str = None
            elif s.startswith("//", k):
                break
            elif s.startswith("/*", k):
                in_block = True
                k += 1
            elif c in "\"'":
                in_str = c
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return j
                if depth < 0:
                    return None
            elif c == ";" and depth == 0:
                return j
            k += 1
        if s.rstrip().endswith("\\") is False and in_str:
            in_str = None
    return None


def chunks(lines):
    out = []
    i = 0
    n = len(lines)
    while i < n:
        if not MARK.match(lines[i]):
            i += 1
            continue
        m0 = i
        while i < n and MARK.match(lines[i]):
            i += 1
        marks = [MARK.match(lines[k]) for k in range(m0, i)]
        if i < n and lines[i].strip().startswith("//"):     # nameref
            while i < n and lines[i].strip().startswith("//") and not MARK.match(lines[i]):
                i += 1
            end = i - 1
        else:
            end = scan_body(lines, i) if i < n else None
            if end is None:
                return None
        start = m0
        while start > 0 and is_comment(lines[start - 1]) and not MARK.match(lines[start - 1]) \
                and (not out or start - 1 > out[-1]["end"]):
            start -= 1
        P = [int(m.group(3), 16) for m in marks if m.group(2) == "PLAYER" and not m.group(4)]
        S = [int(m.group(3), 16) for m in marks if m.group(2) == "STUDIO" and not m.group(4)]
        body = "\n".join(lines[start:end + 1])
        out.append({"start": start, "end": end, "P": P, "S": S, "pin": bool(PIN.search(body))})
        i = end + 1
    return out


def gap_ok(lines, a, b):
    """Lines a..b-1 hold only blanks, comments and complete balanced items (unmarked definitions,
    declarations): no preprocessor line, no pragma or REFLECTION_BEGIN/END (they switch data_seg),
    no unbalanced brace (a namespace or class boundary)."""
    i = a
    while i < b:
        s = lines[i].strip()
        if not s or is_comment(s):
            i += 1
            continue
        if s.startswith("#") or "pragma" in s or "REFLECTION_" in s:
            return False
        end = scan_body(lines, i)
        if end is None or end >= b:
            return False
        if any("REFLECTION_" in l or "pragma" in l or l.strip().startswith("#") for l in lines[i:end + 1]):
            return False
        i = end + 1
    return True


def sort_run(lines, new, run):
    """Sort one run by PLAYER address in `new`; return None, or why it stays."""
    if descents([c["P"] for c in run]) == 0:
        return "noop"
    if any(not c["P"] for c in run):
        return "studio-only chunk"
    srt = sorted(run, key=lambda c: c["P"][0])
    if descents([c["S"] for c in srt]) > descents([c["S"] for c in run]):
        return "targets disagree"
    lo, hi = run[0]["start"], run[-1]["end"]
    gaps = [lines[a["end"] + 1:b["start"]] for a, b in zip(run, run[1:])]
    text = []
    for k, c in enumerate(srt):
        if c["pin"] and lo + len(text) != c["start"]:
            return "pinned by assert lines"
        text += lines[c["start"]:c["end"] + 1]
        if k < len(gaps):
            if PIN.search("\n".join(gaps[k])) and lo + len(text) != run[k]["end"] + 1:
                return "pinned by assert lines"
            text += gaps[k]
    new[lo:hi + 1] = text
    return None


def descents(seq_lists):
    flat = [a for l in seq_lists for a in l]
    return sum(1 for a, b in zip(flat, flat[1:]) if b < a)


def plan(path, strict_gaps=False):
    lines = open(path, encoding="utf-8", errors="surrogateescape", newline="").read().split("\n")
    cs = chunks(lines)
    if cs is None:
        return None, "unparsed", lines
    p0, s0 = descents([c["P"] for c in cs]), descents([c["S"] for c in cs])
    if p0 == 0:
        return None, "ok", lines
    runs, cur = [], []
    for c in cs:
        a, b = cur[-1]["end"] + 1 if cur else 0, c["start"]
        if cur and (all(not l.strip() or is_comment(l) for l in lines[a:b]) if strict_gaps else gap_ok(lines, a, b)):
            cur.append(c)
        else:
            if cur:
                runs.append(cur)
            cur = [c]
    if cur:
        runs.append(cur)
    new = list(lines)
    order = []
    reasons = set()

    def strict(a, b):
        return all(not l.strip() or is_comment(l) for l in lines[a:b])

    def do_run(run, can_split):
        why = sort_run(lines, new, run)
        if why == "noop":
            return run
        if why is None:
            return sorted(run, key=lambda c: c["P"][0])
        if can_split:   # retry the comment-only sub-runs (fewer gaps to move across)
            subs, cur = [], [run[0]]
            for c in run[1:]:
                if strict(cur[-1]["end"] + 1, c["start"]):
                    cur.append(c)
                else:
                    subs.append(cur)
                    cur = [c]
            subs.append(cur)
            if len(subs) > 1:
                return [c for sub in subs for c in do_run(sub, False)]
        reasons.add(why)
        return run

    for run in runs:
        order += do_run(run, True)
    p1, s1 = descents([c["P"] for c in order]), descents([c["S"] for c in order])
    if p1 >= p0 or s1 > s0:
        return None, f"P {p0} S {s0}: not fixable ({', '.join(sorted(reasons)) or 'descents cross runs'})", lines
    rest = f" (left: {', '.join(sorted(reasons)) or 'descents cross runs'})" if p1 else ""
    return new, f"P {p0}->{p1} S {s0}->{s1}{rest}", lines


def targets(argv):
    if argv:
        return [os.path.abspath(a) for a in argv]
    out = []
    for dp, dns, fns in os.walk(os.path.join(ROOT, "Client")):
        dns[:] = [d for d in dns if d != "_generated"]
        out += [os.path.join(dp, f) for f in sorted(fns) if f.endswith(".cpp")]
    return sorted(out)


def selftest():
    src = ["// FUNCTION: PLAYER 0x00000020", "void b() { if (1) { } }", "",
           "// FUNCTION: PLAYER 0x00000010", "// FUNCTION: STUDIO 0x00000001", "void a()", "{", "}"]
    cs = chunks(src)
    assert [(c["start"], c["end"]) for c in cs] == [(0, 1), (3, 7)], cs
    import tempfile
    with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as f:
        f.write("\n".join(src))
    new, why, _ = plan(f.name)
    os.unlink(f.name)
    assert new[0] == "// FUNCTION: PLAYER 0x00000010" and new[-1] == "void b() { if (1) { } }", (new, why)
    assert len(new) == len(src)
    print("selftest ok")


def main(argv):
    if argv and argv[0] == "selftest":
        return selftest()
    strict_gaps = "--strict" in argv
    argv = [a for a in argv if a != "--strict"]
    cmd, files = argv[0], targets(argv[1:])
    for p in files:
        new, why, old = plan(p, strict_gaps)
        if why == "ok":
            continue
        print(f"{os.path.relpath(p, ROOT)}: {why}")
        if cmd == "apply" and new is not None and new != old:
            with open(p, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
                f.write("\n".join(new))


if __name__ == "__main__":
    main(sys.argv[1:])

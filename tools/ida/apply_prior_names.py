"""Push the prior 2013 App decomp's function names into a Windows database with the "prior:" tag.

Input: notes/app-marker-placements.csv (one row per STUB marker the import placed: file, line,
function, player/studio address, Mac address and name, pairing class, basis). Per address:
  - IDA auto name (sub_, nullsub_, j_<target>, unknown_libname_): renamed to the prior name, spelled
    as an Itanium-mangled oracle name like tools/ida/apply_names.py does: the row's Mac name when it
    demangles to the prior name, else the one mangled spelling in the iOS/Mac symbol tables
    (ida/exports/{IOS_1029,IOS_1021,MACSTUDIO}-symbols.tsv) whose demangled name normalises to it,
    else the prior qualified name itself (spellings with libstdc++ internals such as __gnu_cxx are
    not used). Several different prior names at one address (ICF
    folding) or a name already used elsewhere: comment only, the latter also a conflict row.
  - an ios:/mac:/ida: name that agrees after normalisation (return type, parameters, template
    arguments, cv-qualifiers, anonymous namespace dropped): comment only.
  - a disagreeing name: kept, comment "prior: <name> (conflict)" and a row in
    ida/exports/<TARGET>-prior-conflicts.csv.
  - match:/lib: (third-party library body) and FLIRT (FUNC_LIB) names: never renamed; the comment
    line is written and a disagreement is a conflict row like any other.
Repeatable function comment lines: "prior: <function> | <verdict> | <file>:<line> | <class>, basis
<basis>". Lines this script wrote before (prior: lines other than apply_names.py's
"prior: in 2013 App decomp:") are replaced, so a rerun changes nothing. Log: ida/logs/<T>-prior.tsv.

Run (snapshot first; saves packed):
    TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/apply_prior_names.py" -Lida/logs/PLAYER-05-prior.log ida/PLAYER.i64
Argument "nosave": measure only. Without IDA (python3 -P tools/ida/apply_prior_names.py) it runs the
normaliser self-test.
"""
import csv
import os
import re
from collections import Counter, defaultdict


def _strip_templates(s):
    out, depth = [], 0
    for c in s:
        if c == "<":
            depth += 1
        elif c == ">":
            depth = max(depth - 1, 0)
        elif depth == 0:
            out.append(c)
    return "".join(out)


OPS = sorted(["()", "[]", "->*", "->", "<<=", ">>=", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "++", "--",
              "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "new[]", "delete[]", "new", "delete",
              "+", "-", "*", "/", "%", "^", "&", "|", "~", "!", "=", "<", ">", ","], key=len, reverse=True)


def norm(s):
    """Qualified function name without return type, parameters, template arguments, cv-qualifiers."""
    s = s.replace("(anonymous namespace)::", "").replace("`anonymous namespace'::", "")
    m = re.search(r"(?:^|::|\s)operator\b", s)
    if m:
        k = m.end()
        rest = s[k:].lstrip()
        op = next((o for o in OPS if rest.startswith(o)), None) or re.match(r"[\w:\s\*&]+?(?=\s*[(<]|$)", rest).group(0)
        qualified = s[m.start():m.start() + 2] == "::"
        head, tail = (s[:m.start()] + "::" if qualified else ""), "operator" + re.sub(r"\s+", "", op)
    else:
        depth, cut = 0, len(s)
        for i, c in enumerate(s):
            depth += (c == "<") - (c == ">")
            if c == "(" and depth == 0:
                cut = i
                break
        head, tail = s[:cut], ""
    head = _strip_templates(head).strip()
    head = head.split()[-1] if head.split() else ""
    head = re.sub(r"^((?:\w+::)+)\1", r"\1", head)      # prior tree artefact: RBX::Reflection::RBX::Reflection::
    return head + tail


def same(prior, existing):
    """The prior name (often missing its enclosing namespace) names the existing function."""
    p, e = norm(prior), norm(existing)
    return p == e or e.endswith("::" + p)


def _selftest():
    eq = [
        ("RBX::AdornBillboarder::isVisibleAndValid", "RBX::AdornBillboarder::isVisibleAndValid() const"),
        ("RBX::SendPhysics::reportSimJobs", "int RBX::SendPhysics::reportSimJobs<RBX::Network::X::JobSender>(RBX::A*)"),
        ("RBX::Lua::Bridge::on_newindex", "RBX::Lua::Bridge<RBX::Lua::AllEnumDescriptors const*, true>::on_newindex(lua_State*)"),
        ("createIdUrl", "(anonymous namespace)::createIdUrl(std::string&, std::string const&, std::string const&)"),
        ("RBX::Edge::~Edge", "public: virtual __thiscall RBX::Edge::~Edge(void)"),
        ("RBX::operator>>", "RakNet::BitStream& RBX::operator>><RBX::StreamRegion::Id>(RakNet::BitStream&, RBX::StreamRegion::Id&)"),
        ("RBX::Lua::ArgumentPusher::operator()", "RBX::Lua::ArgumentPusher::operator()(bool) const"),
        ("RBX::IndexedMesh::getConstIndexedMeshParent", "public: class RBX::IndexedMesh const * __thiscall RBX::IndexedMesh::getConstIndexedMeshParent(void)const"),
    ]
    for a, b in eq:
        assert norm(a) == norm(b), (a, norm(a), norm(b))
    assert norm("bool operator==(G3D::Vector3 const&, G3D::Vector3 const&)") == "operator=="
    assert norm("RBX::Foo::operator bool() const") == "RBX::Foo::operatorbool"
    assert same("Camera::frustum", "RBX::Camera::frustum(G3D::Rect2D const&) const")
    assert same("RBX::Reflection::RBX::Reflection::Variant::genericConvert", "RBX::Reflection::Variant::genericConvert<int>()")
    assert not same("Item::formatValue", "RBX::StatsItem::formatValue()")
    assert norm("RBX::StatsItem::formatValue") != norm("RBX::Stats::Item::formatValue(double, char const*, ...)")
    print("apply_prior_names: selftest ok")


try:
    import ida_auto, ida_bytes, ida_funcs, ida_loader, ida_name, idaapi, idc
except ImportError:
    _selftest()
    raise SystemExit(0)

ida_auto.auto_wait()
SAVE = "nosave" not in idc.ARGV[1:]
IDB = idc.get_idb_path()
TARGET = os.path.splitext(os.path.basename(IDB))[0]
DBDIR = os.path.dirname(IDB)
EXP = os.path.join(DBDIR, "exports")
REPO = os.path.dirname(DBDIR)
COL = {"PLAYER": "player", "STUDIO": "studio"}[TARGET]
AUTO = re.compile(r"^(?:j_)*(?:sub|nullsub|unknown_libname)_[0-9A-Fa-f]+$")
OURS = "prior: "
APPLY_NAMES_LINE = "prior: in 2013 App decomp: "
APPLY_NAMES_BLOCK = ("ios: ", "mac: ", "guess: ", APPLY_NAMES_LINE, "... ")
STAT = Counter()
LOG, CONFLICTS = [], []


def say(*a):
    print("apply_prior_names: " + " ".join(str(x) for x in a))


DEM, FOLDED = {}, defaultdict(list)      # oracle demangled names; every oracle name at an address
for r in csv.DictReader(open(os.path.join(EXP, TARGET + "-names-proposed.csv"))):
    DEM.setdefault(r["mangled"], r["demangled"])
    FOLDED[int(r["address"], 16)].append((r["source_tag"], r["demangled"]))


def demangle(n):
    if n in DEM:
        return DEM[n]
    if n.startswith("_Z"):
        d = ida_name.demangle_name("_" + n, 0) or ida_name.demangle_name(n, 0)
    else:
        d = ida_name.demangle_name(n, 0)
    return d or n


def source(ea, n, f):
    if f.flags & ida_funcs.FUNC_LIB:
        return "flirt:"
    lines = (idc.get_func_cmt(ea, 1) or "").split("\n")
    for l in lines:
        if l.startswith("match: ") and " identical" in l:
            return "match:"
        if l.startswith("lib: ") and " located" in l:
            return "lib:"
    if n.startswith("_Z"):
        for l in lines:
            if l.startswith(("ios: ", "mac: ")):
                return l.split(" ", 1)[0]
    return "ida:"


def is_auto(ea, f, n):
    if ida_bytes.has_dummy_name(ida_bytes.get_flags(ea)) or AUTO.match(n):
        return True
    if n.startswith("j_"):
        tgt = ida_funcs.calc_thunk_func_target(f)[0]
        return tgt != idaapi.BADADDR and n == "j_" + ida_name.get_name(tgt)
    return False


# mangled spellings by normalised name, from the oracle symbol tables
spell = defaultdict(set)
for t in ("IOS_1029", "IOS_1021", "MACSTUDIO"):
    p = os.path.join(EXP, t + "-symbols.tsv")
    if os.path.exists(p):
        for r in csv.DictReader(open(p), delimiter="\t"):
            if r["mangled"].startswith("__Z") and r["type"] in ("T", "t"):
                spell[norm(r["demangled"])].add(r["mangled"][1:])

rows = defaultdict(list)
for r in csv.DictReader(open(os.path.join(REPO, "notes", "app-marker-placements.csv"))):
    if r[COL]:
        rows[int(r[COL], 16)].append(r)


LIBSTDCXX = re.compile(r"__gnu_cxx|St3__|_M_|NSt9__")   # Mac/iOS-only spellings (cf. tools/pair_names.py)


def spelling(fn, r):
    mac = r["mac_name"]
    if mac.startswith("__Z") and not LIBSTDCXX.search(mac):
        d = ida_name.demangle_name(mac[1:], 0) or ida_name.demangle_name(mac, 0)
        if d and same(fn, d):
            return mac[1:], "mac name"
    cands = spell.get(norm(fn), set())
    cands = {c for c in cands if not LIBSTDCXX.search(c)}
    if len(cands) == 1:
        return next(iter(cands)), "oracle symbol table"
    return fn, "prior qualified name" + (" (%d overloads in the symbol tables)" % len(cands) if cands else "")


for ea in sorted(rows):
    rs = rows[ea]
    f = ida_funcs.get_func(ea)
    if not f or f.start_ea != ea:
        STAT["skipped: not a function start"] += 1
        LOG.append((ea, "skip", "not a function start", "", ""))
        continue
    cur = ida_name.get_name(ea)
    auto = is_auto(ea, f, cur) and not (f.flags & ida_funcs.FUNC_LIB)
    src = "auto" if auto else source(ea, cur, f)
    curd = "" if auto else demangle(cur)
    names = sorted({r["function"] for r in rs})
    verdicts = {}
    old = idc.get_func_cmt(ea, 1) or ""
    named_before = {l.split(" | ")[0][len(OURS):]: l.split(" | ")[1] for l in old.split("\n")
                    if l.startswith(OURS) and " | named from the 2013 App decomp" in l}
    if auto:
        names.sort(key=len, reverse=True)       # most qualified spelling first
        if any(not same(n, names[0]) for n in names):
            STAT["auto name, several prior names (folded): comment only"] += 1
            for n in names:
                verdicts[n] = "folded with %d other prior names, not named" % (len(names) - 1)
            LOG.append((ea, "folded", " / ".join(names), cur, ""))
        else:
            fn = names[0]
            want, how = spelling(fn, rs[0])
            mac = rs[0]["mac_name"]
            macd = (ida_name.demangle_name(mac[1:], 0) or mac) if mac.startswith("__Z") else ""
            other = ida_name.get_name_ea(idaapi.BADADDR, want)
            if macd and not same(fn, macd):
                # the prior author's name and the Mac symbol at the Mac address it cites compete: no name
                STAT["auto name, prior and Mac names compete: conflict"] += 1
                verdicts[fn] = "(conflict) Mac symbol at the cited address is %s; not named" % macd
                CONFLICTS.append((ea, cur, "auto", "", fn, rs[0], "Mac name at the cited Mac address: " + macd))
                LOG.append((ea, "compete", fn, cur, macd))
            elif other != idaapi.BADADDR and other != ea:
                STAT["auto name, prior name used elsewhere: conflict"] += 1
                verdicts[fn] = "name in use at 0x%08x (conflict)" % other
                CONFLICTS.append((ea, cur, "auto", "", fn, rs[0], "prior name already names 0x%08x" % other))
                LOG.append((ea, "in-use", want, cur, "0x%08x" % other))
            elif ida_name.set_name(ea, want, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
                got = ida_name.get_name(ea)
                STAT["auto name: renamed (%s)" % how.split(" (")[0]] += 1
                verdicts[fn] = "named from the 2013 App decomp (%s)" % how
                LOG.append((ea, "name", got, cur, how))
            else:
                STAT["auto name: set_name failed"] += 1
                verdicts[fn] = "set_name failed"
                LOG.append((ea, "fail", want, cur, how))
    else:
        for fn in names:
            fold = [t for t, d in FOLDED.get(ea, []) if same(fn, d)]
            if fn in named_before and same(fn, curd):
                want, how = spelling(fn, next(x for x in rs if x["function"] == fn))
                if want != cur and ida_name.set_name(ea, want, ida_name.SN_NOWARN | ida_name.SN_NOCHECK):
                    verdicts[fn] = "named from the 2013 App decomp (%s)" % how
                    STAT["named by an earlier run: respelled"] += 1
                    LOG.append((ea, "respell", want, cur, how))
                    continue
                verdicts[fn] = named_before[fn]
                STAT["named by an earlier run: unchanged"] += 1
            elif same(fn, curd):
                verdicts[fn] = "agrees with %s name" % src
                STAT["%s name agrees: comment only" % src] += 1
            elif fold:
                verdicts[fn] = "agrees with a folded %s name at this address" % fold[0]
                STAT["%s name, prior name is another folded oracle name: comment only" % src] += 1
            else:
                verdicts[fn] = "(conflict) existing %s name kept" % src
                STAT["%s name disagrees: conflict" % src] += 1
                r = next(x for x in rs if x["function"] == fn)
                CONFLICTS.append((ea, cur, src, demangle(cur), fn, r, ""))
            LOG.append((ea, "conflict" if "conflict" in verdicts[fn] else "agree", fn, cur, src))
    lines = []
    for fn in names:
        r = next(x for x in rs if x["function"] == fn)
        lines.append("prior: %s | %s | %s:%s | %s, basis %s" % (
            fn, verdicts[fn], r["file"], r["decl_line_before_markers"], r["evidence_class"], r["basis"]))
    keep = [l for l in old.split("\n") if l and not (l.startswith(OURS) and not l.startswith(APPLY_NAMES_LINE))]
    # before apply_names.py's block (it always appends its block last), so neither script reorders the other
    at = next((i for i, l in enumerate(keep) if l.startswith(APPLY_NAMES_BLOCK)), len(keep))
    new = "\n".join(keep[:at] + lines + keep[at:])
    if new != old:
        idc.set_func_cmt(ea, new, 1)
        STAT["comments written"] += 1

with open(os.path.join(EXP, TARGET + "-prior-conflicts.csv"), "w", newline="") as fh:
    w = csv.writer(fh)
    w.writerow(["address", "existing_name", "existing_source", "existing_demangled", "prior_name",
                "prior_file", "prior_line", "map_evidence_class", "basis", "mac_name", "note"])
    for ea, cur, src, dem, fn, r, note in CONFLICTS:
        w.writerow(["0x%08x" % ea, cur, src, dem, fn, r["file"], r["decl_line_before_markers"],
                    r["evidence_class"], r["basis"], r["mac_name"], note])
with open(os.path.join(DBDIR, "logs", TARGET + "-prior.tsv"), "w") as fh:
    fh.write("address\taction\tdetail\told_name\tnote\n")
    for ea, act, det, cur, note in LOG:
        fh.write("0x%08x\t%s\t%s\t%s\t%s\n" % (ea, act, det, cur, note))
say("addresses", len(rows), "conflict rows", len(CONFLICTS))
for k, v in sorted(STAT.items()):
    say("  %-55s %d" % (k, v))
if SAVE:
    ida_loader.save_database(IDB, ida_loader.DBFL_COMP)
    say("saved packed")
else:
    idaapi.set_database_flag(idaapi.DBFL_KILL)
idc.qexit(0)

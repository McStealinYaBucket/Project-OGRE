"""Signature check of proposed PLAYER names against x86 MSVC ABI facts (adversarial review).

1. On a scratch copy of ida/PLAYER.i64, dump the facts for every proposed address:
     cut -d, -f1 ida/exports/PLAYER-names-proposed.csv | tail -n +2 | sort -u > $D/addrs.txt
     TVHEADLESS=1 ~/ida-pro-9.4/idat -A -S"tools/ida/abi_facts.py $D/addrs.txt $D/abi.jsonl" -L$D/l $D/PLAYER.i64
2. python3 -P tools/check_names_abi.py $D/abi.jsonl [--sample N --seed S] [--out rows.csv]

Each proposed row is tested on the stack bytes the callee pops (`ret N`, IDA's argsize) and on
whether ecx is live at entry. Expected purge from the Itanium parameter list under MSVC x86:
  __thiscall member: N = params (+4 when the return value goes through a hidden pointer);
  cdecl (free function, static member, any variadic): N = 0, ecx not an argument;
  D0 row (paired with the MSVC scalar deleting destructor): N = 4; D1/D2: N = 0; C1/C2: N = params.
A by-value class whose MSVC size is not in SIZES counts as >= 4 bytes with no upper bound.
A two-parameter binary operator is a free function. A method the 2016 headers declare `static`
(same class, parameter count and first parameter type; no non-static overload) is cdecl. One extra
stack word with ecx unused is LTCG's this-on-stack convention: uncertain, not wrong.
Facts for every PLAYER function are kept in ida/exports/PLAYER-evidence/abi.jsonl (abi_facts.py
with `nohr`); tools/pair_names.py uses classifier() from here as a gate.
Verdicts: wrong (impossible), uncertain (only consistent through an assumption), ok.
Self-check: python3 -P tools/check_names_abi.py --selftest
"""
import csv, json, random, re, sys
from collections import Counter, defaultdict

sys.path.insert(0, "tools")
from pair_names import norm_class, strip_templates  # noqa: E402

EXP = "ida/exports"
B4 = {"bool", "char", "signed char", "unsigned char", "short", "unsigned short", "int", "unsigned int",
      "long", "unsigned long", "float", "wchar_t"}
B8 = {"double", "long long", "unsigned long long", "long double", "__int64"}
# MSVC 9 sizes of by-value classes, rounded up to the 4-byte stack slot
SIZES = {"G3D::Vector2": 8, "G3D::Vector3": 12, "G3D::Vector4": 16, "G3D::Color3": 12, "G3D::Color4": 16,
         "G3D::Color3uint8": 4, "G3D::Color4uint8": 4, "G3D::Vector2int16": 4, "G3D::Vector3int16": 8,
         "G3D::Matrix3": 36, "G3D::CoordinateFrame": 48, "G3D::Rect2D": 16, "G3D::Quat": 16,
         "std::string": 28, "boost::shared_ptr": 8, "boost::weak_ptr": 8, "boost::intrusive_ptr": 4,
         "shared_ptr": 8, "weak_ptr": 8, "RBX::Velocity": 24, "RBX::Extents": 24}
INF = 10 ** 6
THUNK_SIZE = {}
try:
    for _r in csv.DictReader(open(EXP + "/PLAYER-functions.csv")):
        if int(_r["size"], 0) <= 6:
            THUNK_SIZE[int(_r["address"], 16)] = int(_r["size"], 0)
except (OSError, KeyError, ValueError):
    pass
VMP = set()
try:
    for _l in open("notes/vmprotect-player.csv"):
        _m = re.search(r"func ([0-9a-f]{8})", _l)
        if _m:
            VMP.add(int(_m.group(1), 16))
except OSError:
    pass


def split_top(s, sep=","):
    out, depth, cur = [], 0, ""
    for c in s:
        if c in "<([":
            depth += 1
        elif c in ">)]":
            depth -= 1
        if c == sep and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += c
    if cur.strip():
        out.append(cur.strip())
    return out


def parse(dem):
    """-> dict(name, qual, params[list], const, ret[str or None]) or None."""
    s = re.sub(r"\s*\[[^\]]*\]\s*$", "", dem).strip()  # pair_names' "[D0, deleting ...]" note
    const = False
    while True:
        m = re.search(r"\s+(const|volatile|&&|&)$", s)
        if not m:
            break
        const |= m.group(1) == "const"
        s = s[:m.start()]
    if not s.endswith(")"):
        return None
    depth = 0
    for i in range(len(s) - 1, -1, -1):
        depth += {")": 1, "(": -1}.get(s[i], 0)
        if depth == 0:
            break
    head, plist = s[:i], s[i + 1:-1]
    params = [] if plist.strip() in ("", "void") else split_top(plist)
    # return type: a top-level space before the qualified name (template functions only)
    parts = split_top(head, " ")
    ret = " ".join(parts[:-1]) if len(parts) > 1 and not head.startswith("operator") else None
    qname = parts[-1] if parts else head
    if "operator" in head and not ret:
        qname = head
    comps = split_top(qname.replace("::", "\0"), "\0")
    return {"name": comps[-1], "qual": "::".join(comps[:-1]), "params": params, "const": const, "ret": ret}


def psize(t):
    """-> (lo, hi) stack bytes of one parameter."""
    t = t.strip()
    if t == "...":
        return 0, 0
    if t.endswith(("*", "&", "* const", "& const")) or "(*)" in t or "(&)" in t or t.endswith("*)"):
        return 4, 4
    base = re.sub(r"\b(const|volatile)\b", "", t).strip()
    base = re.sub(r"\s+", " ", base)
    if base in B4:
        return 4, 4
    if base in B8:
        return 8, 8
    key = strip_templates(base).strip()
    if key in SIZES:
        return SIZES[key], SIZES[key]
    if base.startswith("std::basic_string<char"):
        return 28, 28
    if "::*" in base:
        return 4, 16  # MSVC pointer to member: 4, 8, 12 or 16 bytes
    return 4, INF


BINOP = re.compile(r"^operator\s*(?:==|!=|<=|>=|<<|>>|&&|\|\||[<>+\-*/%&|^])$")


def expect(row, p, virtual, static=False):
    """-> (kind, allowed list of (lo, hi) purge ranges, ecx must be an argument?)."""
    mg = row["mangled"]
    vari = "..." in p["params"]
    lo = sum(psize(x)[0] for x in p["params"])
    hi = min(INF, sum(psize(x)[1] for x in p["params"]))
    if re.search(r"D0Ev$", mg):
        return "D0", [(4, 4)], True
    if re.search(r"D[12]Ev$", mg):
        return "D12", [(0, 0)], True
    ctor = re.search(r"C[123]E", mg) and p["qual"] and p["name"] == strip_templates(p["qual"].split("::")[-1])
    if vari:
        return "variadic", [(0, 0)], False
    sret = [(lo + 4, hi + 4 if hi < INF else INF)]
    if p["ret"] is not None:  # template: return type known
        r = p["ret"].strip()
        b = re.sub(r"\b(const|volatile)\b", "", r).strip()
        if r.endswith(("*", "&")) or b in B4 or b in B8 or b == "void":
            sret = []
    if ctor:
        return "ctor", [(lo, hi)], True
    if virtual or p["const"]:
        return "member", [(lo, hi)] + sret, True
    if not mg.startswith(("_ZN", "_ZZ")):  # _ZZ: a local class's member (e.g. a factory in a function)
        return "free", [(0, 0)], False
    if BINOP.match(p["name"]) and len(p["params"]) == 2:
        return "free", [(0, 0)], False  # a member binary operator takes one parameter
    if static:
        return "static", [(0, 0)], False
    return "member-or-static", [(0, 0), (lo, hi)] + sret, None


def check(row, p, f, virtual, is_class, vbase=None, static=False):
    """vbase: True / False / None (class has virtual bases in PLAYER's RTTI / has none / no RTTI).
    static: the 2016 source declares this method `static` inside its class."""
    kind, allowed, ecx = expect(row, p, virtual, static and not virtual)
    if kind == "ctor" and vbase is not False:
        # MSVC passes a hidden `int` most-derived flag to constructors of classes with virtual bases
        allowed = allowed + [(a + 4, b + 4) for a, b in allowed]
    if p["qual"] and is_class is False and kind == "member-or-static":
        kind, allowed, ecx = "namespace-free", [(0, 0)], False
    if f is None or f.get("start") is None:
        return kind, "uncertain", "no PLAYER function at this address"
    if f.get("start") in VMP:
        return kind, "uncertain", "VMProtect-wrapped body (notes/vmprotect-player.csv); purge unreadable"
    if f.get("noret"):
        return kind, "uncertain", "noreturn body (throws); purge unknown"
    rets = set(f.get("rets", []))
    trets = {r for r in f.get("tail_chunk_rets", []) if r}
    tail = ""
    if len(rets - {0}) == 1:
        n = (rets - {0}).pop()
        if 0 in rets:
            tail = " (mixed ret 0 / ret %d in the entry chunk)" % n
    elif rets == {0}:
        n = 0
    elif not rets and len(trets) == 1:
        n, tail = trets.pop(), " (ret in a tail chunk)"
    else:
        ts = {a for t, a in f.get("tails", []) if THUNK_SIZE.get(t, 99) > 6}  # not an import jmp thunk
        if rets or len(ts) != 1:
            return kind, "uncertain", "no usable ret (tail-jump only or several purges); purge unknown"
        n, tail = ts.pop(), " (purge of the tail-jump target)"
    ok = any(a <= n <= b for a, b in allowed)
    why = "ret %d, expected %s for %s %s" % (n, "/".join("%d" % a if a == b else "%d..%s" % (a, "inf" if b >= INF else b)
                                                    for a, b in allowed), kind, p["params"]) + tail
    if ok and kind == "ctor" and vbase is None and n > allowed[0][1]:
        return kind, "uncertain", "needs a virtual base, and the class has no RTTI in PLAYER: " + why
    if not ok and kind in ("D12", "ctor", "member", "member-or-static") and not f.get("ecx_in") \
            and any(a + 4 <= n <= b + 4 for a, b in allowed):
        # LTCG custom convention: `this` passed on the stack (CRenderSettings 0x0044f380-0x00458f40)
        return kind, "uncertain", "ecx unused and one extra stack word: LTCG this-on-stack? " + why
    if not ok:
        return kind, "wrong", why
    if ecx is False and f.get("ecx_in") and n == 0 and kind != "variadic":
        return kind, "uncertain", "ecx live at entry but %s is cdecl (LTCG custom cc, or a member): %s" % (kind, why)
    if kind == "member-or-static" and n == 0 and f.get("ecx_in") and allowed[1][0] > 0:
        return kind, "uncertain", "ret 0 with ecx live: a member with stack params would pop them (LTCG?): " + why
    if kind in ("member-or-static",) and n == 0 and not f.get("ecx_in") and allowed[1][0] > 0:
        return kind, "ok", "static (ret 0, no ecx)"
    return kind, "ok", why


def src2016_scopes(root="ref/src2016/extracted/robloxsrc/Trunk2012"):
    """(namespace identifiers, class/struct identifiers) declared anywhere in the 2016 tree."""
    import os
    ns, cl = Counter(), set()
    nsre = re.compile(r"\bnamespace\s+([A-Za-z_]\w*)")
    clre = re.compile(r"\b(?:class|struct)\s+(?:\w+\s+)?([A-Za-z_]\w*)\s*(?:final\s*)?(?::[^;{]*)?\{", re.S)
    for d, _, fs in os.walk(root):
        for fn in fs:
            if fn.endswith((".h", ".hpp", ".cpp", ".inl", ".mm")):
                t = open(os.path.join(d, fn), encoding="latin-1").read()
                ns.update(nsre.findall(t))
                cl.update(clre.findall(t))
    return {k for k, v in ns.items() if v >= 3}, cl


def param_sig(params):
    """-> (parameter count, last type identifier of the first parameter) for a list of parameter
    texts, with or without parameter names ("lua_State *L", "RBX::ContentId", "unsigned int i")."""
    if not params:
        return 0, ""
    t = re.sub(r"\w+::", "", re.sub(r"=.*$", "", strip_templates(params[0])))
    w = [x for x in re.findall(r"\w+", t) if x not in ("const", "volatile", "unsigned", "signed", "struct", "class", "typename")]
    named = len(w) >= 2 and not re.search(r"[*&]\s*$", t) and not t.strip().endswith(w[-1] + ">")
    return len(params), (w[-2] if named and len(w) >= 2 else w[-1]) if w else ""


def src2016_statics(root="ref/src2016/extracted/robloxsrc/Trunk2012"):
    """{(class name, method name, param_sig)} declared `static` directly inside a class body in a
    2016 header, minus those also declared without `static` there. Static-ness rarely changes but
    is still a 2016 fact, so a match needs the same parameter count and first parameter type."""
    import os
    tok = re.compile(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\\n])*\"|'(?:\\.|[^'\\\n])*'"
                     r"|\b(?:class|struct)\s+(?:[A-Z_][A-Z0-9_]*\s+)*(\w+)\s*(?:final\s*)?(?::[^;{()]*)?\{"
                     r"|\{|\}|\bstatic\s+[\w:<>,*&\s]+?\b(operator\s*[^\s(]+|~?\w+)\s*\((?:([^()]*)\))?"
                     r"|\b(operator\s*[^\s(]+|~?\w+)\s*\((?:([^()]*)\))?", re.S)
    out, other = set(), set()
    for d, _, fs in os.walk(root):
        for fn in fs:
            if not fn.endswith((".h", ".hpp", ".inl")):
                continue
            stack = []
            for m in tok.finditer(open(os.path.join(d, fn), encoding="latin-1").read()):
                t = m.group(0)
                if m.group(1):
                    stack.append(m.group(1))
                elif t == "{":
                    stack.append(None)
                elif t == "}":
                    if stack:
                        stack.pop()
                elif stack and stack[-1] and (m.group(2) or m.group(4)):
                    nm, pl = (m.group(2), m.group(3)) if m.group(2) else (m.group(4), m.group(5))
                    if pl is None:
                        continue
                    ps = [] if pl.strip() in ("", "void") else split_top(pl)
                    (out if m.group(2) else other).add((stack[-1], nm.replace(" ", ""), param_sig(ps)))
    return out - other


def selftest():
    p = parse("RBX::Foo::bar(int, G3D::Vector3 const&, double) const")
    assert p["name"] == "bar" and p["qual"] == "RBX::Foo" and p["const"] and len(p["params"]) == 3, p
    assert sum(psize(x)[0] for x in p["params"]) == 16
    p = parse("RBX::Name const& RBX::Name::doDeclare<RBX::sPluginManager>()")
    assert p["ret"] == "RBX::Name const&" and p["params"] == [], p
    p = parse("boost::detail::sp_counted_impl_pd<A<B, C>, D>::~sp_counted_impl_pd() [D0, deleting: x]")
    assert p["name"] == "~sp_counted_impl_pd" and p["params"] == [], p
    assert psize("boost::shared_ptr<RBX::Instance>") == (8, 8)
    assert psize("void (*)(int)") == (4, 4)
    assert expect({"mangled": "_ZN3RBXeqERKNS_9ContentIdES2_"}, parse("RBX::operator==(RBX::ContentId const&, RBX::ContentId const&)"), False)[0] == "free"
    assert expect({"mangled": "_ZN1A1fEi"}, parse("A::f(int)"), False, True)[1] == [(0, 0)]
    assert param_sig(["lua_State *L", "Param1 param1"]) == param_sig(["lua_State*", "G3D::Vector3"]) == (2, "lua_State")
    assert param_sig(["const Instance* context"])[1] == "Instance" and param_sig(["RBX::ContentId"])[1] == "ContentId"
    assert param_sig(["unsigned int index"])[1] == "int" == param_sig(["unsigned int"])[1]
    print("selftest ok")


def classifier(target="PLAYER", oracles=("IOS_1029", "MACSTUDIO")):
    """-> verdict(row, facts) -> (kind, verdict, why); row needs mangled, demangled, evidence_class.
    Also used by tools/pair_names.py as a gate (a "wrong" verdict rejects a candidate)."""
    if target != "PLAYER":  # the module-level tables describe PLAYER
        VMP.clear()
        THUNK_SIZE.clear()
        for r in csv.DictReader(open(EXP + "/%s-functions.csv" % target)):
            if int(r["size"], 0) <= 6:
                THUNK_SIZE[int(r["address"], 16)] = int(r["size"], 0)
    player_cls = set()
    for l in open(EXP + "/%s-evidence/vtables.jsonl" % target):
        player_cls.add(norm_class(json.loads(l)["class"]))
    virt_names, oracle_cls = set(), set()
    for t in oracles:
        for l in open(EXP + "/%s-evidence/vtables.jsonl" % t):
            d = json.loads(l)
            oracle_cls.add(norm_class(d.get("class") or ""))
            for g in d.get("groups", []):
                for s in g.get("slots", []):
                    virt_names.add((s.get("name") or "").lstrip("_"))
    # classes = anything with a vtable, a ctor/dtor or a const member in the oracle symbols
    known_cls = set(player_cls) | oracle_cls
    for l in open(EXP + "/%s-evidence/functions.jsonl" % oracles[0]):
        d = json.loads(l)
        dm = d.get("demangled") or ""
        nm = (d.get("name") or "").lstrip("_")
        if nm.startswith("ZNK") or re.search(r"(C[123]|D[012])E", nm):
            p = parse(dm)
            if p and p["qual"]:
                known_cls.add(norm_class(p["qual"]))
    known_bare = {strip_templates(k) for k in known_cls}
    ns2016, cls2016 = src2016_scopes()
    statics = src2016_statics()
    vb = {}
    for l in open(EXP + "/%s-evidence/vtables.jsonl" % target):
        d = json.loads(l)
        vb[strip_templates(norm_class(d["class"]))] = any(b.get("pdisp", -1) != -1 for b in d.get("hierarchy", {}).get("bases", []))

    def verdict(r, f):
        p = parse(r["demangled"])
        if not p:
            return "unparsed", "uncertain", "demangled name has no parameter list"
        mg = r["mangled"]
        virtual = mg in virt_names or r["evidence_class"] == "rtti-slot"
        qc = norm_class(p["qual"]) if p["qual"] else ""
        # a template-id qualifier is always a class; otherwise compare with templates stripped.
        # cdecl is only expected when the qualifier is provably a namespace: its last component
        # is a namespace in the 2016 tree and no class of that name exists anywhere we know
        last = strip_templates(p["qual"].split("::")[-1]) if p["qual"] else ""
        is_class = None
        if not p["qual"]:
            is_class = False
        elif "<" in p["qual"].split("::")[-1] or qc in known_cls or strip_templates(qc) in known_bare:
            is_class = True
        elif last in ns2016 and last not in cls2016 and not any(k.endswith("::" + last) or k == last for k in known_bare):
            is_class = False
        static = bool(last) and (last, strip_templates(p["name"]).replace(" ", ""), param_sig(p["params"])) in statics
        return check(r, p, f, virtual, is_class, vb.get(strip_templates(qc)), static)
    return verdict


def main(argv):
    if "--selftest" in argv:
        return selftest()
    facts = {}
    for l in open(argv[0]):
        d = json.loads(l)
        facts[d["ea"]] = d
    seed = int(argv[argv.index("--seed") + 1]) if "--seed" in argv else 20261112
    nsample = int(argv[argv.index("--sample") + 1]) if "--sample" in argv else 80
    out = argv[argv.index("--out") + 1] if "--out" in argv else None
    rows = list(csv.DictReader(open(EXP + "/PLAYER-names-proposed.csv")))

    verdict = classifier()
    res = []
    for r in rows:
        kind, v, why = verdict(r, facts.get(int(r["address"], 16)))
        res.append((r, kind, v, why))

    c = Counter((v) for _, _, v, _ in res)
    print("all rows:", dict(c))
    print("by class:")
    for ec in sorted({r["evidence_class"] for r in rows}):
        print("  %-16s %s" % (ec, dict(Counter(v for r, _, v, _ in res if r["evidence_class"] == ec))))
    print("by kind:", dict(Counter((k, v) for _, k, v, _ in res if v == "wrong")))
    if out:
        with open(out, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["address", "mangled", "demangled", "evidence_class", "oracle", "kind", "verdict", "why"])
            for r, k, v, y in res:
                w.writerow([r["address"], r["mangled"], r["demangled"], r["evidence_class"], r["oracle"], k, v, y])
    # fixed-seed sample of rows with a parameter list, stratified over classes
    pool = [x for x in res if x[1] != "unparsed" and parse(x[0]["demangled"])["params"]]
    rnd = random.Random(seed)
    bycls = defaultdict(list)
    for x in pool:
        bycls[x[0]["evidence_class"]].append(x)
    sample = []
    per = max(1, nsample // len(bycls))
    for ec in sorted(bycls):
        sample += rnd.sample(bycls[ec], min(per, len(bycls[ec])))
    rest = [x for x in pool if x not in sample]
    sample += rnd.sample(rest, nsample - len(sample))
    print("\nsample seed %d, %d rows:" % (seed, len(sample)))
    for r, k, v, y in sample:
        print("%s\t%s\t%s\t%s\t%s\t%s" % (r["address"], r["evidence_class"], v, k, r["demangled"][:110], y[:160]))


if __name__ == "__main__":
    main(sys.argv[1:])

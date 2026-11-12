"""Pair a Windows target's functions with Itanium-symbolled oracle builds and propose names.

Works only from the evidence exports (ida/exports/<TARGET>-evidence/{functions,vtables}.jsonl);
no database is opened. Follows CLAUDE.md "Mac Symbol Transfer": pair by evidence, never by
address. Each oracle is paired on its own, strongest evidence class first:

  rtti-slot      class matched by name between MSVC RTTI and Itanium _ZTV; slots aligned after
                 folding Itanium D1+D0 into MSVC's one scalar deleting destructor and dropping
                 Itanium primary-group copies of overriders of non-primary bases (they live only
                 in the secondary vftable on MSVC). A class whose slot counts still disagree, or
                 whose pure-virtual slots disagree, pairs nothing. Slots inside the span of an
                 overload set (MSVC regroups overloads) are skipped.
  string-unique  a literal referenced by exactly one function on each side. Asserts compare as
                 "expr file: <basename> line: N"; a bare __FILE__ path is never used.
  string-set     identical string sets, each unique on its side.
  import-set     identical sets of external callees (both manglings demangled, reduced to the
                 qualified name, restricted to names both sides import), unique on both sides,
                 with matching strings and block/callee counts of the same shape.
  callgraph      seeded from the pairs above: when a paired function has exactly one unpaired
                 callee (or caller) on each side, those two are a candidate. Two independent
                 seeds, or both oracles naming the same function, make it strong; one seed is
                 only a guess.

  body-match     (--body-match B) the target function's body is byte-identical, addresses masked, to
                 a function of build B that B's own names-proposed.csv names (tools/body_match.py
                 writes ida/exports/<TARGET>-<B>-bodymatch.csv). Two uses: (1) each oracle is
                 seeded with the oracle function of that name (unique after merging C1/C2/D1/D2)
                 when B's row is strong, so the callgraph pass grows from it; (2) B itself is a
                 pseudo-oracle in write_names, tagged with B's own source_tag and evidence
                 "player: body-identical to <B> 0x...". An oracle naming the function otherwise
                 is a conflict and names nothing.
                 Body-match pairs rank last as callgraph witnesses: validate() lets them refute
                 only body-match and callgraph pairs (ranked second, they dropped 25 correct
                 string-unique/string-set pairs in STUDIO, e.g. the GuiBuilder::build* family,
                 where MSVC and clang keep different call levels out of line).

Every candidate must also pass a string test (strings one side has and the other side's
vocabulary knows must sit in the partner or its callees, which is what inlining moves) and a
callgraph test (paired callees must agree, to depth 2) before it is accepted. A function that
gets two names from one oracle is kept only when the oracle bodies look like ICF folding;
iOS/Mac disagreements go to <TARGET>-names-conflicts.csv and name nothing.

Gates added after the 2026-11-12 adversarial review, for every class but rtti-slot:
  - x86 ABI: `ret N` and ecx liveness from ida/exports/<TARGET>-evidence/abi.jsonl (written by
    tools/ida/abi_facts.py over every function) must fit the Itanium parameter list
    (tools/check_names_abi.py; a "wrong" verdict rejects). Catches an inlined function's name
    landing on its out-of-line callee or caller (serialize<T> on Property::getValue<T>).
  - a name the target imports (same qualified name with template arguments, same arity) has no
    body in the image; Apple-only names (NSObject, Mac*:: classes) and libstdc++ internals
    (_Rb_tree::_M_erase, std::__x, __gnu_cxx) cannot be Windows names.
  - an oracle C1/C2/D1/D2 that sets a vftable needs a target body that sets one or calls a structor.
  - inlining level shift (Pairing.level_shift): the pair is one call level apart (Name::declare<X>
    vs doDeclare<X>); rejected, and in the callgraph pass re-aimed at the shared level when free.
    validate() re-checks it after propagation and propagation runs again after drops.
  - import-set keys of only std::/C-runtime imports need a third import or agreeing paired callees.
Names with libstdc++ types in template arguments keep their row but become guess: (the pairing
is right, the spelling is not MSVC's). At a folded address the most-voted name comes first and
each row carries only its own oracle function's evidence.

Run from the repo root:
  python3 -P tools/pair_names.py --target PLAYER --oracle ios1029=IOS_1029 --oracle mac=MACSTUDIO \
      --pe bin/player_1023/RobloxPlayerBeta.exe --app-decomp "2013 55% App"
  python3 -P tools/pair_names.py --target STUDIO --oracle ios1021=IOS_1021 --oracle mac=MACSTUDIO \
      --pe bin/studio/RobloxStudioBeta.exe --app-decomp "2013 55% App" --body-match PLAYER

Writes ida/exports/<TARGET>-names-proposed.csv, <TARGET>-names-conflicts.csv and, with
--app-decomp, <TARGET>-app-decomp-map.csv; prints the counts. `--selftest` runs the checks of
the small helpers.
"""
import argparse
import bisect
import csv
import json
import os
import re
import struct
import subprocess
import sys
from collections import Counter, defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))  # check_names_abi

EXP = "ida/exports"
UNNAMED = ("sub_", "nullsub_", "j_", "loc_", "unknown_")
DTOR = re.compile(r"D[012]Ev$")
ASSERT = re.compile(r"^(.*) file: (.+?) line: (\d+)$", re.S)
PATHLIKE = re.compile(r"\.(?:cpp|cc|c|h|hpp|inl|mm|m)$", re.I)
STRONG = ("rtti-slot", "string-unique", "string-set", "import-set", "callgraph", "body-match")
# compiler-dependent externals (intrinsics, allocation): never part of an import-set key
IMPORT_NOISE = {"operator new", "operator delete", "operator new[]", "operator delete[]", "memcpy",
                "memmove", "memset", "strlen", "strcmp", "memcmp", "abs", "fabs", "sqrt", "sqrtf"}
# names that cannot exist in a Windows build: Objective-C / CoreFoundation types, Apple-only classes
APPLE = re.compile(r"[-+]\[\w+ |_block_invoke|\bNS[A-Z][a-z]\w*|\bobjc_|\bCF[A-Z]\w*Ref\b|\b(?:Mac|IOS|iOS|Ios|Cocoa)[A-Z]\w*::")
# libstdc++/libc++ implementation functions (Dinkumware has none of them): reject the name
GNU_FN = re.compile(r"^(?:std::(?:__\w+|_[A-Z]\w*)|__gnu_cxx::)|::_M_\w+$")
# libstdc++ types in template arguments: the pairing may be right, the spelling is not MSVC's
GNU_ARG = re.compile(r"__gnu_cxx::|\bstd::_[A-Z]\w*|\bstd::__\w+")


def base_name(n):
    """`__ZThn24_N3RBX8Instance...` (a this-adjusting or virtual thunk) -> `ZN3RBX8Instance...`."""
    n = (n or "").lstrip("_")
    return re.sub(r"^ZT[hv](?:n?\d+_)+", "Z", n)


def strip_templates(s):
    out, depth = [], 0
    i = 0
    while i < len(s):
        c = s[i]
        if c == "<" and not re.search(r"operator\s*<*$", s[:i]):
            depth += 1
        elif c == ">" and depth and not re.search(r"operator\s*-?$", s[:i]):
            depth -= 1
        elif depth == 0:
            out.append(c)
        i += 1
    return "".join(out)


def qualified(dem):
    """Demangled signature (either ABI) -> qualified name without templates, params or return."""
    s = strip_templates(dem or "")
    s = s.split("(", 1)[0] if "(" in s and "operator()" not in s else s
    m = re.search(r"((?:[\w~`']+::)*(?:operator\s*\S+(?:\s\w+)?|operator\(\)|[\w~`']+))\s*$", s.strip())
    return m.group(1) if m else s.strip()


def unqualified(dem):
    return qualified(dem).split("::")[-1]


def norm_class(c):
    c = c.replace("`anonymous namespace'", "anon").replace("(anonymous namespace)", "anon")
    c = re.sub(r"\b(?:class|struct|union|enum) ", "", c)
    c = c.replace("char const * const ", "").replace("char const* const ", "")
    c = re.sub(r"\([^()]*\)", "", c).replace("&", "").replace(" ", "")
    return c.replace("std::basic_string<char,std::char_traits<char>,std::allocator<char>>", "std::string")


def sig_key(dem):
    """Demangled function (either ABI) -> (qualified name, parameter count), None for data."""
    from check_names_abi import parse
    p = parse(re.sub(r"\s*\[[^\]]*\]\s*$", "", dem or ""))
    if not p:
        return None
    qn = ((p["qual"] + "::") if p["qual"] else "") + p["name"]
    return norm_class(qn.replace("\0", "::")), len(p["params"])


def name_problem(dem):
    """-> reason the oracle name cannot be a Windows function name, or None."""
    if APPLE.search(dem or ""):
        return "Apple-only name"
    if GNU_FN.search(qualified(dem)):
        return "libstdc++ implementation function"
    return None


def basename(p):
    return re.split(r"[\\/]", p)[-1].lower()


def norm_str(s):
    """-> (key, is_file). Assert records keep expression and line, path reduced to its basename."""
    m = ASSERT.match(s)
    if m:
        return "%s file: %s line: %s" % (m.group(1), basename(m.group(2)), m.group(3)), False
    if PATHLIKE.search(s) and ("/" in s or "\\" in s):
        return "FILE:" + basename(s), True
    return s, False


def canon(n):
    """Itanium name with complete/base/deleting ctor and dtor variants merged (MSVC has one)."""
    return re.sub(r"(?<=[0-9A-Za-z_])(C[1-3]|D[12])(?=E)", lambda m: m.group(1)[0] + "*", n)


def out_name(n):
    """Mach-O symbol -> plain Itanium/C name (drop the Mach-O underscore)."""
    return n[1:] if n.startswith("_") else n


def demangle(names, msvc):
    """{name: demangled} through llvm-undname / llvm-cxxfilt (C names come back as themselves)."""
    names = sorted(set(names))
    if not names:
        return {}
    if msvc:
        mangled = [n for n in names if n.startswith("?")]
        out = {n: n for n in names}
        if mangled:
            txt = subprocess.run(["llvm-undname"], input="\n".join(mangled) + "\n", capture_output=True, text=True).stdout
            lines = txt.split("\n")
            want = set(mangled)
            for i, l in enumerate(lines[:-1]):
                if l in want and not lines[i + 1].startswith("error"):
                    out[l] = lines[i + 1]
        return out
    txt = subprocess.run(["llvm-cxxfilt"], input="\n".join(names) + "\n", capture_output=True, text=True).stdout.split("\n")
    return {n: (d if n.lstrip("_").startswith("Z") else n.lstrip("_")) for n, d in zip(names, txt)}


class Side:
    def __init__(self, target, msvc):
        self.target, self.msvc = target, msvc
        self.F = {}
        for l in open(os.path.join(EXP, target + "-evidence", "functions.jsonl")):
            d = json.loads(l)
            self.F[d["address"]] = d
        self.addrs = sorted(self.F)
        # Itanium-only wrappers MSVC has no counterpart for: a C1/D1 that only calls its C2/D2
        # sibling, IDA `j_` thunks and armv7 `$shim` islands. They are removed and their edges
        # redirected to what they call, so the callgraph never pairs a target function with one.
        self.alias = {}
        self.allF, self.alladdrs = dict(self.F), self.addrs
        if not msvc:
            sib = defaultdict(set)
            for a, d in self.F.items():
                n = base_name(d["name"])
                if re.search(r"(C[1-3]|D[12])E", n):
                    sib[canon(n)].add(a)
            for a, d in self.F.items():
                cs = set(d["callees"]) - {a}
                n = d["name"]
                if len(cs) != 1 or d["size"] > 64:
                    continue
                c = next(iter(cs))
                if n.startswith("j_") or n.endswith("$shim") or c in sib.get(canon(base_name(n)), ()):
                    self.alias[a] = c
            for a in list(self.alias):
                seen = {a}
                while self.alias[a] in self.alias and self.alias[a] not in seen:
                    seen.add(self.alias[a])
                    self.alias[a] = self.alias[self.alias[a]]
            for a in self.alias:
                del self.F[a]
            self.addrs = sorted(self.F)
        self.calls, self.callers = {}, defaultdict(set)
        for a, d in self.F.items():
            c = {self.alias.get(x, x) for x in d["callees"] + d.get("func_refs", [])}
            c = {x for x in c if x in self.F and x != a}
            self.calls[a] = c
            for x in c:
                self.callers[x].add(a)
        self.sset, self.sall, self.sref = {}, {}, defaultdict(set)
        for a, d in self.F.items():
            ks = [norm_str(s) for s in d["strings"]]
            self.sall[a] = tuple(sorted(k for k, _ in ks))
            self.sset[a] = frozenset(k for k, f in ks if not f)
            for k, _ in ks:
                self.sref[k].add(a)
        self.byname = defaultdict(list)
        for a, d in self.F.items():
            self.byname[base_name(d["name"])].append(a)
        self._reach = {}
        self.imp = None

    def named(self, a):
        """The symbol name, or None for IDA's own labels (on Mach-O every real symbol starts with
        `_`; `InitFunc_96`, `sub_…` and the like are IDA's)."""
        n = self.F[a]["name"]
        if n.startswith(UNNAMED) or n.startswith(" ") or (not self.msvc and not n.startswith("_")):
            return None
        return n

    def dem(self, a):
        d = self.F[a]
        return d.get("demangled") or out_name(d["name"])

    def reach_strings(self, a):
        """Strings of a's callees and their callees (where inlining on the other side moves them)."""
        if a not in self._reach:
            s = set()
            for c in self.calls[a]:
                s |= self.sset[c]
                for c2 in self.calls[c]:
                    s |= self.sset[c2]
            self._reach[a] = s
        return self._reach[a]

    def reach2(self, a):
        r = set(self.calls[a])
        for c in self.calls[a]:
            r |= self.calls[c]
        return r

    def load_imports(self):
        allimp = {i for d in self.F.values() for i in d["imports"]}
        dm = demangle(allimp, self.msvc)
        self.imp = {a: frozenset(qualified(dm.get(i, i)) for i in d["imports"]) for a, d in self.F.items()}
        # (qualified name, parameter count) of every imported function: no body in this image
        self.impkeys = {k for k in (sig_key(dm.get(i, i)) for i in allimp) if k}

    def own_vt(self, a):
        """a stores or reads a vftable itself (MSVC ??_7, Itanium _ZTV)."""
        pre = ("??_7",) if self.msvc else ("__ZTV", "_ZTV")
        return any((n or "").startswith(pre) for _, n in self.F[a]["globals"])

    def vt1(self, a):
        return self.own_vt(a) or any(self.own_vt(c) for c in self.calls[a])


# ---------------------------------------------------------------------------------------------
# pairing state for one oracle

class Pairing:
    def __init__(self, T, O, tag):
        self.T, self.O, self.tag = T, O, tag
        self.t2o = {}      # P -> set(O addrs)
        self.o2t = {}      # O -> P
        self.info = {}     # P -> (class, evidence, strong)
        self.rejected = Counter()
        self.mismatch = []  # (class, reason, msvc counts, itanium counts)
        self.conflicts = []  # (P, reason)
        self.ev_o = {}     # (P, O) -> evidence for that oracle function alone (folded addresses)
        self.votes = {}    # (P, O) -> number of vftable slots that voted for it

    def add(self, p, os_, cls, ev, strong=True):
        self.t2o[p] = set(os_)
        for o in os_:
            self.o2t[o] = p
        self.info[p] = (cls, ev, strong)

    def drop(self, p):
        for o in self.t2o.pop(p, ()):
            self.o2t.pop(o, None)
        self.info.pop(p, None)

    def strong_t(self, p):
        return p in self.info and self.info[p][2]

    # -- tests shared by every class ---------------------------------------------------------
    def strings_ok(self, p, o):
        T, O = self.T, self.O
        sp, so = T.sset[p], O.sset[o]
        ep = {s for s in sp - so if s in O.sref and len(O.sref[s]) <= 50}
        if ep and ep - O.reach_strings(o):
            return False
        eo = {s for s in so - sp if s in T.sref and len(T.sref[s]) <= 50}
        if eo and eo - T.reach_strings(p):
            return False
        return True

    def graph_violations(self, p, o, upto=None):
        """(violations, checked): paired callees of either side that the partner does not reach.
        `upto` limits the witnesses to pairs of that class or a stronger one."""
        T, O = self.T, self.O
        r_o, r_p = O.reach2(o), T.reach2(p)
        ok = (lambda q: self.strong_t(q) and len(self.t2o[q]) == 1 and
              (upto is None or STRONG.index(self.info[q][0]) <= STRONG.index(upto)))
        bad = chk = 0
        for c in T.calls[p]:
            if c in self.t2o and ok(c):
                chk += 1
                bad += not (self.t2o[c] & r_o)
        for c in O.calls[o]:
            q = self.o2t.get(c)
            if q is not None and ok(q):
                chk += 1
                bad += q not in r_p
        return bad, chk

    def graph_ok(self, p, o):
        bad, chk = self.graph_violations(p, o)
        return bad == 0 or (bad == 1 and chk >= 4)

    def extra_ok(self, p, o, cls):
        """Tests the adversarial review asked for, for every class but rtti-slot (a structural
        pairing): -> None, or the reason the candidate is impossible."""
        T, O = self.T, self.O
        n = O.named(o)
        if n:
            dem = O.dem(o)
            why = name_problem(dem)
            if why:
                return why
            mg = out_name(n)
            if not re.search(r"D0Ev$", mg) and sig_key(dem) in T.impkeys:
                return "imported by the target (no body in the image)"
            if T.abi_verdict and T.abi_verdict({"mangled": mg, "demangled": dem, "evidence_class": cls},
                                               T.abi_facts.get(p))[1] == "wrong":
                return "x86 ABI (ret N / ecx) contradicts the signature"
        if n and re.search(r"(C[12]|D[12])E", base_name(n)) and O.own_vt(o) and not self.structor_like(p):
            return "oracle structor sets a vftable; target body neither does nor calls a structor"
        if self.level_shift(p, o):
            return "inlining level shift"
        return None

    def structor_like(self, p):
        """p sets a vftable (itself or in a direct callee), or calls a constructor/destructor:
        an imported one (??0/??1) or one paired with an Itanium C*/D* name. MSVC may drop a dead
        vfptr store in a destructor, so a base or member structor call counts too."""
        T = self.T
        if T.vt1(p) or any(i.startswith(("??0", "??1")) for i in T.F[p]["imports"]):
            return True
        for c in T.calls[p]:
            for o in self.t2o.get(c, ()):
                n = self.O.named(o)
                if n and re.search(r"(C[123]|D[012])E", base_name(n)):
                    return True
        return False

    def level_shift(self, p, o):
        """When MSVC and clang keep different levels of a call chain out of line, the one
        unpaired neighbour left is the wrapper on one side and its callee on the other. Detected
        when none of one side's paired callees is a direct callee of the partner, but all of them
        are direct callees of exactly one unpaired callee of the partner, and the pair has a
        paired callee the partner does not reach (the wrapper does something the callee does
        not: Name::declare<X>'s call_once and getNullName around doDeclare<X>). Without that last
        condition it is MSVC inlining a small helper, and the pair is right. -> (P, O) the
        pairing should have been (one side None when ambiguous), or None."""
        T, O = self.T, self.O
        if not self.graph_violations(p, o)[0]:
            return None
        single = lambda q: q in self.t2o and len(self.t2o[q]) == 1 and self.info[q][2]
        pc = {next(iter(self.t2o[c])) for c in T.calls[p] if single(c)}
        if pc and not pc & O.calls[o]:
            c2 = [x for x in O.calls[o] if x not in self.o2t and pc <= O.calls[x]]
            if c2:  # several (clang's doDeclare<X> and its copy in callDoDeclare<X>): a shift all the same
                return p, (c2[0] if len(c2) == 1 else None)
        oc = {self.o2t[c] for c in O.calls[o] if c in self.o2t and single(self.o2t[c])}
        if oc and not oc & T.calls[p]:
            c2 = [x for x in T.calls[p] if x not in self.t2o and not T.named(x) and oc <= T.calls[x]]
            if c2:
                return (c2[0] if len(c2) == 1 else None), o
        return None

    def tests(self, p, o, cls):
        if not (self.strings_ok(p, o) and self.shape_ok(p, o) and self.graph_ok(p, o)):
            return "strings/shape/callees"
        return self.extra_ok(p, o, cls)

    def shape_ok(self, p, o):
        tp, to = self.T.F[p], self.O.F[o]
        a, b = tp["size"], to["size"]
        if max(a, b) > 96 and not (0.25 <= a / max(b, 1) <= 4):
            return False
        return True

    # -- accept a batch of one-to-one candidates ---------------------------------------------
    def accept(self, cands, cls, strong=True):
        """cands: {P: (O, evidence)} built so each P and each O occurs once. Applies the tests."""
        n = 0
        for p, (o, ev) in cands.items():
            if p in self.t2o or o in self.o2t:
                continue
            why = self.tests(p, o, cls)
            if why:
                self.rejected[cls + ": " + why] += 1
                continue
            self.add(p, [o], cls, ev, strong)
            n += 1
        return n


def one_to_one(pairs):
    """[(P, O, ev)] -> {P: (O, ev)} keeping only P and O that occur in exactly one distinct pair."""
    tp, to = defaultdict(set), defaultdict(set)
    for p, o, _ in pairs:
        tp[p].add(o)
        to[o].add(p)
    out = {}
    for p, o, ev in pairs:
        if len(tp[p]) == 1 and len(to[o]) == 1 and p not in out:
            out[p] = (o, ev)
    return out


# ---------------------------------------------------------------------------------------------
# evidence classes

def load_vtables(target, kinds):
    out, dup = {}, set()
    for l in open(os.path.join(EXP, target + "-evidence", "vtables.jsonl")):
        d = json.loads(l)
        if d["kind"] not in kinds or not d.get("class"):
            continue
        k = norm_class(d["class"])
        if k in out:
            dup.add(k)
        out[k] = d
    for k in dup:
        del out[k]
    return out


def itanium_entries(slots):
    out, i = [], 0
    while i < len(slots):
        n = base_name(slots[i]["name"])
        if DTOR.search(n) and i + 1 < len(slots) and DTOR.search(base_name(slots[i + 1]["name"])):
            out.append({"dtor": True, "key": n, "d0": slots[i + 1]})
            i += 2
        else:
            out.append({"dtor": False, "key": n, "slot": slots[i]})
            i += 1
    return out


def guided(groups, ggroups, O, G):
    """Mac slots with IDA labels (`sub_…`, internal-linkage classes) take the symbol of the same
    slot in the guide oracle's vtable of the same class, for structure only (destructor pair,
    thunk, pure, overloads); the address stays the Mac one. None unless every group has the same
    slot count on both sides."""
    if len(groups) != len(ggroups) or any(len(a["slots"]) != len(b["slots"]) for a, b in zip(groups, ggroups)):
        return None
    out = []
    for g, h in zip(groups, ggroups):
        sl = []
        for s, t in zip(g["slots"], h["slots"]):
            n = s["name"] or ""
            if n.startswith("_"):
                sl.append(dict(s))
            else:
                hit = G.byname.get(base_name(t["name"]), [])
                sl.append({"ea": s["ea"], "name": t["name"], "guided": True,
                           "dem": G.dem(hit[0]) if len(hit) == 1 else None})
        out.append(dict(g, slots=sl))
    return out


def rtti_slot(pr, tv, ov, guide=None):
    T, O = pr.T, pr.O
    votes = defaultdict(list)  # (P, O) -> [evidence]
    nguided = 0
    for cname in sorted(set(tv) & set(ov)):
        mg = sorted(tv[cname]["groups"], key=lambda g: g["col_offset"])
        ig = sorted(ov[cname]["groups"], key=lambda g: -g["offset_to_top"])
        via = ""
        if guide and cname in guide[1] and any(not (s["name"] or "").startswith("_") for g in ig for s in g["slots"]):
            gg = guided(ig, sorted(guide[1][cname]["groups"], key=lambda g: -g["offset_to_top"]), O, guide[0])
            if gg:
                ig, via = gg, " (layout from %s)" % guide[2]
                nguided += 1
        mc, ic = [len(g["slots"]) for g in mg], [len(g["slots"]) for g in ig]
        thunked = {base_name(s["name"]) for g in ig[1:] for s in g["slots"]
                   if (s["name"] or "").lstrip("_").startswith("ZT")}
        vbase = False
        if len(mg) != len(ig):
            # MSVC gives a virtual base its own vftable (cd_offset != 0); Itanium shares it as the
            # primary base of the subobject that holds it, entering its slots as `_ZTv` virtual
            # thunks. Retry without both.
            nv = [g for g in mg if not g["cd_offset"]]
            if len(nv) != len(ig):
                pr.mismatch.append((cname, "vftable count", mc, ic))
                continue
            mg, vbase = nv, True
            ig = [dict(g, slots=[x for x in g["slots"] if not (x["name"] or "").lstrip("_").startswith("ZTv")])
                  if k else g for k, g in enumerate(ig)]
        aligned, why = [], None
        for k, (g, h) in enumerate(zip(mg, ig)):
            E = itanium_entries(h["slots"])
            if k == 0:
                cand = [i for i, e in enumerate(E) if not e["dtor"] and e["key"] in thunked]
                dc = [i for i, e in enumerate(E) if e["dtor"] and e["key"] in thunked]
                need = len(E) - len(g["slots"])
                if need == len(cand):
                    drop = set(cand)
                elif need == len(cand) + len(dc):
                    drop = set(cand) | set(dc)
                else:
                    why = "slot count, vftable 0"
                    break
                E = [e for i, e in enumerate(E) if i not in drop]
            if len(E) != len(g["slots"]):
                why = "slot count, vftable %d" % k
                break
            aligned.append((k, g["slots"], E))
        if why is None:
            for k, ms, E in aligned:
                if any(bool(s.get("purecall")) != (e["key"] == "cxa_pure_virtual") for s, e in zip(ms, E)):
                    why = "pure-virtual slots differ, vftable %d" % k
                    break
        if why:
            pr.mismatch.append((cname, why, mc, ic))
            continue
        for k, ms, E in aligned:
            real = []
            for e in E:
                s = e["d0"] if e["dtor"] else e["slot"]
                n = s["name"] or ""
                if n.lstrip("_").startswith("ZT"):
                    hit = O.byname.get(base_name(n), [])
                    real.append(hit[0] if len(hit) == 1 else None)
                else:
                    real.append(s["ea"] if s["ea"] in O.F else None)
            uq = [None if (e["dtor"] or r is None) else
                  unqualified(e["slot"].get("dem") or (O.dem(r) if O.named(r) else None) or "") or None
                  for e, r in zip(E, real)]
            if any(r is not None and u is None for e, r, u in zip(E, real, uq) if not e["dtor"]):
                continue  # a slot whose name is unknown could hide an overload set
            unsafe = set()
            for name, c in Counter(u for u in uq if u).items():
                if c > 1:
                    idx = [i for i, u in enumerate(uq) if u == name]
                    unsafe |= set(range(min(idx), max(idx) + 1))
            for i, (s, e, r) in enumerate(zip(ms, E, real)):
                if s.get("purecall") or r is None or i in unsafe:
                    continue
                p = s["thunk"]["target"] if "thunk" in s else s["ea"]
                if p in T.F and not T.named(p):
                    votes[(p, r)].append("%s vftable %d slot %d%s%s%s" % (cname, k, i, " (dtor)" if e["dtor"] else "", via,
                                                                          " (virtual-base vftables set aside)" if vbase else ""))
    byp, byo = defaultdict(set), defaultdict(set)
    for p, o in votes:
        byp[p].add(o)
        byo[o].add(p)
    bad_o = {o for o, ps in byo.items() if len(ps) > 1}
    n = 0
    for p, os_ in byp.items():
        if os_ & bad_o:
            pr.conflicts.append((p, "rtti-slot: oracle function also paired elsewhere"))
            continue
        evs = sorted({e for o in os_ for e in votes[(p, o)]})
        ev = evs[0] + ("" if len(evs) == 1 else " (+%d more slots)" % (len(evs) - 1))
        for o in os_:
            e1 = sorted(set(votes[(p, o)]))
            pr.ev_o[(p, o)] = e1[0] + ("" if len(e1) == 1 else " (+%d more slots)" % (len(e1) - 1))
            pr.votes[(p, o)] = len(votes[(p, o)])
        if len(os_) > 1:
            recs = [O.F[o] for o in os_]
            sizes = [r["size"] for r in recs]
            if len({O.sset[o] for o in os_}) == 1 and len({len(O.calls[o]) for o in os_}) == 1 and \
                    (max(sizes) <= 32 or max(sizes) <= 2 * min(sizes)):
                ev = "folded (%d oracle functions): " % len(os_) + ev
            else:
                pr.conflicts.append((p, "rtti-slot: several oracle functions that do not look folded"))
                continue
        pr.add(p, os_, "rtti-slot", ev)
        n += 1
    if guide:
        print("  rtti classes whose layout came from %s: %d" % (guide[2], nguided))
    return n


def string_unique(pr):
    T, O = pr.T, pr.O
    pairs = []
    for s, ps in T.sref.items():
        if len(ps) != 1 or s.startswith("FILE:") or (len(s) < 5 and not ASSERT.match(s)):
            continue
        os_ = O.sref.get(s)
        if os_ and len(os_) == 1:
            pairs.append((next(iter(ps)), next(iter(os_)), s))
    free = [(p, o, s) for p, o, s in pairs if p not in pr.t2o and o not in pr.o2t]
    c = one_to_one(free)
    evs = defaultdict(list)
    for p, o, s in free:
        evs[(p, o)].append(s)
    cands = {p: (o, "unique string %r%s" % (evs[(p, o)][0][:80], "" if len(evs[(p, o)]) == 1 else " (+%d more)" % (len(evs[(p, o)]) - 1)))
             for p, (o, _) in c.items()}
    return pr.accept(cands, "string-unique")


def string_set(pr):
    T, O = pr.T, pr.O
    def index(S, done):
        idx = defaultdict(list)
        for a in S.F:
            k = S.sall[a]
            if k and a not in done and any(not x.startswith("FILE:") for x in k):
                idx[k].append(a)
        return {k: v[0] for k, v in idx.items() if len(v) == 1}, {k for k, v in idx.items() if len(v) > 1}
    # uniqueness over the whole side, not only over unpaired functions
    ti, tdup = index(T, ())
    oi, odup = index(O, ())
    cands = {}
    for k, p in ti.items():
        o = oi.get(k)
        if o is not None and p not in pr.t2o and o not in pr.o2t and not T.named(p):
            cands[p] = (o, "same %d strings, e.g. %r" % (len(k), k[0][:60]))
    return pr.accept(cands, "string-set")


def import_set(pr):
    T, O = pr.T, pr.O
    shared = ({i for s in T.imp.values() for i in s} & {i for s in O.imp.values() for i in s}) - IMPORT_NOISE
    def index(S):
        idx = defaultdict(list)
        for a, s in S.imp.items():
            k = frozenset(s & shared)
            if len(k) >= 2:
                idx[k].append(a)
        return {k: v[0] for k, v in idx.items() if len(v) == 1}
    ti, oi = index(T), index(O)
    cands = {}
    for k, p in ti.items():
        o = oi.get(k)
        if o is None or p in pr.t2o or o in pr.o2t or T.named(p):
            continue
        tp, to = T.F[p], O.F[o]
        if T.sset[p] != O.sset[o]:
            continue
        if not (abs(tp["blocks"] - to["blocks"]) <= 3 or 0.5 <= tp["blocks"] / max(to["blocks"], 1) <= 2):
            continue
        if abs(len(T.calls[p]) - len(O.calls[o])) > max(2, len(O.calls[o]) // 2):
            continue
        # a key of two generic runtime imports (std::, C runtime) is unique only by accident:
        # it needs a third import, a non-runtime one, or paired callees that agree
        if len(k) < 3 and all(i.startswith("std::") or "::" not in i for i in k):
            bad, chk = pr.graph_violations(p, o)
            if not chk or bad:
                pr.rejected["import-set: generic 2-import key"] += 1
                continue
        cands[p] = (o, "same %d imports, e.g. %s" % (len(k), sorted(k)[0]))
    return pr.accept(cands, "import-set")


def load_body_match(target, b):
    """-> {target addr: (B addr, [B names-proposed rows])} for accepted body pairs B has names for."""
    rows = defaultdict(list)
    for r in csv.DictReader(open(os.path.join(EXP, b + "-names-proposed.csv"))):
        rows[int(r["address"], 16)].append(r)
    out = {}
    for r in csv.DictReader(open(os.path.join(EXP, "%s-%s-bodymatch.csv" % (target, b)))):
        if r["status"].startswith("pair") and int(r["b_address"], 16) in rows:
            q = int(r["b_address"], 16)
            out[int(r["a_address"], 16)] = (q, rows[q])
    return out


def body_seed(pr, body, b):
    """Seed an oracle with the function of the name B gave the body twin (strong B rows only;
    the oracle name must be unique after merging structor variants)."""
    O = pr.O
    bycanon = defaultdict(set)
    for o in O.F:
        n = O.named(o)
        if n:
            bycanon[canon(base_name(n))].add(o)
    cands = []
    for p, (q, rs) in body.items():
        strong = [r for r in rs if r["source_tag"] != "guess:"]
        if len(strong) != 1:  # a folded B address: which name the body is cannot be told
            continue
        os_ = bycanon.get(canon(base_name(strong[0]["mangled"])), ())
        if len(os_) == 1:
            cands.append((p, next(iter(os_)), "%s body-identical to %s 0x%08x (%s %s)"
                          % ("player:" if b == "PLAYER" else b.lower() + ":", b, q, strong[0]["source_tag"], strong[0]["evidence_class"])))
    return pr.accept(one_to_one(cands), "body-match")


def callgraph_round(pr):
    """One propagation round. Returns the number of new strong pairs."""
    T, O = pr.T, pr.O
    sup = defaultdict(set)  # (P, O) -> {(seedP, dir)}
    for p, os_ in list(pr.t2o.items()):
        if len(os_) != 1 or not pr.info[p][2]:
            continue
        o = next(iter(os_))
        for d, tn, on in (("callee", T.calls, O.calls), ("caller", T.callers, O.callers)):
            up = [x for x in tn[p] if not (x in pr.t2o and pr.info[x][2]) and not T.named(x)]
            uo = [x for x in on[o] if not (x in pr.o2t and pr.info[pr.o2t[x]][2])]
            if len(up) == 1 and len(uo) == 1:
                sup[(up[0], uo[0])].add((p, d))
            elif up and uo:
                # several unpaired neighbours: pair those whose non-empty string sets are equal
                # and occur once among the neighbours on each side
                ku = Counter(T.sset[x] for x in up)
                ko = Counter(O.sset[x] for x in uo)
                for x in up:
                    k = T.sset[x]
                    if k and ku[k] == 1 and ko.get(k) == 1:
                        sup[(x, next(y for y in uo if O.sset[y] == k))].add((p, d + "/strings"))
    byp, byo = defaultdict(set), defaultdict(set)
    for p, o in sup:
        byp[p].add(o)
        byo[o].add(p)
    new_s = 0
    for (p, o), s in sup.items():
        if len(byp[p]) > 1 or len(byo[o]) > 1:
            continue
        strong = len({sp for sp, _ in s}) >= 2
        if p in pr.t2o or o in pr.o2t:  # an earlier guess: only a strong pairing replaces it
            old = [q for q in {p, pr.o2t.get(o)} if q is not None and q in pr.t2o]
            if not strong or any(pr.info[q][2] for q in old):
                continue
            for q in old:
                pr.drop(q)
        sh = pr.level_shift(p, o)
        if sh:
            # pair the level the two sides share instead, if it is free
            p2, o2 = sh
            if p2 is None or o2 is None or p2 in pr.t2o or o2 in pr.o2t or p2 in byp or o2 in byo:
                pr.rejected["callgraph: inlining level shift"] += 1
                continue
            p, o = p2, o2
            s = {(sp, d + "/level-shift") for sp, d in s}
        why = pr.tests(p, o, "callgraph")
        if why:
            pr.rejected["callgraph: " + why] += 1
            continue
        seeds = sorted(s)
        ev = "%s of paired 0x%08x%s" % (seeds[0][1], seeds[0][0], "" if len(seeds) == 1 else " (+%d more seeds)" % (len(seeds) - 1))
        pr.add(p, [o], "callgraph", ev, strong)
        new_s += strong
    return new_s


def promote(pairings):
    """A single-seed callgraph pair becomes strong when the other oracle names the same function."""
    n = 0
    prs = list(pairings.values())
    for pa in prs:
        for p, (cls, ev, strong) in list(pa.info.items()):
            if strong or len(pa.t2o[p]) != 1:
                continue
            o = next(iter(pa.t2o[p]))
            nm = pa.O.named(o)
            if not nm:
                continue
            for pb in prs:
                if pb is pa or p not in pb.t2o or len(pb.t2o[p]) != 1:
                    continue
                ob = next(iter(pb.t2o[p]))
                if pb.O.named(ob) and canon(out_name(pb.O.named(ob))) == canon(out_name(nm)):
                    pa.info[p] = (cls, ev + "; %s agrees" % pb.tag, True)
                    if not pb.info[p][2]:
                        pb.info[p] = (pb.info[p][0], pb.info[p][1] + "; %s agrees" % pa.tag, True)
                    n += 1
                    break
    return n


def callgraph(pairings, max_rounds=60):
    """Propagate every oracle in lock-step so cross-oracle agreement can seed the next round."""
    for _ in range(max_rounds):
        if not sum(callgraph_round(pr) for pr in pairings.values()) + promote(pairings):
            break
    for pr in pairings.values():
        c = Counter(pr.info[p][2] for p in pr.info if pr.info[p][0] == "callgraph")
        print("  %-8s callgraph %6d strong, %d single-seed" % (pr.tag, c[True], c[False]))


def validate(pr):
    """Second look at every single-target pair once all classes have run."""
    n = 0
    for p in list(pr.t2o):
        os_ = pr.t2o[p]
        if len(os_) != 1:
            continue
        o = next(iter(os_))
        bad, chk = pr.graph_violations(p, o, pr.info[p][0])
        if pr.info[p][0] != "rtti-slot" and pr.level_shift(p, o):
            pr.rejected["validate:%s: inlining level shift" % pr.info[p][0]] += 1
            pr.conflicts.append((p, "%s pair dropped: inlining level shift" % pr.info[p][0]))
            pr.drop(p)
            n += 1
        elif bad >= 2 and bad * 3 > chk:
            pr.rejected["validate:" + pr.info[p][0]] += 1
            pr.conflicts.append((p, "%s pair dropped: %d of %d paired callees disagree" % (pr.info[p][0], bad, chk)))
            pr.drop(p)
            n += 1
    return n


# ---------------------------------------------------------------------------------------------

def pe_text(path):
    d = open(path, "rb").read(4096)
    pe = struct.unpack_from("<I", d, 0x3c)[0]
    n, opt = struct.unpack_from("<H", d, pe + 6)[0], struct.unpack_from("<H", d, pe + 20)[0]
    base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
    for i in range(n):
        o = pe + 24 + opt + 40 * i
        if d[o:o + 8].rstrip(b"\0") == b".text":
            va, sz = struct.unpack_from("<II", d, o + 8)[::-1]
            return base + va, base + va + sz
    return None


def selftest():
    assert base_name("__ZThn32_N3RBX12AccoutrementD0Ev") == "ZN3RBX12AccoutrementD0Ev"
    assert base_name("__ZTv0_n12_N3RBX12Accoutrement11getLocationEv") == "ZN3RBX12Accoutrement11getLocationEv"
    assert qualified("public: __thiscall std::basic_string<char, struct std::char_traits<char>, class std::allocator<char>>::basic_string<char, struct std::char_traits<char>, class std::allocator<char>>(char const *)") == "std::basic_string::basic_string"
    assert qualified("std::basic_string<char, std::char_traits<char>, std::allocator<char>>::basic_string(char const*, std::allocator<char> const&)") == "std::basic_string::basic_string"
    assert qualified("void * __cdecl operator new(unsigned int)") == "operator new"
    assert qualified("operator new(unsigned long)") == "operator new"
    assert qualified("void __cdecl FLog::FastLog(unsigned char, char const *, void const *)") == "FLog::FastLog"
    assert unqualified("RBX::Instance::setName(std::string const&)") == "setName"
    assert norm_class("RBX::DescribedCreatable<class RBX::Network::Player,class RBX::Instance,char const * const RBX::Network::sPlayer,19,0>") == \
        norm_class("RBX::DescribedCreatable<RBX::Network::Player, RBX::Instance, RBX::Network::sPlayer, (RBX::Reflection::ClassDescriptor::Functionality)19, (RBX::Security::Permissions)0>")
    assert norm_str("line > 0 file: C:\\BuildAgent\\work\\x\\Client\\App\\script\\DebuggerManager.cpp line: 1434") == \
        norm_str("line > 0 file: ../App/script/DebuggerManager.cpp line: 1434")
    assert norm_str("/Applications/buildAgent/work/x/Client/App/Foo.cpp") == ("FILE:foo.cpp", True)
    assert sig_key("public: void __thiscall Ogre::SharedPtr<class Ogre::DataStream>::destroy(void)") == \
        sig_key("Ogre::SharedPtr<Ogre::DataStream>::destroy()")
    assert sig_key("Ogre::SharedPtr<Ogre::DataStream>::destroy()") != sig_key("Ogre::SharedPtr<Ogre::Mesh>::destroy()")
    assert sig_key("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl "
                   "std::operator+<char,struct std::char_traits<char>,class std::allocator<char> >(class std::basic_string"
                   "<char,struct std::char_traits<char>,class std::allocator<char> > const &,class std::basic_string<char,"
                   "struct std::char_traits<char>,class std::allocator<char> > const &)") == \
        sig_key("std::basic_string<char, std::char_traits<char>, std::allocator<char>> std::operator+<char, "
                "std::char_traits<char>, std::allocator<char>>(std::basic_string<char, std::char_traits<char>, "
                "std::allocator<char>> const&, std::basic_string<char, std::char_traits<char>, std::allocator<char>> const&)")
    assert name_problem("joinGameTeleport(std::string, NSObject*)") and name_problem("RBX::MacSystemUtil::getGPUMake()")
    assert name_problem("std::_Rb_tree<int, int>::_M_erase(std::_Rb_tree_node<int>*)") and not name_problem("RBX::UIEvent::x()")
    assert name_problem("__48-[TFTimer initWithInterval:dispatchQueue:block:]_block_invoke")
    assert name_problem("___ZN10RobloxView18doRestartDataModelEv_block_invoke")
    assert not name_problem("RBX::Machine::run()") and not name_problem("std::basic_iostream<char>::x()")
    assert one_to_one([(1, 10, "a"), (2, 10, "b"), (3, 30, "c"), (3, 30, "d")]) == {3: (30, "c")}
    print("selftest ok")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target")
    ap.add_argument("--oracle", action="append", default=[], help="tag=TARGET, first is primary")
    ap.add_argument("--pe", help="target PE, for the .text statistics")
    ap.add_argument("--app-decomp", help="prior App decomp tree; writes <TARGET>-app-decomp-map.csv")
    ap.add_argument("--app-oracle", default="mac", help="oracle tag whose addresses the tree comments use")
    ap.add_argument("--body-match", help="build B: use ida/exports/<TARGET>-<B>-bodymatch.csv and <B>-names-proposed.csv")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    T = Side(a.target, True)
    T.load_imports()
    T.abi_facts, T.abi_verdict = {}, None
    abi = os.path.join(EXP, a.target + "-evidence", "abi.jsonl")
    if os.path.exists(abi):  # written by tools/ida/abi_facts.py (see the docstring)
        from check_names_abi import classifier
        for l in open(abi):
            d = json.loads(l)
            T.abi_facts[d["ea"]] = d
        T.abi_verdict = classifier(a.target, tuple(x.split("=")[1] for x in a.oracle))
    else:
        print("  no %s: the x86 ABI gate is off" % abi)
    tv = load_vtables(a.target, ("vtable",))
    a._body = load_body_match(a.target, a.body_match) if a.body_match else {}
    pairings, guide = {}, None
    for spec in a.oracle:
        tag, tgt = spec.split("=")
        O = Side(tgt, False)
        O.load_imports()
        pr = Pairing(T, O, tag)
        print("== %s (%s)" % (tag, tgt))
        ov = load_vtables(tgt, ("vtable", "nosymbol"))
        print("  rtti-slot     %6d" % rtti_slot(pr, tv, ov, guide))
        if guide is None:
            guide = (O, ov, tag)
        print("  string-unique %6d" % string_unique(pr))
        print("  string-set    %6d" % string_set(pr))
        print("  import-set    %6d" % import_set(pr))
        if a._body:
            print("  body-match    %6d" % body_seed(pr, a._body, a.body_match))
        print("  validate drop %6d" % validate(pr))
        pairings[tag] = pr
    print("== callgraph (all oracles together)")
    callgraph(pairings)
    for _ in range(5):  # a drop can free the level the two sides share: propagate again
        dropped = sum(validate(pr) for pr in pairings.values())
        print("  validate drop %d" % dropped)
        if not dropped:
            break
        callgraph(pairings)
    for pr in pairings.values():
        print("  %-8s rejected by tests: %s" % (pr.tag, dict(sorted(pr.rejected.items()))))
    write_names(a, T, pairings)
    if a.app_decomp:
        write_app_map(a, T, pairings)


def write_names(a, T, pairings):
    tags = list(pairings)
    prim = tags[0]
    rows, conflicts = [], []
    for tag, pr in pairings.items():
        for p, why in pr.conflicts:
            conflicts.append({"address": "0x%08x" % p, "oracle": tag, "names": "", "reason": why})
    # per P: {tag: {canon name: (name, cls, ev, strong, dem)}}
    per = defaultdict(dict)
    for tag, pr in pairings.items():
        for p, os_ in pr.t2o.items():
            cls, ev, strong = pr.info[p]
            d = {}
            for o in os_:
                n = pr.O.named(o)
                if n:
                    why = name_problem(pr.O.dem(o))
                    if why:
                        conflicts.append({"address": "0x%08x" % p, "oracle": tag, "names": out_name(n), "reason": why})
                        continue
                    d[canon(out_name(n))] = (out_name(n), cls, pr.ev_o.get((p, o), ev), strong, pr.O.dem(o),
                                             pr.votes.get((p, o), 1))
            if d:
                per[p][tag] = d
    btag = {}
    for p, (q, rs) in getattr(a, "_body", {}).items():
        d = {}
        for r in rs:
            dem = r["demangled"].replace(" [D0, deleting: the MSVC scalar deleting destructor]", "")
            ev = "%s body-identical to %s 0x%08x (%s %s)" % ("player:" if a.body_match == "PLAYER" else a.body_match.lower() + ":",
                                                          a.body_match, q, r["source_tag"], r["evidence_class"])
            d[canon(r["mangled"])] = (r["mangled"], "body-match", ev, r["source_tag"] != "guess:", dem, 1)
            btag[(p, canon(r["mangled"]))] = r["source_tag"]
        per[p]["player"] = d
    # a name proposed for two different P by different oracles is a conflict too
    who = defaultdict(set)
    for p, by in per.items():
        for tag, d in by.items():
            for n in d:
                who[n].add(p)
    agreement = Counter()
    for p in sorted(per):
        by = per[p]
        sets = [set(d) for d in by.values()]
        if len(sets) > 1:
            ok = bool(set.intersection(*sets))
            agreement[("agree" if ok else "disagree",) + tuple(sorted({v[1] for d in by.values() for v in d.values()}))] += 1
            if not ok:
                conflicts.append({"address": "0x%08x" % p, "oracle": "/".join(by),
                                  "names": " | ".join("%s=%s" % (t, ";".join(sorted(v[0] for v in d.values()))) for t, d in by.items()),
                                  "reason": "oracles disagree"})
                continue
        dup = sorted(n for d in by.values() for n in d if len(who[n]) > 1)
        if dup:
            conflicts.append({"address": "0x%08x" % p, "oracle": "/".join(by), "names": ";".join(sorted({by[t][n][0] for t in by for n in dup if n in by[t]})),
                              "reason": "name proposed for several target functions: " + ",".join("0x%08x" % q for q in sorted(who[dup[0]]))})
            continue
        # most-voted name first: at a folded address that is the one apply_names.py sets
        names = sorted(set().union(*sets), key=lambda n: (-sum(d[n][5] for d in by.values() if n in d), n))
        folded = len(names) > 1
        for n in names:
            have = [t for t in by if n in by[t]]
            info = {t: by[t][n] for t in have}
            oracle = "+".join(have) if "player" in have else "both" if len(have) > 1 else have[0]
            t0 = prim if prim in info else next(t for t in tags + ["player"] if t in info)
            mangled, cls, ev, strong, dem, _ = info[t0]
            if re.search(r"D0E", mangled):
                dem += " [D0, deleting: the MSVC scalar deleting destructor]"
            agree = len(have) > 1
            ok = strong or agree
            ev = ("folded (%d names); " % len(names) if folded else "") + " | ".join("%s: %s" % (t, info[t][2]) for t in have)
            if not strong and agree:
                ev += "; both oracles agree"
            if ok and GNU_ARG.search(dem):
                ok = False  # right body, but the template arguments are libstdc++'s, not Dinkumware's
                ev += "; name carries libstdc++ types (not the MSVC spelling)"
            tagname = btag[(p, n)] if t0 == "player" else (t0[:3] if t0.startswith("ios") else t0) + ":"
            rows.append({"address": "0x%08x" % p, "mangled": mangled, "demangled": dem, "oracle": oracle,
                         "evidence_class": cls if ok else cls + "-guess",
                         "evidence": ev,
                         "source_tag": tagname if ok else "guess:"})
    path = os.path.join(EXP, a.target + "-names-proposed.csv")
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, ["address", "mangled", "demangled", "oracle", "evidence_class", "evidence", "source_tag"])
        w.writeheader()
        w.writerows(rows)
    with open(os.path.join(EXP, a.target + "-names-conflicts.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, ["address", "oracle", "names", "reason"])
        w.writeheader()
        w.writerows(conflicts)
    # ---- report
    named = {r["address"] for r in rows if r["source_tag"] != "guess:"}
    guess = {r["address"] for r in rows if r["source_tag"] == "guess:"} - named
    print("\n== %s: %d rows, %d functions named (+%d guess-only) of %d" % (path, len(rows), len(named), len(guess), len(T.F)))
    c = Counter()
    for r in rows:
        c[(r["evidence_class"], r["oracle"])] += 1
    for k, v in sorted(c.items()):
        print("  %-22s %-8s %6d rows" % (k[0], k[1], v))
    fc = Counter()
    for addr in named | guess:
        cls = min((r["evidence_class"] for r in rows if r["address"] == addr), key=lambda x: (x.endswith("-guess"), STRONG.index(x.replace("-guess", ""))))
        fc[cls] += 1
    print("  functions by best class:", dict(fc))
    print("  both oracles named the function (agree/disagree by classes involved):")
    for k, v in sorted(agreement.items(), key=lambda kv: -kv[1]):
        print("    %-9s %-40s %6d" % (k[0], "+".join(k[1:]), v))
    print("  folded addresses (several names):", sum(1 for x in Counter(r["address"] for r in rows).values() if x > 1))
    print("  conflicts:", len(conflicts), dict(Counter(c["reason"].split(":")[0] for c in conflicts)))
    if a.pe:
        lo, hi = pe_text(a.pe)
        big = {p for p, d in T.F.items() if lo <= p < hi and d["size"] > 32 and not T.named(p)}
        nb = {int(x, 16) for x in named} & big
        print("  .text functions > 32 bytes (unnamed in IDA): %d, named strong: %d (%.1f%%), +guess %d"
              % (len(big), len(nb), 100.0 * len(nb) / len(big), len({int(x, 16) for x in guess} & big)))
    for tag, pr in pairings.items():
        for pre in (("ZN3RBX",), ("ZN3RBX", "ZNK3RBX")):
            rbx = {o for o, d in pr.O.F.items() if (d["name"] or "").startswith("_") and (d["name"] or "").lstrip("_").startswith(pre)}
            homed = {o for o in rbx if o in pr.o2t and "0x%08x" % pr.o2t[o] in named}
            print("  %s functions named %s: %d, with a %s home: %d (%.1f%%)"
                  % (tag, "/".join("_" + x for x in pre), len(rbx), a.target, len(homed), 100.0 * len(homed) / max(1, len(rbx))))
        mm = Counter(m[1].split(",")[0] for m in pr.mismatch)
        print("  %s rtti classes not paired: %s" % (tag, dict(mm)))
        with open(os.path.join(EXP, "%s-rtti-mismatch-%s.csv" % (a.target, tag)), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["class", "reason", "msvc_slots_per_vftable", "itanium_slots_per_group"])
            for m in pr.mismatch:
                w.writerow([m[0], m[1], " ".join(map(str, m[2])), " ".join(map(str, m[3]))])
    a._rows = rows


def write_app_map(a, T, pairings):
    sys.dont_write_bytecode = True
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import app_decomp_addr_census as cen
    pr = pairings[a.app_oracle]
    M = pr.O
    files = defaultdict(set)
    for d, _, fs in os.walk(a.app_decomp):
        for fn in fs:
            if not fn.endswith(cen.EXTS):
                continue
            p = os.path.join(d, fn)
            text = open(p, encoding="utf-8", errors="replace").read()
            for _, c in cen.comments(text):
                for m in cen.ADDR.finditer(c):
                    ad = int(m.group(1), 16)
                    i = bisect.bisect_right(M.alladdrs, ad) - 1
                    if i >= 0:
                        f0 = M.alladdrs[i]
                        if f0 <= ad < f0 + M.allF[f0]["size"]:
                            files[f0].add(os.path.relpath(p, a.app_decomp))
    # final accepted names -> P (only rows that survived the conflict pass)
    final = defaultdict(set)
    for r in a._rows:
        final[canon(r["mangled"])].add((int(r["address"], 16), r["evidence_class"], r["source_tag"]))
    others = [t for t in pairings if t != a.app_oracle]
    out, c = [], Counter()
    for m0 in sorted(files):
        m = M.alias.get(m0, m0)  # a Mac-only wrapper (C1 calling C2, j_ thunk) stands for its target
        n = M.allF[m0]["name"] if m0 != m else M.named(m)
        n = n if n and n.startswith("_") and not n.startswith(UNNAMED) else None
        mn = canon(out_name(n)) if n else ""
        if m0 != m and M.named(m):
            mn = canon(out_name(M.named(m)))
        p, cls = None, ""
        if m in pr.o2t:
            q = pr.o2t[m]
            hit = [x for x in final.get(mn, ()) if x[0] == q] if mn else []
            if hit:
                p, cls = q, hit[0][1]
            elif not mn and pr.info.get(q) and pr.info[q][2]:
                p, cls = q, pr.info[q][0] + " (unnamed in Mac)"
        if p is None and mn and final.get(mn):
            hit = sorted(final[mn])
            if len({x[0] for x in hit}) == 1:
                p, cls = hit[0][0], hit[0][1] + " via " + "/".join(others) + " name"
        if p is not None and m0 != m:
            cls += " (Mac wrapper of 0x%08x)" % m
        c[cls.split(" ")[0] if p is not None else "unpaired"] += 1
        out.append({"mac_address": "0x%08x" % m0, "mac_name": n or M.allF[m0]["name"],
                    "player_address": "0x%08x" % p if p is not None else "", "evidence_class": cls,
                    "files": ";".join(sorted(files[m]))})
    path = os.path.join(EXP, a.target + "-app-decomp-map.csv")
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, ["mac_address", "mac_name", "player_address", "evidence_class", "files"])
        w.writeheader()
        w.writerows(out)
    print("\n== %s: %d Mac functions, %d with a %s address %s"
          % (path, len(out), sum(1 for r in out if r["player_address"]), a.target, dict(c)))


if __name__ == "__main__":
    main()

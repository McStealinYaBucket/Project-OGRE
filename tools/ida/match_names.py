"""Plan the reccmp-proven names (and the hand-checked Phase 4 facts) for a Windows database. Offline: no IDA.

    python3 -P tools/ida/match_names.py PLAYER --json build/sweep2/P-final3.json --exe build/RobloxPlayerBeta.exe
    python3 -P tools/ida/match_names.py STUDIO --json build/sweep2/S-final3.json --exe build-studio/RobloxStudioBeta.exe
    python3 -P tools/ida/match_names.py --selftest

--exe is the build the --json run scored (its .pdb and .map beside it; the PDB view is cached as
<exe>.pdbview.tsv by tools/ida/gen_reccmp_csv.pdb_view, which runs tools/lib/pdb_view.py when missing).
Inputs: ida/exports/<T>-functions.csv, -names.csv, -lib-resolved.csv, STUDIO-PLAYER-bodymatch.csv (for the
STUDIO addresses of the hand facts). Outputs (read by tools/ida/apply_db_updates.py):
  ida/exports/<T>-db-updates.csv     address,action,name,tag,line
      action rename: set `name`; comment: write `line` only; unname: give the address back its dummy name.
      `line` is the repeatable-comment line, "p4 <tag> ..." (no other apply_*.py script strips a "p4 " line).
  ida/exports/<T>-match-conflicts.csv  every reccmp 100% function whose existing oracle (ios:/mac:) or FLIRT
      name disagrees with our recompiled name, with the verdict and the evidence that decided it.
STUDIO also takes PLAYER's reccmp-proven renames (run PLAYER first) on bodies STUDIO-PLAYER-bodymatch.csv
pairs uniquely, where STUDIO still has a dummy name and its own --json run did not decide the address.
Rules (step 3 of the Phase 4 sync): an entry of the --json run of type function at effective 100% pairs
original address A with our function R. Our name at R is its PUBLICS (decorated) name, or the PDB proc name
for a static; every map symbol at R counts (our linker folds identical bodies). Addresses the library
matchers own (lib-resolved identical/located) are left to tools/ida/apply_lib_names.py. Then:
  IDA name is a dummy                          -> rename, tag match:
  IDA name is one of our names at R            -> comment
  same signature (gen_reccmp_csv.norm_sig)     -> comment (the oracle spelling is kept)
  prior:/ida: name, other signature            -> rename (the 100% body outranks a prior or auto name)
  ios:/mac:/flirt:, other signature            -> conflict, listed in <T>-match-conflicts.csv and resolved by
                                                  resolve(): the oracle name stays when the original folded
                                                  bodies (rtti-slot "folded" evidence, a body of 8 bytes or
                                                  less, or our build defines the oracle signature elsewhere)
                                                  or the oracle evidence is strong (string, rtti slot); our
                                                  name replaces it (old one kept as a "superseded" line)
                                                  when only callgraph/body-match evidence placed it, and
                                                  always over a FLIRT name except the entry point
"""
import argparse
import csv
import json
import os
import re
import sys
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools", "ida"))
import gen_reccmp_csv as G  # noqa: E402  (norm_sig, undname, read_map, pdb_view)

EXP = os.path.join(ROOT, "ida", "exports")
DUMMY = re.compile(r"^(?:j_)*(?:sub|nullsub|unknown_libname)_[0-9A-Fa-f]+$")
REPORT = "notes/runs/2026-11-12-phase4-pairing"

# Phase 4 facts (phase4-core.md "Database updates to make", g3d-2013.md and pairing.md unresolved items).
# (PLAYER address, STUDIO address or None = find through STUDIO-PLAYER-bodymatch.csv, action, name, tag, evidence)
# Facts at 100% in the --json run are left to the bulk rule unless they need more than a rename.
FACTS = [
    (0x004022d0, None, "rename", "??0?$atomic@H@rbx@@QAE@XZ", "str:",
     "rbx::atomic<int>::atomic(): stores 0 to v, then the rbx/atomic.h assert '(((long)(&(this->v))) & 0x3) == 0' "
     "(0x00bd7a60/0x00bd7aa8); out of line, our Client/Base/include/rbx/atomic.h, reccmp 72% (phase4-core.md)"),
    (0x0070cf60, None, "rename", "_ZN3RBX16DataModelArbiterC2Ev", "prior:",
     "RBX::DataModelArbiter::DataModelArbiter(); STUB in Client/App/v8datamodel/DataModelJob.cpp, reccmp 77%, "
     "size 0xa20 from the layout (phase4-core.md)"),
    (0x0063ee60, None, "rename", "_ZN3RBX9DataModel10GenericJobC2EN5boost10shared_ptrIS0_EEPKcNS_12DataModelJob8TaskTypeE", "prior:",
     "RBX::DataModel::GenericJob::GenericJob; STUB in Client/App/v8datamodel/DataModel.cpp, reccmp 50%, "
     "new 0x248 three times in doDataModelSetup (phase4-core.md)"),
    (0x009b8f00, None, "rename", "_ZN3RBX23RbxInterlockedDecrementEPVl", "ios:",
     "RBX::RbxInterlockedDecrement(long volatile*) | iOS 1029 symbol; body = push arg, call kernel32 "
     "InterlockedDecrement (IAT 0x00aa1124), ret; reccmp 40% (import thunk)"),
    (0x009b8f10, None, "rename", "_ZN3RBX23RbxInterlockedIncrementEPVl", "ios:",
     "RBX::RbxInterlockedIncrement(long volatile*) | iOS 1029 symbol (next to Decrement there too); body = push "
     "arg, call kernel32 InterlockedIncrement (IAT 0x00aa123c), ret; called by the Diagnostics::Countable ctor "
     "in Instance() 0x00620efc"),
    (0x0061bbd0, None, "comment", "", "guess:",
     "the Instance allocator (phase4-core.md): calls 0x0061b100, throws std::bad_alloc on null, subtracts the size "
     "from the counter at 0x00de31d4; no source names it"),
    (0x0066ee10, 0x00773760, "rename", "?raiseEventInvocation@EventSource@Reflection@RBX@@UAEXABVEventDescriptor@23@ABV?$vector@VVariant@Reflection@RBX@@V?$allocator@VVariant@Reflection@RBX@@@std@@@std@@PBVSystemAddress@3@@Z",
     "rtti:", "EventSource vftable slot 2 (raiseEventInvocation), phase4-core.md; the body is `ret 0Ch`, ICF-folded "
     "with RakNet::CCRakNetSlidingWindow::Update(uint64,bool) and other empty 12-byte-argument methods"),
    (0x009dab20, None, "rename", "?maxFinite@Vector3@G3D@@SAABV12@XZ", "match:",
     "G3D::Vector3::maxFinite() | G3D 8.00-roblox identical (tools/lib/g3d_match.py; lib-resolved has it as a "
     "conflict only because of the ios: name): three stores of FLT_MAX from 0x00b06308, 53 calls; not "
     "RBX::Extents::vv (g3d-2013.md, pairing.md)"),
    (0x009da9e0, None, "rename", "?unitY@Vector3@G3D@@SAABV12@XZ", "match:",
     "G3D::Vector3::unitY() | G3D 8.00-roblox identical, not CoordinateFrame() (g3d-2013.md)"),
    (0x009daa20, None, "rename", "?unitZ@Vector3@G3D@@SAABV12@XZ", "match:",
     "G3D::Vector3::unitZ() | G3D 8.00-roblox identical, not Vector3::ignore (g3d-2013.md)"),
    (0x009d7c30, None, "rename", "?gray@Color3@G3D@@SAABV12@XZ", "match:",
     "G3D::Color3::gray() | G3D 8.00-roblox identical: loads 0.7f, not Color3::one (g3d-2013.md)"),
    (0x007681b0, None, "unname", "", "lib:",
     "not G3D::Color4::one (that name was wrong): stores 0.6f to all four channels (g3d-2013.md); unplaced"),
    (0x009df4e0, None, "rename",
     "?collisionLocationForMovingPointFixedAABox@CollisionDetection@G3D@@SA_NABVVector3@2@0ABVAABox@2@AAV32@AA_N2@Z",
     "match:", "G3D::CollisionDetection::collisionLocationForMovingPointFixedAABox | G3D 8.00-roblox identical; "
     "not rayAABox (g3d-2013.md)"),
    (0x004a2e90, None, "rename", "?toObjectSpace@CoordinateFrame@G3D@@QBE?AV12@ABV12@@Z", "lib:",
     "G3D::CoordinateFrame::toObjectSpace(const CoordinateFrame&) | G3D 8.00-roblox located, not identical: "
     "inline copy in App code computing inverse() * c, calls CoordinateFrame::operator* 0x0048f920; not "
     "toObjectSpace(Box) (g3d-2013.md)"),
    (0x004a2dc0, 0x00532180, "rename", "?inverse@CoordinateFrame@G3D@@QBE?AV12@XZ", "lib:",
     "G3D::CoordinateFrame::inverse() | G3D 8.00-roblox located, not identical: inline copy in App code, "
     "93% against ours when paired (g3d-2013.md)"),
]


def read_csv(name):
    p = os.path.join(EXP, name)
    return list(csv.DictReader(open(p))) if os.path.exists(p) else []


def studio_of(player_addr):
    """STUDIO address of a PLAYER function: a unique body match (pair or pair-callpos)."""
    hits = [int(r["a_address"], 16) for r in read_csv("STUDIO-PLAYER-bodymatch.csv")
            if r["status"] in ("pair", "pair-callpos") and int(r["b_address"], 16) == player_addr]
    return hits[0] if len(hits) == 1 else None


def split_top(q):
    """Split a qualified name at the '::' outside template brackets."""
    out, depth, k0 = [], 0, 0
    for k, ch in enumerate(q):
        depth += ch == "<"
        depth -= ch == ">"
        if depth == 0 and q[k:k + 2] == "::" and k >= k0:
            out.append(q[k0:k])
            k0 = k + 2
    return out + [q[k0:]]


def canon(sig):
    """norm_sig plus what still differs between the ABIs' demanglers: bool template arguments (Itanium
    prints true/false, llvm-undname 1/0) and template arguments repeated on a ctor/dtor name (undname
    prints ~EnumDesc<X>, c++filt ~EnumDesc)."""
    if not sig:
        return sig
    sig = re.sub(r"(?<=[<,])true(?=[,>])", "1", sig)
    sig = re.sub(r"(?<=[<,])false(?=[,>])", "0", sig)
    q = G.qual(sig)
    parts = split_top(q)
    if len(parts) >= 2:
        last, cls = parts[-1], parts[-2].split("<")[0]
        if last.lstrip("~").split("<")[0] == cls and "<" in last:
            parts[-1] = last.split("<")[0]
            sig = "::".join(parts) + sig[len(q):]
    return sig


def last_name(q):
    """Last component of a qualified name, without template arguments."""
    if q.startswith("?") and not q.startswith("??"):
        return q[1:].split("@")[0]      # an MSVC name reccmp could not undecorate: ?name@scope@...
    return split_top(q)[-1].split("<")[0]


def resolve(v, cur, src, dem, want, nm, size, oev, elsewhere):
    """Decide a conflict (our 100% body vs an oracle or FLIRT name) by the evidence that placed the old name.
    oev: (evidence_class, evidence) of the oracle row in <T>-names-proposed.csv. Returns (verdict, note)."""
    cls, ev = oev
    if src == "flirt:":
        if cur in ("start",):
            return "keep-entry", "the PE entry point keeps IDA's name"
        return "body-wins", "superseded flirt: %s (vc32rtf signature, contradicted by the 100%% body)" % cur
    if ev.startswith("folded (") or size <= 8:
        return "folded-original", ("the original linker folded identical bodies (%s); the oracle name is one of "
                                   "them, ours another; oracle name kept" % (ev.split(";")[0] if ev else
                                                                             "%d-byte body" % size))
    if elsewhere:
        return "folded-original", ("our build defines the oracle signature at 0x%s too: the original linker folded "
                                   "the two; oracle name kept" % ",0x".join("%08x" % x for x in elsewhere))
    if nm and dem and last_name(G.qual(G.norm_sig(dem) or dem)) == last_name(nm):
        return "same-function", "same function name (ours %s); the class or parameters are spelled differently; " \
                                "oracle name kept" % nm
    if cls in ("callgraph", "body-match", "body-match-guess"):
        return "body-wins", "superseded %s %s (%s evidence only, contradicted by the 100%% body)" % (src, dem, cls)
    return "same-function", ("oracle evidence %s (%s) names this function; our name differs in spelling or source; "
                             "oracle name kept" % (cls or "-", ev[:80]))


def classify(cur, src, sig_cur, ours, our_sigs, elsewhere):
    """Verdict for one 100% pairing. cur: IDA name, src: its export source, sig_cur: its norm_sig,
    ours: our names at R, our_sigs: their norm_sigs, elsewhere: our build defines sig_cur at another body."""
    if src == "dummy" or DUMMY.match(cur):
        return "rename"
    if cur in ours:
        return "agree"
    if sig_cur and sig_cur in our_sigs:
        return "agree-sig"
    if sig_cur and G.qual(sig_cur) in {G.qual(x) for x in our_sigs}:
        return "agree-qual"     # same function, parameters or cv spelled differently (our source, not 2013)
    if src in ("prior:", "ida:"):
        return "rename"
    if src in ("ios:", "mac:", "flirt:"):
        return "conflict-folded" if elsewhere else "conflict"
    return "conflict"   # match:/lib: names the library pass owns; reported, not touched


def plan(target, json_path, exe):
    funcs = {int(r["address"], 16): r["name"] for r in read_csv(f"{target}-functions.csv")}
    names = {int(r["address"], 16): r for r in read_csv(f"{target}-names.csv") if r["kind"] == "func"}
    libown = {int(r["address"], 16) for r in read_csv(f"{target}-lib-resolved.csv")
              if r["status"] in ("identical", "located")}
    size = {int(r["address"], 16): int(r["size"]) for r in read_csv(f"{target}-functions.csv")}
    oev = {}
    for r in read_csv(f"{target}-names-proposed.csv"):
        oev.setdefault(int(r["address"], 16), (r["evidence_class"], r["evidence"]))
    view = G.pdb_view(exe)
    addr, _, _ = G.read_map(exe[:-4] + ".map")
    at = defaultdict(set)
    for d, a in addr.items():
        at[a].add(d)
    entries = [e for e in json.load(open(json_path))["data"]
               if e.get("type", 1) == 1 and e.get("recomp") and (e["matching"] >= 1.0 or e.get("effective"))]
    # every decorated name we need a signature for: ours at each R, and the current MSVC names
    dec = set()
    for e in entries:
        r = int(e["recomp"], 16)
        dec |= {d for d in at[r] if d.startswith("?")}
        if view.get(r, (0, None))[1]:
            dec.add(view[r][1])
    dec |= {n for n in funcs.values() if n.startswith("?")}
    sig = G.undname(sorted(dec))
    oursig_all = defaultdict(set)   # signature -> our addresses defining it
    for d, a in addr.items():
        if sig.get(d):
            oursig_all[canon(sig[d])].add(a)

    updates, conflicts, stat = {}, [], Counter()
    jname = os.path.basename(json_path)
    for e in entries:
        a, r = int(e["address"], 16), int(e["recomp"], 16)
        if a not in funcs:
            stat["not an IDA function start"] += 1
            continue
        if a in libown:
            stat["library-owned (apply_lib_names.py)"] += 1
            continue
        t, d, nm = view.get(r, ("-", None, None))
        want = d or nm
        if not want:
            stat["no recompiled name"] += 1
            continue
        ours = ({d} if d else set()) | at[r] | ({nm} if nm else set())
        our_sigs = {canon(sig[x]) for x in ours if sig.get(x)} | ({canon(G.norm_sig(nm + "()"))} if nm and not d else set())
        cur = funcs[a]
        row = names.get(a)
        src = row["source"] if row else "dummy"
        dem = (row["demangled"] if row else "") or cur
        sig_cur = canon(sig.get(cur) if cur.startswith("?") else G.norm_sig(dem))
        elsewhere = bool(sig_cur and oursig_all.get(sig_cur, set()) - {r})
        v = classify(cur, src, sig_cur, ours, our_sigs, elsewhere)
        stat[f"{v} ({src})"] += 1
        pct = "effective 100%" if e["matching"] < 1.0 else "100%"
        line = f"p4 match: reccmp {pct} vs our {want} @0x{r:08x} ({jname})"
        if v == "rename":
            updates[a] = ("rename", want, "match:", line)
        elif v.startswith("agree"):
            updates[a] = ("comment", "", "match:", line)
        else:
            v, note = resolve(v, cur, src, dem, want, nm, size.get(a, 0), oev.get(a, ("", "")),
                              sorted(oursig_all.get(sig_cur, set()) - {r}))
            stat[f"  resolved: {v}"] += 1
            if v == "body-wins":
                updates[a] = ("rename-lib" if src == "flirt:" else "rename", want, "match:", line + "; " + note)
            else:
                updates[a] = ("comment", "", "match:", line + "; " + note)
            conflicts.append((f"0x{a:08x}", cur, src, dem, want, f"0x{r:08x}", e["name"], v, note))

    for pa, sa, act, name, tag, ev in FACTS:
        a = pa if target == "PLAYER" else (sa or studio_of(pa))
        if a is None:
            stat["fact: no STUDIO address"] += 1
            print(f"{target}: no STUDIO address for fact 0x{pa:08x} {name or ev[:40]}")
            continue
        where = "" if target == "PLAYER" else f" [STUDIO: body-identical to PLAYER 0x{pa:08x}]"
        if a in updates and updates[a][0] == "rename" and act == "rename" and updates[a][1] != name:
            print(f"{target}: fact 0x{a:08x} {name} overrides the reccmp name {updates[a][1]}")
        updates[a] = (act, name, tag, f"p4 {tag} {ev}{where}")
        stat[f"fact {act} ({tag})"] += 1

    if target == "STUDIO":
        # PLAYER's reccmp-proven renames carried to the STUDIO body that is identical to it (unique pair in
        # STUDIO-PLAYER-bodymatch.csv), where STUDIO's name is still a dummy and nothing above decided it.
        pmap = {int(r["b_address"], 16): int(r["a_address"], 16) for r in read_csv("STUDIO-PLAYER-bodymatch.csv")
                if r["status"] in ("pair", "pair-callpos")}
        for r in read_csv("PLAYER-db-updates.csv"):
            pa = int(r["address"], 16)
            sa = pmap.get(pa)
            if r["action"] != "rename" or r["tag"] != "match:" or "reccmp" not in r["line"] or sa is None:
                continue
            if sa in updates or sa in libown or sa not in funcs or not DUMMY.match(funcs[sa]):
                continue
            updates[sa] = ("rename", r["name"], "match:",
                           r["line"] + f" [STUDIO: body-identical to PLAYER 0x{pa:08x}, measured there]")
            stat["rename from PLAYER via body match"] += 1
    with open(os.path.join(EXP, f"{target}-db-updates.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["address", "action", "name", "tag", "line"])
        for a in sorted(updates):
            w.writerow([f"0x{a:08x}", *updates[a]])
    with open(os.path.join(EXP, f"{target}-match-conflicts.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["address", "ida_name", "source", "demangled", "recompiled", "recomp_address", "reccmp_name",
                    "verdict", "evidence"])
        w.writerows(conflicts)
    for k, v in sorted(stat.items()):
        print(f"{target}: {k:55s} {v}")
    print(f"{target}: entries at 100% {len(entries)}, updates {len(updates)}, conflicts {len(conflicts)}")


def selftest():
    s = G.norm_sig
    assert classify("sub_401000", "dummy", None, {"?f@@YAXXZ"}, {"C:f"}, False) == "rename"
    assert classify("?f@@YAXXZ", "match:", "f()", {"?f@@YAXXZ"}, {"f()"}, False) == "agree"
    x = s("RBX::Name::Name(char const* const&)")
    assert classify("_ZN3RBX4NameC2ERKPKc", "ios:", x, {"?y"}, {x}, False) == "agree-sig"
    assert classify("_ZN1A1fEv", "ios:", "A::f()", {"?g"}, {"A::g()"}, True) == "conflict-folded"
    assert classify("_ZN1A1fEv", "ios:", "A::f()", {"?g"}, {"A::g()"}, False) == "conflict"
    assert classify("_ZN1A1fEv", "prior:", "A::f()", {"?g"}, {"A::g()"}, False) == "rename"
    assert resolve("conflict", "_Z", "ios:", "A::f()", "?g", "A::g", 6, ("rtti-slot", "x"), [])[0] == "folded-original"
    assert resolve("conflict", "_Z", "ios:", "A::f()", "?g", "A::g", 76, ("callgraph", "x"), [])[0] == "body-wins"
    assert resolve("conflict", "_Z", "ios:", "W(X*)", "W", "W", 177, ("callgraph", "x"), [])[0] == "same-function"
    assert resolve("conflict", "_Z", "ios:", "A::f()", "?g", "A::g", 99, ("string-unique", "s"), [])[0] == "same-function"
    assert resolve("conflict", "_Z", "ios:", "D<X, s>::cd()", "?cd@", "D<X,&s>::cd", 99, ("callgraph", ""), [])[0] \
        == "same-function"
    assert resolve("conflict", "_Z", "ios:", "D<X, s>::cd()", "?cd@?$D", "?cd@?$D", 99, ("callgraph", ""), [])[0] \
        == "same-function"
    assert canon("A::Bridge<X,true>::on_index(lua_State*)") == "A::Bridge<X,1>::on_index(lua_State*)"
    assert canon("R::EnumDesc<R::Material>::~EnumDesc<R::Material>()") == "R::EnumDesc<R::Material>::~EnumDesc()"
    assert canon("rbx::trie<P*,50>::trie<P*,50>()") == "rbx::trie<P*,50>::trie()"
    assert classify("_ZN1A1fEv", "ios:", "A::f()const", {"?f"}, {"A::f()"}, False) == "agree-qual"
    print("match_names selftest ok")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("target", nargs="?")
    ap.add_argument("--json")
    ap.add_argument("--exe")
    ap.add_argument("--selftest", action="store_true")
    o = ap.parse_args()
    if o.selftest:
        selftest()
    else:
        plan(o.target, o.json, o.exe)

"""Coverage of a DECOMP_PLAN Phase 5 row: every PLAYER function of a set of source files, with its score.

    python3 -P tools/row_coverage.py P.json [--row 5a|5b] [--files Client/App/util/Math.cpp ...] [--csv out.csv]

With no --files it measures --row (default 5a: math and G3D helpers in Client/App/util + include/Util;
5b: DataModel and the core services). Math.cpp's range is one 2013 .obj; an address counts once.
Per file, the original .obj range is [lowest PLAYER marker, highest marker + its size] widened over
neighbouring functions whose oracle name (ida/exports/PLAYER-names.csv) carries one of the file's
class qualifiers (`RBX::Math::`). Every IDA function in that range is listed with its oracle name and
its score in P.json (reccmp --json): "100" (effective), the ratio, or "-" (no body / not paired).
Coverage = functions at effective 100% / functions in range. A range is a hypothesis about .obj
bounds: /Gy keeps .obj order, but a function from another .obj can sit at a range edge.
"""
import argparse
import bisect
import csv
import json
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MARK = re.compile(r"// (?:STUB|FUNCTION): PLAYER 0x([0-9a-f]{8})")
ROW_5A = {
    "Client/App/util/Math.cpp": ["RBX::Math::"],
    "Client/App/util/Extents.cpp": ["RBX::Extents::"],
    "Client/App/util/Face.cpp": ["RBX::Face::"],
    "Client/App/util/NormalId.cpp": ["RBX::normalIdTo", "RBX::Vector3ToNormalId", "RBX::intToNormalId",
                                     "RBX::normalIdOpposite", "RBX::normalIdToMatrix3", "RBX::uvwToObject",
                                     "RBX::objectToUvw"],
    "Client/App/util/Region3.cpp": ["RBX::Region3::"],
    "Client/App/util/Region3int16.cpp": ["RBX::Region3int16::"],
    "Client/App/util/IndexedMesh.cpp": ["RBX::IndexedMesh::"],
    "Client/App/util/InterpolatedCFrame.cpp": ["RBX::InterpolatedCFrame::"],
    "Client/App/util/Quaternion.cpp": ["RBX::Quaternion::"],
    "Client/App/util/Velocity.cpp": ["RBX::Velocity::"],
    "Client/App/util/Units.cpp": ["RBX::Units::"],
    "Client/App/util/PV.cpp": ["RBX::PV::"],
    "Client/App/util/ExtentsInt32.cpp": ["RBX::ExtentsInt32::"],
    "Client/Rendering/RbxG3D/Frustum.cpp": ["RBX::Frustum::"],
    "Client/Rendering/RbxG3D/RbxRay.cpp": ["RBX::RbxRay::"],
    "Client/Rendering/RbxG3D/RbxCamera.cpp": ["RBX::RbxCamera::"],
}

ROW_5B = {
    "Client/App/v8datamodel/DataModel.cpp": ["RBX::DataModel::"],
    "Client/App/v8datamodel/DataModelJob.cpp": ["RBX::DataModelJob::", "RBX::DataModelArbiter::"],
    "Client/App/v8datamodel/GuiBuilder.cpp": ["RBX::GuiBuilder::"],
    "Client/App/v8tree/Service.cpp": ["RBX::ServiceProvider::"],
    "Client/App/v8datamodel/Workspace.cpp": ["RBX::Workspace::"],
    "Client/App/v8datamodel/Lighting.cpp": ["RBX::Lighting::"],
    "Client/App/v8datamodel/Teams.cpp": ["RBX::Teams::"],
    "Client/App/v8datamodel/Team.cpp": ["RBX::Team::"],
    "Client/Network/Players.cpp": ["RBX::Network::Players::"],
    "Client/Network/Player.cpp": ["RBX::Network::Player::"],
    "Client/App/v8datamodel/RunService.cpp": ["RBX::RunService::"],
    "Client/App/v8datamodel/StarterGuiService.cpp": ["RBX::StarterGuiService::"],
    "Client/App/v8datamodel/StarterPackService.cpp": ["RBX::StarterPackService::"],
    "Client/App/v8datamodel/DebrisService.cpp": ["RBX::DebrisService::"],
    "Client/App/v8datamodel/PhysicsInstructions.cpp": ["RBX::PhysicsInstructions::"],
}
ROWS = {"5a": ROW_5A, "5b": ROW_5B}
# Largest gap between two markers of one .obj. DataModel.obj spans 0x0062d1e0-0x00652800 with
# 0x9000-byte runs of template code between its own functions.
SPAN = {"5a": 0x2000, "5b": 0xa000}


GAP = 8


def densest(addrs, span=0x2000):
    """The largest run of addresses with consecutive gaps under span (inline copies sit elsewhere)."""
    best, cur = [], []
    for ad in addrs:
        cur = cur + [ad] if cur and ad - cur[-1] < span else [ad]
        if len(cur) > len(best):
            best = cur
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("json")
    ap.add_argument("--row", default="5a", choices=sorted(ROWS))
    ap.add_argument("--files", nargs="*")
    ap.add_argument("--csv")
    a = ap.parse_args()
    funcs = sorted((int(r["address"], 16), int(r["size"])) for r in
                   csv.DictReader(open(os.path.join(ROOT, "ida/exports/PLAYER-functions.csv"))))
    starts = [f[0] for f in funcs]
    names = {}
    for r in csv.DictReader(open(os.path.join(ROOT, "ida/exports/PLAYER-names.csv"), encoding="utf-8")):
        if r["kind"] == "func":
            names[int(r["address"], 16)] = r["demangled"] or r["name"]
    score = {int(e["address"], 16): e for e in json.load(open(a.json))["data"]}
    row = ROWS[a.row]
    files = {f: row.get(f, []) for f in a.files} if a.files else row
    rows, tot, ok, seen = [], 0, 0, set()
    for f, quals in files.items():
        p = os.path.join(ROOT, f)
        marks = sorted({int(m, 16) for m in MARK.findall(open(p, encoding="utf-8", errors="replace").read())}) \
            if os.path.exists(p) else []
        own = lambda ad: any(q in names.get(ad, "") for q in quals)
        seeds = densest(marks, SPAN[a.row]) or densest(sorted(ad for ad in names if own(ad)))
        if not seeds:
            print(f"{f}: no marker and no oracle name")
            continue
        lo, hi = bisect.bisect_left(starts, min(seeds)), bisect.bisect_left(starts, max(seeds))
        # widen over neighbours named for the file's classes, allowing GAP unnamed functions between
        while True:
            d = next((i for i in range(1, GAP + 1) if lo - i >= 0 and own(starts[lo - i])), 0)
            if not d:
                break
            lo -= d
        while True:
            d = next((i for i in range(1, GAP + 1) if hi + i < len(starts) and own(starts[hi + i])), 0)
            if not d:
                break
            hi += d
        n = k = 0
        for ad, sz in funcs[lo:hi + 1]:
            if ad in seen:
                continue
            seen.add(ad)
            e = score.get(ad)
            s = "-" if e is None else ("100" if e["matching"] >= 1 or e.get("effective") else f"{e['matching']*100:.1f}")
            rows.append((f, f"0x{ad:08x}", sz, s, "M" if ad in marks else "", names.get(ad, "")))
            n += 1
            k += s == "100"
        tot += n
        ok += k
        out = [m for m in marks if m not in seeds]
        if out:
            print(f"  {f}: markers outside the .obj cluster: " + " ".join(f"0x{m:08x}" for m in out))
        print(f"{f}: 0x{starts[lo]:08x}-0x{starts[hi]:08x}  {k}/{n} at 100%, {len(marks)} markers")
    print(f"TOTAL {ok}/{tot} = {100*ok/max(tot,1):.1f}%")
    if a.csv:
        with open(a.csv, "w", newline="") as o:
            w = csv.writer(o)
            w.writerow(["file", "address", "size", "score", "marker", "oracle_name"])
            w.writerows(rows)


if __name__ == "__main__":
    main()

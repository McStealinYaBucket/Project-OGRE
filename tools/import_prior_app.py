"""Copy the prior 2013 App decomp ("2013 55% App/") into Client/ at the 2013 __FILE__ paths.

Run from the repo root:
    python3 -P tools/import_prior_app.py [--force] [--dry-run]

Bodies are copied unchanged. Paths change in two ways only, both decided by notes/paths-2013.txt:
  * directory case: each path component takes the spelling the 2013 records use most often
    (records that are lowercase from end to end carry no case information and are ignored; a tie
    keeps the tree's spelling). Under wine, cl resolves #include case-insensitively, so this is
    cosmetic for the build and only fixes the on-disk names.
  * moves: a non-empty .cpp goes to the directory the 2013 binary records for that file name, when
    exactly one directory is recorded and the tree's file there is missing or a 0-byte stub.
    MOVE_EXCEPTIONS lists the ones that are a different class under the same file name.
Not copied: App.BulletPhysics (not linked in any 2013 build), App/Lua-5.1.4 (the 2016 hardened fork;
a separate agent reconstructs the 2013 Lua), the G3D/ shadow copy (in no include path), MSBuild /
Xcode project files, boostlibs/, PropertySheets/ and HANDOFF.md.

Existing destination files are left alone unless --force (the marker pass edits them in place).
Writes notes/app-import-manifest.csv: old path, new path, kind, how the path was decided.
"""
import collections, csv, os, shutil, sys

SRC, DST = "2013 55% App", "Client"
SKIP_TOP = {"App.BulletPhysics", "G3D", "boostlibs", "PropertySheets"}
SKIP_DIRS = {"App/Lua-5.1.4"}
SKIP_EXT = (".vcxproj", ".sln", ".filters", ".vspscc", ".pbxproj", ".props", ".user")
SKIP_FILES = {"HANDOFF.md"}
VENDORED = ("Rendering/g3d/", "Network/raknet/", "fmod/", "App/include/fmod/", "BuiltInPlugins/",
            "shaders/", "App/lz4/")
# App/v8datamodel/Item.cpp is RBX::GuiItem ("Item"); the 2013 Network/Item.cpp is the replication item.
MOVE_EXCEPTIONS = {"App/v8datamodel/Item.cpp"}


def records():
    out = []
    for line in open("notes/paths-2013.txt"):
        if line.startswith("#") or "\t" not in line:
            continue
        p = line.rstrip("\n").split("\t")[1].replace("\\", "/")
        if p.lower().startswith("client/"):
            out.append(p[len("client/"):])
    return out


def component_case(recs):
    """lowercase dir prefix -> Counter of spellings of its last component."""
    votes = collections.defaultdict(collections.Counter)
    for p in recs:
        if p == p.lower():
            continue
        parts = p.split("/")[:-1]
        for i in range(len(parts)):
            votes["/".join(parts[:i + 1]).lower()][parts[i]] += 1
    return votes


def recase(rel_dir, votes):
    parts, out = rel_dir.split("/") if rel_dir else [], []
    for i, comp in enumerate(parts):
        c = votes.get("/".join(parts[:i + 1]).lower())
        if c:
            best = c.most_common()
            if len(best) == 1 or best[0][1] > best[1][1]:
                comp = best[0][0]
        out.append(comp)
    return "/".join(out)


def walk():
    for d, dirs, files in os.walk(SRC):
        rel_d = os.path.relpath(d, SRC).replace(os.sep, "/")
        rel_d = "" if rel_d == "." else rel_d
        dirs[:] = sorted(x for x in dirs if not (rel_d == "" and x in SKIP_TOP)
                         and (rel_d + "/" + x).lstrip("/") not in SKIP_DIRS and not x.endswith(".xcodeproj"))
        for f in sorted(files):
            if f.endswith(SKIP_EXT) or (rel_d == "" and f in SKIP_FILES):
                continue
            yield (rel_d + "/" + f).lstrip("/")


def plan():
    recs = records()
    votes = component_case(recs)
    by_name = collections.defaultdict(set)
    rec_dirs = set()
    for p in recs:
        by_name[os.path.basename(p).lower()].add(os.path.dirname(p).lower())
        rec_dirs.add(os.path.dirname(p).lower())
    files = list(walk())
    present = {f.lower(): f for f in files}
    size = {f: os.path.getsize(os.path.join(SRC, f)) for f in files}
    moved_into = set()
    rows = []
    for f in files:
        d, name = os.path.dirname(f), os.path.basename(f)
        kind = "vendored" if f.startswith(VENDORED) else "code"
        how = "unrecorded"
        dirs = by_name.get(name.lower(), set())
        if d.lower() in dirs:
            how = "2013-file"
        elif d.lower() in rec_dirs:
            how = "2013-dir"
        if (kind == "code" and name.endswith(".cpp") and size[f] and len(dirs) == 1 and d.lower() not in dirs
                and f not in MOVE_EXCEPTIONS):
            target = next(iter(dirs))
            there = present.get(target + "/" + name.lower())
            if there is None or size[there] == 0:
                d, how = target, "2013-file (moved)"
                moved_into.add((target + "/" + name).lower())
        rows.append([f, d, name, kind, how])
    out = []
    for f, d, name, kind, how in rows:
        if (f.lower() in moved_into) and size[f] == 0:
            how = "replaced by a moved file"  # the 0-byte stub the moved file supersedes
            out.append((f, "", kind, how))
            continue
        out.append((f, DST + "/" + "/".join(x for x in (recase(d, votes), name) if x), kind, how))
    return out


def main():
    force, dry = "--force" in sys.argv, "--dry-run" in sys.argv
    rows = plan()
    c = collections.Counter()
    for old, new, kind, how in rows:
        c[kind, how] += 1
        if not new or dry:
            continue
        if os.path.exists(new) and not force:
            c["kept existing"] += 1
            continue
        os.makedirs(os.path.dirname(new), exist_ok=True)
        shutil.copy2(os.path.join(SRC, old), new)
        os.chmod(new, os.stat(new).st_mode | 0o200)  # some prior files are read-only
    if not dry:
        with open("notes/app-import-manifest.csv", "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["old", "new", "kind", "path_source"])
            w.writerows(rows)
    for k, n in sorted(c.items(), key=str):
        print("%6d  %s" % (n, k))
    for old, new, kind, how in rows:
        if "moved" in how or (new and old.split("/")[:-1] != new.split("/")[1:-1]):
            print("  %-60s -> %s" % (old, new or "(dropped)"))


if __name__ == "__main__":
    main()

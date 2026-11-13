# ida-sync (workflow phase4-core2, wf_fef33d15-1aa)

I synced the proven names and the proven class layouts into ida/PLAYER.i64 and ida/STUDIO.i64. Both databases are saved packed, with no .id0/.id1/.nam/.til files left behind.

**What I found from the killed attempts.** Steps 2 and 3 had already been applied at 16:15. I checked them before redoing anything:
- Running the planner (tools/ida/match_names.py) again reproduces ida/exports/<T>-db-updates.csv and <T>-match-conflicts.csv byte for byte. It has to read copies of the PDB and map that P-final3/S-final3 actually scored (scratchpad bp/ and bs/). build/ was relinked at 17:04 by another agent, so its PDB and map no longer match those JSONs.
- Applying the updates a second time sets no new names: PLAYER 712 already set, STUDIO 696. The one "unnamed" per database is 0x007681b0, which already had its dummy name.
- The 06 snapshots were taken before any change and I left them alone. They could no longer be cmp-checked against the canonical files, because those had already changed. Before the type import I took ida/snapshots/<T>-07-pre-types.i64, cmp-verified, as the undo point.

**Step 2, the Phase 4 facts.** All are now in place and confirmed in the exports, PLAYER (STUDIO addresses where known):
- Renamed: 0x004022d0 `rbx::atomic<int>::atomic` (str:), 0x0070cf60 `DataModelArbiter()` and 0x0063ee60 the `GenericJob` constructor (prior:), 0x009b8f00/0x009b8f10 `RbxInterlockedDecrement`/`Increment` (ios:).
- Wrong names fixed: 0x0066ee10 / STUDIO 0x00773760 is `EventSource::raiseEventInvocation` (rtti:). 0x009dab20 is `Vector3::maxFinite`, not `Extents::vv`.
- G3D functions under the match: tag: `unitY` 0x009da9e0, `unitZ` 0x009daa20, `Color3::gray` 0x009d7c30, `collisionLocationForMovingPointFixedAABox` 0x009df4e0.
- G3D functions under the lib: tag: `CoordinateFrame::toObjectSpace(const CoordinateFrame&)` 0x004a2e90 and `CoordinateFrame::inverse` 0x004a2dc0 / STUDIO 0x00532180.
- 0x007681b0 is unnamed (it is not `Color4::one`). The Instance allocator 0x0061bbd0 has a guess: comment only.
- The names the report lists as proven at 100% (the DescribedBase, ServiceProvider and VerbContainer constructors, the `GuidItem<Instance>` and `Limits::Countable` destructors, the trie and object_pool functions) came through the bulk rule.

**Step 3, bulk promotion of names reccmp proved.**

| | PLAYER | STUDIO |
|---|---|---|
| Functions at effective 100% | 2,808 | 2,725 |
| Left to the library-name script | 1,571 | 1,520 |
| Renamed with match: | 697 | 666, plus 15 carried over from identical PLAYER bodies |
| Name already agreed, comment only | 517 | 515 |
| Conflicts with an ios:/mac:/FLIRT name | 23 | 24 |

How the conflicts were resolved:

| Verdict | PLAYER | STUDIO |
|---|---|---|
| Our recompiled name wins (callgraph-only evidence or a FLIRT false positive) | 2 | 3 |
| Oracle name kept: the original linker folded identical bodies | 18 | 18 |
| Oracle name kept: same function, spelled differently | 3 | 2 |
| Entry point keeps IDA's name | 0 | 1 |

One rename was not set: STUDIO 0x0054b3b0, because its `_Tree::_Erase` name is already used at its identical twin 0x004f3770.

**Step 4, the class layouts.**
- **New, lean `dump`.** I rewrote it in tools/ida/import_types.py. It streams cvdump -t to a temporary file and reads back only the 268 records the listed classes need. It peaks at about 50 MB and takes 2 seconds, against the 13–22 GB that crashed the machine.
- **Why the old JSON was wrong.** It mixed up template instantiations: Instance's `Described` base came out as 0xd00 bytes, and `NonFactoryProduct` came out as a base of DataModel. Two causes: cvdump's UDT() index is per printed name, and this incrementally linked PDB keeps stale records, such as a 76-byte Instance. The new dump identifies a class by its decorated name and takes the newest definition.
- **Layouts.** The 19 listed classes plus 29 base classes have identical layouts in PLAYER and STUDIO. Every listed size equals its `DECOMP_SIZE_ASSERT`; `Verb` (0xc) has no assert.
- **Applied.** 48 of 48 local types in each database. `this` is typed on 34 functions in each: 31 constructors and destructors plus `Instance::getRootAncestor`, `isAncestorOf` and `remove`. Constructors and destructors that only have iOS/Mac names (IDA gave them cdecl/stdcall with `this` first) were converted to thiscall.

**Step 5, exports and coverage.** tools/ida/export_names.py now also writes ida/exports/<T>-types.h (all local types: PLAYER 475, STUDIO 383). The names, functions and types exports are regenerated, and ida/LOG.md has the session note.

| Functions | PLAYER (49,091) | STUDIO (44,814) |
|---|---|---|
| Named | 9,468 → 10,205 | 10,305 → 11,017 |
| match: | 1,749 → 2,473 | 1,654 → 2,360 |
| Unnamed | 39,623 → 38,886 | 34,509 → 33,797 |

Logs are ida/logs/<T>-06-db-updates.{log,tsv}, <T>-07-db-updates-rerun.log, <T>-07-types.log, <T>-07-types-pass2.log and <T>-07-export-names.log.

## Structured fields

```json
{
 "files": [
  "tools/ida/import_types.py",
  "tools/ida/export_names.py",
  "tools/ida/match_names.py",
  "tools/ida/apply_db_updates.py",
  "ida/PLAYER.i64",
  "ida/STUDIO.i64",
  "ida/snapshots/PLAYER-07-pre-types.i64",
  "ida/snapshots/STUDIO-07-pre-types.i64",
  "ida/exports/PLAYER-names.csv",
  "ida/exports/PLAYER-functions.csv",
  "ida/exports/PLAYER-types.h",
  "ida/exports/PLAYER-proven-types.json",
  "ida/exports/PLAYER-db-updates.csv",
  "ida/exports/PLAYER-match-conflicts.csv",
  "ida/exports/STUDIO-names.csv",
  "ida/exports/STUDIO-functions.csv",
  "ida/exports/STUDIO-types.h",
  "ida/exports/STUDIO-proven-types.json",
  "ida/exports/STUDIO-db-updates.csv",
  "ida/exports/STUDIO-match-conflicts.csv",
  "ida/LOG.md"
 ],
 "unresolved": [
  "build/ was relinked at 17:04, so its PDB and map no longer match build/sweep2/P-final3.json. Anything that joins that JSON with build/ (gen_reccmp_csv.py, match_names.py) must use the PDB/map that was scored (scratchpad bp/ and bs/, copied by the first attempt) or a fresh rescore.",
  "ida/exports/<T>-lib-resolved.csv (written by tools/lib/lib_consistency.py, outside my ownership) still has 0x007681b0 as Color4::one and 0x009dab20 as a conflict. apply_lib_names.py leaves both alone today, but the CSV should be corrected at its source.",
  "`this` is not typed on `GuidItem<Instance>::~GuidItem` (PLAYER 0x0061b060, STUDIO 0x006efd20), because IDA gives that function no arguments. It is also not typed for classes whose constructors and destructors have no name yet (Debugable, Object, EventSource, Verb, Name, LibraryService), or on methods that only have Itanium names: the Itanium name does not say whether a method is static.",
  "STUDIO 0x0054b3b0 kept its dummy name: the recompiled `_Tree::_Erase` name is already at its identical twin 0x004f3770.",
  "ida/exports/<T>-names-proposed.csv is older than today's thunk splits and renames. Regenerate it with tools/pair_names.py before the next apply_names.py run.",
  "The ida/snapshots/<T>-06-phase4-sync.i64 snapshots from the first attempt could not be cmp-checked against the canonical files, which had already changed by then. Their sizes fit the 10:53 state."
 ]
}
```


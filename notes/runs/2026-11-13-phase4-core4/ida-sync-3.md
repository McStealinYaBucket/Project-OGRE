# ida-sync-3 (workflow phase4-core4, wf_cf7a5075-c0f)

Both databases are synced, saved packed and closed, with no .id0/.id1/.nam/.til files left. Before any change I snapshotted them to ida/snapshots/PLAYER-09-core3-sync.i64 and STUDIO-09-core3-sync.i64; cmp showed each identical to its canonical database.

**Step 2, facts.** I checked every fact against the 2013 bytes with capstone. Each STUDIO address comes from a body-match pair or from the operands of the STUDIO DataModel ctor (0x0076b720).
- Time::now<0> (Fast): 0x009bb680 / 0x00a724b0, ios:. It does `cmp preciseOverride,0; jg`, and nowFast calls it. The CSV's name "now<2>" for it is wrong.
- Time::now<1>: 0x009bb6e0 / 0x00a72510, ios:.
- Time::now<2>, the QueryPerformanceCounter clock: 0x009bb400 / 0x00a72250, ios:.
- New global names:
  - FFlag::NewLightAPI 0x00de4760 / 0x00f83b31 and FFlag::ImageRectEnabled 0x00de4761 / 0x00f83b32 (ios:).
  - DataModel::count 0x00de4c0c / 0x00f83fd8 (ios:).
  - The three call_once flags: image 0x00de4768 / 0x00f83b38, light 0x00de4770 / 0x00f83b40, network 0x00de4778 / 0x00f83b48. These are tagged guess:: the call_once calls prove each flag's role, but the names come from our source because no oracle symbol exists.
- Comments only, no rename:
  - 0x005ba6f0 / 0x0065e110: a second atomic ctor, assert at atomic.h line 182.
  - 0x0061c800 / 0x006f1310: the ctor FWInstance calls on this+0x18.
  - 0x009b1a20 / 0x00917010: Surface(PartInstance*, NormalId) shares this body with the RakNet RangeNode ctor, so the RakNet name stays.
- Already set from last session: STUDIO ~DataModel, RemovePlace, the GuiBuilder ctor, STUDIO Initialize (0x008a8950, shared with SetWriteOffset).
- Came in through the bulk rule at 100%:
  - RbxInterlockedExchange / CompareExchange: 0x009b8f20 / 0x009b8f40, STUDIO 0x00a70830 / 0x00a70850.
  - getPrimaryPart: 0x00723500 / 0x0079f430.
  - The getLocation vtordisp thunks: PLAYER 0x00723590 / 0x006d2960, STUDIO 0x0079f4c0 / 0x00817ef0. For these, match_names.py now also reads reccmp's type-10 (vtordisp thunk) entries.

**Step 3, bulk match.** Run from P-final.json / S-final.json, joined only with the scored-i5 copies.

| | PLAYER | STUDIO |
|---|---|---|
| Functions at effective 100% | 3,195 | 3,085 |
| Left to the library-name script | 1,651 | 1,580 |
| Renamed match: | 156 | 143 |
| Conflicts kept by evidence | 25 (17 folded-original, 8 same-function) | 23 |

- PLAYER's 156 renames: 138 unnamed functions, 17 prior: names, and the stale shared_ptr<RelativePanel>::operator= at 0x006eda50, now shared_ptr<Instance>::operator=.
- STUDIO's 143: 132 unnamed functions, 8 prior: names, the same stale shared_ptr name at 0x00751110, and two wrong FLIRT names (FUNC_LIB cleared): 0x00751980 `??1strstreambuf..._20`, which is the signal<UIEvent> ctor, and 0x00a70830 `__beep`, which is RbxInterlockedExchange.
- STUDIO 0x0054b3b0 and 0x00622570 stay unnamed: their names already sit on identical twin functions.
- New rule, names from vftable slots (tag rtti:). A function gets the name of our slot function when reccmp pairs it with that function in every vftable that lists it. It is skipped if any vftable slot disagrees, the body is 8 bytes or less, or our build merged several names at that address.
  - Named this way: PLAYER 34, STUDIO 35. They include the PartInstance and ModelInstance overrides from vtables-3 (reportTouches, onClumpChanged, getRenderSize, render3dSelect, ...), EnumDesc<X>::lookup(Variant), and Instance::raiseEventInvocation 0x00402750.
  - Skipped as tiny or merged bodies: 26 / 18.
- A rerun of the apply on a scratch copy changes nothing.

**Step 4, types.** All dumped from the scored PDB copies.
- New local types:
  - GuiBuilder: 0x20 in PLAYER, 0x1c in STUDIO (no networkStatsPage).
  - Edge: 0x1c, with the two byte enums at +8 and +9.
  - IHasPrimaryPart, IHasLocation, CameraSubject, Selectable, IMoving.
  - RBX::DataModel_vtbl with 34 named slots. The 7 slots whose original body is shared by several slots are named from Instance.h's declaration order.
- DataModel is 0xe10 in the tree now. The STUDIO correction (no networkStatsWindowsOn, members 0xc64–0xcac 4 bytes lower, parentDataModel at 0xe08) now keys on the target instead of the old 0xe18 size. FWBase and FWInstance need no correction any more.
- Result: 75 of 75 types in each database. `this` is typed on 57 PLAYER / 58 STUDIO functions, including the GuiBuilder ctor and dtor and ~Edge (0x0081f840 / 0x00912ee0).

**Step 5, pairing and coverage.**
- The STUDIO–PLAYER body match reproduced its CSV byte for byte (20,129 pairs).
- pair_names: PLAYER 6,713 functions with a strong name, STUDIO 6,854. No proposed name changed. One row was lost per target: LRUCache::remove at 0x007e02b0 / 0x008a5830; its name stays in the database. apply_names renamed nothing.
- PLAYER App block (0x00606a00–0x009ad3a0, 21,631 functions): those with an ios:/mac:/match: name went from 5,923 to 6,027. 15,604 (72.1%) still have none.
- Those 15,604 are grouped by the class of their callers, or by the class of the vftable they sit in, in ida/exports/PLAYER-app-unnamed-by-class.csv. The largest groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,472 (631 KB) |
| Ogre::RbxSceneManager vftable slots (6-byte stubs) | 306 |
| FactoryProduct::Creator | 288 |
| DataModel | 255 |
| ScriptContext | 224 |
| RBX free functions | 168 |
| EnumDesc | 143 |
| Lua::Bridge | 135 |
| GuiService | 109 |
| ContentProvider | 108 |

  After those come InsertService 95, Humanoid 94, ScriptDebugger 92, PartInstance 92 and Workspace 89.

**Step 6, coverage before → after**

| Functions with a real name | PLAYER (49,091) | STUDIO (44,814) |
|---|---|---|
| Named | 10,290 → 10,469 | 11,105 → 11,278 |
| match: | 2,563 → 2,718 | 2,451 → 2,593 |
| rtti: | 1 → 35 | 1 → 36 |
| ios: | 6,207 → 6,210 | 6,265 → 6,268 |
| prior: | 36 → 19 | 15 → 7 |

PLAYER ida: went from 1,115 to 1,119; the 4 new ones are IDA's own `j_` thunk names. The names, functions and types exports are regenerated (504 PLAYER / 412 STUDIO local types), and ida/LOG.md has the session note.

**Tool changes:**
- match_names.py: the new facts, vtordisp entries, the vftable-slot rule, replacing prior: and stale match: names, picking the name reccmp reports when our linker merged several, and making the comment line record the name actually kept.
- apply_db_updates.py: data1/data4 actions for globals.
- export_names.py: reads the source tag of globals.
- import_types.py: the new classes, the STUDIO GuiBuilder/DataModel correction, vftable slot names from the reccmp JSON.
- New tools/ida/app_name_coverage.py.

## Files changed

- ida/PLAYER.i64
- ida/STUDIO.i64
- ida/snapshots/PLAYER-09-core3-sync.i64
- ida/snapshots/STUDIO-09-core3-sync.i64
- ida/LOG.md
- tools/ida/match_names.py
- tools/ida/apply_db_updates.py
- tools/ida/export_names.py
- tools/ida/import_types.py
- tools/ida/app_name_coverage.py
- ida/exports/PLAYER-db-updates.csv
- ida/exports/STUDIO-db-updates.csv
- ida/exports/PLAYER-match-conflicts.csv
- ida/exports/STUDIO-match-conflicts.csv
- ida/exports/PLAYER-proven-types.json
- ida/exports/STUDIO-proven-types.json
- ida/exports/PLAYER-names.csv
- ida/exports/STUDIO-names.csv
- ida/exports/PLAYER-functions.csv
- ida/exports/STUDIO-functions.csv
- ida/exports/PLAYER-types.h
- ida/exports/STUDIO-types.h
- ida/exports/PLAYER-names-proposed.csv
- ida/exports/STUDIO-names-proposed.csv
- ida/exports/PLAYER-names-conflicts.csv
- ida/exports/STUDIO-names-conflicts.csv
- ida/exports/PLAYER-app-decomp-map.csv
- ida/exports/STUDIO-app-decomp-map.csv
- ida/exports/STUDIO-PLAYER-bodymatch.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv

## Unresolved

- The three call_once flags (PLAYER 0x00de4768 / 0x00de4770 / 0x00de4778) carry our own source names tagged guess:. They are function-local statics, so no oracle symbol exists; the call_once calls prove only which flag guards which register function.
- RBX::DataModel_vtbl (34 slots) exists as a local type but is not attached to DataModel's __vftable, because DataModel's vfptr lives in its Debugable base. Slots 0x2c, 0x34, 0x40, 0x44, 0x48, 0x60 and 0x68 point at bodies the linker shares between several slots, so their names come from Instance.h's declaration order, not from the binary.
- STUDIO 0x0054b3b0 and 0x00622570 stay unnamed: each recompiled name is already used at an identical twin (0x004f3770, 0x00413040).
- pair_names lost one row per target: LRUCache<std::string, ScriptInformationProvider::CachedScriptInfo>::remove, PLAYER 0x007e02b0 / STUDIO 0x008a5830. The name stays in both databases.
- CameraSubject's three data dwords (+0x08..+0x14) are imported as an unnamed byte array. IAdornable was left out of the type import because our header still carries the 2016 `float depth`, which 2013 does not have (vtables-3.md). PartInstance itself is not imported because its members are still the Mac layout.
- apply_lib_names.py was not rerun; the library table did not change. Last session's known side effect remains: it and apply_db_updates.py swap the order of their comment lines at the fact-renamed library addresses.
- LuaSettings 2013 layout (ctor 0x0085e360) and IScriptFilter's novtable status, both from core-bodies.md, are not imported: our headers are not fixed yet, so no layout is proven.
- 4,472 unnamed PLAYER App functions (631 KB) have no caller whose class is known, so app_name_coverage.py cannot group them; they need another grouping, such as their .obj neighbours.

# ida-sync-4 (workflow phase5-wave1, wf_322cba92-6a4)

I synced the wave-1 results into ida/PLAYER.i64 and ida/STUDIO.i64. Both databases are saved packed and closed, with no .id0/.nam/.til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G, and none hit the cap.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-10-wave1-sync.i64 and STUDIO-10-wave1-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** The source was build/sweep7/P-final.json and S-final.json, joined only with the scored-i7/{P,S} copies of the exe/pdb/map.
- Functions at effective 100%: PLAYER 4,690, STUDIO 4,523.
- PLAYER: 1,176 unnamed functions renamed `match:`, plus 54 `rtti:` names (53 from vftable slots, 1 from a report).
- STUDIO: 1,153 renamed `match:`. This includes 5 carried over from PLAYER through the body match, and 11 wrong FLIRT names (FUNC_LIB cleared), such as `??1strstreambuf..._N` sitting on signal constructors, FWFinal destructors and ~Teams. Plus 57 `rtti:`.
- Conflicts (`exports/<T>-match-conflicts.csv`): PLAYER 100 (70 folded-original, 27 same-function, 3 body-wins), STUDIO 96.
- New "keep" rule: Stats::Item::formatValue<double/float/int> (0x0073b3f0/420/450, STUDIO 0x007c3a90/ac0/af0) keeps its ios: name. Our 100% name for it, StatsItem::formatValueId/If/Ii, is a name the prior tree invented, and CLAUDE.md says oracle names win over those.
- One wrong verdict fixed: the Value<CoordinateFrame> constructor 0x0078c7c0 is now matched as the same function (it was "body-wins" because its decorated name could not be read).
- From core-close.md, each checked against the 2013 bytes with capstone (PLAYER / STUDIO):
  - getStart 0x009bb130 / 0x00a71f80 (ios:).
  - directCallback 0x009bb180 / 0x00a71fd0 and interpolatedCallback 0x009bb230 / 0x00a72080 (src2016:, Windows-only, no iOS/Mac symbol).
  - FWFinal<FWPartInstance> copy constructor 0x00845c00 / 0x0098b5d0 (rtti:).
  - Instance D0 (MSVC: scalar deleting destructor) 0x007bc3e0 / 0x00884ef0 (ios:). Its adjustor thunks and the client-shell twin 0x0040b010 / 0x00477e20 get comments only.
  - __TBB_LockByte 0x00402240 / 0x00443510 (lib:).
  - IndexedMesh::isUpperRoot at STUDIO 0x00a0e170 (ios:).
- Globals (STUDIO addresses from Time.cpp's GLOBAL markers):
  - ClassRegistrar<Instance>::registrar 0x00e0c9dc / 0x00f83380 and getStart()::start 0x00e1b640 / 0x00f9e828 (ios:).
  - The 10 Time.cpp statics 0x00e1b608–0x00e1b638 / 0x00f9e7f0–0x00f9e820 (src2016:).

**3. Function bounds** (new tools/ida/fix_bounds.py).
- 25 Bridge<T>::on_newindex wrappers per target (14 named, 11 still unnamed) now end at 0x30 instead of 0x28. IDA had cut each one off after its throwing callee (near-miss-2.md).
- STUDIO 0x00a0e170 (isUpperRoot, 0x7a bytes) was a tail piece of Clump::isClumpRootPrimitive 0x009d00f0. It is now its own function, so STUDIO has 44,815 functions.

**4. Types.** Imported from the scored PDB copies.
- New classes, each with a constructor or destructor at 100% in sweep7:
  - Team 0x6c, Teams 0x6c
  - ContentProviderJob 0x288, LegacyContentTable 0x34
  - Average<double> 0x18
  - FWPartInstance 0x54, FWFinal<FWInstance>, FWFinal<FWPartInstance>
  - and their base classes.
- Local types imported per database: 92, was 75. `this` is typed on 81 PLAYER / 82 STUDIO functions, was 57 / 58.
- DataModel_vtbl slots 0x40/0x44/0x48 keep the names declared in Instance.h. The new run pairs those slots with RakNet's identical IsSendThrottled body, which would have put that name on them.

**5. Name coverage, before → after**

| Functions | PLAYER | STUDIO |
|---|---|---|
| Total | 49,091 | 44,814 → 44,815 |
| Named | 10,469 → 11,715 | 11,278 → 12,493 |
| match: | 2,718 → 3,897 | 2,593 → 3,746 |
| rtti: | 35 → 89 | 36 → 93 |

- Globals per database: ios: 3 → 5, src2016: 0 → 10.
- PLAYER App block (21,631 functions) with an ios:/mac:/match: name: 6,026 → 6,869. 14,762 (68.2%) still have none.
- Largest unnamed groups in ida/exports/PLAYER-app-unnamed-by-class.csv:

| Group | Functions |
|---|---|
| No caller with a known class | 4,443 (628 KB) |
| Ogre::RbxSceneManager vftable stubs | 306 |
| DataModel | 244 |
| FactoryProduct<>::Creator | 225 |
| ScriptContext | 218 |
| RBX free functions | 166 |
| EnumDesc<> | 141 |
| GuiService | 109 |
| Ogre::RbxSceneNode | 104 |
| Creatable<> | 102 |

The names, functions and types exports are regenerated, and ida/LOG.md has the session note.

## Files changed

- ida/PLAYER.i64
- ida/STUDIO.i64
- ida/snapshots/PLAYER-10-wave1-sync.i64
- ida/snapshots/STUDIO-10-wave1-sync.i64
- ida/LOG.md
- tools/ida/match_names.py
- tools/ida/apply_db_updates.py
- tools/ida/import_types.py
- tools/ida/fix_bounds.py
- tools/ida/clear_funclib.py
- ida/exports/PLAYER-names.csv
- ida/exports/STUDIO-names.csv
- ida/exports/PLAYER-functions.csv
- ida/exports/STUDIO-functions.csv
- ida/exports/PLAYER-types.h
- ida/exports/STUDIO-types.h
- ida/exports/PLAYER-proven-types.json
- ida/exports/STUDIO-proven-types.json
- ida/exports/PLAYER-db-updates.csv
- ida/exports/STUDIO-db-updates.csv
- ida/exports/PLAYER-match-conflicts.csv
- ida/exports/STUDIO-match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/PLAYER-10-db-updates.tsv
- ida/logs/STUDIO-10-db-updates.tsv
- ida/logs/PLAYER-10-bounds.log
- ida/logs/STUDIO-10-bounds.log
- ida/logs/PLAYER-10-types.log
- ida/logs/STUDIO-10-types.log

## Unresolved

- For Integrate: regenerate the function-size rows from the new ida/exports/<T>-functions.csv. Then the 25 Bridge<T>::on_newindex wrappers per target are scored at 0x30 (near-miss-2 expects 100%), and the 11 still-unnamed wrappers would be named on the next sync. STUDIO 0x00a0e170 (isUpperRoot) is now a function there.
- pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv were not regenerated this session. They predate the new STUDIO function 0x00a0e170, so rerun them before the next apply_names.py.
- No class layout was imported for PhysicsInstructions (ctor 0x0084d2f0 at 83.8%), LuaSettings (0x0085e360 at 58.7%) or TaskScheduler (ctor 97.2%, dtor 99.5%); none is at 100% yet. DataModel+0xbe8 is now the member physicsInstructions, kept as a byte array.
- Unnamed because no oracle symbol exists: now<Fast>::inited 0x00e1b664 and getStart's guard 0x00e1b648. The 10 Time.cpp statics and directCallback/interpolatedCallback are tagged src2016: because they are Windows-only, so iOS and Mac have no symbol for them.
- Instance D0 has a client-shell (LTCG) twin at PLAYER 0x0040b010 / STUDIO 0x00477e20, which has a comment only. The adjustor thunks have comments, not names: MSVC adjustor-thunk names were not verified, so none were made up.
- Left unnamed: STUDIO 0x0054b3b0 and 0x007db230. Each recompiled name is already used at an identical twin function (0x004f3770, 0x006df080).
- lib_consistency.py was rerun. It changed only the note text on 3 rows ("oracle agrees" is gone where the name is now match:), so the previous lib-resolved.csv files were kept. The tss_data_node and vector _Buy library rows that Integrate reported as unpaired still need a gen_reccmp_csv.py fallback; that file is not mine.

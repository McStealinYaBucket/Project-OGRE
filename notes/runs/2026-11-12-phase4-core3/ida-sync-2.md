# ida-sync-2 (workflow phase4-core3, wf_4f741962-18b)

Both databases are synced and saved packed, with no .id0/.id1/.nam/.til files left behind. Before any change I snapshotted them to ida/snapshots/PLAYER-08-core2-sync.i64 and STUDIO-08-core2-sync.i64; cmp showed each identical to its canonical database.

Two places where I did not do what the task said:
- **0x005c30c0 keeps the name RakNet::BitStream::SetWriteOffset.** Its 9-byte body (`mov eax,[esp+4]; mov [ecx],eax; ret 4`) is both SetWriteOffset (defined out of line in BitStream.cpp) and GuiBuilder::Initialize: the linker merged the two identical bodies into one. RakPeer::ShiftIncomingTimestamp and two other RakNet functions call it as SetWriteOffset, and only the DataModel ctor (call at 0x0064f1ce) calls it as Initialize. It also sits in RakNet's code block. So the RakNet name stays and a comment records Initialize as FOLDED. STUDIO 0x008a8950 is handled the same way.
- **0x009bc7d0 and 0x009bc810 now have names, not just comments.** 0x009bc7d0 was already RbxDbgInfo::AddPlace at reccmp 100%. 0x009bc810 / STUDIO 0x00a72c00 is RbxDbgInfo::RemovePlace(long), tagged ios:: it decrements PlaceCounter at 0x00e1b9d4 and searches PlaceIDs at 0x00e1b9c4, and ~DataModel calls it on placeID at 0x0064d8d4.

**Step 2, names (PLAYER / STUDIO).** I checked each one against the 2013 bytes before naming it. STUDIO addresses come from body pairs, or else from the same DataModel ctor call order, the vftable stores, or jobId sitting 4 bytes lower.

| Function | PLAYER | STUDIO | Tag |
|---|---|---|---|
| DataModel::MouseStats ctor | 0x00641bd0 | 0x0075f3c0 | ios: |
| GuiBuilder ctor | 0x00644f30 | 0x00761ef0 | prior: |
| GuiBuilder dtor | 0x0056b810 | 0x005d0cc0 | prior: |
| getIsPersonalServer / setIsPersonalServer | 0x0062d240 / 0x0062d250 | 0x0074d560 / 0x0074d570 | ios: |
| getJobId | 0x0062d1e0 | 0x0074d500 | ios: |
| ~ScriptContext | 0x0068b1a0 | 0x007f0900 | ios: |
| Soundscape::SoundChannel ctor | 0x006f2fb0 | 0x00807fb0 | ios: |
| PhysicsInstructions ctor | 0x0084d2f0 | 0x009a4fe0 | ios: |
| DataModel ctor / dtor (were unnamed in STUDIO) | already named | 0x0076b720 / 0x0076a340 | ios: |

GuiBuilder's ctor takes no stack argument, so it is named as the default ctor. FWValue<std::string>::set 0x00619a70 was already named; it gets a comment only.

**Step 3, bulk promotion** from P3.json / S3.json, joined only with the scored PDB/map copies:

| | PLAYER | STUDIO |
|---|---|---|
| Functions at effective 100% | 2,905 | 2,823 |
| Left to the library-name script | 1,574 | 1,527 |
| Renamed in the database | 77 (67 match:, 8 ios:, 2 prior:) | 79 (67 match:, 10 ios:, 2 prior:) |
| Conflicts with ios:/mac:/FLIRT names | 23 (19 folded-original, 4 same-function) | 22 (19 folded-original, 2 same-function, 1 entry point) |

STUDIO 0x0054b3b0 is still skipped: its name is already at the identical twin 0x004f3770.

**A bug I found and fixed.** Rerunning tools/ida/apply_lib_names.py undid the EventSource::raiseEventInvocation name (rtti:) at 0x0066ee10 / 0x00773760, putting back the merged RakNet CCRakNetSlidingWindow::Update. The script now keeps any name set by a hand-checked fact, and a second apply pass restored it. One side effect remains: those two scripts swap the order of their comment lines at 20 / 25 library addresses on every run. Names are stable.

**Step 4, types.** All 7 new types are imported in both databases; tools/ida/import_types.py dumped them from the scored PDB copies:
- FWBase (0x18) and FWInstance (0x38). FWBase's three FWValue<bool> bytes (0x18–0x1a) are moved to FWInstance, where the 2013 ctor stores them.
- IFWHolder, with an `RBX::IFWHolder_vtbl` type holding its 5 slots.
- ScriptContext (0x288).
- DataModel (0xe10). Our tree's copy is 0xe18 because its GuiBuilder is 0x24 bytes, so I corrected it to the 2013 layout: GuiBuilder 0x20 with parentDataModel at 0xe0c in PLAYER, GuiBuilder 0x1c with parentDataModel at 0xe08 in STUDIO. STUDIO also lacks networkStatsWindowsOn, so its members 0xc64–0xcac sit 4 bytes lower. These corrections are listed under `fixups_2013` in the proven-types JSON.
- MouseStats (0x80) and RunningAverage<double,double> (0x30).

Each database has 67 of 67 types (26 listed, 41 base classes), and `this` is typed on 49 / 50 functions, up from 34. That now includes ~DataModel, ~ScriptContext, the RunningAverage ctor and ~GuidItem<Instance>, which the script previously had to skip.

**Step 5, library table and proposals.**
- 0x009dab20 / 0x006aea20 is now `identical` G3D::Vector3::maxFinite with no conflict (conflicts 13→12 / 7→6). The rule I added in tools/lib/lib_consistency.py: an identical library body over 8 bytes beats an oracle name placed only by call position; RTTI-slot and string evidence still win. It has a self-test.
- 0x007681b0 already had no row in lib-resolved.csv, so nothing claims it is Color4::one.
- Rerunning tools/pair_names.py on the named databases first gave only 2,534 rows, because it skips functions that already have names. It now treats only FLIRT and IDA names as existing names. Before regenerating I re-exported the evidence files, recomputed abi.jsonl on scratch copies, and re-ran the STUDIO–PLAYER body match (20,129 pairs, was 20,115).
- Result: PLAYER 12,352 rows with 6,714 functions strongly named (was 6,712; 6,708 the same, none changed, 4 lost, 6 gained). STUDIO 12,088 rows with 6,855 strong (was 6,849, none changed).
- Applying the new proposals named 6 more PLAYER functions and 7 STUDIO.

**Step 6, exports and coverage.** Names, functions, types and evidence are re-exported, and ida/LOG.md has the session note. I also fixed tools/ida/export_names.py: 26 names per database whose comment line was cut inside a long template name were reported as ida: and are now match:. Local types: PLAYER 475→495, STUDIO 383→403.

| Functions with a real name | PLAYER (49,091) | STUDIO (44,814) |
|---|---|---|
| Named | 10,205 → 10,290 | 11,017 → 11,105 |
| match: | 2,473 → 2,563 | 2,360 → 2,451 |
| ios: | 6,195 → 6,207 | 6,258 → 6,265 |
| prior: | 34 → 36 | 13 → 15 |
| Unnamed | 38,886 → 38,801 | 33,797 → 33,709 |

## Files changed

- ida/PLAYER.i64
- ida/STUDIO.i64
- ida/snapshots/PLAYER-08-core2-sync.i64
- ida/snapshots/STUDIO-08-core2-sync.i64
- ida/LOG.md
- tools/ida/match_names.py
- tools/ida/import_types.py
- tools/ida/apply_lib_names.py
- tools/ida/export_names.py
- tools/lib/lib_consistency.py
- tools/pair_names.py
- ida/exports/PLAYER-db-updates.csv
- ida/exports/STUDIO-db-updates.csv
- ida/exports/PLAYER-match-conflicts.csv
- ida/exports/STUDIO-match-conflicts.csv
- ida/exports/PLAYER-lib-resolved.csv
- ida/exports/STUDIO-lib-resolved.csv
- ida/exports/PLAYER-proven-types.json
- ida/exports/STUDIO-proven-types.json
- ida/exports/PLAYER-names-proposed.csv
- ida/exports/STUDIO-names-proposed.csv
- ida/exports/STUDIO-PLAYER-bodymatch.csv
- ida/exports/PLAYER-names.csv
- ida/exports/STUDIO-names.csv
- ida/exports/PLAYER-functions.csv
- ida/exports/STUDIO-functions.csv
- ida/exports/PLAYER-types.h
- ida/exports/STUDIO-types.h
- ida/exports/PLAYER-evidence/
- ida/exports/STUDIO-evidence/

## Unresolved

- 0x005c30c0 / STUDIO 0x008a8950 keeps the name RakNet::BitStream::SetWriteOffset rather than GuiBuilder::Initialize. The linker merged the two identical bodies; three RakNet callers use it as SetWriteOffset and only the DataModel ctor (0x0064f1ce) uses it as Initialize, and it lies in the RakNet block. Whoever owns GuiBuilder's source should mark Initialize as `// FUNCTION: PLAYER 0x005c30c0 FOLDED`.
- The 3 destructor names that the earlier thunk split moved onto adjustor-thunk targets have no proposal row any more, because the regenerated pairing does not reproduce their old evidence: PLAYER 0x00649590 PhysicsService, 0x00869fd0 Selection, 0x00924460 Rocket; STUDIO 0x00766180, 0x0079de00, 0x0094c830. The names stay but are now tagged ida:.
- apply_db_updates.py and apply_lib_names.py swap the order of their comment lines at 20 (PLAYER) / 25 (STUDIO) library addresses that carry a hand-checked fact rename, on every run. Names are stable, but the pair is not idempotent on comments.
- GuiBuilder's dtor 0x0056b810 is also called from Network::Players::createLocalPlayer at 0x0056dd48. It is either a local GuiBuilder there or an identical destructor the linker merged; I did not check which.
- The DataModel and FWBase/FWInstance layouts imported into IDA are corrected to 2013 by hand in import_types.py (fix_2013). The source tree still has GuiBuilder at 0x24 bytes (DataModel 0xe18, size assert disabled) and the three FWValue<bool> bytes still declared in FWBase. When those headers are fixed, fix_2013 becomes a no-op and can be deleted.
- Debugable::doCrash (0x009b19f0 / 0x00a68ae0) has no `this` typed: IDA sees no argument, and it is probably static.
- STUDIO 0x0054b3b0 is still unnamed: its recompiled _Tree::_Erase name is already at the identical twin 0x004f3770.

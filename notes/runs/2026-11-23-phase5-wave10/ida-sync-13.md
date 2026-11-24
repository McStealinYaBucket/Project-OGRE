# ida-sync-13 (workflow phase5-wave10)

The wave 10 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G; none hit the cap. ida/Archive.zip was not touched, nothing under Client/ was edited, and the shared reccmp/ CSVs were not written.

**1. Snapshots.** Before any change both databases were copied to ida/snapshots/PLAYER-19-wave10-sync.i64 and STUDIO-19-wave10-sync.i64; `cmp` showed each identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep17/P-final.json and build-studio/sweep17/S-final.json, joined only with build/sweep17/scored/ and build-studio/sweep17/scored/.
- Functions at effective 100%: PLAYER 8,649, STUDIO 8,292 (sweep16: 8,230 / 7,874). Library-owned rows (1,739 / 1,646) are left to apply_lib_names.py.
- PLAYER renamed 380: 366 `match:` (359 dummies, 6 IDA auto names, 1 body-wins) and 14 `rtti:` (9 vftable slots, 5 facts). STUDIO renamed 387: 370 `match:` (2 of them via PLAYER body match) and 17 `rtti:` (9 slots, 8 facts).
- **Conflicts:** PLAYER 119 (99 folded-original, 19 same-function, 1 body-wins), STUDIO 120 (100 / 18 / 1 / 1 keep-entry). The one new verdict, in both targets, is the body-wins at PLAYER 0x0044d750 / STUDIO 0x00410a40. It had the iOS name `error_info_injector<rbx::bad_placement_any_cast>::~error_info_injector()` (D2, callgraph evidence only). The body stores the clone_impl<...bad_placement_any_cast> vftables first, so reccmp's 100% `~clone_impl` replaces that name. The STUDIO skip at 0x0054b3b0 stays.
- **Facts added in tools/ida/match_names.py** (the integrate report's IDA item, and its "check STUDIO" request). I checked each one from the exe bytes: the throw info's catchable types, and the RTTI COL of every vftable the function stores.

| PLAYER | STUDIO | Now named | Evidence |
|---|---|---|---|
| 0x0044d400 | 0x00411090 | `throw_exception<rbx::bad_placement_any_cast>` | _TI 0x00cd6854 / 0x00e295f0 |
| 0x0044d950 | 0x004109f0 | `enable_error_info<bad_placement_any_cast>` | stores 0x00b11c70 / 0x00b11c68 |
| 0x0044d9a0 | 0x004109c0 | `enable_current_exception<error_info_injector<bad_placement_any_cast>>` | called by 0x0044d400, calls 0x0044da20 |
| 0x0044da20 | 0x004108f0 | `clone_impl<...bad_placement_any_cast>(const error_info_injector&)` | stores 0x00b11c94 / 8c / 7c |
| 0x0044db50 | 0x00410f30 | `error_info_injector<bad_placement_any_cast>` ??_E adjustor{12} | slot 0 of 0x00b11c68 (STUDIO 0x00bc97e0) |
| — | 0x0041fdb0 / 0x0041f8d0 / 0x0041f8a0 | the same three of **std::bad_alloc** (STUDIO only) | _TI 0x00e296c4, vftables 0x00bbd6cc / c4 |

  - All eight PLAYER/STUDIO addresses had carried boost-iostreams `std::exception` names from lib-resolved "identical"/"located" matches. Those bodies differ only in vftable and throw-info addresses.
  - A FACTS row can now take `None` as its PLAYER address, for STUDIO-only facts.
  - The selftest passes.
- **lib-resolved fix (tools/lib/lib_consistency.py, rerun).** A new rule marks a library row `conflict` when all of these hold: the body is over 8 bytes, the current name is an `rtti:` fact, no row at that address carries the name, and no row agrees with it exactly.
  - PLAYER 0x0044d400/950/9a0/a20 and STUDIO 0x0041fdb0/8d0/8a0 move to conflict, so the next gen_reccmp_csv.py run drops their std::exception library rows. That should recover vftable 0x00b11c68 and the six `any_cast<T&>` at 97.5% (script-5e-6.md item 9). I have not measured it; the shared CSVs are the integrate step's.
  - The 8-byte floor keeps ICF folds such as 0x0066ee10 (`ret 0Ch`, raiseEventInvocation folded with RakNet Update).
  - No other status changed. The 0x009d9550, 0x006ad450 and 0x006ae820 notes lose their "oracle agrees" text, because those names are now `match:`.

**3. Types.** New classes, each with a ctor or dtor at 100% in sweep17 in both targets:

| Class | Size (PDB) | Proof (PLAYER / STUDIO) |
|---|---|---|
| Reflection::Type | 0x1c (base Descriptor) | ctor 0x006f97e0 / 0x006db8a0 |
| Reflection::EnumDescriptor | 0x34 (base Type) | ctor 0x006fa1a0 / 0x006dcae0, dtor 0x006f9e40 / 0x006dc260 |
| Reflection::SignatureDescriptor::Item | 0x4c | ctors 0x0079f820 / 0x0073e060 and 0x0079ffa0 / 0x0073e7c0 |

- **No tree size assertion.** The tree has no DECOMP_SIZE_ASSERT for these three, so each size is the PDB's. The 100% ctors initialise every member, since the members are const. This is a lower bar than the Kernel precedent in wave 7–9; the CLASSES comment in import_types.py says so.
- **Counts.** Local types: PLAYER 290 (103 listed), STUDIO 282 (101); before, 287 / 279. `this` is typed on 427 / 416 functions (270 / 257 ctor/dtor); before, 393 / 384. The new ones are the 23 `Type<T>` ctors and this sync's renames.
- **No other type changed.** No size mismatch, and no type left the set. The STUDIO DataModel dump differs only in one vftable slot comment (`canClientCreate` -> `isReadOnly`).
- **Not imported:**
  - ReflectionMetadata's shell classes (shell-5k): their vftables are at 100%, but they have no ctor/dtor row, because the ctors are inlined into `Creator::create` (0.899).
  - Network::Player and Players: SIZE is asserted, but their ctor and dtor are unpaired (network-5f.md).
  - Network::Client and Peer were already imported.
  - Still waiting from earlier waves: TaskScheduler, JointInstance, BallBall/BallBlockContact, AssemblyHistory, GlobalAdvancedSettings, Workspace.

**4. Unnamed functions.** ida/exports/PLAYER-app-unnamed-by-class.csv was regenerated.
- PLAYER App block (0x00606a00–0x009ad3a0, 21,631 functions): ios:/mac:/match: 8,345 -> 8,444; 13,187 (61.0%) still without one.
- Network block (0x0051f6b0–0x00606a00, 5,336): 1,903 -> 1,981; 3,355 (62.9%) without.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,062 (596 KB), was 4,076 |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191, was 192 |
| EnumDesc<> | 180 |
| DataModel | 164, was 166 |
| RBX free functions | 160 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| Scripting::ScriptDebugger | 88 |
| Creatable<> | 86 |
| PartInstance | 82 |
| RemoteFunction | 82 |
| SceneUpdater | 81 |

In the Network block, Network::Replicator is 375 (was 379). Network::Player has 48 unnamed functions in the App block.

**5. Name coverage, before -> after.** The names, functions and types exports were regenerated: PLAYER 720 type declarations, STUDIO 620 (before, 717 / 617). Function starts are unchanged (49,091 / 44,815), so pair_names.py and body_match.py were not rerun. ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 14,099 -> 14,468 | 14,754 -> 15,136 |
| match: | 6,260 -> 6,620 | 6,018 -> 6,383 |
| rtti: | 176 -> 190 | 171 -> 188 |
| ios: | 6,210 -> 6,209 (0x0044d750) | 6,270 -> 6,268 |
| ida: | 1,066 -> 1,063 | 1,811 -> 1,813 |
| lib: | 74 -> 73 (0x0044da20) | unchanged |

Globals are unchanged in both.

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-19-wave10-sync.i64, ida/snapshots/STUDIO-19-wave10-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (8 facts; STUDIO-only fact rows)
- tools/ida/import_types.py (Type, EnumDescriptor, SignatureDescriptor::Item in CLASSES)
- tools/lib/lib_consistency.py (rtti-fact conflict rule)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv, -lib-resolved.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-19-db-updates.log/.tsv, -19-types.log, -19-export-names.log

## Unresolved

- **Integrate step:** regenerate the reccmp CSVs from the new lib-resolved.csv, which drops the std::exception library rows at PLAYER 0x0044d400/950/9a0/a20 and STUDIO 0x0041fdb0/8d0/8a0. Then measure vftable 0x00b11c68 and the `any_cast<T&>` at 97.5%.
- **Stale pairing data:** pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170.
- **STUDIO 0x004337a0** (ContentId(const string&), shell /GS copy) is still unnamed.
- **program_options facet shell copies** 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry the filesystem names.
- **0x009b7020** (Thread.obj push_back chain) is still a dummy.
- **RBX::operator<<(ostream&, const Name&)** PLAYER 0x006131f0: a CSV row claims it, but App's Util/Name.h/.cpp does not define it (integrate.md).
- **Type candidates that need a tree change first:**
  - ReflectionMetadata classes: a ctor/dtor at 100%.
  - Network::Player/Players: a paired ctor/dtor.
  - TaskScheduler, JointInstance: a size assertion.
  - BallBall/BallBlockContact: their ctors (0.86 / 0.83).
  - AssemblyHistory, GlobalAdvancedSettings, Workspace.
  - A DECOMP_SIZE_ASSERT for Type (0x1c), EnumDescriptor (0x34) and SignatureDescriptor::Item (0x4c) would raise those three to the usual bar.

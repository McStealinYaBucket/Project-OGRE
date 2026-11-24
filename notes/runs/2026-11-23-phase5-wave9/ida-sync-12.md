# ida-sync-12 (workflow phase5-wave9)

The wave 9 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G; none hit the cap. ida/Archive.zip was not touched and nothing under Client/ was edited.

**1. Snapshots.** Before any change both databases were copied to ida/snapshots/PLAYER-18-wave9-sync.i64 and STUDIO-18-wave9-sync.i64; `cmp` showed each identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep16/P-final.json and build-studio/sweep16/S-final.json, joined only with build/sweep16/scored/ and build-studio/sweep16/scored/.
- Functions at effective 100%: PLAYER 8,230, STUDIO 7,874 (sweep15: 8,052 / 7,699). Library-owned rows (1,731 / 1,645) are left to apply_lib_names.py.
- PLAYER renamed 141: 136 dummies, 5 stale `match:`. STUDIO renamed 130: 121 dummies, 4 via PLAYER body match, 5 stale `match:`.
- The stale names are identical-body twins where sweep16 pairs a different template instance (reccmp's pick, the same in both targets):
  - `~shared_ptr<spin_mutex>` -> `~exception_ptr` (0x00405b50 / 0x0041feb0; the shell copy integrate.md names)
  - `_Destroy_range<shared_ptr<GuiBase>>` -> `<shared_ptr<Instance>>` (0x00427f30 / 0x004ebe20)
  - `_Allocate<ChatOption*>` -> `<const EnumDescriptor::Item*>` (0x00429c80 / 0x004791e0)
  - `_Eqrange` of `map<const Name*, const ICreator*>` -> `map<const Name*, Verb*>` (0x004b3e60 / 0x005437d0; swapped, the ICreator name moved to 0x00411f50 / 0x0040f3b0)
  - `shared_ptr<Instance>::operator=` -> `operator=<Lighting>` (0x006eda50 / 0x00751110)
- **Conflicts:** PLAYER 119 (99 folded-original, 20 same-function), STUDIO 120 (101 / 18 / 1 keep-entry). The only new one is `Ogre::RbxImage::getOriginalWidth` 0x00501530 / 0x009131f0 against our `EnumDescriptor::getEnumCountMSB`: folded-original, oracle name kept. The STUDIO skip at 0x0054b3b0 stays.
- **Fact added in tools/ida/match_names.py:** `unname` (match:) PLAYER 0x009b7020. It had carried `vector<shared_ptr<Instance>>::push_back` since an earlier sweep. sweep16 pairs that name with 0x00564bc0 at 100%. 0x009b7020 is the Thread.obj push_back chain beside 0x009b6af0 (shell-base.md), and its element type is unproven. Pass 1 skipped 0x00564bc0 ("name in use"), pass 2 unnamed 0x009b7020, and pass 3 set the name. Selftest passes.
- No wave9 report listed a name for the IDA sync. world-5d.md's Assembly +0x6c/+0x70 member names are a tree item, not a database name.

**3. Types.** New classes. Each has a ctor or dtor at effective 100% in sweep16 and a PDB size equal to the tree's DECOMP_SIZE_ASSERT:

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| Kernel | 0x30 | ctor 0x00831590 / 0x009a3bf0, dtor 0x0082f670 / 0x009a1c20 |
| ContactManagerSpatialHash | 0xdc | ctor 0x008ef090 / 0x00a0ce10; base `SpatialHash<Primitive,Contact,ContactManager,4>` 0xdc, ctor 0x008eef80 / 0x00a0cd00 at 100% |
| BlockBlockContact | 0x30 | ctor 0x00853750 / 0x009ab400; bases Contact (0x2c) and `Allocator<BlockBlockContact>` |
| Stats::Item (STUDIO) | 0x88 | ctor 0x0043e560 is now 100% (shell-base.md), so it is no longer PLAYER only |

- Joint was re-dumped after this wave's Joint.h edits and is still 0x94. ScriptContext is unchanged (dtor 100%, ctor 0.976 in both targets).
- Local types: PLAYER 287 (100 listed), STUDIO 279 (98); before, 281 / 269. `this` is typed on 393 / 384 functions (237 / 226 ctor/dtor); before, 375 / 363. No size mismatch, and no type left the set.
- Not imported: BallBall/BallBlockContact (ctor 0.857 / 0.825), BallPoly/PolyPolyContact (0.28 / 0.72), JointInstance (dtor 0.22, no size assertion), Reflection::Type (ctors 0.80–0.97, no size assertion), TaskScheduler (dtor 100%, no size assertion), and, as before, AssemblyHistory, GlobalAdvancedSettings, STUDIO ScreenGui/RunService and Workspace. No wave9 shell class qualifies: the shell stand-in in placeholder.cpp is not original code.

**4. Bounds.** During this session, IDA's auto-analysis created a 1-byte STUDIO function at 0x005fed30. It is `int3` padding right after the noreturn call that ends sub_5FEC10. tools/ida/fix_bounds.py has a new step 3 that deletes functions made only of int3 bytes. It removed this one; PLAYER had none. STUDIO is back to 44,815 functions, and no other function start changed.

**5. Unnamed functions.** ida/exports/PLAYER-app-unnamed-by-class.csv regenerated.
- PLAYER App block (0x00606a00–0x009ad3a0, 21,631 functions): ios:/mac:/match: 8,262 -> 8,345; 13,286 (61.4%) still without one.
- Network block (0x0051f6b0–0x00606a00, 5,336): 1,890 -> 1,903; 3,433 (64.3%) without.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,076 (597 KB), unchanged |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 192, was 190 (propagation moved two in) |
| EnumDesc<> | 180 |
| DataModel | 166, was 172 |
| RBX free functions | 159 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 93 |
| Scripting::ScriptDebugger | 88 |
| Creatable<> | 86 |
| EventDescImpl<> | 83 |
| PartInstance | 82 |
| RemoteFunction | 82 |

**6. Name coverage, before -> after.** The names, functions and types exports were regenerated (PLAYER 717 type declarations, STUDIO 617; before, 710 / 606). ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 13,959 -> 14,099 | 14,629 -> 14,754 |
| match: | 6,125 -> 6,260 | 5,893 -> 6,018 |
| ida: | 1,061 -> 1,066 (IDA `j_` names on five `singleton_pool<XContact>::get_pool` thunks, 0x00a5e4e0..0x00a7bb40, whose targets were renamed) | 1,811, unchanged |
| others (ios:, mac:, flirt:, lib:, prior:, rtti:) | unchanged | unchanged |

Globals are unchanged in both. pair_names.py, body_match.py and lib_consistency.py were not rerun, because no function start changed (the deleted STUDIO stub was this session's own).

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-18-wave9-sync.i64, ida/snapshots/STUDIO-18-wave9-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (the 0x009b7020 unname fact)
- tools/ida/import_types.py (Kernel, ContactManagerSpatialHash, BlockBlockContact in CLASSES; Stats::Item out of PLAYER_ONLY)
- tools/ida/fix_bounds.py (step 3: delete int3-only functions)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-18-db-updates.log/.tsv, PLAYER-18-db-updates-pass2/-pass3.log/.tsv, -18-types.log, -18-bounds.log, -18-export-names.log

## Unresolved

- **Stale pairing data:** pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170.
- **STUDIO 0x004337a0**, ContentId(const string&) (shell /GS copy), still unnamed.
- **program_options facet slots:** the shell copies of `do_in` / `do_out` / `do_length` (0x00477aa0 / 0x00477c30 / 0x00477dd0) still carry the filesystem `utf8_codecvt_facet` names.
- **0x009b7020** (Thread.obj push_back chain) is a dummy again. It gets a name once its row exists (shell-base.md: needs twin rows like 0x009b6af0).
- **`RBX::callback`** PLAYER 0x0086f850 / STUDIO 0x009bc9e0 is unpaired in sweep16 (integrate.md). Its ios: name is unchanged in the database.
- **Type candidates that need a tree change first:** TaskScheduler and JointInstance (add a size assertion; JointInstance's dtor is at 0.22), BallBall/BallBlockContact (ctor 0.86 / 0.83), Reflection::Type (no size assertion, ctors below 100%), AssemblyHistory, GlobalAdvancedSettings, Workspace.

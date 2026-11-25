# ida-sync-16 (workflow phase5-wave13)

The wave 13 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/ or the shared reccmp/ CSVs, and committed nothing. The 2016-copy check is silent.

## 1. Snapshots

ida/snapshots/PLAYER-22-wave13-sync.i64 and STUDIO-22-wave13-sync.i64 were taken before any change and are `cmp`-identical to the canonical databases at that point.

## 2. Names from the scored runs

The inputs were build/sweep20/P-final.json and build-studio/sweep20/S-final.json, joined only with build*/sweep20/scored/.

- **Functions at effective 100%:** PLAYER 9,139, STUDIO 8,708 (sweep19: 8,963 / 8,548). The library-owned rows (1,747 / 1,654) are left to apply_lib_names.py.
- **PLAYER renamed 152:**
  - 141 `match:`: 139 dummies plus the two `resume_adapter<...>` proc names (see the fix below).
  - 8 `ios:` facts.
  - 3 `rtti:` vftable slots.
- **STUDIO renamed 142:**
  - 132 `match:`, which include:
    - 13 taken from PLAYER through body match.
    - Two IDA `j_` thunks, now `Singleton<EnumDesc<GameBasicSettings::ControlMode / RenderQualitySetting>>::initSingleton`.
    - Three FLIRT false positives, `??1strstreambuf@std@@UAE@XZ_0/_1/_2` at 0x005bc4e0 / 0x005bc5a0 / 0x005bc660. These are the 100% `rbx::signals::signal<...>` constructors of the Players signals, so the body wins.
  - 8 `ios:` facts.
  - 2 `rtti:` vftable slots.
- **New match: names**, all wave 13 code:
  - `MemberDescriptorContainer<CallbackDescriptor>::checkLookup` 0x00614db0 and `checkLookups` 0x00615b20.
  - `TaskScheduler::printJobs` 0x009b6730 / 0x00a6ec30.
  - The GameBasicSettings constructor, its EnumDescs and singletons, and the GlobalBasicSettings getters.
  - The Players and IdSerializer templates.
  - The CrashReporter seh functions.
- **Conflicts:**
  - PLAYER 118 (98 folded-original, 20 same-function), the same address set as wave 12.
  - STUDIO 122 on the first plan (the 3 body-wins above), 119 after the post-apply rerun.
  - The STUDIO 0x0054b3b0 skip is unchanged from earlier waves.

**Facts** (tools/ida/match_names.py), from app-13.md "IDA owner (names)":
- These accessors are not declared in our tree yet, so our build has no decorated name to copy. I used the iOS 1029 Itanium symbols, tagged `ios:`.
- Each one was checked from the exe bytes. The STUDIO bodies are byte-identical, apart from the raise target (PLAYER 0x0061c750, STUDIO 0x006f1260) and the descriptor addresses.

| PLAYER | STUDIO | Name | Bytes |
|---|---|---|---|
| 0x006f61f0 | 0x00792fd0 | GameBasicSettings::getFullScreen() const | `mov al,[ecx+7Ch]; ret` |
| 0x006f6200 | 0x00792fe0 | GameBasicSettings::setControlMode(ControlMode) | +0x6c, desc 0x00df2628 |
| 0x006f6220 | 0x00793000 | GameBasicSettings::setRenderQuality(RenderQualitySetting) | +0x70, desc 0x00df26fc |
| 0x006f6240 | 0x00793020 | GameBasicSettings::setMouseLock(bool) | `mov [ecx+74h],al; ret 4` (no signal) |
| 0x006f6250 | 0x00793030 | GameBasicSettings::setUploadVideoSetting(UploadSetting) | +0x78, desc 0x00df27e8 |
| 0x006f6270 | 0x00793050 | GameBasicSettings::setAllTutorialsDisabled(bool) | byte +0x8c, desc 0x00df26a0 |
| 0x006f4810 | 0x00791df0 | GameSettings::setPostImageSetting(UploadSetting) | +0x8c, desc 0x00df1fec |
| 0x006f4840 | 0x00791e20 | GameSettings::setVideoQualitySetting(VideoQuality) | +0x90, desc 0x00df207c |

**Not named:**
- **The Game constructor helper 0x00653790.** It builds Game+0x30 through call_once 0x0040c2c0, but no oracle names it.
- **STUDIO's two checkLookup copies, 0x006db970 and 0x00703620.** 0x00703620 pairs at 98.1%, but which T each copy belongs to is unproven.

**match_names.py fix.** IDA stores a name with its bad characters replaced by `_` (`IDA_BAD`). The planner now counts an IDA name equal to one of our names in that form as agreement. Before this, the two `resume_adapter<...>` proc names came back as renames on every sync. In the post-apply rerun the only bulk rename left is STUDIO 0x0054b3b0. The selftest passes.

## 3. Types

import_types.py, dumped from the scored PDBs. Each size below equals the tree's DECOMP_SIZE_ASSERT.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| RBX::GameBasicSettings | 0x90 | ctor 0x006f8e70 / 0x00794e10 |
| GlobalBasicSettingsItem<GameBasicSettings, sGameBasicSettings> (base) | 0x64 | ctor 0x006f8550 / 0x00794580 |
| RBX::Network::Players | 0x1f8 | ctor 0x00570980 / 0x005d6a80, dtor 0x005705c0 / 0x005d66c0 |
| RBX::Network::IdSerializer | 0x1094 | ctor 0x005aafd0 / 0x0061b2d0, dtor 0x005a53a0 (PLAYER) |
| RBX::Network::Player (now in STUDIO too) | 0x1d8 | STUDIO dtor 0x00600220 |

- **Bases imported with them:** GlobalBasicSettings::Item and the Described / FactoryProduct / DescribedNonCreatable chains.
- **Dump fix.** The PDB records the GlobalBasicSettingsItem instantiation under its bare name, `RBX::GlobalBasicSettingsItem`. `cname()` now spells such a template from its unique name through llvm-undname, so two instantiations cannot collide on one local type name.
- **Changed members:**
  - GameSettings +0x8c / +0x90 are now postImageSetting / videoQuality, from app-13's header fix.
  - VideoControl +0x0c is now `path`.
- **Counts:**
  - Local types: PLAYER 317 → 328 (120 listed), STUDIO 301 → 317 (115 listed).
  - `this` typed: PLAYER 496 → 512 functions (309 ctor/dtor), STUDIO 450 → 478 (288 ctor/dtor).
  - No size mismatch and no type removed.
- **Not imported:**
  - Game (0x34): its ctor 0x00655e70 is unpaired, because no App code includes Game.h.
  - GlobalBasicSettings (0xb4): ctor 0x00658640 is unpaired.
  - Network::Server: ctor 0.51, dtor 0.91.
  - MainLogManager's ctor (0.886 / 0.61). Its layout is already proven by the dtor.
  - VistaAPIs: no size annotation.

## 4. Unnamed functions

I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.

| Block | Functions | Named (ios:/mac:/match:), before → after | Still without one |
|---|---|---|---|
| PLAYER App (0x00606a00–0x009ad3a0) | 21,631 | 8,488 → 8,541 | 13,090 (60.5%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,037 → 2,099 | 3,237 |

Largest App-block groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,046 (595 KB) |
| Ogre::RbxSceneManager (vftable slots) | 306 |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 |
| EnumDesc<> | 180 |
| RBX free functions | 166 |
| DataModel | 163 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| ScriptDebugger | 86 |
| Creatable<> | 86 |
| PartInstance | 82 |
| RemoteFunction | 82 |
| SceneUpdater | 81 |
| Workspace | 80 |

Network-block groups: Replicator 363 (was 375), Players 154 (was 160), ServerReplicator 118, Player 113, Server 56.

## 5. Coverage, before → after

I regenerated the names, functions and types exports. Type declarations went from 748 to 759 in PLAYER and from 640 to 656 in STUDIO. Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py or body_match.py. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 14,746 → 14,903 | 15,362 → 15,497 |
| match: | 6,876 → 7,015 | 6,599 → 6,729 |
| ios: | 6,213 → 6,221 | 6,272 → 6,280 |
| rtti: | 185 → 188 | 181 → 183 |
| flirt: | 97 → 97 | 180 → 177 |
| ida: | 1,067 → 1,074 (IDA's `j_` names on thunks to newly named targets) | 1,809 → 1,807 |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/{PLAYER,STUDIO}-22-wave13-sync.i64
- ida/LOG.md
- tools/ida/match_names.py: 8 facts, and the IDA_BAD agree rule
- tools/ida/import_types.py: 3 classes; Network::Player removed from PLAYER_ONLY; the bare-template name fix
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-22-db-updates.log/.tsv, -22-types.log, -22-export-names.log

## Unresolved
- **App owner:**
  - Declare the GameBasicSettings / GameSettings accessors named above, so they get MSVC decorations and can be paired. The original keeps them out of line.
  - Game (0x34) and GlobalBasicSettings (0xb4) need their ctors (0x00655e70 / 0x00658640) at 100% before their layouts can be imported.
- **Not named:**
  - The Game ctor helper 0x00653790: no oracle.
  - STUDIO checkLookup copies 0x006db970 / 0x00703620: T unproven.
  - MemberDescriptorContainer<T>::compare 0x00614cd0.
  - GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>::singleton 0x00407230 and its createInstance 0x00409020.
  - The 10 unknown link-stub callees (CSHA1 ×6, TCPInterface ×2, PacketizedTCP ×2) have no 2013 body.
- **Prime_list constructors** (LibraryService, LegacyContentTable, DataModel, StarterGuiService, ContactManager) are at 100% in sweep20 only through the no-.reloc layout coincidence. Their names were already match: or ios:, so nothing in IDA depends on that.
- **Carried:**
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv predate STUDIO isUpperRoot 0x00a0e170.
  - STUDIO 0x004337a0 is still unnamed.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - 0x009b7020 is still a dummy.

# ida-sync-17 (workflow phase5-wave14)

The wave 14 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/ or the shared reccmp/ CSVs, and committed nothing. The 2016-copy check is silent.

## 1. Snapshots

ida/snapshots/PLAYER-23-wave14-sync.i64 and STUDIO-23-wave14-sync.i64 were taken before any change and are `cmp`-identical to the canonical databases at that point.

## 2. Names from the scored runs

The inputs were build/sweep21/P-final.json and build-studio/sweep21/S-final.json, joined only with build*/sweep21/scored/.

- **Functions at effective 100%:** PLAYER 9,298, STUDIO 8,842 (sweep20: 9,139 / 8,708). The library-owned rows (1,747 / 1,654) are left to apply_lib_names.py.
- **PLAYER renamed 130:**
  - 121 `match:`: 115 dummies, 3 IDA `j_` thunks (two `callDoDeclare<sSoundChannel / sGameBasicSettings>`, one `Singleton<EnumDesc<GameSettings::UploadSetting>>::initSingleton`) and 3 match: facts.
  - 4 `rtti:`: 3 vftable slots plus renderGameCursor.
  - 1 `ios:`, 3 `src2016:`, 1 `str:` (facts below).
- **STUDIO renamed 111:**
  - 105 `match:`, including 10 taken from PLAYER through body match.
  - Two FLIRT false positives, `??1strstreambuf@std@@UAE@XZ_6/_7` at 0x006102f0 / 0x0061cf70. These are the 100% `rbx::signals::signal<...FilterResult...>` ctor and `~GuidRegistryService`, so the body wins and FUNC_LIB is cleared.
  - 2 `rtti:`, 1 `ios:`, 2 `src2016:`, 1 `str:`.
- **New match: names**, all wave 14 code: the GlobalBasicSettings ctor and `basicSettingsFile`, `Settings(const std::string&)`, the UserInputBase ctor, `ObjectBridge::getInstance`, `DataModel::TakeScreenshotTask`, and the View/shell and Server functions.
- **Conflicts:** PLAYER 119 after the post-apply rerun (99 folded-original, 20 same-function), STUDIO 120. The new ones are the SoundJob `error`/`sleepTime` oracle names, kept because those bodies are folded with PacketReceiveJob's. The STUDIO 0x0054b3b0 skip is unchanged.
- The post-apply rerun leaves no pending rename except STUDIO 0x0054b3b0. The selftest passes.

**Facts** (tools/ida/match_names.py), each checked from the exe bytes:

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x006f61f0 | 0x00792fd0 | `?inFullScreen@GameBasicSettings@RBX@@QAE_NXZ` | match: | Replaces the wave-13 ios: `getFullScreen() const` fact (app-14). The InFullScreen BoundFuncDesc stores it; getFullScreen and setFullScreen are inline (shell copies 0x004659c0 / 0x004659d0). reccmp 100% in both targets. Without the fact the resolver would keep the oracle name, because the body is a folded 4-byte one. |
| 0x00656320 | 0x0073ee60 | `globalAdvancedSettingsFile` | src2016: | A static: getUserDirectory 0x006ea4c0, then appends '\GlobalSettings_13.xml' 0x00b31094. |
| 0x0072aaf0 | 0x0082a3e0 | `?renderGameCursor@UserInputBase@RBX@@UAEXPAVAdorn@2@@Z` | rtti: | Slot 7 of UserInput's vftable 0x00be4d9c; iOS 1029 name. |
| 0x006afb40 | 0x0080e860 | `?playerRemoving@FriendService@RBX@@QAEXH@Z` | ios: | The only caller is Players::onChildRemoving 0x0056bd8e (STUDIO 0x005d123e), after find<FriendService> 0x00548490, with player->userId [+0xc8]. |
| 0x008445f0 | 0x0098a120 | `list1<value<weak_ptr<Instance>>>` ctor | match: | shell-5k-5 item 3. The only caller is the bind below. |
| 0x008447e0 | 0x0098a310 | `bind<void, weak_ptr<Instance>>` | match: | The only caller is DebrisService::addItem 0x00845087 (STUDIO 0x0098aa57). Our build folds the Instance and DataModel instantiations into one body; the Instance spelling is DebrisService.cpp.obj's name at the paired recomp address. |
| 0x0045f1b0 | n/a | `?PostUserInputMessage@UserInput@RBX@@QAEXIIJ@Z` | src2016: | Link-stub callee. The only caller is View::HandleWindowsMessage at 0x004665e1, which passes View+0xb0 as `this` on the stack. |
| 0x009c5e10 | 0x006c21e0 | `?getResolutionPreset@CRenderSettings@RBX@@QAEABURESOLUTIONENTRY@12@W4ResolutionPreset@12@@Z` | str: | Link-stub callee. Assert 'preset < ResolutionMaxIndex' (Rendering\GfxBase\RenderSettings.cpp:66). The only caller is View::initializeSizes 0x004679d1. |
| 0x00464f30 | 0x0048caa0 | `?startRecording@VideoControl@RBX@@QAEXPAVSoundService@Soundscape@2@@Z` | src2016: | Link-stub callee. The only caller is RecordToggleVerb::startAction 0x00439402. The STUDIO copy sits at the same place after ~VideoControl, with 199 of 199 mnemonics equal. |

- **Kept:** the weak_ptr<DataModel> names on 0x00844de0 / 0x00844f60 / 0x0073da60 / 0x0073db90. Our build has no Instance spelling at their recomp addresses, and 0x0073db90 has five callers, so it is an ICF-folded body.
- **Not named:** `g_unk0x00de8bd9` (STUDIO 0x00f89c61). IDA's `byte_DE8BD9` already says the same thing.
- **Link-stub callees:** every unresolved callee that has a 2013 body now has an exact names.csv row. Only CSHA1 ×6 and TCPInterface/PacketizedTCP ×4 remain, and they have no 2013 body.

## 3. Types

import_types.py, dumped from the scored PDBs. Each size below equals the tree's // SIZE or DECOMP_SIZE_ASSERT.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| RBX::GlobalBasicSettings (+ Settings and the DescribedNonCreatable chains) | 0xb4 | ctor 0x00658640 / 0x00740dd0 |
| RBX::Network::Server (+ its DescribedCreatable / FactoryProduct chain) | 0xb78 | ctor 0x005b4fd0 / 0x00615420, dtor 0x005b32b0 / 0x006132b0 |
| RBX::UserInputBase | 0x38 | ctor 0x0072af80 / 0x0082a870 |
| XmlNameValuePair (was a base of XmlAttribute only) | 0x10 | ctors 0x00558f00 / 0x00627be0, STUDIO 0x005b79c0 / 0x00748d10 |

- **Changed members** (from the tree):
  - CRenderSettings +0x32 is now fullscreenSize and +0x36 windowSize (they were windowSize and m_unk0x36).
  - Players +0x180 is now `shared_ptr<GuidItem<Instance>::Registry>`.
- **Counts:**
  - Local types: PLAYER 328 → 341 (124 listed), STUDIO 317 → 330 (119 listed).
  - `this` typed: PLAYER 512 → 522 functions (316 ctor/dtor), STUDIO 478 → 488 (295 ctor/dtor).
  - No size mismatch and no type removed.
- **Not imported:**
  - Game (0x34): ctor 0x00655e70 is unpaired.
  - Replicator: its ctor and dtor are unpaired.
  - ScreenshotVerb: ctor is at 0.759.
  - VistaAPIs: no size annotation.

## 4. Unnamed functions

I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.

| Block | Functions | Named (ios:/mac:/match:), before → after | Still without one |
|---|---|---|---|
| PLAYER App (0x00606a00–0x009ad3a0) | 21,631 | 8,541 → 8,578 | 13,053 (60.3%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,099 → 2,152 | 3,184 (59.7%) |

Largest App-block groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,033 (593 KB) |
| Ogre::RbxSceneManager (vftable slots) | 306 |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 |
| EnumDesc<> | 179 |
| DataModel | 162 |
| RBX free functions | 158 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| ScriptDebugger | 86 |
| Creatable<> | 86 |
| PartInstance | 82 |
| RemoteFunction | 82 |
| SceneUpdater | 81 |
| Workspace | 80 |

Network-block groups: Replicator 363, Players 154, ServerReplicator 118, Player 113, Replicator::StreamJob 51. Network::Server dropped out of the top groups; it had 56.

## 5. Coverage, before → after

I regenerated the names, functions and types exports. Type declarations went from 759 to 772 in PLAYER and from 656 to 669 in STUDIO. Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py or body_match.py. lib-resolved was not rerun either. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 14,903 → 15,029 | 15,497 → 15,604 |
| match: | 7,015 → 7,137 | 6,729 → 6,834 |
| rtti: | 188 → 191 | 183 → 185 |
| src2016: | 16 → 19 | 16 → 18 |
| str: | 7 → 8 | 6 → 7 |
| flirt: | 97 → 97 | 177 → 175 |
| ida: | 1,074 → 1,071 | 1,807 → 1,806 |
| ios: | 6,221 (unchanged) | 6,280 (unchanged) |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/{PLAYER,STUDIO}-23-wave14-sync.i64
- ida/LOG.md
- tools/ida/match_names.py: the inFullScreen fact replaces getFullScreen, plus 8 new facts
- tools/ida/import_types.py: 4 classes added
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-23-db-updates.log/.tsv, -23b-db-updates.log/.tsv, -23-types.log, -23-export-names.log

## Unresolved
- **Generator / integrate:**
  - The six DebrisService bind rows: 0x008445f0 / 0x008447e0 now carry the weak_ptr<Instance> names (STUDIO 0x0098a120 / 0x0098a310). Our build folds both spellings into one body, so pairing should not change, but measure on the next regeneration.
  - Next round, gen_link_stubs should bind PostUserInputMessage 0x0045f1b0, getResolutionPreset 0x009c5e10 and startRecording 0x00464f30 / 0x0048caa0.
- **App owner:** 0x00656390 carries our tree's static name `basicSettingsFile`, but 2016 names it `globalBasicSettingsFile`, and its twin 0x00656320 now has 2016's `globalAdvancedSettingsFile`. Consider the 2016 spelling.
- **Rendering contributor:** CRenderSettings::getResolutionPreset 0x009c5e10 lives in GfxBase RenderSettings.cpp (assert line 66, /Od).
- **Not named:**
  - The Game ctor helper 0x00653790.
  - STUDIO checkLookup copies 0x006db970 / 0x00703620.
  - MemberDescriptorContainer<T>::compare 0x00614cd0.
  - GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>::singleton 0x00407230 and its createInstance 0x00409020.
  - The weak_ptr function1 / basic_vtable rows 0x00844de0 / 0x00844f60 / 0x0073da60 / 0x0073db90.
  - CSHA1 ×6 and TCPInterface/PacketizedTCP ×4 have no 2013 body.
- **Carried:**
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv predate STUDIO isUpperRoot 0x00a0e170.
  - STUDIO 0x004337a0 is still unnamed.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - 0x009b7020 is still a dummy.

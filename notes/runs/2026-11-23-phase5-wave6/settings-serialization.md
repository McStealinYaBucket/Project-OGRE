# Wave 6: settings items, class names and serialization follow-ups

Label: settings-serialization. Builds: `build-set/` (PLAYER), `build-set-studio/` (STUDIO). Private
reccmp root with regenerated CSVs: `build-set/root/` (symlinks plus its own `reccmp/` and
`reccmp-project.yml`), scored from `build-set/priv/` and `build-set-studio/priv/`. Runs:
`build-set/runs/P*.json`, `build-set-studio/runs/S*.json`; scored exe/pdb/map copies in
`build-set/scored*/`. The shared `reccmp/` was not written. No commits.

## Result

| | sweep12 | now (private CSVs, current link) | 100% lost, mine |
|---|---|---|---|
| PLAYER | 9,721 | **9,892** (P23) | 2 (COMDAT selection, see Losses) |
| STUDIO | 9,479 | **9,623** (S11) | 2 (the same two) |

Each figure pools two runs of the final link and the CSVs regenerated from them (PLAYER four rounds,
STUDIO three). The total includes other agents' concurrent
edits in the shared tree (World/Primitive/SleepStage, Lua RobloxExtraSpace, ScriptContext, Profiling,
Kernel). About 110 of the gains are settings/serialization entries listed below.

## 1. Settings items on GlobalAdvancedSettings::Item (RTTI is the authority)

- **`V8DataModel/GlobalSettings.h`** now holds GlobalAdvancedSettings, its `Item` and the
  `GlobalAdvancedSettingsItem<Class, sClassName>` template. The 2013 asserts name this file:
  `s.get()==sing` at `include\V8DataModel/GlobalSettings.h:132` (PLAYER 0x00654600) and `:176` (the
  basic-settings twin, not modelled). `GlobalAdvancedSettings.h` only forwards to it.
- **Settings is 0xac**: `std::string settingsFile` +0x8c, `bool` +0xa8 (~Settings 0x00657e00 destroys the
  string at +0x8c). **GlobalAdvancedSettings is 0xb4**: `boost::mutex` at +0xac/+0xb0 (ctor 0x00658580
  zeroes both; the deleting dtor 0x00658430 closes the event at +0xb0). `DECOMP_SIZE_ASSERT` 0xb4.
- **`Item::askAddChild`** override (dynamic_cast to Item) fills vtable slot 0x3c of every item vftable;
  2013's slot holds the shell COMDAT 0x00455840.
- **`singleton()`** (fast path on the slot; else `GlobalAdvancedSettings::singleton().get()`, lock
  `gs->mutex`, `createInstance()`, `setParent(gs)`, the assert): **LuaSettings 0x00654600 0.117 → 100%**,
  **NetworkSettings 0x00521d60 → 100%** (newly paired). Debug 0x00407700 and Game 0x00439860 are the
  client shell's LTCG copies (74.1%, not reachable from App's `.obj` codegen).
- **The item ctor pushes the class-name literal** because each item's own ctor is now out of line in its
  `.cpp`, so the template ctor is only instantiated where `sClassName` is defined:
  Lua 0x0085e220 74.5% → 100%, Debug 0x0065ebf0 → 100%, Game 0x006f5e10 → 100%, TaskScheduler 0x0065ecd0
  → 100% (needed `sTaskSchedulerSettings = "TaskScheduler"`: PLAYER has no "TaskSchedulerSettings"
  string at all), CRenderSettingsItem's 0x00456ee0 → 100% (shell).
- **Item layouts from the 2013 ctors** (all derive `GlobalAdvancedSettingsItem<T, sT>`, base 0x64):
  - DebugSettings 0x7c (ctor 0x0065fa10, name "Diagnostics"): stackTracingEnabled +0x64 (the byte
    printCallStack reads), floats +0x68/+0x6c, bools +0x70..+0x73 (isFmodProfilingEnabled +0x72 from
    Mac +0x6a), int +0x74 = 2, bool +0x78. Ctor 31.6% → **100%**.
  - GameSettings 0x94 (ctor 0x006f6030, "Game Options"): order and defaults agree with the 2016 header
    minus overscan; the four members other code reads keep their names. 26.1% → **100%**.
  - TaskSchedulerSettings 0x68 (ctor 0x0065fb60, "Task Scheduler"): one dword +0x64. 41.9% → **100%**.
  - NetworkSettings 0x120 (ctor 0x005b7cf0, "Network"): placeholder members typed by their stores
    (two std::strings, "127.0.0.1" and ""; port 0x94eb). 18.0% → 47.4% (store order differs; STUB).
  - LuaSettings ctor 0x0085e360 out of line (`unknown104(0.03)`, a plain literal): 73.9% → **100%**.
- **Client shell**: `ClientBase/RenderSettingsItem.h` drops its stand-in GlobalAdvancedSettings and
  template and includes App's `GlobalSettings.h` (the 2013 RTTI has NonFactoryProduct<Instance,
  sSettingsItem> in CRenderSettingsItem's hierarchy). CRenderSettingsItem ctor 0x004567d0 95.0% → 96.3%.
- **Callers**: `Network::Client::Client` 0x005b0880 stores `&NetworkSettings::singleton()` (93.1% →
  **100%**). ScriptContext's `LuaSettings::singleton()`/`DebugSettings::singleton()` calls now resolve to
  the template: printCallStack 0x0067abc0 99.0% → **100%**, onCheckTimeout 0x0067a1e0 → 100%.
- Static-local `singleton()`s in Debug/Game/LuaSettings removed (the template's replaces them).
- `TEMPLATE`/`SYNTHETIC` markers for the FactoryProduct members whose IDA `match:` names still spell the
  old base (13 per target) in Debug/Game/Network/TaskSchedulerSettings.h, and a `SYNTHETIC` in
  `Settings.h` pinning PLAYER 0x00644360 (the folded ServiceProvider/DataModel/Settings slot-0 thunk;
  GlobalBasicSettings' own thunk stopped being that twin once Settings got its string).
- Not done: FLog::FastLogSettingsItem and PhysicsSettings have no class in the tree yet (nothing reads
  them); NetworkSettings ctor ordering; GlobalAdvancedSettings::singleton 0x00658980 (38.1%, a
  call_once static the tree models differently).

## 2. Class-name strings

Checked against the doDeclare<> pointers in PLAYER (and RTTI):
- `sGeometryService = "Geometry"` (doDeclare 0x0062e430 → 0x00b79530 → 0x00b45800).
- `Stats::sStatsService = "Stats"` (doDeclare 0x00480f00 → 0x00b509d8 → 0x00b019b4).
- `sStatsItem` is "StatsItem" (0x00b50934) and lives in `RBX::Stats` per RTTI/iOS, but moving it renames
  every template instantiated on it (53 entries, 40 at 100% in sweep12 pair by those names), and the class
  itself is `RBX::Stats::Item` in 2013. Left for a dedicated Stats::Item rename.
- `sTaskSchedulerSettings = "TaskScheduler"` (see above).

## 3. Serialization

- `TextXmlWriter::serializeNode` 0x00628a60: uses `contentId.getName()` for the mime attribute,
  isNull/isAsset/isHttp, `encodedWrite(…, 0)` for URLs (2013 passes 0), `encodedWrite(stream,
  toString(this))`. 72.6% → 99.4% in both targets (STUDIO 0x0074c8d0). The last diff is one branch target: 2013 sends the `xsi:nil` case to a separate destructor stub placed before the URL block. Two source shapes (nested `if`, early `return`) give the same 99.4%; it stays a STUB.
- `SerializerV2::load(istream&)` 0x006fd070: `good()` checks (2013 compares the whole rdstate),
  `clear()` in each branch, no zero-init of the version, `char name[33]` with `name[32]=0`,
  `ServiceProvider::create<ContentProvider>(dataModel)->readContent(...)` with the result discarded,
  streamoff seekg, `memcmp == 0` → deserialize else loadXML. 26.2% → **100%** in PLAYER and STUDIO 0x0074b600 (FUNCTION both).
- `ContentProvider::readContent` returns **ContentId**, not std::string: the 2013 body builds
  `ContentId(name)` (0x004c4720) in the hidden return slot and returns it. The body stays a STUB (6.5% →
  14.5%): the copy-to-cache branch (path helper 0x00661a30, `content.good()` assert at
  ContentProvider.cpp:1093) is not written.

## 4. DataModel.obj's Object.h spelling: reverted (needs a per-target define)

Making `#include "util/object.h"` the first App include of DataModel.cpp gave PLAYER 15 entries:
isNullClassName<sContentFilter> 0x00632290 and static_getCreator<Visit> 0x00630f20 reached 100%, plus
Creator<Visit>::getClassName, isNullClassName<Visit> and 11 more isNullClassName<> in
0x006323d0–0x006329f0. But **STUDIO's DataModel.obj spells `include\Util/Object.h`**: STUDIO has no
`include\util/object.h` string at all, only `Include\util/object.h` (RbxOgre and the shell) and
`include\Util/Object.h`. The same edit cost STUDIO eight entries, including Visit's 0x0042a0f0/0x0042a150,
and the build has no preprocessor symbol for the target (`OGRE_TARGET` only reaches winewrap). It also
moved 13 PLAYER copies whose 2013 originals come from Network TUs and the LTCG shell onto DataModel.obj's
spelling (isNullClassName<Spawner/StarterGui/StarterPack/…>, MarketplaceService's templates,
Creator<Lighting>). Reverted; DataModel.cpp is unchanged.
Needed: a per-target compile definition (for example `/DOGRE_TARGET_PLAYER` from the toolchain file)
and `#ifdef` around the include. In PLAYER, the Network TUs that own the other copies must also
instantiate them.

## 5. SIZE asserts

DECOMP_SIZE_ASSERT (appended at file ends, no lines move): XmlNameValuePair 0x10, XmlAttribute 0x18,
XmlElement 0x28 (XmlElement.cpp), XmlWriter 0x14 (XmlSerializer.cpp), MemoryBinder 0x20, ArchiveBinder
0x2c (SerializerV2.cpp), StarterGear 0x60 (create<StarterGear> 0x00421ee0 allocates 0x60),
GlobalAdvancedSettings 0xb4, DebugSettings 0x7c, GameSettings 0x94, NetworkSettings 0x120,
TaskSchedulerSettings 0x68. SerializerBinary's remaining functions were not attempted (the unnamed ones
are STL helpers instantiated by readPropertyValues, 5,546 B).

**Row 5c**: 86/152 (56.6%, sweep12) → **87/152 (57.2%)** (SerializerV2::load; P23). CSVs `row-5c-before.csv`,
`row-5c-after.csv`.

## 6. __TBB_LockByte

It has C++ linkage in our build: `?__TBB_LockByte@@YAIAAE@Z` (returns `uintptr_t`, TBB 2.1), and the
shared `player-libdata.csv`/`studio-libdata.csv` rows already carry that spelling, so spin_mutex call
sites pair. The IDA `lib:` name `?__TBB_LockByte@@YAEAAE@Z` (unsigned char) comes from TBB 4.x headers;
the callers' `mov ebx,eax … mov [esi],bl` (Body::getPV_Spin_Lock 0x007554e0) fit 2.1's uintptr. The IDA
owner should rename 0x00402240 / STUDIO 0x00443510.

## Losses read with --verbose (against sweep12)

Only two are mine (one pair per target), and both come from COMDAT selection, not codegen:
- **Described<GlobalAdvancedSettings>::classDescriptor** PLAYER 0x005268b0 / STUDIO 0x00607940 and
  **Described<LuaSettings>::classDescriptor** 0x00788570 / 0x00735cc0 (86.2%). In 2013 these copies come
  from registration TUs that read the name through the extern: Network's API.obj, and an App block whose
  dynamic initializers are at 0x00a75a20/0x00a75b80. Ours used to come from Client.obj and LuaLibrary.obj,
  and only because the old inline GlobalAdvancedSettings/LuaSettings ctors were instantiated there. With
  the ctors out of line (which is what made the item ctors and four item ctors match), the copy now comes
  from the defining `.cpp` and pushes the literal. 2016's Network/API.cpp calls
  `GlobalAdvancedSettings::classDescriptor()` in its init function. In 2013 that is probably inside the
  VMProtect-mutated Network init, so it was not written.
- Not mine: ChatMode `_Allocate` twin swap (P 0x00429c80 ↔ 0x005cee00) and boost unordered 0x00429b60
  (both targets). Lua `exp2reg` 0x009a7430 and STUDIO's RobloxExtraSpace/luaE_freethread/lua_close/
  symbexec/luaV_execute/constructor/patchtestreg/removevalues/luaK_setreturns belong to the Lua work and
  the known no-.reloc list.
- Fixed along the way: STUDIO briefly lost ~ServiceProvider 0x0073f7d0, the Settings/GlobalAdvanced
  classDescriptors and several map<Name const*, shared_ptr<Instance>> bodies. The cause was GfxBase's
  `/Od` RenderSettings.cpp, which registers App's non-2013 `RBX::RenderSettings` class. It became the
  first `.obj` to instantiate ~ServiceProvider once Client.obj stopped instantiating it, and STUDIO links
  GfxBase ahead of App. App's `V8DataModel/RenderSettings.h` now derives the item template, so that TU no
  longer pulls the ServiceProvider/Settings chain. 2013 keeps ~ServiceProvider from DataModel.obj (PLAYER
  0x00641d70, between DataModel methods), and ours does too.
- The folded ServiceProvider/DataModel/Settings slot-0 thunk (P 0x00644360, S 0x0073fce0) is pinned by a
  SYNTHETIC marker in Settings.h. The stale IDA `match:` name `??_EGlobalBasicSettings` used to pair it
  with GlobalBasicSettings' thunk, which stopped being its twin.

## Checks

- Both targets build (`build-set/`, `build-set-studio/`); final scored copies are in `*/scored-final/`.
- Markers flipped at effective 100% in both targets: SerializerV2::load, the Lua/Debug/Game/TaskScheduler
  settings ctors and Network::Client::Client (FUNCTION), and LuaSettings' singleton and item ctor
  (TEMPLATE). CRenderSettingsItem's item ctor 0x00456ee0 is PLAYER only (TEMPLATE).
- `check_2016_copies`: silent. `reccmp_addr_padding`: 0 badly padded.
- New markers are in ascending order per file (item ctors moved below the EnumDesc ctors).

## Unresolved (for other owners)

- **IDA owner**: rename the old-base `match:` names (FactoryProduct<Debug/Game/Network/TaskScheduler
  Settings, GlobalAdvancedSettings, …> → `VItem@GlobalAdvancedSettings@2@`, 13 per target, addresses in
  the headers' markers); 0x00644360 is the folded ServiceProvider/DataModel thunk, not
  `??_EGlobalBasicSettings`; `__TBB_LockByte` 0x00402240/0x00443510 is `?__TBB_LockByte@@YAIAAE@Z`;
  0x00654190 is `FactoryProduct<LuaSettings,…>::createInstance`, 0x004c4720 is `ContentId(const char*)`.
- **Network owner**: the Network TUs that own isNullClassName<…>/MarketplaceService templates in 2013
  (0x0052a000–0x0052f700, 0x00583dc0–0x00588da0, 0x005b9e40); API.cpp's registration of Client, Server,
  Player, Players, GlobalAdvancedSettings and NetworkSettings (classDescriptor 0x00526130–0x005268b0).
- **Stats owner**: `StatsItem` is `RBX::Stats::Item` with `Stats::sStatsItem` in 2013.
- **Toolchain owner**: a per-target compile definition, so DataModel.cpp can spell `util/object.h` in
  PLAYER only (item 4).
- **Rendering contributor**: `GfxBase/RenderSettings.cpp` registers `RBX::RenderSettings`, which has no 2013
  class (2013's is the shell's CRenderSettingsItem). Its `/Od` template copies sit ahead of App's in STUDIO.
- ContentProvider::readContent's cache branch (needs 0x00661a30 and the assert on line 1093).
- GlobalBasicSettingsItem<> (GlobalSettings.h:176) is not modelled.

## Files changed

- Client/App/include/V8DataModel/GlobalSettings.h (was empty), GlobalAdvancedSettings.h, Settings.h,
  LuaSettings.h, DebugSettings.h, GameSettings.h, NetworkSettings.h, TaskSchedulerSettings.h,
  ContentProvider.h (readContent return type), RenderSettings.h (base only)
- Client/App/v8datamodel/GlobalAdvancedSettings.cpp, LuaSettings.cpp, DebugSettings.cpp, GameSettings.cpp,
  NetworkSettings.cpp, TaskSchedulerSettings.cpp, GeometryService.cpp, StatsService.cpp, StarterGear.cpp,
- Client/App/v8xml/XmlSerializer.cpp, SerializerV2.cpp, XmlElement.cpp; Client/App/util/ContentProvider.cpp
  (readContent only)
- Client/Network/Client.cpp (ctor initializer and marker; outside my listed files, but its comment waited
  for exactly this change); Client/ClientBase/RenderSettingsItem.h
- notes/runs/2026-11-23-phase5-wave6/row-5c-before.csv, row-5c-after.csv

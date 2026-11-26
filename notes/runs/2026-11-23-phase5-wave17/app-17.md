# Wave 17: App and Base follow-ups (app-17)

- **Owner and scope:** Client/App and Client/Base (no Base file changed). No commits; the shared reccmp/ was not written.
- **Build dirs:** build-ap/ (PLAYER) and build-ap-studio/ (STUDIO). Logs are in build-ap/w17/.
- **Snapshots and runs:** build-ap/w/snaps/T1..T6 (PLAYER) and U1..U3 (STUDIO); runs are in build-ap/w/runs/.
- **How I measured:** with a private root (build-ap/w/root). Its tools/, reccmp/ and reccmp-project.yml were refreshed from the repo at sweep23 (HEAD d5180c5). I then regenerated it in place with `gen_reccmp_csv.py`, each time from two pooled runs of the current link (ab.sh, then rounds.sh).
- **Headers note for network-5f-8:** app-17-headers.md, written at 08:12 when item 1 compiled. Item 2 also landed at 08:12, and network-5f-8 removed its copies before my second link.

## Figures (effective 100%, against sweep23)

| | sweep23 | this wave (final) | gained / lost |
|---|---|---|---|
| PLAYER | 12,597 | **12,727** (T6z) | +130 / **0** |
| STUDIO | 11,880 | **11,977** (U3z) | +97 / **0** |

- **Shared-tree caveat:** the tree also held the network and shell agents' uncommitted work (ItemSender, Replicator, Players, ConcurrentRakPeer, View/Application), so the totals include their gains.
- **App-attributable entries:** about 50 PLAYER and 44 STUDIO. Lists: build-ap/w17/T6-vs-sweep23.txt and U3-vs-sweep23.txt.
- **Losses I read with --verbose in intermediate runs (none remain in the final run):**
  - **Pairing only:** the `sp_pointer_construct<…Job>` `_internal_accept_owner` twin (×5), and the bind<Thread> / shared_ptr<Player> twins in queueResumeThread and callMemberYieldFunction.
  - **No-.reloc immediates:** Lua symbexec/luaK_* (STUDIO).
  - **Artifacts of scoring against live source:**
    - View::* (T5): the shell agent edited View.cpp after my link.
    - EnumDesc<SwipeDirection> and stepDataModelJob (U2): I moved markers after that link.
  - **Fixed in place:** Game::getDataModel 0x0041ce40 and ScreenshotVerb::doIt 0x00438460 (T4). My Game ctor had called the inline getDataModel(), which our compiler kept out of line, so it took the shell copy's pairing. The final spelling has no such call, and both are back at 100%.

## Changes

### 1. For Network (app-17-headers.md)
- **NetworkSettings.h:**
  - `bool printSplitMessages` (+0x64) and `int dataSendPriority` (+0xd8) are now public, along with `getDataSendPriority()`.
  - The ctor init list uses the new names.
  - Result: Replicator::OnInternalPacket 0x00581b50 / 0x00623b00 and the ItemSender functions are at 100%.
- **Instance.h:**
  - Added public inline `unlockParent() { fw()->b21.set(false, fwRef()); }`, FUNCTION PLAYER 0x00585ac0 / STUDIO 0x0060af20 (I found the STUDIO COMDAT by its bytes). Both are at 100% through Server::stop.
  - The comment that said 2013 has no unlockParent is replaced. The line count is unchanged.
- **Util/Http.h:** the TEMPLATE namerefs at PLAYER 0x005e7920 and STUDIO 0x006d13d0 now carry the `list<rbx::signals::connection>::_Buynode` PUBLICS name. Both are at 100%.
  - The InstancePacketCache ctor and onServiceProvider pair too (PLAYER 0x005d7cb0 / 0x005d5ef0, STUDIO 0x006673c0).
  - The STUDIO chain_base / GuiService list helpers are kept.

### 2. Statics moved into App
- **FactoryRegistration.cpp:** the three packet-cache `ClassRegistrar<…>::registrar(0)` definitions and their GLOBAL markers, with includes `Network/NetworkPacketCache.h` and `Network/NetworkClusterPacketCache.h`.
- **ScriptContext.cpp lines 82-86:** `FLog::NetworkCache = 1` with its Initer and GLOBAL markers. It replaced comment lines, so no assert moved.

### 3. FriendService (all FUNCTION markers at 100% in both targets)
- **playerAdded** (PLAYER 0x006b36a0 / STUDIO 0x008123a0):
  - backendProcessing gate, then `players.size() > 0`, then `!m_unk0x7c.empty() && userId >= 0`.
  - Builds an ostringstream from `RBX::format(url, userId)` plus `&otherUserIds=` for each non-negative id.
  - Then either `Http(ostr.str()).get(boost::bind(&ProcessBulkFriendResponse, weak_from(this), userId, players, _1, _2))` or a direct `ProcessBulkFriendResponse(…, NULL, NULL)`.
  - Ends with `if (players.count(userId) == 0) players.insert(userId)`.
- **friendEventReplicationChanged** (0x006b14e0 / 0x008101e0): `find<Players>(this)->friendEventFired(…)`.
- **friendStatusReplicationChanged** (0x006b2440 / 0x00811140):
  - If backendProcessing is false, it calls storeAndReplicateFriendStatus.
  - Then `friendStatusChanged` runs twice: once as given, once reversed through a static `InvertFriendStatus` switch (3<->4).
- **New STUBs:**
  - static `ProcessBulkFriendResponse` 0x006b2c80 / 0x00811980.
  - `storeAndReplicateFriendStatus` 0x006b1500 / 0x00810200. It is `__declspec(noinline)` as a stand-in: VC9 inlined the same-TU STUB, and the caller scored 47%.

### 4. DataModel per target
- **`OGRE_TARGET_STUDIO` drops two members:**
  - `DataModel::networkStatsWindowsOn` (header plus ctor init list; the line count is kept).
  - `GuiBuilder::networkStatsPage`, with `nextNetworkStats` and `getNetworkStatsPage`.
- **STUDIO DataModel ctor 0x0076b720 and dtor 0x0076a340 are now at 100%** (STUB → FUNCTION). So are getJobId 0x0074d500, getSyncronizationArbiter 0x0074d6c0, ConcurrentRakPeer::rawPeer 0x0061e8d0 (Network's marker) and the itemChanged signal helper 0x007519e0.
- **Correction:** STUDIO's DataModel is 0xe10, not 0xe0c (`push 0xe10` in Creatable::create, STUDIO 0x0076c22f).
  - STUDIO's parentDataModel ends at 0xe0c, and the class pads to 8.
  - DataModel.cpp's size asserts now say this (GuiBuilder 0x1c in STUDIO, 0x20 in PLAYER).
  - The header comments in DataModel.h and GuiBuilder.h that say "0xe0c" mean the end of parentDataModel. They are not edited.
- **PLAYER is unaffected:** no PLAYER source changed.

### 5. Game, openState
- **New `v8datamodel/Game.cpp`. Game::Game is FUNCTION, PLAYER 0x00655e70: 100%.** I mapped the callees from the iOS 10-29 thumb disassembly (`llvm-objdump --triple=thumbv7-apple-ios` gives symbolized `bl` targets):
  - When `shouldShowLoadingScreen && FFlag::OverlayDataModelEnabled` (0x00de98d0): `overlayDataModel = OverlayDataModel::createOverlayDataModel(true, new NullVerb(NULL, ""))`.
  - Then `dataModel = DataModel::createDataModel(true, lockVerb, overlayDataModel.get())`.
  - Under the same condition:
    - `overlayDataModel->FUN_00652f00(shared_ptr<DataModel>(dataModel))` (a weak_ptr parameter; the explicit temporary is what the bytes show).
    - A `LegacyLock(overlay, Write)`.
    - `overlay->game = this`.
    - `create<ScriptInformationProvider>()->setAssetUrl(std::string(baseUrl) + "/asset/")`.
    - `create<ContentProvider>()->setBaseUrl(baseUrl)`.
    - `ServiceProvider::create<UserInputService>(overlay)` → `setProcessToolCommandsSeparately(true)`.
    - `ServiceProvider::create<ControllerService>(overlay)`.
    - `ServiceProvider::create<RunService>(overlay)->setRunState(RS_RUNNING)`.
  - Finally `dataModel->submitTask(boost::bind(&Game::setupDataModel, this, std::string(baseUrl)), Write)`.
  - **STUDIO 0x007cddd0 stays STUB at 99.65%.** The only diff is the `shared_ptr` operator= twin it calls: our 0x00489f70 copy pairs as `operator=<Players>`.
- **Game.h layout:**
  - `overlayDataModel` is `shared_ptr<OverlayDataModel>`.
  - `commonVerbs` (shared_ptr<CommonVerbs>, 2016 name) is at +0x28.
  - `Unk0x30` is a placeholder struct with an out-of-line ctor 0x00653790 and dtor 0x00654e80. Its ctor touches a SAFE_HEAP_STATIC.
    - The ctor STUB is noinline and makes one opaque `boost::call_once`. Without that call VC9 proves it nothrow and drops the ctor's EH state 3.
  - Also: VTABLE PLAYER 0x00b31044 and SYNTHETIC D0 0x00656230, both at 100%. The STUDIO D0 0x007ce200 pairs at 100% unmarked.
- **DataModel::createDataModel now returns `shared_ptr<DataModel>`** (2016 type). It is FUNCTION at 100% in both targets (0x0064fc30 / 0x0076c6e0), up from a STUB.
- **New STUB declarations, so the Game ctor's callees pair:**
  - `OverlayDataModel::createOverlayDataModel` 0x00693310 / 0x00850680.
  - `OverlayDataModel::FUN_00652f00(weak_ptr<DataModel>)` 0x00652f00 / 0x007cba50. Unnamed: inline in 2013, with no iOS symbol.
  - `ScriptInformationProvider::setAssetUrl(std::string)` 0x00652d70 / 0x007cb930 (iOS name).
  - `UserInputService::setProcessToolCommandsSeparately(bool)` 0x0069ff10 / 0x0083c6d0 (iOS name).
  - `Game::setupDataModel` 0x00655630 / 0x007cd6d0.
  - `~Game` 0x00655d30 / 0x007cdc90.
  - STUB markers on the existing `ContentProvider::setBaseUrl` 0x0065fe50 / 0x0078a2c0 and `RunService::setRunState` 0x0072db60 / 0x0079b7e0.
- **FFlag `OverlayDataModelEnabled` (false)** is defined in OverlayDataModel.cpp. Its 2013 initializer 0x00a6a1f4 follows the ConnectToGame flags.
- **New `ControllerService` (V8DataModel/UserController.h, v8datamodel/UserController.cpp; the files were empty):**
  - `DescribedNonCreatable<…, INTERNAL_LOCAL>` + Service, from RTTI `$00$0A@`.
  - SIZE 0x68, from `new(0x68)` at 0x00469490.
  - The ctor is a STUB at 0x006ea020 / 0x00817010.
  - Its vftables, isNullClassName, callDoDeclare and classDescriptor pair at 100% in both targets.
- **ScriptContext::openState 0x00688370: 0.583 → 0.691 (still a STUB).**
  - The game/workspace block now follows 2013: no try/catch. The root is re-read through the inline `getRootAncestor()`, and an OverlayDataModel root gets only `App`. Otherwise it pushes game/Game and workspace/Workspace through `shared_from(ServiceProvider::find<Workspace>(this))`.
  - Removing the EBP frame was the big step.
  - **Correction to the task text and the old comment:** our `LUA_GLOBALSINDEX` already is -10002, so the "register into -10002" delta does not exist.
  - **Remaining deltas:** a 0x60 vs 0x50 frame, and LuaAllocator `new(0x24)` vs our 0x20.
- **DataModel's unnamed group (162):** not started.

## Patterns proven
- **A same-TU STUB that VC9 can prove nothrow drops the caller's EH state and its state store.** This is not just inlining: `noinline` alone was not enough for Game's Unk0x30 ctor. One opaque call in the STUB (here `boost::call_once`) restored state 3 (Game::Game 0x00655e70, 86% → 100%).
- **A `shared_ptr<T>(member)` temporary converted to a `weak_ptr<T>` parameter** gives 2013's use-count +1, then weak-count +1, sequence (Game::Game → 0x00652f00).
- **iOS 10-29 thumb code disassembles with symbolized callees:** `llvm-objdump -d -C --triple=thumbv7-apple-ios --start-address=… ref/ios_1029/robloxmobile`. That is the fastest way to name a function's unnamed PLAYER callees.

## Checks
- `check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,705 files, 0 bad.
- **`reccmp-decomplint`** (PLAYER and STUDIO) on every touched file:
  - Clean, apart from the pre-existing `byname_function_in_cpp` warnings in RunService.cpp.
  - Two new descents were fixed: setRunState was moved with `reorder_markers.py apply` (RunService.cpp has no asserts), and the UserInputService STUB was moved ahead of the EnumDesc block.
- **Assert lines:**
  - ScriptContext.cpp, DataModel.cpp, GuiBuilder.cpp, ContentProvider.cpp and RunService.cpp keep their line counts (each edit replaced lines one for one).
  - Instance.h keeps its line count.
- **Every FUNCTION marker added or flipped this wave is at 100%** in T6z / U3z: playerAdded, both handlers, unlockParent, Game::Game (PLAYER), createDataModel ×2, and STUDIO DataModel ctor/dtor.

## Unresolved
1. **IDA owner (names export):**
   - New names:
     - Game::Game's callees: 0x00653790 / 0x00654e80 (Game::Unk0x30 ctor/dtor), 0x00652f00 (OverlayDataModel, unnamed), 0x00652d70 (ScriptInformationProvider::setAssetUrl), 0x0069ff10 (UserInputService::setProcessToolCommandsSeparately), 0x0065fe50 (ContentProvider::setBaseUrl), 0x0072db60 (RunService::setRunState).
     - FriendService: 0x006b2c80 (ProcessBulkFriendResponse), 0x006b1500 (storeAndReplicateFriendStatus).
     - ControllerService: ctor 0x006ea020 / 0x00817010; Creatable::create<ControllerService> is the shell copy 0x00469490.
     - FFlag OverlayDataModelEnabled at 0x00de98d0.
   - The STUDIO counterparts are in the markers above.
2. **Integrator:** regenerate the shared CSVs. The ControllerService templates, Game D0/vftable, the FriendService bind/function helpers and the SwipeDirection rows pair only after a regen.
3. **STUDIO Game::Game:** stays at 99.65% until the `shared_ptr` operator= twin (0x00489f70) pairs with its own name rather than `operator=<Players>`.
4. **App, next:**
   - Game::setupDataModel 0x00655630 (1446 B), ~Game 0x00655d30 (shutdown 0x00654360) and SecurePlayerGame's ctor 0x00656250 (it calls `call_once(0x00526e10)`; the shell's `new SecurePlayerGame` is its only caller).
   - Name and type Unk0x30: which class has a 4-byte member whose ctor touches a SAFE_HEAP_STATIC (getter 0x004265c0, an 8-byte heap object)?
   - openState: frame size and LuaAllocator size (0x24).
   - DataModel's unnamed group.

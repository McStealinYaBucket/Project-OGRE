# Wave 16: App and Base follow-ups (app-16)

- **Owner and scope:** Client/App and Client/Base. No commits, and the shared reccmp/ was not written.
- **Build dirs:** build-ap/ (PLAYER) and build-ap-studio/ (STUDIO).
- **Scripts, snapshots and runs:** build-ap/w/. `snap.sh`, `score.sh` and `cmp.py` are the same as in wave 15. Snapshots are in `snaps/Q1..Q5` (PLAYER) and `snaps/R1..R3` (STUDIO), runs in `runs/Q*` and `runs/R*`.
- **How I measured:** with a private root (build-ap/w/root). Its tools/, reccmp/ and reccmp-project.yml were refreshed from the repo at sweep22, then regenerated in place by `gen_reccmp_csv.py` from pooled pairs of runs of each link (two rounds).
- **Headers note for network-5f-7:** app-16-headers.md, written at 03:54, when item 1 had compiled.

## Figures (effective 100%, against sweep22)

| | sweep22 | this wave | gained / lost |
|---|---|---|---|
| PLAYER | 12,496 | **12,590** (Q4a, CSVs from the Q3 rounds) | Q3z: +99 / -13, then Q4a: +8 / 0 over Q3z (all 8 network) |
| STUDIO | 11,780 | **11,871** (R2a, CSVs from the R1 rounds) | R1e: +96 / -23, then R2a: +18 / 0 over R1e (network) |

- **Caveat:** the shared tree also held the network and shell agents' uncommitted work (Replicator, Players, Peer, ConcurrentRakPeer, packet caches, GameVerbs, View), so their gains and losses are in these totals.
- **My losses, each read with --verbose:**
  - **Both targets: the misnamed FriendService deleting destructor** (PLAYER 0x006aaf50, 0.769; STUDIO 0x00809d30). The names export gives `RBX::FriendService::`scalar deleting destructor'` to 0x006aaf50, and libdata pins `??_GFriendService@RBX@@UAEPAXI@Z` there. But 0x006aaf50 stores the FactoryProduct<FriendService> vftables and calls ~Instance, so it is a base-class D0. FriendService's own D0 is 0x006b2b30, which calls the out-of-line implicit dtor 0x006b2970.
    - Before this wave they "matched" only because our FriendService had no members.
    - The same row costs the FactoryProduct vftable 0x00b394b0 (0.5; STUDIO 0x00c13c50).
    - I added the correct SYNTHETIC markers. They pair once the IDA owner renames 0x006aaf50.
  - **PLAYER 0x00419510 "boost::bad_any_cast::what": pairing only.** It is a folded shell `what()` used by three shell exception vftables (0x00bda0a8, 0x00bda0bc, 0x00bda1b4; referenced from 0x00415000-0x00418e00). Our only copy came from the invented pointer-returning `rbx::any_cast(const placement_any&)` that I removed (item 2). The real shell exception classes or program_options `variable_value::as<T>` bring it back.
  - **STUDIO 0x00734740 `Described<FriendService>::classDescriptor`, 0.862: TU placement.** The ctor is now out of line in FriendService.cpp, which defines sFriendService, so our copy folds the literal. The 2013 STUDIO copy loads `sFriendService` from data, so it comes from an earlier TU in link order (the 2013 Network/Players.cpp includes FriendService.h; see wave-15 unresolved item 1).
- **Not mine:** every other loss.
  - The five `sp_pointer_construct<...>` entries are the `_internal_accept_owner` twin: they call the Registry or PacketReceiveJob copy.
  - The Lua and RakNet entries at 0.95-0.99 are no-.reloc immediates.
  - The rest is network work in progress (Players::onChildChanged/~Players, Server::onCreateRakPeer, Peer::*). Several of those came back in Q4a/R2a.

## Changes

### 1. NetworkSettings MTU adjusts (done first; app-16-headers.md)
- **Members:** `int physicsMtuAdjust` (+0xfc) and `int replicationMtuAdjust` (+0x100) replace m_unk0xfc and m_unk0x100.
- **Accessors:** public inline `getPhysicsMtuAdjust()` and `getReplicationMtuAdjust()`. The 2013 assert strings at Replicator.cpp:3424 and :3418 name them.
- **Result, with the network agent's code:** Replicator::getAdjustedMtuSize and getPhysicsMtuSize are at 100% in both targets (PLAYER 0x00581c10/0x00581ca0, STUDIO 0x00623bc0/0x00623c50).
- **ReplicationData / PacketReceiveJob size annotations:** not added, because their headers are Network's (see Unresolved).

### 2. EventDescImpl::fireEvent fetches its argument by value
- **2013 shape:**
  - CFrame's fireEvent 0x0079b760 calls a thiscall Variant member 0x0058f560 with its return slot in the outgoing argument area.
  - That member calls the by-value `rbx::any_cast<CoordinateFrame>(const placement_any&)` 0x0058ebd0, which calls `any_cast<const CoordinateFrame&>` 0x00701380 and copies.
  - For bool everything inlines, and the result goes through a stack temp (`mov [esp+0x18],dl; mov eax,[esp+0x18]; push eax`).
- **Variant::cast is unchanged.** I added a separate member, `template<T> T Variant::cast_unk0x0058f560() const { return rbx::any_cast<T>(value); }` (Type.h). No symbol names it (it is inline on iOS), hence the placeholder. Event.h's 28 `args[i].cast<argN_type>()` now call it, with no line changes.
- **`rbx/boost.hpp` (Base):**
  - Added boost's const overload `ValueType any_cast(const placement_any<S>&)`. It forwards to `any_cast<const nonref&>` on a const_cast operand.
  - Removed the invented pointer-returning `any_cast(const placement_any&)` and its friend declaration. It would be ambiguous with the new overload, and the data members are public anyway.
  - Its only users, `withVariantValue` (LuaArguments.h, a STUB), now call `rbx::any_cast<const T&>(value.value)`. That is what 2013 does: withVariantValue 0x008a67b0 calls any_cast<bool const&> 0x0044d360.
- **Now at 100% (PLAYER):**
  - GBS fireEvent 0x00643ea0.
  - The sCFrameValue, sColor3Value and sRayValue fireEvents 0x0079b760 / 0x0079c1e0 / 0x0079ae20.
  - ScriptContext <3> 0x00680a20 and 0x00681460, ChatService 0x00802130, GuiButton <2> 0x007c6560, StringValue 0x007f6e40, ObjectValue 0x00911410 and Instance <2> 0x009156e0.
  - The cast_unk0x0058f560 copies 0x0058f560 / 0x0058f530 / 0x00542830 / 0x00542860 and the by-value any_casts 0x0058ebd0 / 0x0058eb80 / 0x00542720 / 0x00542750.
  - STUDIO gains the same set.

### 3. Settings::verifyAddDescendant (V8DataModel/Settings.h, v8datamodel/Settings.cpp)
- **`Settings::InvalidDescendentDetector`:** a private nested struct holding one bool, `anyInvalid`.
  - `invalid()` is a cdecl static that runs three `dynamic_cast`s, to GlobalBasicSettings::Item, GlobalAdvancedSettings::Item and Selection.
  - `operator()(shared_ptr<Instance>)` ORs into the bool.
- **verifyAddDescendant:** the protected virtual override `verifyAddDescendant(const Instance*, const Instance*) const`. It tests `invalid(x)`, then visits descendants with a detector, throws `RBX::runtime_error("Not allowed to add that under settings")` if anything is invalid, and finally calls `Super::verifyAddDescendant`.
- **FUNCTION markers:**
  - invalid: PLAYER 0x006562c0 / STUDIO 0x0073edd0
  - operator(): PLAYER 0x006566d0 / STUDIO 0x0073f1b0
  - verifyAddDescendant: PLAYER 0x00656a80 / STUDIO 0x0073f440
- **Also at 100%:** `Instance::visitDescendants<InvalidDescendentDetector>` 0x006569a0 (the out-of-line template copy, unmarked) and seven Settings/GAS/GBS vftables (0x00b312b4 ... 0x00b3166c; STUDIO 0x00bf208c ... 0x00bf2444).

### 4. FriendService
- **Layout, from the 2013 ctor 0x006b2780 and its EH states:**

  | Offset | Member |
  |---|---|
  | +0x64 | friendEventReplicatingSignal (remote_signal<void(int,int,FriendEventType)>) |
  | +0x6c | friendStatusReplicatingSignal |
  | +0x74, +0x78 | friendStatusConnection, friendEventConnection (scoped_connection) |
  | +0x7c .. +0x108 | six URL strings, `m_unk0x7c ... m_unk0x108` (2013 does not say which is which; +0x7c is the one playerAdded formats with the user id) |
  | +0x124 | friendStatusTable (`map<int, map<int, FriendStatus>>`, node isnil +0x1d) |
  | +0x130 | friendRequestSet (`set<pair<int,int>>`, +0x15) |
  | +0x13c | players (`set<int>`, +0x11) |
  | +0x148 | enable |

- **Size:** SIZE 0x14c (`new(0x14c)` in Creatable::create 0x00421820).
- **FUNCTION markers:**
  - **Ctor** (`: enable(false) { setName("FriendService"); }`): PLAYER 0x006b2780 / STUDIO 0x00811480.
  - **playerRemoving** (end iterators cached in locals; erase(iter++); the inner find/erase only when key < userId; then players.erase): PLAYER 0x006afb40 / STUDIO 0x0080e860. Byte-identical in a probe before the link.
  - **onServiceProvider:** PLAYER 0x006b2b50 / STUDIO 0x00811850. It disconnects each connection only if connected(), then, when newProvider is set, connects the status signal first and the event signal second through `boost::bind(&FriendService::...ReplicationChanged, this, _1, _2, _3)`. It makes no Super call. This was the missing vftable slot 0x20, so FriendService's Debugable vftable 0x00b3976c is now 100%.
- **SYNTHETIC markers:** ~FriendService 0x006b2970 / 0x00811670 (PLAYER 100%) and the D0 0x006b2b30 / 0x00811830 (see the losses above).
- **STUBs:** friendEventReplicationChanged 0x006b14e0 / 0x008101e0 and friendStatusReplicationChanged 0x006b2440 / 0x00811140. They call Network::Players members the Network headers do not declare (0x00566790, 0x0055f070).
- **Also at 100%:** the two signal ctors, mutexes, disconnectAll and ~signal, both connect<> wrappers 0x006af950/0x006af980, the map erase/destroy helpers, the Described ctor 0x006b0460, classDescriptor 0x006afe70 (PLAYER), and the callable/icallable vftables.
- **playerAdded (773 bytes) is still a STUB.** It uses ostringstream, RBX::format and an async Http call with ProcessBulkFriendResponse 0x006b2c80.

### 5. Settings file names, the Game ctor, openState, DataModel
- **Renamed:** `advancedSettingsFile` / `basicSettingsFile` to 2016's `globalAdvancedSettingsFile` / `globalBasicSettingsFile`. Both are statics, and 2013 records no name, which is the case the task names. Both stay at 100% (0x00656320, 0x00656390).
- **Game ctor 0x00655e70: not attempted.** It is 0x3b4 bytes with about 15 unnamed callees, among them OverlayDataModel::createOverlayDataModel, DataModel::createDataModel, LegacyLock, a Verb ctor, the 0x006538d0 submit with &Game::setupDataModel 0x00655630, and 0x00652f00 / 0x006421c0 / 0x00563f50 / 0x00652d70 / 0x0065fe50 / 0x0069ff10 / 0x00468f30 / 0x0072db60 / 0x00654f10 / 0x0063de50.
- **ScriptContext::openState 0x00688370 (0.583): read only.** Ours keeps an EBP frame because of two try/catch blocks around shared_from (the root and Workspace globals), and 2013 has none. It also registers into LUA_GLOBALSINDEX where 2013 uses the -10002 sandbox env. Each is a structural rewrite, not a quick fix.
- **DataModel's unnamed group (162 functions):** not started.

## Checks
- `check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,702 files, 0 bad.
- `reccmp-decomplint` (PLAYER and STUDIO) on Settings.cpp, FriendService.cpp, FriendService.h, Settings.h, Type.h and NetworkSettings.cpp: clean.
- `assert_lines`: Type.h is unchanged on every anchor (41/45/66/77/98). Settings, FriendService and NetworkSettings have no asserts. Event.h's line count is unchanged.
- **Every FUNCTION marker added this wave is at 100%** in Q4a (PLAYER) and R2a (STUDIO). The final relink after a comment-only header edit is Q5/R3, whose numbers are in the last section.

## Unresolved
1. **IDA owner (names export):**
   - 0x006aaf50 is not FriendService's D0. It is the FactoryProduct/DescribedCreatable<FriendService> base D0 (it stores vftables 0x00b394c4/0x00b394bc/0x00b394b0/0x00b394a0 and calls ~Instance).
   - FriendService's D0 is 0x006b2b30 (thunks 0x006b2950 / 0x006b2960), and its implicit dtor is 0x006b2970. STUDIO: dtor 0x00811670, D0 0x00811830.
   - New names: Settings::InvalidDescendentDetector::invalid / operator(), verifyAddDescendant (PLAYER 0x00656a80 is named; STUDIO 0x0073f440), FriendService::FriendService 0x006b2780 / 0x00811480, onServiceProvider 0x006b2b50 / 0x00811850, friendEventReplicationChanged 0x006b14e0, friendStatusReplicationChanged 0x006b2440, and the placeholder Variant::cast_unk0x0058f560<CoordinateFrame> 0x0058f560.
2. **Network agent:**
   - **Size annotations:** `Replicator::ReplicationData` (0x1c if ClusterReplicationData 0x34 = ReplicationData + 0x18 ClusterUpdateBuffer; confirm against its ctor) and `PacketReceiveJob`. Both headers are Network's.
   - **Players members FriendService calls:** declare 0x00566790 (friendEventReplicationChanged's target) and 0x0055f070 (friendStatusReplicationChanged's target, called with the status remapped 3<->4) so the two FriendService STUBs can be written.
3. **Shell agent:** program_options `variable_value::as<T>` or the real shell exception classes restore the folded shell `what()` 0x00419510.
4. **Integrator:** regenerate the shared CSVs. These pair only after a regen: the cast_unk0x0058f560 copies, the by-value any_casts, visitDescendants<InvalidDescendentDetector>, and the FriendService helpers.
5. **App, next:**
   - FriendService::playerAdded 0x006b36a0.
   - The two replication-changed handlers (after item 2).
   - The Game ctor 0x00655e70 with its unnamed callees.
   - openState's try/catch and sandbox-env deltas.

## Final relink (Q5 / R3)

- **What changed:** after a comment-only edit to FriendService.h, only FriendService.cpp and DataModel.cpp recompiled.
- **Scores**, against the same private CSVs, with no regen:

  | | Run | Entries at effective 100% | Change from Q4a / R2a |
  |---|---|---|---|
  | PLAYER | Q5a | 12,587 | -3 |
  | STUDIO | R3a | 11,866 | -5 |

- **Losses: all pairing only.** They are the network packet-cache ctors and their `Creatable::create<>` callers. The calls now go to an unpaired `<OFFSET>` after the relink moved addresses, and STUDIO also loses one `list<connection>::_Buynode` twin. A CSV regen of this link should restore them.
- **Every marker from this wave** is at 100% in Q5a and R3a.

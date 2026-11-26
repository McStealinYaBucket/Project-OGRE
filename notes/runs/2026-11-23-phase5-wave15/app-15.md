# Wave 15: App and Base follow-ups (app-15)

- **Owner and scope:** Client/App only (no Base edits were needed). No commits.
- **Build dirs:** build-ap/ and build-ap-studio/.
- **Scripts, snapshots and runs:** build-ap/w/. `snap.sh`, `rounds.sh` and `cmp.py` are the same as in wave 14. The runs are in `runs/P{1,2,3}*`, `runs/S{1,2}*` and the `*-vs-s21.txt` files.
- **How I measured:** with a private root (build-ap/w/root). Its tools/ and reccmp/ CSVs were refreshed from the repo at sweep21, then regenerated in place from two pooled runs of each link. The shared reccmp/ was not written.
- **Headers for network-5f-6:** app-15-headers.md, written when items 1-3 landed.

## Figures (effective 100%, against sweep21)

| | sweep21 | this wave | gained / lost |
|---|---|---|---|
| PLAYER | 12,200 | **12,394** (P3z) | +207 / -13 |
| STUDIO | 11,547 | **11,704** (S2z) | +171 / -14 |

- **Caveat:** the shared tree also held the network and shell agents' uncommitted work (Replicator, Server, NetworkPacketCache, RenderSettingsItem, GameVerbs, View), so their gains and losses are in these totals.
- **Not mine, read with --verbose:** every loss listed below.
  - PLAYER:
    - View::getCurrentDesktopResolution and getFullscreenSizes (shell edits after my build).
    - Four weak_ptr<DataModel> bind/function entries (the wave-14 shell/engine twin).
    - AsyncHttpQueue::dispatchGenericCallback (boost::function dtor twin).
    - Six Lua functions (`and/test reg, 0x7fc000` read as an address: no-.reloc).
  - STUDIO: 14, all no-.reloc immediates (the Lua 0x7fc000 masks, Math 0x7fffff, and a RakNet and a WinHttp mask that hit an assert string).
- **Earlier intermediate runs:**
  - P1z: items 1-3 + Selection/GAS, 12,270.
  - P2z: + fireEvent/getSignalPtr, 12,356.
  - P3z: + Selection virtual and noinline, 12,394.
  - S1z: = P2 tree, 11,691.

## Changes

### 1. Network-facing headers
Details are in app-15-headers.md.
- **NetworkSettings:**
  - Public usePhysicsPacketCache (+0x79), useInstancePacketCache (+0x7a), distributedPhysicsEnabled (+0xdc) and networkOwnerRate (+0xf0).
  - `getReceiveRate()` for +0x110.
  - **PacketReceiveJob ctor 0x005b92c0 reached 100%** with the network agent's use of it.
- **PhysicsService:** `numSenders()` reads +0x64. **PeerStatsItem::update 0x005ba060 is at 100%** in P3z.
- **FastLogSettings.cpp:**
  - `FLog::RobloxWndInit`: GLOBAL PLAYER 0x00d30edd / STUDIO 0x00ec17c9, default 1.
  - **`DFFlag::UseClusterCache`, not FFlag:** registered with kind 1 (iOS `__ZN6DFFlag15UseClusterCacheE`). GLOBAL PLAYER 0x00de3f88 / STUDIO 0x00f84e62.
- **DebugSettings:** `gfxcard()` and `videoMemory()` are declared, with STUB bodies at PLAYER 0x00659170 / 0x00659910 and STUDIO 0x00773bf0 / 0x00774340.

### 2. FriendService include spelling: no App-side fix
- **Why App cannot fix it:** Network's 2013 objects spell `include\Util/Object.h`. The original's FriendService Creator copies (0x0041eb70, 0x004200e0, 0x00422ee0) are /GL shell copies spelled `include\util/object.h`.
- **`extern template` fails too:** VC9 still emits the inline `Creator::getClassName` (probe in build-ap/w/probe15).
- **The fix is a shell stand-in.** See app-15-headers.md.

### 3. EventDescBase / EventDescImpl casts, reflection/property.h
- **The casts:** member-pointer `connect` and `disconnectAll`, every `fireEvent(DescribedBase*, args)` and `RemoteEventDesc::getSignalPtr` now use `static_cast<EventClass*>(source)`.
  - Evidence: GBS connect 0x006440c0, ScriptContext disconnectAll 0x006806b0, sBoolValue fireEvent 0x00643ea0 and getSignalPtr 0x00928030.
  - The isA check was a guess, flagged as such in the old comment.
  - **Restored:** Lighting connectGeneric PLAYER 0x00707830 / STUDIO 0x007d3620. GBS connect 0x006440c0 now pairs and is at 100%.
- **Event.h layout:** 15 blank lines are removed so that `args.size() == 0..4` lands on 2013 lines 295/320/349/380/413.
- **property.h layout:** 24 blank lines are removed, which puts all 7 asserts on their 2013 lines (93/98/103/255/289/341/415). CRLF is kept.
- **Effect:** about 60 PLAYER entries reach 100%. Most are EventDesc connect/disconnectAll/fireEvent copies, the Value classes' EventDescBase members, and the `rbx::signals::signal<...>::disconnectAll`, `rbx::callable` and `signal` helpers they call.

### 4. STUDIO EnumDesc<UploadSetting>::convertToValue 0x0073cfc0 (0.864): not fixed
- **Cause:** both GameSettings.obj and GameBasicSettings.obj instantiate it (GBS's VideoUploadPromptBehavior EnumProp uses the enum). The App glob compiles GameBasicSettings.cpp ahead of GameSettings.cpp, so our link keeps GBS.obj's copy, which calls `singleton()` out of line.
- **Possible fix (integrator, CMakeLists.txt):** put GameSettings.cpp ahead of GameBasicSettings.cpp in the App source list. PLAYER's copy already matches.

### 5. Selection and the GlobalAdvancedSettings ctor
- **Selection (`V8DataModel/Selection.h`)**
  - It is now `DescribedNonCreatable<Selection, Instance, sSelection>, public Service`, SIZE 0x90 (Creatable::create<Selection> 0x00656d40). The ctor stores Service::isPublic = 1.
  - Ctor and dtor are out of line, as STUBs: PLAYER 0x0086b070 / 0x00869e20, STUDIO 0x0079f000 / 0x0079dc50.
  - `removeFromSelection` is a STUB at 0x0086aa30 / 0x0079e860.
  - The dtor and removeFromSelection are `__declspec(noinline)` stand-ins, so the deleting dtor 0x00869fd0 calls the dtor out of line.
  - New first own virtual, `onAncestryChanged(Instance*)` at vtable+0x70: **FUNCTION 0x0086adb0 / 0x0079ebe0**.
  - Now at 100% (PLAYER): `ServiceProvider::find<Selection>` 0x00656dc0, `create<Selection>` 0x00656f10, the Selection D0, the Selection Debugable vftable 0xb7f104 and the NonFactoryProduct<Selection> vftables.
- **GlobalAdvancedSettings (GlobalAdvancedSettings.cpp)**
  - `advancedSettingsFile()` returns `"<user dir>\GlobalSettings_13.xml"`: **FUNCTION 0x00656320 / 0x0073ee60**.
  - **The ctor is FUNCTION 0x00658580 / 0x00740d10.** It calls `Settings(advancedSettingsFile())` through the forwarding ctors, then `setName("Global Settings")`, then `create<Selection>()`.
  - `Creatable::create<GlobalAdvancedSettings>` 0x00658750 also reached 100%.
- **Not reached:** the FriendService playerAdded/playerRemoving bodies (they need FriendService's layout: a map at +0x130/+0x134 and +0x128), the Game ctor, openState and DataModel's unnamed group.

## Checks
- `check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1696 files, 0 bad.
- `reccmp-decomplint`: clean in both modules on DebugSettings.cpp, FastLogSettings.cpp, GlobalAdvancedSettings.cpp, Selection.cpp, NetworkSettings.cpp, Selection.h and DebugSettings.h.
- `assert_lines`:
  - property.h now matches on every anchor.
  - DebugSettings.h's getRobloxVersion stays on 85.
  - GlobalSettings.h is unchanged.
  - Event.h still has the pre-existing sendEvent and getSignalPtr asserts off (159 vs 187, and 797 vs 1108).
- Every FUNCTION marker added this wave is verified at 100% in P3z / S2z.

## Unresolved
1. **Shell agent (Client/Win/placeholder.cpp or a real shell class):** add a FriendService className()/Creator stand-in whose TU spells `#include "util/object.h"` first. Then network can include `V8DataModel/FriendService.h` in Players.cpp. Measure it A/B.
2. **Network agent:**
   - Read `DFFlag::UseClusterCache`, declared with `DYNAMIC_FASTFLAG`.
   - The NetworkSettings names above.
   - `PhysicsService::numSenders()`.
3. **Integrator:**
   - The App source order for item 4 (GameSettings.cpp ahead of GameBasicSettings.cpp, STUDIO 0x0073cfc0).
   - Regenerate the shared CSVs. sSelection (PLAYER 0x00b7edd0 / STUDIO 0x00bfdce8) and the Selection/GAS helpers pair only after a regen.
4. **App, next:**
   - EventDescImpl fireEvent still differs in how it fetches the argument. 2013 calls a by-value Variant member: CFrame 0x0058f560 calls `any_cast<T>` by value at 0x0058ebd0 with a hidden return slot. For bool it copies the any_cast result through a stack temp. GBS fireEvent 0x00643ea0 is at 0.943 and the CFrame/Color3/Ray copies at 0.6-0.75. Do not change `Variant::cast` globally (about 660 Variant functions depend on it).
   - `Settings::verifyAddDescendant` 0x00656a80 (GAS vftable slot 0x38) needs InvalidDescendentDetector (0x006562c0 static invalid, visitor 0x006569a0).
   - FriendService layout and bodies, the Game ctor 0x00655e70, and openState.
5. **IDA owner (names):**
   - Selection::Selection, PLAYER 0x0086b070 (unnamed in PLAYER names; ios name only in STUDIO 0x0079f000).
   - DebugSettings::gfxcard 0x00659170 / 0x00773bf0 and videoMemory 0x00659910 / 0x00774340.
   - FLog::RobloxWndInit 0x00d30edd / 0x00ec17c9 and DFFlag::UseClusterCache 0x00de3f88 / 0x00f84e62.
   - advancedSettingsFile 0x00656320 / 0x0073ee60.
   - GlobalAdvancedSettings ctor STUDIO 0x00740d10.
6. **Types:** Selection is 0x90 (Service at +0x60).

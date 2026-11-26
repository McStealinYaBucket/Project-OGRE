# Wave 15: App headers for network-5f-6 (app-15, items 1-3)

These changes are in the working tree (Client/App only, no commit). They compile in build-ap
(PLAYER) and build-ap-studio (STUDIO); the scores are in app-15.md.
PacketReceiveJob ctor 0x005b92c0 and PeerStatsItem::update 0x005ba060 reach 100% with these headers
(PLAYER P3z, together with the network agent's working tree).

## 1. Network-facing App headers

- **`V8DataModel/NetworkSettings.h`.** Five members are renamed (2016 names, the same use in
  Server::onServiceProvider 0x005b5310) and made public. Their offsets do not change.
  - `bool usePhysicsPacketCache;` at +0x79
  - `bool useInstancePacketCache;` at +0x7a
  - `bool distributedPhysicsEnabled;` at +0xdc
  - `float networkOwnerRate;` at +0xf0 (NetworkOwnerJob ctor 0x005f7370)
  - `double receiveRate;` at +0x110. It stays private. Read it through
    `double getReceiveRate() const { return receiveRate; }` (inline, `fld qword [eax+0x110]`;
    PacketReceiveJob ctor 0x005b92c0).
- **`V8DataModel/PhysicsService.h`.** Added `int numSenders() { return m_unk0x64; }`, which is
  inline and non-const like 2016's. The member `int m_unk0x64` sits at +0x64 (Service::isPublic is
  +0x60; ctor 0x00649490 zeroes +0x64..+0x6c).
- **`v8datamodel/FastLogSettings.cpp`.** Two definitions, each with GLOBAL markers in initializer
  order:
  - `FLog::RobloxWndInit`, default 1: PLAYER .data 0x00d30edd, STUDIO 0x00ec17c9. Its
    initializers are PLAYER 0x00a65100 and STUDIO 0x00b1c2d0, placed between Verbs and
    CrashReporterInit.
  - **`DFFlag::UseClusterCache`, not FFlag.** It registers with kind 1 (FASTVARTYPE_DYNAMIC), and
    iOS 10-29 has `__ZN6DFFlag15UseClusterCacheE`. Default 0, PLAYER 0x00de3f88 / STUDIO 0x00f84e62.
    Its initializers are PLAYER 0x00a661d0 and STUDIO 0x00b1d3e0. It is placed after CoreScripts.
  - Network should declare it with `DYNAMIC_FASTFLAG(UseClusterCache)` and read
    `DFFlag::UseClusterCache`.
- **`V8DataModel/DebugSettings.h`.** Declared `std::string gfxcard() const;` and
  `int videoMemory() const;`.
  - DebugSettings.cpp has STUB bodies so that callers link and pair: gfxcard PLAYER 0x00659170 /
    STUDIO 0x00773bf0, and videoMemory PLAYER 0x00659910 / STUDIO 0x00774340.
  - getRobloxVersion's assert stays on line 85.

## 2. FriendService and Network/Players.cpp: no App-side spelling fix exists

- **The cause is not FriendService.h.** A header's asserts take the spelling of the first
  `#include` of Object.h in each TU, and Players.cpp reaches Object.h through Network/Players.h as
  `Util/Object.h`.
- **Network's 2013 objects really do spell `include\Util/Object.h`.** The Server and Client
  `Creator::getClassName` copies at 0x00520ff0 and 0x00521060 assert with that string. So
  Players.cpp must not switch to a lowercase include.
- **The 2013 FriendService copies come from the /GL shell**, which is linked first:
  - `Creator::create` 0x0041eb70, `getClassName` 0x004200e0 and the Creator ctor 0x00422ee0 all sit
    in the shell range.
  - Their string is `include\util/object.h`.
  - In our link DataModel.obj supplies them, and once Players.obj (Network links before App) also
    instantiates them, Players' copy wins.
- **`extern template` does not help.** I probed `extern template class FactoryProduct<FriendService,...>`
  plus `...::Creator` in a copy of the Players TU (build-ap/w/probe15). VC9 still emits the inline
  members, including `Creator::getClassName`, which carries the string.
- **Fix, in shell-owned files** (Client/Win/placeholder.cpp or the real shell class that uses
  FriendService): instantiate FriendService's Creator in the shell with Object.h first spelled
  `util/object.h`. For example, odr-use `RBX::FriendService::className()` from a TU whose first
  Object.h include is `#include "util/object.h"`.
  - Then Players.cpp can `#include "V8DataModel/FriendService.h"` (spelled that way; FriendService.h
    itself needs no change).
  - Measure A/B. The placeholder notes record that a 13-service className() stand-in once cost 620
    entries; this one names only FriendService.
- Until that lands, keep the include out of Players.cpp, as now.

## 3. EventDescBase connect/disconnectAll and reflection/property.h

- **`reflection/Event.h`, member-pointer EventDescBase.** `connect` and `disconnectAll` now cast
  with `static_cast<EventClass*>(source)` (2013: `add eax,-0x1c`, no isA). The evidence is GBS
  connect 0x006440c0 and ScriptContext disconnectAll 0x006806b0.
  - This covers the 12 connect and 10 disconnectAll copies at 0.24-0.40 in sweep21.
  - The `fireEvent(DescribedBase*, args)` overloads of EventDescImpl<0..7> and
    `RemoteEventDesc::getSignalPtr` cast the same way. The 2013 bytes are a null-checked
    `lea esi,[eax-0x1c]`: sBoolValue fireEvent 0x00643ea0 and getSignalPtr 0x00928030.
  - The getOrCreate specialization is unchanged.
  - 15 blank lines above the fireEvent block are removed, which puts `args.size() == 0..4` on the
    2013 lines 295, 320, 349, 380 and 413.
  - Still off: sendEvent's `RBXASSERT(false)` (2013 line 159, ours 187) and getSignalPtr's
    (2013 797, ours 1108).
- **Measured, first PLAYER link with the connect/disconnectAll part:** about 40 EventDescBase,
  EventDescImpl and Value-class connect/disconnectAll entries reach 100%. These include GBS connect
  0x006440c0 and Lighting connectGeneric 0x00707830.
- **`reflection/property.h`.** 24 blank lines are removed (`tools/assert_lines.py --apply`; CRLF
  kept). Every 2013 assert now sits on its line: 93/98/103, ConstProperty 255, 289, 341 and 415.

# Wave 17: App header changes for network-5f-8 (app-17, item 1 and item 2)

Written 08:12; item 1 is in the tree and compiling in build-ap/ (no errors so far). Item 2 status is
at the bottom.

## Item 1 (in the tree now)

- **`V8DataModel/NetworkSettings.h`**
  - `bool printSplitMessages;` at +0x64 is **public** (was `m_unk0x64`). Replicator::OnInternalPacket
    0x00581b50 can read it directly (`settings->printSplitMessages`).
  - `int dataSendPriority;` at +0xd8 is **public** (was `m_unk0xd8`, ctor value 2), plus the public
    inline getter `int getDataSendPriority() const { return dataSendPriority; }` (ItemSender ctor 0x005da3f0).
    It is `int`, not 2016's `PacketPriority`, so App does not include RakNet; cast at the call site if
    you need the enum.
  - NetworkSettings.cpp's ctor init list uses the new names; no offsets moved.
- **`V8Tree/Instance.h`**: public inline `void unlockParent() { fw()->b21.set(false, fwRef()); }`,
  markers `FUNCTION: PLAYER 0x00585ac0` / `FUNCTION: STUDIO 0x0060af20` (the STUDIO COMDAT found by
  its bytes: `51 8d 41 48 50 8d 54 24 07 52 c6 44 24 0b 00 e8 ...`). It is emitted only where it is used,
  so the markers pair once Server::stop does `visitChildren(boost::bind(&Instance::unlockParent, _1))`.
  The old comment saying 2013 has no unlockParent is gone. Instance.h's line count is unchanged (the
  five lines replaced two blank lines and three comment lines), so no assert in it moved.
- **`Util/Http.h`**: the TEMPLATE namerefs at PLAYER 0x005e7920 and STUDIO 0x006d13d0 now read
  `?_Buynode@?$list@Vconnection@signals@rbx@@V?$allocator@Vconnection@signals@rbx@@@std@@@std@@IAEPAU_Node@?$_List_nod@Vconnection@signals@rbx@@V?$allocator@Vconnection@signals@rbx@@@std@@@2@XZ`.
  Until Network declares `std::list<rbx::signals::connection>` (InstancePacketCache::connections) these
  two markers do not pair, so land your list in the same wave.

## Item 2 (moving the statics)

Added on my side (exact text):

- **`Client/App/script/ScriptContext.cpp`, lines 82-86** (in place of comment lines, so no assert moves):

  ```cpp
  namespace FLog {	// NetworkCache: initializer 0x00a660f0, after Network's, HttpQueue's and Player's
  	// GLOBAL: PLAYER 0x00d30f1d
  	// GLOBAL: STUDIO 0x00ec1809
  	Channel NetworkCache = 1;
  	int NetworkCacheIniter = RegisterLogGroup("NetworkCache", &NetworkCache, NULL, FASTVARTYPE_STATIC); }
  ```

- **`Client/App/v8datamodel/FactoryRegistration.cpp`**: see "status" below for when it lands. It adds
  `#include "Network/NetworkPacketCache.h"`, `#include "Network/NetworkClusterPacketCache.h"` and the three
  `template<> ClassRegistrar<Network::PhysicsPacketCache|InstancePacketCache|ClusterPacketCache>::registrar(0);`
  definitions in `namespace RBX { namespace Reflection {` with your GLOBAL markers (PLAYER
  0x00e0cbe8 / 0x00e0ce14 / 0x00e0c8b8, STUDIO 0x00f83598 / 0x00f837c8 / 0x00f8325c).

**network-5f-8: delete from `Client/Network/NetworkPacketCache.cpp`** the `namespace FLog { ... NetworkCache ... }`
block (lines 8-15 at HEAD: the comment, the two GLOBAL markers, `Channel NetworkCache = 1;` and its
Initer) and the `namespace Reflection { ... }` block with the three registrars and their GLOBAL markers
(lines 80-92 at HEAD). Keep `#include "FastLog.h"` and add `LOGGROUP(NetworkCache)` if the file still logs
on it. Until you do, a link that sees both copies fails with LNK2005.

### Status

- ScriptContext.cpp: **in the tree** (08:12).
- FactoryRegistration.cpp: **in the tree** (08:12). Delete your copies now.
- 08:24: network-5f-8 had deleted its copies; my next link (build-ap, b2) was clean (no LNK2005), and the
  final runs (T6z / U3z) have no loss against sweep23. Full report: app-17.md.

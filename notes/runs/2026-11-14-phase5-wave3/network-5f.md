# network-5f (phase5-wave3, 2026-11-14)

PHASE 5f (Roblox Network library). Both targets build and link in build-net/ and build-net-studio/. Scored PLAYER twice (scratchpad scoreP1, scoreP2) and STUDIO once (scoreS1), against build/sweep8/P-final.json and build-studio/sweep8/S-final.json. The scored exe/pdb/map copies under scratchpad/scored-i8/ had been deleted, so every comparison is by original address only.

LEFTOVERS (previous attempts): all kept. The only Network-range losses are CSV pairing or twin effects, read with --verbose; none is a code regression.
- Client::Client and ~Client, Peer::Peer and ~Peer, and Server::serverIsPresent produce the same instructions as the original. They miss 100% only because the CSVs still use the old symbol names (Client used to derive from Instance, now Peer). Scores: ~Client 82.7/84.6%, Peer 81.4%, ~Peer 90.8%, serverIsPresent 98.5% (its call to find<Server> has no name in the CSV).
- Three Client functions that were at 100% lost their pairing for the same reason: FactoryProduct<Client>::static_getCreator 0x00520e10, getClassName 0x005ac3f0, and Players::isNetworkClient 0x0056b8c0 (now 96.7% in both targets). I rewrote those rows to the new FactoryProduct<Client,Peer,...> names and added a classDescriptor row in a private CSV copy: isNetworkClient returns to 100% in both targets. static_getCreator is still 90.5% there because its isConstructed/creatorPrivate data rows also need the new names.
- BitStream::Read<unsigned int> 0x005a7130 (STUDIO 0x00616d50) went from 100% to unpaired. The original keeps two copies, one in RakNet and one in Streaming.obj; we have one body, so the Streaming operator>> for long/int/unsigned long/float stay at 96.9%. DatagramHeaderFormat::Deserialize 0x005c94f0 dropped to 99.2% for the same reason.
- Markers set to what was measured. PLAYER backendProcessing 0x005689c0 became FUNCTION and ~ItemQueue 0x005d42a0 became STUB (97.5%). STUDIO backendProcessing 0x005ccbc0 became FUNCTION. 15 STUDIO markers the earlier attempt had set to FUNCTION measured below 100% and are now STUB (pairing-only in every case read): ~Client 0x0060c1a0, Compressor 0x00686400/0x00686600, ~ItemQueue 0x0066d630, Peer 0x0064b710/0x0064ab60, serverIsPresent 0x006134f0, and Streaming 0x00616aa0, 0x00616e70, 0x00616ff0, 0x00617070, 0x00617210, 0x00617370, 0x00617420, 0x00617f30.
- Now at 100% (PLAYER, STUDIO the same): serializeEnum 0x005a6670, deserializeEnum, serializeEnumProperty, deserializeEnumProperty, readBrickVector 0x005a7bd0, 10 BitStream operators, 8 ItemQueue methods 0x005d3e20 to 0x005d4260, Compressor::readCompressionType 0x005ece30, Client::findClient 0x005adbd0, Players::clientIsPresent 0x005adc70 (with Client::clientIsPresent FOLDED onto it), Players::backendProcessing.

NEW CODE (this run):
- Item.h gains PooledItem (Item + AutoMemPool::Object, vftable PLAYER 0x00b17c20 / STUDIO 0x00bd229c, SIZE 0x20).
- New files Replicator.PingBackItem/PingItem/MarkerItem .h/.cpp. The Mac symbols show the 2013 constructors take (Replicator*, unsigned long long time) for the two ping items and (Replicator*, long) for MarkerItem; 2016's extraStats argument does not exist yet.
  - Constructors PLAYER 0x005e9e40, 0x005ecb20, 0x005eccc0 (STUDIO 0x00681f50, 0x00684b50, 0x00684cf0) produce the same instructions as the original. With the shared CSVs they score 91.7–92.9%: no ??_7 vftable row exists for nested classes yet. With those rows in a private CSV copy all six are 100%, so they are marked FUNCTION.
  - VTABLE markers on the nested classes are left out on purpose: reccmp names a nested class without its outer class.
  - The write() methods are STUBs (VMProtect-mutated in PLAYER).
- Replicator.h now declares ReplicatorJob : DataModelJob (no members yet) and a Replicator shell with nested-class declarations only (no members, no SIZE). New Replicator.PingJob.h; Replicator.cpp holds PingJob::sleepTime and error at 2 Hz (2013 calls computeStandardError directly). Both measured at 100% with the shared CSVs: PLAYER 0x00598da0/0x00598dd0, STUDIO 0x0063be70/0x0063bea0, marked FUNCTION. stepDataModelJob is a STUB (0x0059ea20, VMProtect).
- PooledItem's deleting destructor (PLAYER 0x005e9e70 / STUDIO 0x00632650, shared by every pooled item) produces the same instructions as the original (ours at 0x0045e180). reccmp would not pair a SYNTHETIC marker on it, with either the demangled or the decorated name ("Failed to find a match"), so it carries a comment instead of a marker.
- Gains in the second PLAYER run against the first: PingJob sleepTime and error. The one PLAYER loss in the Network range, RakNet IsOlderOrderedPacket 0x005c8930 (95%), comes from an immediate that now equals our Team RTTI address (a no-.reloc coincidence from another agent's change). reccmp_addr_padding: 0 bad markers.

ROW 5f (tools/row_coverage.py --row 5f, which already existed): before 422/3257 = 13.0% (baseline); after 451/3259 = 13.8%. Regenerating the CSVs would add the 3 item constructors and isNetworkClient, and probably the Client/Peer pairing rows (not measured).

CLASSES: 30 RBX::Network classes have RTTI in 2013, plus 26 nested Replicator/ClientReplicator/ServerReplicator classes.
- 2013 only (absent from the 2016 Network folder): ChatFilter, ClusterPacketCache, IdSerializer, InstancePacketCache, PacketReceiveJob, PeerStatsItem, PhysicsPacketCache, ProfiledRakPeer, RakStatsItem, Replicator::SendClusterJob, ClientReplicator::MemoryCheckerJob/MemoryCheckerCheckerJob/ClientCapacityUpdateItem.
- 2016 only (no symbol or RTTI in PLAYER): Replicator::HashItem, TagItem, Rijndael/DataBlockEncryptor, MovementHistoryJob, NetworkProfiler, GamePerfMonitor. Replicator::RockyItem exists in 2013.

LAYOUTS proven this run: Item 0x20 (ItemQueue's 0x10 proven by the 8 matched methods), PooledItem 0x20, PingBackItem/PingItem/MarkerItem 0x28. The Peer 0xac0 constructor and destructor are code-identical in both targets; only pairing keeps them below 100%. Scouted but not written:
- Players: size 0x1f8, ctor 0x00570980, dtor 0x005705c0, about 30 members (our header 0x5c).
- Player: size 0x1d8, ctor/dtor 0x00553fa0 / 0x00550d40, which is which not settled (our header 0x5c).
- Replicator: ctor 0x005a5e70 (1670 bytes), dtor 0x005a55a0 (2231 bytes), 75 vftable slots against iOS's 84.
- ServerReplicator: size 0x2bc8, ctor 0x0057c3b0, dtor 0x00578f20.
- ClientReplicator: ctor 0x005bfd70, dtor 0x005bee00.
- NetworkSettings: size 0x120, ctor 0x005b7cf0 (our header 0x78).

Coverage CSVs (data only): notes/runs/2026-11-14-phase5-wave3/5f-coverage-{before,after}.csv.

## Files

- /home/thebigw/Documents/Project OGRE/Client/Network/Item.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Item.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.PingJob.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.PingBackItem.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.PingBackItem.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.PingItem.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.PingItem.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.MarkerItem.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Replicator.MarkerItem.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Players.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Client.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/include/Network/Client.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Server.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Peer.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Peer.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Compressor.h
- /home/thebigw/Documents/Project OGRE/Client/Network/Compressor.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/Streaming.cpp
- /home/thebigw/Documents/Project OGRE/Client/Network/streaming.h
- /home/thebigw/Documents/Project OGRE/notes/runs/2026-11-14-phase5-wave3/5f-coverage-before.csv
- /home/thebigw/Documents/Project OGRE/notes/runs/2026-11-14-phase5-wave3/5f-coverage-after.csv

## Unresolved

- Integrate: regenerate the CSVs from a fresh build. Client now derives from Peer, so every FactoryProduct<Client,Instance,...> row is stale. PLAYER: 0x00520e10 static_getCreator, 0x005ac3f0 getClassName, 0x005261b0 classDescriptor, and the isConstructed/creatorPrivate/??_B globals. STUDIO: 0x005bad50, 0x005bc740, 0x005bd4f0, 0x005cc7b0, 0x0060afa0, 0x00f5a328, 0x00f5b04c, 0x00f5b2c8, plus the ~Client row 0x0060c1a0. Measured with a private copy: isNetworkClient (0x0056b8c0 / 0x005d0d70) returns to 100%.
- Integrate: add ??_7 vftable rows for the nested item classes: PLAYER PingBackItem 0x00b2611c, PingItem 0x00b26b1c, MarkerItem 0x00b26b70; STUDIO 0x00bddfe8, 0x00bde9ec, 0x00bdea40. With them the six item constructors (already marked FUNCTION) measure 100%. The PooledItem vftable 0x00b17c20 row is sized 12 instead of 8 (2 slots).
- PooledItem deleting destructor PLAYER 0x005e9e70 / STUDIO 0x00632650 is code-identical (ours 0x0045e180, named ??_GPingBackItem after folding) but reccmp will not pair a SYNTHETIC marker on it, with either nameref. A CSV row naming it would fix it.
- Twin limit: the original keeps two copies of BitStream::Read<unsigned int> (RakNet and Streaming.obj, PLAYER 0x005a7130 / STUDIO 0x00616d50). We have one body, so operator>> for long/int/unsigned long/float stay at 96.9% and DatagramHeaderFormat::Deserialize 0x005c94f0 at 99.2%.
- Pairing-only misses (same instructions as the original): Client::Client 0x005b0880 90.6% (it also lacks the GlobalAdvancedSettingsItem<NetworkSettings> singleton store, 0x00521d60), ~Client 0x005ad760, Peer 0x005b9ac0 / ~Peer 0x005b8bf0 (aesKey, lerp, registrar and vftable rows), Server::serverIsPresent 0x005b34f0 (find<Server> 0x005641d0 has no name in the CSV), ~ItemQueue 0x005d42a0 (clear_and_dispose has no name), Compressor write/readTranslation 99.4/98.8%.
- Central layouts not written yet: Players 0x1f8 (ctor 0x00570980 / dtor 0x005705c0, about 30 members, ours 0x5c); Player 0x1d8 (0x00553fa0 / 0x00550d40); Replicator (0x005a5e70 / 0x005a55a0, 75 slots); ServerReplicator 0x2bc8 (0x0057c3b0 / 0x00578f20); ClientReplicator (0x005bfd70 / 0x005bee00); NetworkSettings 0x120 (0x005b7cf0, ours 0x78, file is in Client/App/v8datamodel). Replicator.h and ReplicatorJob are member-less shells until those constructors are written.
- Every write() of the Replicator items, PingJob::stepDataModelJob, and other job step functions are VMProtect-mutated in PLAYER and cannot match there. STUDIO bodies are plain but need Replicator's members.
- Not mine: another agent's change created a no-.reloc coincidence that dropped RakNet ReliabilityLayer::IsOlderOrderedPacket 0x005c8930 to 95% (an immediate equals our Team RTTI COL). The STUDIO run also lost several sp_pointer_construct and copy_on_write_ptr bodies from Base and GUI header changes (the STUDIO total fell 7687 -> 7455).

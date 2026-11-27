#pragma once

// Network/ServerReplicator.h -- October 2013, partial (RTTI DescribedNonCreatable<ServerReplicator,
// Replicator, sServerReplicator>, vftable PLAYER 0x00b1848c / STUDIO 0x00bd7404, classDescriptor PLAYER
// 0x00525ea0). It overrides 34 slots of the Debugable-primary vftable; the ones declared below are
// written. Members start at sizeof(Replicator) (PLAYER 0x2aa8, STUDIO 0x2328) with the same layout in
// both targets (STUDIO offsets are PLAYER's - 0x780). Offsets from the ctor PLAYER 0x0057c3b0 (STUDIO
// 0x0065ad00); names are 2016's where a written function proves the use.

#include "Replicator.h"

namespace RBX {
namespace Network {

extern const char* const sServerReplicator;

class ServerReplicator : public DescribedNonCreatable<ServerReplicator, Replicator, sServerReplicator, Reflection::ClassDescriptor::INTERNAL_LOCAL>
{
	typedef Replicator Super;

	undefined m_unk0x2aa8[0x2af8 - 0x2aa8];   // +0x00 2016 PropSync::Master propSync (ctor 0x005becb0)
	undefined4 m_unk0x2af8;                   // +0x50 ctor 0
	bool acceptsTerrainChanges;               // +0x54 ctor true; receiveCluster 0x005718b0
	undefined m_unk0x2afd[0x2b3c - 0x2afd];   // +0x55 descriptors +0x58..+0x64, std::string +0x68, shared_ptr<Player> remotePlayer +0x8c
	int remoteProtocolVersion;                // +0x94 ctor 0x12; canUseProtocolVersion 0x00571870
	undefined4 m_unk0x2b40;                   // +0x98
	bool waitingForMarker;                    // +0x9c ctor true; sendItemsPacket 0x00571a90, onSentMarker clears it

protected:
	/*override*/ bool canUseProtocolVersion(int protocolVersion) const;                 // vtable+0x9c
	/*override*/ bool sendItemsPacket();                                                // vtable+0xf8
	/*override*/ void receiveCluster(RakNet::BitStream& inBitstream, Instance* instance); // vtable+0x118
	/*override*/ void serializeSFFlags(RakNet::BitStream& outBitStream);              // vtable+0x120
};

} // namespace Network
} // namespace RBX

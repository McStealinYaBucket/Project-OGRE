#pragma once

// Network/Replicator.StreamJob.h -- October 2013, partial: only the members ServerReplicator's written
// functions call (iOS names). Replicator+0x2184 holds it. 2016 derives it from ReplicatorJob (its
// `replicator` at +0x218, which readInstanceRemoval reads) and a second base; the members after
// ReplicatorJob are not declared yet (ctor PLAYER 0x005d2220, STUDIO 0x006709d0). Offsets are the same
// in both targets (readInstanceRemoval PLAYER 0x005d29f0 / STUDIO 0x006711a0).

#include "Replicator.h"

namespace RBX {
namespace Network {

class Replicator::StreamJob : public ReplicatorJob
{
	undefined m_unk0x220[0x2a5 - 0x220];       // 0x220 not declared yet
	bool readyToWork;                          // 0x2a5 (2016 name) ServerReplicator::onSentMarker 0x00572270; stepDataModelJob 0x005d2a20 tests it

public:
	void setReady(bool isReady) { readyToWork = isReady; }

	void sendPackets(int maxPackets);                         // PLAYER 0x005cff10
	void updateClientQuota(int diff, short maxRegionRadius);  // PLAYER 0x005d1310
	void receiveInstanceGcMessage(const Guid::Data& id);      // PLAYER 0x005d25f0
	void readRegionRemoval(RakNet::BitStream& bitStream);     // PLAYER 0x005d2860
	void readInstanceRemoval(RakNet::BitStream& bitStream);   // PLAYER 0x005d29f0
};

} // namespace Network
} // namespace RBX

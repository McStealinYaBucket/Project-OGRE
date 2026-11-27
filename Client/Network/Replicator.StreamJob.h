#pragma once

// Network/Replicator.StreamJob.h -- October 2013, partial: only the members ServerReplicator's written
// functions call (iOS names). The 2013 class is a job (Replicator+0x2184 holds it); its base and
// layout are not declared yet.

#include "Replicator.h"

namespace RBX {
namespace Network {

class Replicator::StreamJob
{
public:
	void sendPackets(int maxPackets);                         // PLAYER 0x005cff10
	void readRegionRemoval(RakNet::BitStream& bitStream);     // PLAYER 0x005d2860
	void readInstanceRemoval(RakNet::BitStream& bitStream);   // PLAYER 0x005d29f0
};

} // namespace Network
} // namespace RBX

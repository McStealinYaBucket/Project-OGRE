#pragma once

#include "decomp.h"

// Network/ConcurrentRakPeer.h -- October 2013, partial: only what Peer calls so far.
// rawPeer is out of line in 2013 (PLAYER 0x005c3a10, STUDIO 0x0061e8d0) and asserts at
// ConcurrentRakPeer.cpp line 257 before returning the wrapped peer. Only the raw peer at +0 is
// placed (ctor ConcurrentRakPeer(RakPeerInterface*, DataModel*) PLAYER 0x005c6770 and dtor 0x005c4330
// are still STUBs).

#include "RakPeerInterface.h"

namespace RBX {
	class DataModel;
}

namespace RBX {
namespace Network {

	// SIZE 0x1c (operator new in Peer::onServiceProvider 0x005ba1d0)
	class ConcurrentRakPeer
	{
		RakNet::RakPeerInterface* peer;              // 0x00 PacketReceiveJob::stepDataModelJob 0x005b9560 reads it inline
		undefined m_unk0x04[0x1c - 0x04];            // 0x04
	public:
		ConcurrentRakPeer(RakNet::RakPeerInterface* peer, DataModel* dataModel);
		~ConcurrentRakPeer();
		RakNet::RakPeerInterface* rawPeer();
		// Inline: 0x005b95f6 calls the raw peer's DeallocatePacket (slot +0x60) without rawPeer().
		void DeallocatePacket(RakNet::Packet* packet) { peer->DeallocatePacket(packet); }
	};

} // namespace Network
} // namespace RBX

#pragma once

// Network/ConcurrentRakPeer.h -- October 2013, partial: only what Peer calls so far.
// rawPeer is out of line in 2013 (PLAYER 0x005c3a10, STUDIO 0x0061e8d0) and asserts at
// ConcurrentRakPeer.cpp line 257 before returning the wrapped peer; the members are not
// reconstructed yet (ctor ConcurrentRakPeer(RakPeerInterface*, DataModel*) PLAYER 0x005c6770,
// operator new(0x1c) in Peer::onServiceProvider 0x005ba1d0).

namespace RakNet {
	class RakPeerInterface;
}

namespace RBX {
namespace Network {

	class ConcurrentRakPeer
	{
	public:
		RakNet::RakPeerInterface* rawPeer();
	};

} // namespace Network
} // namespace RBX

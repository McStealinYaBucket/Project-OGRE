// Network/ConcurrentRakPeer.cpp -- October 2013, partial (see ConcurrentRakPeer.h).

#include "ConcurrentRakPeer.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x005c3a10
// STUB: STUDIO 0x0061e8d0
RakNet::RakPeerInterface* ConcurrentRakPeer::rawPeer()
{
	STUB(0x005c3a10);
	return 0;
}

// STUB: PLAYER 0x005c4330
// STUB: STUDIO 0x0061f1f0
ConcurrentRakPeer::~ConcurrentRakPeer()
{
	STUB(0x005c4330);
}

// iOS: GetBufferHealth(). PeerStatsItem::update 0x005ba060 calls it.
// STUB: PLAYER 0x005c5070
// STUB: STUDIO 0x006201b0
double ConcurrentRakPeer::GetBufferHealth()
{
	STUB(0x005c5070);
	return 0;
}

// STUB: PLAYER 0x005c6580
// STUB: STUDIO 0x006217e0
void ConcurrentRakPeer::addStats(RakNet::SystemAddress address, boost::function<void(const ConcurrentRakPeerStats&)> callback)
{
	STUB(0x005c6580);
}

// STUB: PLAYER 0x005c6770
// STUB: STUDIO 0x006219d0
ConcurrentRakPeer::ConcurrentRakPeer(RakNet::RakPeerInterface* peer, DataModel* dataModel)
{
	STUB(0x005c6770);
}

// FUNCTION: PLAYER 0x005c6c80
// FUNCTION: STUDIO 0x0064c380
std::string RakNetAddressToString(const RakNet::SystemAddress& raknetAddress, bool writePort, char portDelineator)
{
	char buffer[32];
	raknetAddress.ToString(writePort, buffer, portDelineator);
	return buffer;
}

} // namespace Network
} // namespace RBX

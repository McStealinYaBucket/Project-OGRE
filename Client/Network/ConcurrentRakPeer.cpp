// Network/ConcurrentRakPeer.cpp -- October 2013, partial (see ConcurrentRakPeer.h).

#include "ConcurrentRakPeer.h"
#include "V8DataModel/DataModel.h"
#include "rbx/TaskScheduler.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// Blank lines: rawPeer's assert sits on its 2013 line (257).















































































































































































































































// STUDIO reads write_requested at DataModel+0xc9c (its DataModel lacks networkStatsWindowsOn); this
// tree's DataModel.h has PLAYER's layout only, so STUDIO stays a STUB.
// FUNCTION: PLAYER 0x005c3a10
// STUB: STUDIO 0x0061e8d0
RakNet::RakPeerInterface* ConcurrentRakPeer::rawPeer()
{
	RBXASSERT(dataModel->write_requested);
	return peer.get();
}

// FUNCTION: PLAYER 0x005c4330
// FUNCTION: STUDIO 0x0061f1f0
ConcurrentRakPeer::~ConcurrentRakPeer()
{
	TaskScheduler::singleton().remove(packetJob);
	TaskScheduler::singleton().remove(statsUpdateJob);
}

// iOS: GetBufferHealth(). PeerStatsItem::update 0x005ba060 calls it.
// FUNCTION: PLAYER 0x005c5070
// FUNCTION: STUDIO 0x006201b0
double ConcurrentRakPeer::GetBufferHealth()
{
	boost::mutex::scoped_lock lock(statsUpdateJob->mapMutex);
	return statsUpdateJob->bufferHealth;
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

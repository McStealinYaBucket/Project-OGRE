// Network/ConcurrentRakPeer.cpp -- October 2013, partial (see ConcurrentRakPeer.h).

#include "ConcurrentRakPeer.h"
#include "V8DataModel/DataModel.h"
#include "rbx/TaskScheduler.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// Blank lines: rawPeer's assert sits on its 2013 line (257).















































































































































































































































// STUDIO reads write_requested at DataModel+0xc9c (its DataModel lacks networkStatsWindowsOn); 100% in
// both since DataModel.h took the per-target layout (wave 17).
// FUNCTION: PLAYER 0x005c3a10
// FUNCTION: STUDIO 0x0061e8d0
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

// FUNCTION: PLAYER 0x005c5360
// FUNCTION: STUDIO 0x00620510
void ConcurrentRakPeer::Send(boost::shared_ptr<const RakNet::BitStream> data, PacketPriority priority,
	PacketReliability reliability, char orderingChannel, RakNet::SystemAddress systemIdentifier, bool broadcast)
{
	PacketJob::SendData sendData = { data, priority, reliability, orderingChannel, systemIdentifier, broadcast };
	packetJob->sendQueue.push(sendData);
	TaskScheduler::singleton().reschedule(packetJob);
}

// STUB: PLAYER 0x005c5610
// STUB: STUDIO 0x00620870
ConcurrentRakPeer::PacketJob::PacketJob(boost::shared_ptr<RakNet::RakPeerInterface> peer, DataModel* dataModel)
	: DataModelJob("Net Peer Send", DataModelJob::RaknetPeer, false, shared_from(dataModel), Time::Interval(0))
{
	STUB(0x005c5610);
}

// FUNCTION: PLAYER 0x005c5790
// FUNCTION: STUDIO 0x006209f0
Time::Interval ConcurrentRakPeer::PacketJob::sleepTime(const Stats& stats)
{
	return sendQueue.empty() ? Time::Interval::max() : Time::Interval::zero();
}

// STUB: PLAYER 0x005c5900
// STUB: STUDIO 0x00620b60
TaskScheduler::Job::Error ConcurrentRakPeer::PacketJob::error(const Stats& stats)
{
	STUB(0x005c5900);
	return Error();
}

// STUB: PLAYER 0x005c5aa0
// STUB: STUDIO 0x00620d00
TaskScheduler::StepResult ConcurrentRakPeer::PacketJob::stepDataModelJob(const Stats& stats)
{
	STUB(0x005c5aa0);
	return TaskScheduler::Stepped;
}

// STUB: PLAYER 0x005c5f60
// STUB: STUDIO 0x006211c0
ConcurrentRakPeer::StatsUpdateJob::StatsUpdateJob(boost::shared_ptr<RakNet::RakPeerInterface> peer, DataModel* dataModel)
	: DataModelJob("Net Peer Stats", DataModelJob::RaknetPeer, false, shared_from(dataModel), Time::Interval(0))
{
	STUB(0x005c5f60);
}

// Folded with ContentProviderJob::sleepTime.
// FUNCTION: PLAYER 0x008ef650 FOLDED
// FUNCTION: STUDIO 0x00a0d3d0 FOLDED
Time::Interval ConcurrentRakPeer::StatsUpdateJob::sleepTime(const Stats& stats)
{
	return computeStandardSleepTime(stats, 30);
}

// FUNCTION: PLAYER 0x005c61a0
// FUNCTION: STUDIO 0x00621400
TaskScheduler::Job::Error ConcurrentRakPeer::StatsUpdateJob::error(const Stats& stats)
{
	return computeStandardError(stats, 30);
}

// STUB: PLAYER 0x005c6240
// STUB: STUDIO 0x006214a0
TaskScheduler::StepResult ConcurrentRakPeer::StatsUpdateJob::stepDataModelJob(const Stats& stats)
{
	STUB(0x005c6240);
	return TaskScheduler::Stepped;
}

// STUB: PLAYER 0x005c6580
// STUB: STUDIO 0x006217e0
void ConcurrentRakPeer::addStats(RakNet::SystemAddress address, boost::function<void(const ConcurrentRakPeerStats&)> callback)
{
	STUB(0x005c6580);
}

// FUNCTION: PLAYER 0x005c6770
// FUNCTION: STUDIO 0x006219d0
ConcurrentRakPeer::ConcurrentRakPeer(RakNet::RakPeerInterface* peer, DataModel* dataModel)
	: peer(peer)
	, dataModel(dataModel)
{
	packetJob.reset(new PacketJob(this->peer, dataModel));
	statsUpdateJob.reset(new StatsUpdateJob(this->peer, dataModel));
	TaskScheduler::singleton().add(packetJob);
	TaskScheduler::singleton().add(statsUpdateJob);
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

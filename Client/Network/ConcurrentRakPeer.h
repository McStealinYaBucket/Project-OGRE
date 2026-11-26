#pragma once

#include "decomp.h"

// Network/ConcurrentRakPeer.h -- October 2013, partial. rawPeer is out of line in 2013 (PLAYER
// 0x005c3a10, STUDIO 0x0061e8d0) and asserts dataModel->write_requested at ConcurrentRakPeer.cpp line
// 257. Layout from the dtor (0x005c4330): three shared_ptrs (peer, packetJob, statsUpdateJob; the
// dtor removes both jobs from the TaskScheduler first) and the DataModel* rawPeer reads. The ctor
// (PLAYER 0x005c6770) is still a STUB.

#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "V8DataModel/DataModelJob.h"
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <string>

namespace RBX {
	class DataModel;
}

namespace RBX {
namespace Network {

	// iOS: RakNetAddressToString(RakNet::SystemAddress const&, bool, char) (PLAYER 0x005c6c80).
	std::string RakNetAddressToString(const RakNet::SystemAddress& raknetAddress, bool writePort = true, char portDelineator = '|');

	// 0xf0 bytes: Replicator::onStatisticsChanged (PLAYER 0x005804f0) copies it with 0x3c dwords.
	// 8-aligned through RakNetStatistics: Replicator is 0x2aa8 (ServerReplicator's members start at
	// 0x2aa8, ctor 0x0057c3b0) and STUDIO pads 0x129c before its ReplicatorStats.
	struct ConcurrentRakPeerStats
	{
		int mtuSize;                           // 0x00 Replicator::getAdjustedMtuSize 0x00581c10
		undefined m_unk0x04[0x10 - 0x04];      // 0x04 2016: averagePing, lastPing, lowestPing
		RakNet::RakNetStatistics rakStats;     // 0x10 Replicator::getRakStatsString 0x00583a60
	};

	// SIZE 0x1c (operator new in Peer::onServiceProvider 0x005ba1d0)
	class ConcurrentRakPeer
	{
	public:
		class PacketJob;
		class StatsUpdateJob;
	private:
		boost::shared_ptr<RakNet::RakPeerInterface> peer;   // 0x00 PacketReceiveJob::stepDataModelJob 0x005b9560 reads it inline
		boost::shared_ptr<PacketJob> packetJob;             // 0x08
		boost::shared_ptr<StatsUpdateJob> statsUpdateJob;   // 0x10 GetBufferHealth 0x005c5070
		DataModel* dataModel;                               // 0x18 rawPeer 0x005c3a10
	public:
		ConcurrentRakPeer(RakNet::RakPeerInterface* peer, DataModel* dataModel);
		~ConcurrentRakPeer();
		RakNet::RakPeerInterface* rawPeer();
		double GetBufferHealth();
		// mac: addStats(RakNet::SystemAddress, boost::function<void (ConcurrentRakPeerStats const&)>)
		void addStats(RakNet::SystemAddress address, boost::function<void(const ConcurrentRakPeerStats&)> callback);
		// Inline: 0x005b95f6 calls the raw peer's DeallocatePacket (slot +0x60) without rawPeer().
		void DeallocatePacket(RakNet::Packet* packet) { peer->DeallocatePacket(packet); }
	};

	// RTTI ConcurrentRakPeer::PacketJob : DataModelJob. Members and virtuals not written.
	class ConcurrentRakPeer::PacketJob : public DataModelJob
	{
	};

	// RTTI ConcurrentRakPeer::StatsUpdateJob : DataModelJob. Members from GetBufferHealth 0x005c5070
	// (2016 names); the rest is unproven.
	class ConcurrentRakPeer::StatsUpdateJob : public DataModelJob
	{
	public:
		boost::mutex mapMutex;                              // 0x218
		undefined m_unk0x220[0x268 - 0x220];                // 0x220
		double bufferHealth;                                // 0x268 2016: RunningAverage<> (value() inline)
	};

} // namespace Network
} // namespace RBX

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
#include "rbx/threadsafe.h"
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
		// iOS signature. Copies the arguments into a 0x2c-byte packet record, pushes it onto the
		// PacketJob's queue (+0x218, 0x005c50e0) and wakes the job through the TaskScheduler.
		void Send(boost::shared_ptr<const RakNet::BitStream> data, PacketPriority priority,
			PacketReliability reliability, char orderingChannel, RakNet::SystemAddress systemIdentifier,
			bool broadcast);
		// mac: addStats(RakNet::SystemAddress, boost::function<void (ConcurrentRakPeerStats const&)>)
		void addStats(RakNet::SystemAddress address, boost::function<void(const ConcurrentRakPeerStats&)> callback);
		// Inline: 0x005b95f6 calls the raw peer's DeallocatePacket (slot +0x60) without rawPeer().
		void DeallocatePacket(RakNet::Packet* packet) { peer->DeallocatePacket(packet); }
	};

	// RTTI ConcurrentRakPeer::PacketJob : DataModelJob, vftable PLAYER 0x00b207b8 (operator new(0x248) in the
	// ConcurrentRakPeer ctor 0x005c6770). Members (2016 names): sleepTime 0x005c5790 tests the queue's deque
	// size at +0x234; error 0x005c5900 reads the queue (0x005c5830), its size and the bool at +0x23c.
	// SIZE 0x248
	class ConcurrentRakPeer::PacketJob : public DataModelJob
	{
	public:
		// 0x2c bytes, built field by field in Send 0x005c5360.
		struct SendData
		{
			boost::shared_ptr<const RakNet::BitStream> bitStream; // 0x00
			PacketPriority priority;                      // 0x08
			PacketReliability reliability;                // 0x0c
			char orderingChannel;                         // 0x10
			RakNet::SystemAddress systemAddress;          // 0x14
			bool broadcast;                               // 0x28
		};
		rbx::timestamped_safe_queue<SendData> sendQueue;  // 0x218 (deque size at 0x234, spin_mutex 0x238)
		bool isQueueErrorComputed;                          // 0x23c
		boost::weak_ptr<RakNet::RakPeerInterface> peer;     // 0x240

		PacketJob(boost::shared_ptr<RakNet::RakPeerInterface> peer, DataModel* dataModel);
		/*override*/ Time::Interval sleepTime(const Stats& stats);                       // vtable+0x04
		/*override*/ Error error(const Stats& stats);                                    // vtable+0x08
		/*override*/ TaskScheduler::StepResult stepDataModelJob(const Stats& stats);     // vtable+0x18
	};

	// RTTI ConcurrentRakPeer::StatsUpdateJob : DataModelJob. Members from GetBufferHealth 0x005c5070
	// (2016 names); the rest is unproven.
	// Vftable PLAYER 0x00b208c0 (operator new(0x290) in the ConcurrentRakPeer ctor).
	// SIZE 0x290
	class ConcurrentRakPeer::StatsUpdateJob : public DataModelJob
	{
	public:
		boost::mutex mapMutex;                              // 0x218
		undefined m_unk0x220[0x268 - 0x220];                // 0x220
		double bufferHealth;                                // 0x268 2016: RunningAverage<> (value() inline)
		undefined m_unk0x270[0x290 - 0x270];                // 0x270

		StatsUpdateJob(boost::shared_ptr<RakNet::RakPeerInterface> peer, DataModel* dataModel);
		/*override*/ Time::Interval sleepTime(const Stats& stats);                       // vtable+0x04
		/*override*/ Error error(const Stats& stats);                                    // vtable+0x08
		/*override*/ TaskScheduler::StepResult stepDataModelJob(const Stats& stats);     // vtable+0x18
	};

} // namespace Network
} // namespace RBX

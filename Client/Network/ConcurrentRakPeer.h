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
#include <boost/unordered_map.hpp>
#include "rbx/RunningAverage.h"
#include <string>

// boost::hash<RakNet::SystemAddress> for StatsUpdateJob's callback map (2016 puts it in RakNetTypes.h as a
// Roblox edit, a friend of SystemAddress). Inlined into the map's find (PLAYER 0x005c61d0): the dotted
// address through inet_addr (WS2_32 ordinal 11), then the port.
namespace RakNet {
	inline std::size_t hash_value(const SystemAddress& systemAddress)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, inet_addr(systemAddress.ToString(false)));
		boost::hash_combine(seed, systemAddress.GetPort());
		return seed;
	}
}

namespace RBX {
	class DataModel;
}

namespace RBX {
namespace Network {

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

	// RTTI ConcurrentRakPeer::StatsUpdateJob : DataModelJob. Vftable PLAYER 0x00b208c0 (operator new(0x290)
	// in the ConcurrentRakPeer ctor). Layout proven by the ctor 0x005c5f60 (STUDIO 0x006211c0); names and map
	// types are 2016's. The two maps' ctors instantiate boost's prime_list; 2013's copy comes from the /GL
	// shell (Client/Win/placeholder.cpp stand-in), so ConcurrentRakPeer.obj does not supply it.
	// SIZE 0x290
	class ConcurrentRakPeer::StatsUpdateJob : public DataModelJob
	{
		struct SystemAddressHasher
		{
			size_t operator()(const RakNet::SystemAddress& key) const
			{
				return RakNet::SystemAddress::ToInteger(key);
			}
		};
	public:
		boost::mutex mapMutex;                              // 0x218
		// iOS: StatsUpdateJob::ConnectionStats, 0x150 bytes (map node link at +0x168 in stepDataModelJob
		// 0x005c6240). The callbacks get its first 0xf0 bytes as a ConcurrentRakPeerStats. Ctor PLAYER
		// 0x005c5d70 (not written): mtuSize 1492, the pings 0, both averages weight 0.1. Member names 2016's.
		struct ConnectionStats : public ConcurrentRakPeerStats
		{
			RunningAverage<> averageBandwidthExceeded;          // 0xf0
			RunningAverage<> averageCongestionControlExceeded;  // 0x120
			ConnectionStats();
		};
		typedef boost::unordered_map<RakNet::SystemAddress, ConnectionStats, SystemAddressHasher> StatsMap;
		StatsMap statsMap;                                  // 0x220
		typedef boost::unordered_map<RakNet::SystemAddress, boost::function<void(const ConcurrentRakPeerStats&)> > UpdateCallbackMap;
		UpdateCallbackMap updateCallbackMap;                // 0x238
		RunningAverage<> bufferHealth;                      // 0x250 (mean 0x268)
		int prevBufferSize;                                 // 0x280
		weak_ptr<RakNet::RakPeerInterface> peer;            // 0x284

		StatsUpdateJob(boost::shared_ptr<RakNet::RakPeerInterface> peer, DataModel* dataModel);
		/*override*/ Time::Interval sleepTime(const Stats& stats);                       // vtable+0x04
		/*override*/ Error error(const Stats& stats);                                    // vtable+0x08
		/*override*/ TaskScheduler::StepResult stepDataModelJob(const Stats& stats);     // vtable+0x18

	private:
		// 2016 name; PLAYER 0x005c4190 / STUDIO 0x0061f050, not written yet.
		void updateStats(StatsMap::value_type& x, RakNet::RakPeerInterface* peer);
	};

} // namespace Network
} // namespace RBX

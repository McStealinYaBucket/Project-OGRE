#pragma once

// Network/Replicator.h -- October 2013, partial. PLAYER layout from the ctor (0x005a5e70) and the
// dtor (0x005a55a0); member names are 2016's where the type and position agree, placeholders
// otherwise. The virtuals Replicator adds are not declared yet, so its vftables are incomplete.
// STUDIO (Oct 17) is a different revision (ctor STUDIO 0x0064a0f0, dtor 0x006497b0): no
// removingInstance at 0x10c4, the signal sits there instead (ctor 0x00629520, dtor 0x00639040), the
// pools follow at 0x10c8, a fourth zeroed pointer (0x128c) follows deserializingEventInvocation, and
// ReplicatorStats (0x7e8 there, 8-aligned) sits at 0x12a0 between defaultObjects and incomingPackets. From the pools on, STUDIO offsets are PLAYER's minus 0xf6c up to
// defaultObjects and minus 0xf7c from incomingPackets on. SIZE 0x2328 (STUDIO).

#include "V8DataModel/DataModelJob.h"
#include "V8Tree/Instance.h"
#include "Streaming.h"
#include "PluginInterface2.h"
#include "Util/IMetric.h"
#include "Voxel/CellChangeListener.h"
#include "ConcurrentRakPeer.h"
#include "ReplicatorStats.h"
#include "ClusterUpdateBuffer.h"
#include "Item.h"
#include "rbx/threadsafe.h"
#include "rbx/memory.h"
#include "rbx/signal.h"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include <queue>
#include <vector>

namespace RBX {
	class NetworkSettings;
	namespace Reflection { class EventInvocation; }

namespace Network {

// IdSerializer (Replicator's base) is declared in Streaming.h.

extern const char* const sReplicator;

class Players;
class Marker;
class InstancePacketCache;
class ClusterPacketCache;

// The base of Replicator's jobs (RTTI: ReplicatorJob : DataModelJob). Its members (2016: a
// weak_ptr<Replicator>) are not proven yet; the ctor (PingJob() PLAYER 0x00598c90) is not written.
class ReplicatorJob : public DataModelJob
{
protected:
	shared_ptr<Replicator> replicator;                 // 0x218 ProcessPacketsJob::sleepTime 0x00598f20
	ReplicatorJob(const char* name, Replicator& replicator, TaskType taskType);
};

// RTTI: DescribedNonCreatable<Replicator, IdSerializer, sReplicator, 1 (INTERNAL_LOCAL)> (classDescriptor
// PLAYER 0x00525e20, its ctor 0x005a5510), then RakNet::PluginInterface2 at 0x1094, IMetric 0x10a0
// (novtable: only Replicator's vftable is stored there) and CellChangeListener 0x10a4.
// SIZE 0x2aa8 (PLAYER)
class Replicator
	: public DescribedNonCreatable<Replicator, IdSerializer, sReplicator, Reflection::ClassDescriptor::INTERNAL_LOCAL>
	, public RakNet::PluginInterface2
	, public IMetric
	, public Voxel::CellChangeListener
{
public:
	class PingJob;
	class PingBackItem;
	class PingItem;
	class MarkerItem;
	class SendDataJob;
	class ProcessPacketsJob;
	class StreamJob;

	// ~ReplicationData is out of line (PLAYER 0x005837d0): shared_ptr at 0x00, the connection's
	// weak islot pointer at 0x08.
	struct ReplicationData
	{
		shared_ptr<Instance> instance;                     // 0x00
		rbx::signals::connection connection;               // 0x08
		bool deleteOnDisconnect;                           // 0x0c
		bool replicateChildren : 1;                        // 0x0d bit 0
		bool listenToChanges : 1;                          // 0x0d bit 1
	};

	// SIZE 0x34. Ctor out of line (PLAYER 0x005a0090): zeroes the ReplicationData words, calls
	// ClusterUpdateBuffer() on +0x10, stores 0 at +0x2c and 0x1000000 at +0x28, clears both
	// bit-fields and the bytes at +0x0c and +0x30. The dtor is inline in ~Replicator.
	struct ClusterReplicationData
	{
		ClusterReplicationData();

		ReplicationData data;                              // 0x00 placeholder name
		ClusterUpdateBuffer updateBuffer;                  // 0x10
		undefined4 m_unk0x28;                              // 0x28
		undefined4 m_unk0x2c;                              // 0x2c
		bool readyToSendChunks;                            // 0x30
	};

	Replicator(RakNet::SystemAddress remotePlayerId, boost::shared_ptr<ConcurrentRakPeer> rakPeer,
		NetworkSettings* networkSettings, bool clusterDebounceEnabled);
	~Replicator();

	// iOS: onStatisticsChanged(RBX::Network::ConcurrentRakPeerStats const&); bound in the ctor.
	void onStatisticsChanged(const ConcurrentRakPeerStats& stats);

	/*override*/ double getMetricValue(const std::string& metric) const;

	// iOS: both const. max(0, mtu + adjust), asserts at Replicator.cpp 3418/3424.
	size_t getAdjustedMtuSize() const;
	size_t getPhysicsMtuSize() const;

	const NetworkSettings& settings() const { return *networkSettings; }

	// Bound as "DisableProcessPackets" (BoundFuncDesc initializer PLAYER 0x00a61a20).
	void disableProcessPackets();

	std::string getRakStatsString(int verbosityLevel);

	size_t incomingPacketsCount() const { return incomingPackets.size(); }

protected:
	/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	/*override*/ void setRefValue(WaitItem& wi, Instance* instance);
public:

protected:
	boost::shared_ptr<ConcurrentRakPeer> rakPeer;          // 0x10a8
	const RakNet::SystemAddress remotePlayerId;            // 0x10b0
#ifndef OGRE_TARGET_STUDIO
	Instance* removingInstance;                            // 0x10c4
	ReplicatorStats replicatorStats;                       // 0x10c8
#endif

public:
	// 2016 declares a disconnectionSignal here; this signal's signature is unproven (2016's).
	rbx::signal<void(std::string, bool)> disconnectionSignal; // 0x2030 ctor 0x005862e0, dtor 0x00592c60 (STUDIO 0x10c4)

protected:
	// Item pools; ~AutoMemPool is inline (purge_memory 0x004128e0, then delete).
	boost::scoped_ptr<AutoMemPool> newInstancePool;        // 0x2034
	boost::scoped_ptr<AutoMemPool> deleteInstancePool;     // 0x2038
	boost::scoped_ptr<AutoMemPool> changePropertyPool;     // 0x203c
	boost::scoped_ptr<AutoMemPool> eventInvocationPool;    // 0x2040
	boost::scoped_ptr<AutoMemPool> pingPool;               // 0x2044
	boost::scoped_ptr<AutoMemPool> pingBackPool;           // 0x2048
	boost::scoped_ptr<AutoMemPool> referencePropertyChangedPool; // 0x204c

	DescriptorDictionary<Reflection::ClassDescriptor> classDictionary;    // 0x2050
	DescriptorDictionary<Reflection::PropertyDescriptor> propDictionary;  // 0x2078
	DescriptorDictionary<Reflection::EventDescriptor> eventDictionary;    // 0x20a0
	DescriptorDictionary<Reflection::Type> typeDictionary;                // 0x20c8

	typedef boost::unordered_map<const Instance*, ReplicationData> RepConts;
	RepConts replicationContainers;                        // 0x20f0 (size at 0x20f8: dtor assert line 348)

	undefined4 m_unk0x2108;                                // 0x2108 2016: megaClusterInstance
	ClusterReplicationData clusterReplicationData;         // 0x210c

	const bool clusterDebounceEnabled;                     // 0x2140
	bool clusterDebounce;                                  // 0x2141
	bool isClusterSpotted;                                 // 0x2142
	bool isOkToSendClusters;                               // 0x2143
	unsigned int approximateSizeOfPendingClusterDeltas;    // 0x2144

	shared_ptr<InstancePacketCache> instancePacketCache;   // 0x2148
	shared_ptr<ClusterPacketCache> clusterPacketCache;     // 0x2150

	// 2013 keeps the top replication containers in a vector (dtor: delete [0x215c], zero three words).
	std::vector<Instance*> topReplicationContainers;       // 0x2158

	Players* players;                                      // 0x2168

	// Five shared_ptrs; only processPacketsJob is placed (getMetricValue 0x00581d50 reads it).
	// 2016 has physicsReceiver, sendDataJob, pingJob, processPacketsJob and streamJob here.
	shared_ptr<void> m_unk0x216c;                          // 0x216c pointee unproven
	shared_ptr<void> m_unk0x2174;                          // 0x2174 pointee unproven
	shared_ptr<ProcessPacketsJob> processPacketsJob;       // 0x217c
	shared_ptr<void> m_unk0x2184;                          // 0x2184 pointee unproven
	shared_ptr<void> m_unk0x218c;                          // 0x218c pointee unproven

	ItemQueue pendingItems;                                // 0x2194

	typedef boost::unordered_set<Reflection::ConstProperty, boost::hash<Reflection::ConstProperty>,
		std::equal_to<Reflection::ConstProperty>, boost::fast_pool_allocator<Reflection::ConstProperty> > PendingPropertyChanges;
	PendingPropertyChanges pendingChangedPropertyItems;    // 0x21a4

	typedef boost::unordered_set<const Instance*> PendingInstances;
	mutable RBX::mutex pendingInstancesMutex;              // 0x21bc
	PendingInstances pendingNewInstances;                  // 0x21d4
	const Instance* serializingInstance;                   // 0x21ec

	const Reflection::Property* deserializingProperty;     // 0x21f0
	const Reflection::EventInvocation* deserializingEventInvocation; // 0x21f4
#ifdef OGRE_TARGET_STUDIO
	undefined4 m_unk0x128c;                                // STUDIO 0x128c, zeroed by the ctor
#endif

	typedef std::map<const RBX::Name*, shared_ptr<Instance> > DefaultObjects;
	DefaultObjects defaultObjects;                         // 0x21f8
#ifdef OGRE_TARGET_STUDIO
	ReplicatorStats replicatorStats;                       // STUDIO 0x12a0 (ctor 0x00681bd0, dtor 0x0062d510)
#endif

	rbx::timestamped_safe_queue<RakNet::Packet*> incomingPackets; // 0x2204

	bool processAllPacketsPerStep;                         // 0x2228

	std::queue<shared_ptr<Marker> > incomingMarkers;       // 0x222c

	NetworkSettings* networkSettings;                      // 0x224c

	bool m_unk0x2250;                                      // 0x2250
	shared_ptr<void> m_unk0x2254;                          // 0x2254 pointee unproven
	shared_ptr<void> m_unk0x225c;                          // 0x225c pointee unproven
	shared_ptr<void> m_unk0x2264;                          // 0x2264 pointee unproven

	// Three maps of descriptor -> shared_ptr (their erase folds with 0x006ef3c0); 2016 holds
	// strings, protectedStrings and eventStrings here. Pointee types unproven.
	std::map<const Reflection::PropertyDescriptor*, shared_ptr<void> > strings;          // 0x226c
	std::map<const Reflection::PropertyDescriptor*, shared_ptr<void> > protectedStrings; // 0x2278
	std::map<const Reflection::EventDescriptor*, shared_ptr<void> > eventStrings;        // 0x2284

	SharedDictionary<RBX::SystemAddress> systemAddressDictionary; // 0x2290

	rbx::signals::scoped_connection m_unk0x2aa0;           // 0x2aa0
};

} // namespace Network
} // namespace RBX

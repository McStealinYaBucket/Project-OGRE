// Network/Replicator.cpp -- October 2013, partial. ~Replicator's asserts sit on their 2013 lines (348/350).

#include "Replicator.h"
#include "Replicator.PingJob.h"
#include "Replicator.ProcessPacketsJob.h"
#include "V8DataModel/NetworkSettings.h"
#include "FastLog.h"
#include "Util/standardout.h"
#include "InternalPacket.h"
#include "Network/Players.h"
#include "V8DataModel/TouchTransmitter.h"
#include "decomp.h"
#include <boost/bind.hpp>

LOGGROUP(Network)

namespace RBX {
namespace Network {

// A scoped assignment with a destructor (setRefValue keeps an EH state for it); 2016's name (util/ScopedAssign.h).
template<class T>
class ScopedAssign
{
	T& ref;
	T old;
public:
	ScopedAssign(T& r, T value) : ref(r), old(r) { ref = value; }
	~ScopedAssign() { ref = old; }
};

// GLOBAL: PLAYER 0x00b19694
// GLOBAL: STUDIO 0x00bd180c
const char* const sReplicator = "NetworkReplicator";

// FUNCTION: PLAYER 0x005804f0
// FUNCTION: STUDIO 0x00622520
void Replicator::onStatisticsChanged(const ConcurrentRakPeerStats& stats)
{
	replicatorStats.rakStats = stats;
}

// FUNCTION: PLAYER 0x005819d0
// FUNCTION: STUDIO 0x00623980
void Replicator::setRefValue(WaitItem& wi, Instance* instance)
{
	Reflection::Property prop(*wi.desc, wi.instance.get());
	ScopedAssign<const Reflection::Property*> assign(deserializingProperty, &prop);
	IdSerializer::setRefValue(wi, instance);
}

// FUNCTION: PLAYER 0x00581b50
// FUNCTION: STUDIO 0x00623b00
void Replicator::OnInternalPacket(RakNet::InternalPacket* internalPacket, unsigned frameNumber,
	RakNet::SystemAddress remoteSystemAddress, RakNet::TimeMS time, int isSend)
{
	if (isSend)
	{
		if (internalPacket->splitPacketCount == 0)
			replicatorStats.numberOfUnsplitMessages++;
		else
		{
			if (settings().printSplitMessages && internalPacket->splitPacketIndex == 0)
				StandardOut::singleton()->printf(MESSAGE_INFO, "split message, id %u, size %d, split count %d",
					internalPacket->data[0], internalPacket->dataBitLength / 8, internalPacket->splitPacketCount);
			replicatorStats.numberOfSplitMessages++;
		}
	}
}

// FUNCTION: PLAYER 0x00581d30
// FUNCTION: STUDIO 0x00623ce0
void Replicator::disableProcessPackets()
{
	processPacketsJob->sleep();
}

// FUNCTION: PLAYER 0x00581d50
// FUNCTION: STUDIO 0x00623d00
double Replicator::getMetricValue(const std::string& metric) const
{
	if (metric == "Network Receive CPU")
		return 100.0 * processPacketsJob->averageDutyCycle();
	if (metric == "Network Receive Time")
		return 1000.0 * processPacketsJob->averageStepTime();
	return 0;
}

// FUNCTION: PLAYER 0x00583a60
// FUNCTION: STUDIO 0x00625c50
std::string Replicator::getRakStatsString(int verbosityLevel)
{
	char buffer[10000];
	buffer[0] = 0;
	RakNet::StatisticsToString(rakPeer ? &replicatorStats.rakStats.rakStats : NULL, buffer, verbosityLevel);
	return buffer;
}

// STUB: PLAYER 0x00583ad0
// STUB: STUDIO 0x00625cc0
void Replicator::receiveCluster(RakNet::BitStream& inBitstream, Instance* instance)
{
	STUB(0x00583ad0);
}

// STUB: PLAYER 0x00589630
// STUB: STUDIO 0x0062d8c0
bool Replicator::sendItemsPacket()
{
	STUB(0x00589630);
	return false;
}

// STUB: PLAYER 0x0058b3e0
// STUB: STUDIO 0x0062fa50
void Replicator::writeChangedProperty(const Instance* instance, const Reflection::PropertyDescriptor& desc,
	RakNet::BitStream& outBitStream)
{
	STUB(0x0058b3e0);
}

// STUB: PLAYER 0x0058b640
// STUB: STUDIO 0x0062fc70
void Replicator::writeChangedRefProperty(const Instance* instance, const Reflection::RefPropertyDescriptor& desc,
	const Guid::Data& id, RakNet::BitStream& outBitStream)
{
	STUB(0x0058b640);
}

// STUB: PLAYER 0x0058b970
// STUB: STUDIO 0x0062ff20
bool Replicator::sendClusterPacket()
{
	STUB(0x0058b970);
	return false;
}

// FUNCTION: PLAYER 0x0058d3d0
// FUNCTION: STUDIO 0x006326e0
bool Replicator::isSerializePending(const Instance* instance) const
{
	RBX::mutex::scoped_lock lock(pendingInstancesMutex);
	return pendingNewInstances.find(instance) != pendingNewInstances.end();
}

// FUNCTION: PLAYER 0x0058d430
// FUNCTION: STUDIO 0x00632740
bool Replicator::isPropertyChangedPending(const Reflection::ConstProperty& prop) const
{
	if (pendingChangedPropertyItems.find(prop) != pendingChangedPropertyItems.end())
		return true;
	return false;
}

// STUB: PLAYER 0x0058d5f0
// STUB: STUDIO 0x006328f0
void Replicator::readChangedProperty(RakNet::BitStream& bitStream, Reflection::Property prop)
{
	STUB(0x0058d5f0);
}

// STUB: PLAYER 0x00591bc0
// STUB: STUDIO 0x00637ff0
void Replicator::onParentChanged(shared_ptr<Instance> instance)
{
	STUB(0x00591bc0);
}

// FUNCTION: PLAYER 0x00591f00
// STUB: STUDIO 0x00638330
FilterResult Replicator::filterChangedProperty(Instance* instance, const Reflection::PropertyDescriptor& desc) // STUDIO: byte-identical (probe); 98.8 on a shared_from twin pairing
{
	if (instance == removingInstance)
		return Reject;
	Reflection::Property prop(desc, instance);
	if (deserializingProperty != NULL && prop == *deserializingProperty)
	{
		deserializingProperty = NULL;
		return Reject;
	}
	if (desc == Instance::propParent)
	{
		onParentChanged(shared_from(instance));
		return Reject;
	}
	if (isSerializePending(instance))
		return Reject;
	if (!desc.canReplicate())
		return Reject;
	if (!isLegalSendProperty(instance, desc))
		return Reject;
	return isPropertyChangedPending(prop) ? Reject : Accept;
}

// FUNCTION: PLAYER 0x00598da0
// FUNCTION: STUDIO 0x0063be70
Time::Interval Replicator::PingJob::sleepTime(const Stats& stats)
{
	return computeStandardSleepTime(stats, 2);
}

// FUNCTION: PLAYER 0x00598dd0
// FUNCTION: STUDIO 0x0063bea0
Replicator::PingJob::Error Replicator::PingJob::error(const Stats& stats)
{
	return computeStandardError(stats, 2);
}

// STUB: PLAYER 0x0059a220
// STUB: STUDIO 0x0063d2c0
void Replicator::processPacket(RakNet::Packet* packet)
{
	STUB(0x0059a220);
}

// STUB: PLAYER 0x0059b770
// STUB: STUDIO 0x0063e710
__declspec(noinline) void Replicator::requestDisconnect() // noinline: stand-in (VC9 inlines a same-TU STUB)
{
	STUB(0x0059b770);
}

// FUNCTION: PLAYER 0x0059c060
// FUNCTION: STUDIO 0x0063f3b0
void Replicator::dataOutStep()
{
	try
	{
		sendItemsPacket();
	}
	catch (...)
	{
		FASTLOG(FLog::Network, "Exception during data out step, request disconnect");
		requestDisconnect();
		throw;
	}
	if (isClusterSpotted && !isOkToSendClusters)
	{
		clusterReplicationData.readyToSendChunks = true;
		isOkToSendClusters = true;
	}
	if (const RakNet::RakNetStatistics* stats = getRakNetStats())
	{
		Time now = Time::now<Time::Benchmark>();
		replicatorStats.m_unk0xf50 = now;
		replicatorStats.kiloBytesSentPerSecond.sample((int)stats->valueOverLastSecond[RakNet::ACTUAL_BYTES_SENT] / 1024);
		replicatorStats.kiloBytesReceivedPerSecond.sample((int)stats->valueOverLastSecond[RakNet::ACTUAL_BYTES_RECEIVED] / 1024);
	}
}

// FUNCTION: PLAYER 0x0059cc90
// FUNCTION: STUDIO 0x0063ffe0
bool Replicator::wantReplicate(const Instance* source) const
{
	switch (source->getDescriptor().getReplicationLevel())
	{
	case Reflection::PLAYER_REPLICATE:
		if (const Player* player = Instance::fastDynamicCast<Player>(source->getParent()))
			if (const Player* targetPlayer = findTargetPlayer())
				return player->userId == targetPlayer->userId;
		return false;
	case Reflection::NEVER_REPLICATE:
		return false;
	case Reflection::STANDARD_REPLICATE:
		return true;
	}
	return true;
}

// FUNCTION: PLAYER 0x0059cd20
// FUNCTION: STUDIO 0x00640070
bool Replicator::remoteDeleteOnDisconnect(const Instance* instance) const
{
	if (instance->isDescendantOf(players))
		return true;
	if (instance == Players::findConstLocalCharacter(instance))
		return true;
	if (Instance::fastDynamicCast<TouchTransmitter>(instance))
		return true;
	return false;
}

// STUB: PLAYER 0x0059ea20
// STUB: STUDIO 0x00642040
TaskScheduler::StepResult Replicator::PingJob::stepDataModelJob(const Stats& stats) // VMProtect-mutated in PLAYER
{
	STUB(0x0059ea20);
	return TaskScheduler::Done;
}

// FUNCTION: PLAYER 0x005a0090
// FUNCTION: STUDIO 0x006438a0
Replicator::ClusterReplicationData::ClusterReplicationData()
{
	data.deleteOnDisconnect = false;
	data.replicateChildren = false;
	data.listenToChanges = false;
	readyToSendChunks = false;
}

// STUB: PLAYER 0x005a2680
// STUB: STUDIO 0x00646370
void Replicator::addTopReplicationContainer(Instance* instance, bool replicateProperties, bool replicateChildren,
	boost::function<void(shared_ptr<Instance>)> replicationFilter)
{
	STUB(0x005a2680);
}

// Not written: 3492 bytes. The one statement here keeps ProcessPacketsJob's inline ctor (0x00598e00, called
// from 0x005a2f50) and its vftable emitted, as 2013's body does.
// STUB: PLAYER 0x005a2870
void Replicator::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	STUB(0x005a2870);
	processPacketsJob.reset(new ProcessPacketsJob(*this));
}

// STUB: PLAYER 0x005a3ae0
// STUB: STUDIO 0x00647800
void Replicator::serializePropertyValue(const Reflection::ConstProperty& property, RakNet::BitStream& outBitstream,
	bool useDictionary)
{
	STUB(0x005a3ae0);
}

// STUB: PLAYER 0x005a4190
// STUB: STUDIO 0x00647eb0
void Replicator::deserializePropertyValue(RakNet::BitStream& inBitstream, Reflection::Property property,
	bool useDictionary)
{
	STUB(0x005a4190);
}


// The STUDIO revision's ~Replicator asserts sit one line lower (349/351 against PLAYER's 348/350).
#ifdef OGRE_TARGET_STUDIO
#define OGRE_PLAYER_ASSERT(e)
#define OGRE_STUDIO_ASSERT(e) RBXASSERT(e)
#else
#define OGRE_PLAYER_ASSERT(e) RBXASSERT(e)
#define OGRE_STUDIO_ASSERT(e)
#endif


// FUNCTION: PLAYER 0x005a55a0
// FUNCTION: STUDIO 0x006497b0
Replicator::~Replicator()
{
	OGRE_PLAYER_ASSERT(replicationContainers.size()==0);
	OGRE_STUDIO_ASSERT(replicationContainers.size()==0);
	OGRE_PLAYER_ASSERT(!rakPeer);
	OGRE_STUDIO_ASSERT(!rakPeer);
	FASTLOG1(FLog::Network, "Replicator destroyed: %p", this);
}

// readItem and onPropertyChanged precede ~Replicator in 2013; they sit here (a descent) because the
// functions above it no longer fit before its assert lines (348/350).
// STUB: PLAYER 0x005a4b40
// STUB: STUDIO 0x00648860
void Replicator::readItem(RakNet::BitStream& inBitstream, Item::ItemType itemType)
{
	STUB(0x005a4b40);
}

// STUB: PLAYER 0x005a4de0
// STUB: STUDIO 0x00648ba0
void Replicator::onPropertyChanged(Instance* instance, const Reflection::PropertyDescriptor* descriptor)
{
	STUB(0x005a4de0);
}

// FUNCTION: PLAYER 0x005a5e70
// FUNCTION: STUDIO 0x0064a0f0
Replicator::Replicator(RakNet::SystemAddress remotePlayerId, boost::shared_ptr<ConcurrentRakPeer> rakPeer,
	NetworkSettings* networkSettings, bool clusterDebounceEnabled)
	: rakPeer(rakPeer)
	, remotePlayerId(remotePlayerId)
#ifndef OGRE_TARGET_STUDIO
	, removingInstance(NULL)
#endif
	, m_unk0x2108(0)
	, clusterDebounceEnabled(clusterDebounceEnabled)
	, clusterDebounce(false)
	, isClusterSpotted(false)
	, isOkToSendClusters(false)
	, approximateSizeOfPendingClusterDeltas(0)
	, players(NULL)
	, serializingInstance(NULL)
	, deserializingProperty(NULL)
	, deserializingEventInvocation(NULL)
#ifdef OGRE_TARGET_STUDIO
	, removingInstance(NULL)
#endif
	, processAllPacketsPerStep(false)
	, networkSettings(networkSettings)
	, m_unk0x2250(false)
{
	if (rakPeer)
	{
		rakPeer->addStats(remotePlayerId, boost::bind(&Replicator::onStatisticsChanged, this, _1));
		rakPeer->rawPeer()->AttachPlugin(this);
	}

	FASTLOGS(FLog::Network, "Replicator created for player %s", RakNetAddressToString(remotePlayerId).c_str());
	FASTLOG1(FLog::Network, "Replicator created: %p", this);
}

// Blank lines: the two MTU asserts sit on their 2013 lines (3418, 3424).






























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// FUNCTION: PLAYER 0x00581c10
// FUNCTION: STUDIO 0x00623bc0
size_t Replicator::getAdjustedMtuSize() const
{
	RBXASSERT(settings().getReplicationMtuAdjust() <= 0);
	return std::max(0, settings().getReplicationMtuAdjust() + replicatorStats.rakStats.mtuSize);
}
// FUNCTION: PLAYER 0x00581ca0
// FUNCTION: STUDIO 0x00623c50
size_t Replicator::getPhysicsMtuSize() const {
	RBXASSERT(settings().getPhysicsMtuAdjust() <= 0);
	return std::max(0, settings().getPhysicsMtuAdjust() + replicatorStats.rakStats.mtuSize);
}

} // namespace Network
} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::Network::Replicator::ReplicationData, 0x10);
DECOMP_SIZE_ASSERT(RBX::Network::Replicator::ClusterReplicationData, 0x34);

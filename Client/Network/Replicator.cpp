// Network/Replicator.cpp -- October 2013, partial. ~Replicator's asserts sit on their 2013 lines
// (348 and 350); the blank block below keeps them there until the functions before it are written.

#include "Replicator.h"
#include "Replicator.PingJob.h"
#include "Replicator.ProcessPacketsJob.h"
#include "FastLog.h"
#include "decomp.h"
#include <boost/bind.hpp>

LOGGROUP(Network)

namespace RBX {
namespace Network {

// GLOBAL: PLAYER 0x00b19694
// GLOBAL: STUDIO 0x00bd180c
const char* const sReplicator = "NetworkReplicator";

// FUNCTION: PLAYER 0x005804f0
void Replicator::onStatisticsChanged(const ConcurrentRakPeerStats& stats)
{
	replicatorStats.rakStats = stats;
}

// FUNCTION: PLAYER 0x00581d50
double Replicator::getMetricValue(const std::string& metric) const
{
	if (metric == "Network Receive CPU")
		return 100.0 * processPacketsJob->averageDutyCycle();
	if (metric == "Network Receive Time")
		return 1000.0 * processPacketsJob->averageStepTime();
	return 0;
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

// VMProtect-mutated in PLAYER.
// STUB: PLAYER 0x0059ea20
// STUB: STUDIO 0x00642040
TaskScheduler::StepResult Replicator::PingJob::stepDataModelJob(const Stats& stats)
{
	STUB(0x0059ea20);
	return TaskScheduler::Done;
}

// STUB: PLAYER 0x005a0090
// STUB: STUDIO 0x006438a0
Replicator::ClusterReplicationData::ClusterReplicationData()
{
	STUB(0x005a0090);
}























































































































































































































































































// FUNCTION: PLAYER 0x005a55a0
// STUB: STUDIO 0x006497b0
Replicator::~Replicator()
{
	RBXASSERT(replicationContainers.size()==0);

	RBXASSERT(!rakPeer);
	FASTLOG1(FLog::Network, "Replicator destroyed: %p", this);
}

// FUNCTION: PLAYER 0x005a5e70
// STUB: STUDIO 0x0064a0f0
Replicator::Replicator(RakNet::SystemAddress remotePlayerId, boost::shared_ptr<ConcurrentRakPeer> rakPeer,
	NetworkSettings* networkSettings, bool clusterDebounceEnabled)
	: rakPeer(rakPeer)
	, remotePlayerId(remotePlayerId)
	, removingInstance(NULL)
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

} // namespace Network
} // namespace RBX

// Network/ReplicatorStats.cpp -- October 2013, partial (see ReplicatorStats.h). No 2013 __FILE__
// names this file. The STUBs sit outside Replicator.cpp: VC9 inlines a same-TU STUB into
// ~Replicator and proves it nothrow, which drops an EH state from Replicator's ctor.

#include "ReplicatorStats.h"
#include "Replicator.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x00589100
// STUB: STUDIO 0x0062d510
ReplicatorStats::~ReplicatorStats()
{
	STUB(0x00589100);
}

// ReplicatorJob's ctor (iOS ReplicatorJob(char const*, Replicator&, DataModelJob::TaskType)) stands here for
// the same reason: ProcessPacketsJob's ctor keeps its EH states only with the call in another TU.
// STUB: PLAYER 0x00591550
ReplicatorJob::ReplicatorJob(const char* name, Replicator& replicator, TaskType taskType)
	: DataModelJob(name, taskType, false, shared_ptr<DataModelArbiter>(), Time::Interval(0))
{
	STUB(0x00591550);
}

// STUB: PLAYER 0x005d3950
// STUB: STUDIO 0x00681bd0
ReplicatorStats::ReplicatorStats()
{
	STUB(0x005d3950);
}

} // namespace Network
} // namespace RBX

#ifdef OGRE_TARGET_STUDIO
DECOMP_SIZE_ASSERT(RBX::Network::ReplicatorStats, 0x7e8);
DECOMP_STATIC_ASSERT(offsetof(RBX::Network::ReplicatorStats, numberOfUnsplitMessages) == 0x440);
#else
DECOMP_SIZE_ASSERT(RBX::Network::ReplicatorStats, 0xf68);
DECOMP_STATIC_ASSERT(offsetof(RBX::Network::ReplicatorStats, numberOfUnsplitMessages) == 0x940);
#endif

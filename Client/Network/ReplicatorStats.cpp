// Network/ReplicatorStats.cpp -- October 2013, partial (see ReplicatorStats.h). No 2013 __FILE__
// names this file. The STUBs sit outside Replicator.cpp: VC9 inlines a same-TU STUB into
// ~Replicator and proves it nothrow, which drops an EH state from Replicator's ctor.

#include "ReplicatorStats.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x00589100
ReplicatorStats::~ReplicatorStats()
{
	STUB(0x00589100);
}

// STUB: PLAYER 0x005d3950
ReplicatorStats::ReplicatorStats()
{
	STUB(0x005d3950);
}

} // namespace Network
} // namespace RBX

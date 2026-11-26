#pragma once

// Network/ReplicatorStats.h -- October 2013, partial: size and the out-of-line ctor/dtor only.
// Replicator+0x10c8, 0xf68 bytes (the next member is at Replicator+0x2030). Its ctor (PLAYER
// 0x005d3950, STUDIO 0x00629520) and dtor (iOS ~ReplicatorStats, PLAYER 0x00589100, STUDIO
// 0x00639040) are not written; the members are unproven.

#include "ConcurrentRakPeer.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// SIZE 0xf68
struct ReplicatorStats
{
	ReplicatorStats();
	~ReplicatorStats();

	undefined m_unk0x000[0xe58];                // 0x000
	ConcurrentRakPeerStats rakStats;            // 0xe58 Replicator::onStatisticsChanged 0x005804f0
	undefined m_unk0xf48[0xf68 - 0xf48];        // 0xf48
};

} // namespace Network
} // namespace RBX

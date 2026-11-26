#pragma once

// Network/ReplicatorStats.h -- October 2013, partial: size and the out-of-line ctor/dtor only.
// PLAYER: Replicator+0x10c8, 0xf68 bytes (the next member is at Replicator+0x2030). STUDIO:
// Replicator+0x12a0, 0x7e8 bytes, rakStats at +0x6d8 (onStatisticsChanged STUDIO 0x00622520 copies
// to Replicator+0x1978); the STUDIO revision lacks PLAYER's two 10 x 0x30 arrays (eh vector ctor at
// +0x1e0/+0x3c0). Ctor PLAYER 0x005d3950 / STUDIO 0x00681bd0, dtor (iOS ~ReplicatorStats) PLAYER
// 0x00589100 / STUDIO 0x0062d510. Not written; the members are unproven.

#include "ConcurrentRakPeer.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// SIZE 0xf68 (PLAYER), 0x7e8 (STUDIO)
struct ReplicatorStats
{
	ReplicatorStats();
	~ReplicatorStats();

#ifdef OGRE_TARGET_STUDIO
	undefined m_unk0x000[0x6d8];                // 0x000
	ConcurrentRakPeerStats rakStats;            // 0x6d8 Replicator::onStatisticsChanged STUDIO 0x00622520
	undefined m_unk0x7c8[0x7e8 - 0x7c8];        // 0x7c8
#else
	undefined m_unk0x000[0xe58];                // 0x000
	ConcurrentRakPeerStats rakStats;            // 0xe58 Replicator::onStatisticsChanged 0x005804f0
	undefined m_unk0xf48[0xf68 - 0xf48];        // 0xf48
#endif
};

} // namespace Network
} // namespace RBX

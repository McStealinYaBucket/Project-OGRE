#pragma once

// Network/ReplicatorStats.h -- October 2013, partial: size and the out-of-line ctor/dtor only.
// PLAYER: Replicator+0x10c8, 0xf68 bytes (the next member is at Replicator+0x2030). STUDIO:
// Replicator+0x12a0, 0x7e8 bytes, rakStats at +0x6d8 (onStatisticsChanged STUDIO 0x00622520 copies
// to Replicator+0x1978); the STUDIO revision lacks PLAYER's two 10 x 0x30 arrays (eh vector ctor at
// +0x1e0/+0x3c0). Ctor PLAYER 0x005d3950 / STUDIO 0x00681bd0, dtor (iOS ~ReplicatorStats) PLAYER
// 0x00589100 / STUDIO 0x0062d510. Not written; the members are unproven.

#include "ConcurrentRakPeer.h"
#include "rbx/RunningAverage.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// SIZE 0xf68 (PLAYER), 0x7e8 (STUDIO)
struct ReplicatorStats
{
	ReplicatorStats();
	~ReplicatorStats();

#ifdef OGRE_TARGET_STUDIO
	undefined m_unk0x000[0x1e0];                // 0x000
	// Replicator::dataOutStep STUDIO 0x0063f3b0 samples ACTUAL_BYTES_RECEIVED/SENT / 1024 (2016 names).
	RunningAverage<> kiloBytesReceivedPerSecond; // 0x1e0
	undefined m_unk0x210[0x2d0 - 0x210];        // 0x210
	RunningAverage<> kiloBytesSentPerSecond;    // 0x2d0
	// ItemSender::closePacket STUDIO 0x006720a0 samples both (2016 names and types).
	RunningAverageTimeInterval<> dataPacketsSent;  // 0x300
	undefined m_unk0x340[0x370 - 0x340];        // 0x340
	RunningAverage<int> dataPacketsSentSize;    // 0x370
	undefined m_unk0x3a0[0x440 - 0x3a0];        // 0x3a0
	int numberOfUnsplitMessages;                // 0x440 Replicator::OnInternalPacket STUDIO 0x00623b00 (2016 names)
	int numberOfSplitMessages;                  // 0x444
	undefined m_unk0x448[0x6d8 - 0x448];        // 0x448
	ConcurrentRakPeerStats rakStats;            // 0x6d8 Replicator::onStatisticsChanged STUDIO 0x00622520
	undefined m_unk0x7c8[0x7d0 - 0x7c8];        // 0x7c8
	Time m_unk0xf50;                            // 0x7d0 (PLAYER placeholder name) dataOutStep: Time::now<Benchmark>
	undefined m_unk0x7d8[0x7e8 - 0x7d8];        // 0x7d8
#else
	undefined m_unk0x000[0x5a0];                // 0x000
	// Replicator::dataOutStep 0x0059c060 samples ACTUAL_BYTES_RECEIVED/SENT / 1024 (2016 names).
	RunningAverage<> kiloBytesReceivedPerSecond; // 0x5a0
	undefined m_unk0x5d0[0x7d0 - 0x5d0];        // 0x5d0
	RunningAverage<> kiloBytesSentPerSecond;    // 0x7d0
	// ItemSender::closePacket 0x005da460 samples both (2016 names and types).
	RunningAverageTimeInterval<> dataPacketsSent;  // 0x800
	undefined m_unk0x840[0x870 - 0x840];        // 0x840
	RunningAverage<int> dataPacketsSentSize;    // 0x870
	undefined m_unk0x8a0[0x940 - 0x8a0];        // 0x8a0
	int numberOfUnsplitMessages;                // 0x940 Replicator::OnInternalPacket 0x00581b50 (2016 names)
	int numberOfSplitMessages;                  // 0x944
	undefined m_unk0x948[0xe58 - 0x948];        // 0x948
	ConcurrentRakPeerStats rakStats;            // 0xe58 Replicator::onStatisticsChanged 0x005804f0
	undefined m_unk0xf48[0xf50 - 0xf48];        // 0xf48
	Time m_unk0xf50;                            // 0xf50 dataOutStep: Time::now<Benchmark>
	undefined m_unk0xf58[0xf68 - 0xf58];        // 0xf58
#endif
};

} // namespace Network
} // namespace RBX

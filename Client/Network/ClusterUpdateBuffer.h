#pragma once

// Network/ClusterUpdateBuffer.h -- October 2013, partial: the shape Replicator's dtor destroys.
// 0x18 bytes: a vector of 0x2c-byte entries, each owning a vector at +0x18 (_Destroy PLAYER
// 0x00598bf0), then two words (ctor PLAYER 0x005d9190 zeroes +0x10/+0x14 after an out-of-line
// call 0x005d9130 with 0x400). Names and the entry's other members are unproven.

#include "decomp.h"
#include <vector>

namespace RBX {
namespace Network {

// SIZE 0x18
class ClusterUpdateBuffer
{
public:
	struct Entry
	{
		undefined m_unk0x00[0x18];              // 0x00
		std::vector<undefined4> m_unk0x18;      // 0x18
	};

	ClusterUpdateBuffer();

private:
	std::vector<Entry> m_unk0x00;               // 0x00
	undefined4 m_unk0x10;                       // 0x10
	undefined4 m_unk0x14;                       // 0x14
};

} // namespace Network
} // namespace RBX

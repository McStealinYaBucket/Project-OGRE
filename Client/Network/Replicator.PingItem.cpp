// Network/Replicator.PingItem.cpp -- October 2013, partial: the ctor. write (PLAYER 0x005eca80)
// is VMProtect-mutated in PLAYER and needs Replicator's members; not written yet. The ctor is 100% once the CSVs carry the ??_7 vftable row (private CSV run).

#include "Replicator.PingItem.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x005eca80
// STUB: STUDIO 0x00684ad0
bool Replicator::PingItem::write(RakNet::BitStream& bitStream)
{
	STUB(0x005eca80);
	return true;
}

// FUNCTION: PLAYER 0x005ecb20
// FUNCTION: STUDIO 0x00684b50
Replicator::PingItem::PingItem(Replicator* replicator, RakNet::Time time)
	: PooledItem(*replicator), time(time)
{
}

} // namespace Network
} // namespace RBX

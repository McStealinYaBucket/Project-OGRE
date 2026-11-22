// Network/Replicator.PingBackItem.cpp -- October 2013, partial: the ctor. write (PLAYER 0x005e9da0)
// is VMProtect-mutated in PLAYER and needs Replicator's members; not written yet. The ctor is 100% once the CSVs carry the ??_7 vftable row (private CSV run).

#include "Replicator.PingBackItem.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x005e9da0
// STUB: STUDIO 0x00681ed0
bool Replicator::PingBackItem::write(RakNet::BitStream& bitStream)
{
	STUB(0x005e9da0);
	return true;
}

// FUNCTION: PLAYER 0x005e9e40
// FUNCTION: STUDIO 0x00681f50
Replicator::PingBackItem::PingBackItem(Replicator* replicator, RakNet::Time time)
	: PooledItem(*replicator), time(time)
{
}

} // namespace Network
} // namespace RBX

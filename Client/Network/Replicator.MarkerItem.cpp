// Network/Replicator.MarkerItem.cpp -- October 2013, partial: the ctor. write (PLAYER 0x005ecb50)
// needs Replicator's members; not written yet. The ctor is 100% once the CSVs carry the ??_7 vftable row (private CSV run).

#include "Replicator.MarkerItem.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x005ecb50
// STUB: STUDIO 0x00684b80
bool Replicator::MarkerItem::write(RakNet::BitStream& bitStream)
{
	STUB(0x005ecb50);
	return true;
}

// FUNCTION: PLAYER 0x005eccc0
// FUNCTION: STUDIO 0x00684cf0
Replicator::MarkerItem::MarkerItem(Replicator* replicator, long id)
	: Item(*replicator), id(id)
{
}

} // namespace Network
} // namespace RBX

#pragma once

// Network/Replicator.MarkerItem.h -- October 2013 (mac: MarkerItem(Replicator*, long)).

#include "Item.h"
#include "Replicator.h"

namespace RBX {
namespace Network {

// vftable PLAYER 0x00b26b70 (no VTABLE marker: reccmp names a nested class without its outer class;
// STUDIO 0x00bdea40; it pairs through its ??_7 RTTI row)
// SIZE 0x28
class Replicator::MarkerItem : public Item
{
	long id;                                // 0x20
public:
	MarkerItem(Replicator* replicator, long id);

	/*implement*/ virtual bool write(RakNet::BitStream& bitStream);   // vtable+0x04
};

} // namespace Network
} // namespace RBX

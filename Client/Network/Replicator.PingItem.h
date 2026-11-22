#pragma once

// Network/Replicator.PingItem.h -- October 2013. The 2013 ctor takes only the time
// (mac: PingItem(Replicator*, unsigned long long)); 2016's extraStats does not exist yet.

#include "Item.h"
#include "Replicator.h"

#include "RakNetTime.h"

namespace RBX {
namespace Network {

// vftable PLAYER 0x00b26b1c (no VTABLE marker: reccmp names a nested class without its outer class;
// STUDIO 0x00bde9ec; it pairs through its ??_7 RTTI row)
// SIZE 0x28
class Replicator::PingItem : public PooledItem
{
	RakNet::Time time;                      // 0x20
public:
	PingItem(Replicator* replicator, RakNet::Time time);

	/*implement*/ virtual bool write(RakNet::BitStream& bitStream);   // vtable+0x04
};

} // namespace Network
} // namespace RBX

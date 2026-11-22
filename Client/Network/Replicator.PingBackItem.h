#pragma once

// Network/Replicator.PingBackItem.h -- October 2013. The 2013 ctor takes only the time
// (mac: PingBackItem(Replicator*, unsigned long long)); 2016's extraStats does not exist yet.

#include "Item.h"
#include "Replicator.h"

#include "RakNetTime.h"

namespace RBX {
namespace Network {

// vftable PLAYER 0x00b2611c (no VTABLE marker: reccmp names a nested class without its outer class;
// STUDIO 0x00bddfe8; it pairs through its ??_7 RTTI row)
// SIZE 0x28
class Replicator::PingBackItem : public PooledItem
{
	RakNet::Time time;                      // 0x20
public:
	PingBackItem(Replicator* replicator, RakNet::Time time);

	/*implement*/ virtual bool write(RakNet::BitStream& bitStream);   // vtable+0x04

	// Deleting destructor PLAYER 0x005e9e70 / STUDIO 0x00632650 (PooledItem's, shared): our body is
	// byte-identical (0x0045e180 in a 2026-11-14 link) but reccmp does not pair a SYNTHETIC marker on
	// it ("Failed to find a match"), so it carries no marker yet.
};

} // namespace Network
} // namespace RBX

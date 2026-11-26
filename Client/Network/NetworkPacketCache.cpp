// Network/NetworkPacketCache.cpp -- October 2013, partial: names, ctors and onServiceProvider as
// STUBs (pairing targets for Server::onServiceProvider and the create<T> templates).

#include "Network/NetworkPacketCache.h"
#include "Network/NetworkClusterPacketCache.h"

namespace RBX {
namespace Network {

const char* const sPhysicsPacketCache = "PhysicsPacketCache";
const char* const sInstancePacketCache = "InstancePacketCache";
const char* const sClusterPacketCache = "ClusterPacketCache";

// STUB: PLAYER 0x005d5ef0
void InstancePacketCache::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	STUB(0x005d5ef0);
}

// STUB: PLAYER 0x005d74c0
PhysicsPacketCache::PhysicsPacketCache()
{
	STUB(0x005d74c0);
}

// STUB: PLAYER 0x005d7cb0
InstancePacketCache::InstancePacketCache()
{
	STUB(0x005d7cb0);
}

// STUB: PLAYER 0x005d7ea0
void PhysicsPacketCache::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	STUB(0x005d7ea0);
}

// STUB: PLAYER 0x005d97f0
void ClusterPacketCache::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	STUB(0x005d97f0);
}

// STUB: PLAYER 0x005d9a10
ClusterPacketCache::ClusterPacketCache()
{
	STUB(0x005d9a10);
}

} // namespace Network
} // namespace RBX

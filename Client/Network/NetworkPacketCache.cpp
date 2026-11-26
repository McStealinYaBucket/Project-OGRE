// Network/NetworkPacketCache.cpp -- October 2013, partial: names, the ctors, and onServiceProvider as
// STUBs (pairing targets for Server::onServiceProvider and the create<T> templates).

#include "Network/NetworkPacketCache.h"
#include "Network/NetworkClusterPacketCache.h"
#include "FastLog.h"

LOGGROUP(NetworkCache)

namespace RBX {
namespace Network {

const char* const sPhysicsPacketCache = "PhysicsPacketCache";
const char* const sInstancePacketCache = "InstancePacketCache";
const char* const sClusterPacketCache = "ClusterPacketCache";

// FUNCTION: PLAYER 0x005d5ef0
// FUNCTION: STUDIO 0x006673c0
void InstancePacketCache::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	if (oldProvider)
	{
		for (std::list<rbx::signals::connection>::iterator iter = connections.begin(); iter != connections.end(); ++iter)
			iter->disconnect();
		connections.clear();
		streamCache.clear();
	}
	Super::onServiceProvider(oldProvider, newProvider);
}

// FUNCTION: PLAYER 0x005d74c0
// FUNCTION: STUDIO 0x00668990
PhysicsPacketCache::PhysicsPacketCache()
{
	setName(sPhysicsPacketCache);
	FASTLOG(FLog::NetworkCache, "PhysicsPacketCache Start");
}

// The list ctor calls _Buynode (PLAYER 0x005e7920, STUDIO the shell's /GL copy 0x0049de90).
// FUNCTION: PLAYER 0x005d7cb0
// STUB: STUDIO 0x00669180
InstancePacketCache::InstancePacketCache()
{
	setName(sInstancePacketCache);
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

// STUB: PLAYER 0x005d98a0
void ClusterPacketCache::terrainCellChanged(const Voxel::CellChangeInfo& info)
{
	STUB(0x005d98a0);
}

// FUNCTION: PLAYER 0x005d9a10
// FUNCTION: STUDIO 0x00669e30
ClusterPacketCache::ClusterPacketCache()
{
	setName(sClusterPacketCache);
}


} // namespace Network

} // namespace RBX

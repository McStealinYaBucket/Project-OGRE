// Network/NetworkPacketCache.cpp -- October 2013, partial: names, the ctors, and onServiceProvider as
// STUBs (pairing targets for Server::onServiceProvider and the create<T> templates).

#include "Network/NetworkPacketCache.h"
#include "Network/NetworkClusterPacketCache.h"
#include "FastLog.h"

// 2013 defines this channel in App's ScriptContext.obj (initializer PLAYER 0x00a660f0, right after
// Network's); it stands here until that TU takes it.
namespace FLog {
	// GLOBAL: PLAYER 0x00d30f1d
	// GLOBAL: STUDIO 0x00ec1809
	Channel NetworkCache = 1;
	int NetworkCacheIniter = RegisterLogGroup("NetworkCache", &NetworkCache, NULL, FASTVARTYPE_STATIC);
}

namespace RBX {
namespace Network {

const char* const sPhysicsPacketCache = "PhysicsPacketCache";
const char* const sInstancePacketCache = "InstancePacketCache";
const char* const sClusterPacketCache = "ClusterPacketCache";

// Probed IDENTICAL in both with the connection list declared (see the header for why it is not yet):
// if (oldProvider) { disconnect each connection; connections.clear(); streamCache.clear(); }
// Super::onServiceProvider(oldProvider, newProvider).
// STUB: PLAYER 0x005d5ef0
// STUB: STUDIO 0x006673c0
void InstancePacketCache::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	STUB(0x005d5ef0);
}

// FUNCTION: PLAYER 0x005d74c0
// FUNCTION: STUDIO 0x00668990
PhysicsPacketCache::PhysicsPacketCache()
{
	setName(sPhysicsPacketCache);
	FASTLOG(FLog::NetworkCache, "PhysicsPacketCache Start");
}

// Probed IDENTICAL with the connection list declared (it calls the list's _Buynode, PLAYER 0x005e7920,
// STUDIO the shell's /GL copy 0x0049de90); a STUB until the list is (see the header).
// STUB: PLAYER 0x005d7cb0
// STUB: STUDIO 0x00669180
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

// The packet caches' registration statics. 2013 defines them in App's FactoryRegistration.obj
// (initializers PLAYER 0x00a76660/0x00a76680/0x00a766a0, between CharacterAppearance and ChatFilter);
// they stand here, in Network, until that TU takes them.
namespace Reflection {
// GLOBAL: PLAYER 0x00e0cbe8
// GLOBAL: STUDIO 0x00f83598
template<> ClassRegistrar<Network::PhysicsPacketCache> ClassRegistrar<Network::PhysicsPacketCache>::registrar(0);
// GLOBAL: PLAYER 0x00e0ce14
// GLOBAL: STUDIO 0x00f837c8
template<> ClassRegistrar<Network::InstancePacketCache> ClassRegistrar<Network::InstancePacketCache>::registrar(0);
// GLOBAL: PLAYER 0x00e0c8b8
// GLOBAL: STUDIO 0x00f8325c
template<> ClassRegistrar<Network::ClusterPacketCache> ClassRegistrar<Network::ClusterPacketCache>::registrar(0);
} // namespace Reflection
} // namespace RBX

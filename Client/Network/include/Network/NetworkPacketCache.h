#pragma once

// Network/NetworkPacketCache.h -- October 2013, partial: the base chains, sizes and the out-of-line
// functions Server::onServiceProvider (PLAYER 0x005b5310) needs. RTTI: DescribedNonCreatable<T,
// Instance, sT> plus Service at 0x60. Sizes from Creatable<Instance>::create<T>'s operator new
// (PLAYER 0x005b2120: 0x94, 0x005b21a0: 0x98). Members from the ctors (PLAYER 0x005d74c0,
// 0x005d7cb0): the stream cache unordered_map at 0x64, boost::shared_mutex at 0x7c (its out-of-line
// ctor 0x005d5480), then two scoped_connections (Physics) or a std::list of connections (Instance;
// _Buynode 0x005e7920). Names are 2016's. No 2013 __FILE__ names this header; the path is 2016's.

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "rbx/signal.h"
#include "decomp.h"
#include <boost/unordered_map.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <list>

namespace RBX {
	class Assembly;

namespace Network {

	extern const char* const sPhysicsPacketCache;
	extern const char* const sInstancePacketCache;

	// SIZE 0x94
	class PhysicsPacketCache
		: public DescribedNonCreatable<PhysicsPacketCache, Instance, sPhysicsPacketCache>
		, public Service
	{
	public:
		class CachedBitStream;
	private:
		typedef boost::unordered_map<unsigned char, boost::shared_ptr<CachedBitStream> > InnerMap;
		typedef boost::unordered_map<const Assembly*, InnerMap> StreamCacheMap;
		StreamCacheMap streamCache;                                     // 0x64
		boost::shared_mutex sharedMutex;                                // 0x7c
		rbx::signals::scoped_connection addingAssemblyConnection;       // 0x8c
		rbx::signals::scoped_connection removedAssemblyConnection;      // 0x90
	public:
		PhysicsPacketCache();
	protected:
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	};

	// SIZE 0x98
	class InstancePacketCache
		: public DescribedNonCreatable<InstancePacketCache, Instance, sInstancePacketCache>
		, public Service
	{
		class CachedBitStream;
		typedef boost::unordered_map<const Instance*, boost::shared_ptr<CachedBitStream> > StreamCacheMap;
		StreamCacheMap streamCache;                                     // 0x64
		boost::shared_mutex sharedMutex;                                // 0x7c
		std::list<rbx::signals::connection> connections;                // 0x8c 2016 name; ctor _Buynode 0x005e7920
		typedef DescribedNonCreatable<InstancePacketCache, Instance, sInstancePacketCache> Super;
	public:
		InstancePacketCache();
	protected:
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	};

} // namespace Network
} // namespace RBX

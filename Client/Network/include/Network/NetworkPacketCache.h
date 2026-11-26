#pragma once

// Network/NetworkPacketCache.h -- October 2013, partial: the base chains, sizes and the out-of-line
// functions Server::onServiceProvider (PLAYER 0x005b5310) needs. RTTI: DescribedNonCreatable<T,
// Instance, sT> plus Service at 0x60. Sizes from Creatable<Instance>::create<T>'s operator new
// (PLAYER 0x005b2120: 0x94, 0x005b21a0: 0x98). Members past 0x64 are unproven. No 2013 __FILE__
// names this header; the path is 2016's.

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "decomp.h"

namespace RBX {
namespace Network {

	extern const char* const sPhysicsPacketCache;
	extern const char* const sInstancePacketCache;

	// SIZE 0x94
	class PhysicsPacketCache
		: public DescribedNonCreatable<PhysicsPacketCache, Instance, sPhysicsPacketCache>
		, public Service
	{
		undefined m_unk0x64[0x94 - 0x64];               // 0x64
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
		undefined m_unk0x64[0x98 - 0x64];               // 0x64
	public:
		InstancePacketCache();
	protected:
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	};

} // namespace Network
} // namespace RBX

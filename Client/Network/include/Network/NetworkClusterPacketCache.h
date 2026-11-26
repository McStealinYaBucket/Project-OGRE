#pragma once

// Network/NetworkClusterPacketCache.h -- October 2013, partial (see NetworkPacketCache.h). Size 0x98
// from Creatable<Instance>::create<ClusterPacketCache> (PLAYER 0x005b2220). No 2013 __FILE__ names
// this header; the path is 2016's.

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "decomp.h"

namespace RBX {
namespace Network {

	extern const char* const sClusterPacketCache;

	// SIZE 0x98
	class ClusterPacketCache
		: public DescribedNonCreatable<ClusterPacketCache, Instance, sClusterPacketCache>
		, public Service
	{
		undefined m_unk0x64[0x98 - 0x64];               // 0x64
	public:
		ClusterPacketCache();
	protected:
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	};

} // namespace Network
} // namespace RBX

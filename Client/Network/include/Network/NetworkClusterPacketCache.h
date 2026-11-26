#pragma once

// Network/NetworkClusterPacketCache.h -- October 2013, partial (see NetworkPacketCache.h). Size 0x98
// from Creatable<Instance>::create<ClusterPacketCache> (PLAYER 0x005b2220). No 2013 __FILE__ names
// this header; the path is 2016's. Bases from the ctor (PLAYER 0x005d9a10): DescribedNonCreatable,
// then Service (isPublic at 0x64, stored first), then CellChangeListener (vptr at 0x60, one slot:
// terrainCellChanged 0x005d98a0). Members: the stream cache unordered_map at 0x68, boost::shared_mutex
// at 0x80 and two zeroed words at 0x90 (2016: shared_ptr<MegaClusterInstance> clusterInstance).

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Voxel/CellChangeListener.h"
#include "decomp.h"
#include <boost/unordered_map.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace RBX {
namespace Network {

	extern const char* const sClusterPacketCache;

	// SIZE 0x98
	class ClusterPacketCache
		: public DescribedNonCreatable<ClusterPacketCache, Instance, sClusterPacketCache>
		, public Service
		, public Voxel::CellChangeListener
	{
		struct CachedBitStream;
		// 2016 keys the cache by SpatialRegion::Id; the 2013 key type is unproven.
		typedef boost::unordered_map<int, boost::shared_ptr<CachedBitStream> > StreamCacheList;
		StreamCacheList streamCache;                    // 0x68 key and value types unproven
		boost::shared_mutex sharedMutex;                // 0x80
		boost::shared_ptr<void> clusterInstance;        // 0x90 2016: shared_ptr<MegaClusterInstance>
	public:
		ClusterPacketCache();
		/*override*/ void terrainCellChanged(const Voxel::CellChangeInfo& info);
	protected:
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	};

} // namespace Network
} // namespace RBX

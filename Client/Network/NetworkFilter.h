#pragma once

// Network/NetworkFilter.h -- October 2013, partial. No 2013 __FILE__ names it; path from the 2016 tree.
// 4 bytes: ServerReplicator::setBasicFilteringEnabled (PLAYER 0x00573350) news 4 bytes and calls the
// ctor; ctor and dtor are out of line and ICF-folded (PLAYER ctor with Faces::Faces(int) 0x0081c670,
// dtor with the engine `ret` 0x00528720; STUDIO 0x008ed840 / 0x00606c90). The filter members (2016:
// filterChangedProperty, filterParent, filterNew, filterDelete, filterEvent) are not declared yet.

namespace RBX {
namespace Network {

class Replicator;

class NetworkFilter
{
	Replicator* replicator;  // 0x00

public:
	NetworkFilter(Replicator* replicator);
	~NetworkFilter();
};

} // namespace Network
} // namespace RBX

// Network/NetworkFilter.cpp -- October 2013, partial (see NetworkFilter.h).

#include "NetworkFilter.h"

namespace RBX {
namespace Network {

// ICF-folded in 2013: PLAYER 0x0081c670 (Faces::Faces(int)),
// STUDIO 0x008ed840.
NetworkFilter::NetworkFilter(Replicator* replicator)
	: replicator(replicator)
{
}

// ICF-folded in 2013 with the engine `ret`: PLAYER 0x00528720,
// STUDIO 0x00606c90.
NetworkFilter::~NetworkFilter()
{
}

} // namespace Network
} // namespace RBX

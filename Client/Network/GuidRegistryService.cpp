// Network/GuidRegistryService.cpp -- October 2013. Registered from API.cpp (its classDescriptor copy,
// PLAYER 0x00525fa0, is in API.obj); IdSerializer::onServiceProvider creates it.

#include "GuidRegistryService.h"
#include "decomp.h"

namespace RBX {
namespace Network {

const char* const sGuidRegistryService = "GuidRegistryService";

DECOMP_SIZE_ASSERT(GuidRegistryService, 0x6c);

// FUNCTION: PLAYER 0x005ba990
// FUNCTION: STUDIO 0x0061cf70
GuidRegistryService::~GuidRegistryService()
{
}

// FUNCTION: PLAYER 0x005baf10
// FUNCTION: STUDIO 0x0061d900
GuidRegistryService::GuidRegistryService()
	: registry(GuidItem<Instance>::Registry::create())
{
}

} // namespace Network
} // namespace RBX

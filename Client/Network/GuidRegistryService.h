#pragma once

// Network/GuidRegistryService.h -- October 2013. No 2013 assert names the file; the path is 2016's.
// RTTI: DescribedNonCreatable<GuidRegistryService, Instance, sGuidRegistryService> plus Service (its bool
// at 0x60, stored 1 by the ctor PLAYER 0x005baf10). The registry is a shared_ptr in 2013 (the dtor
// 0x005ba990 releases +0x68 with the shared_ptr pattern; 2016 holds an intrusive_ptr).

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/Guid.h"

namespace RBX {
namespace Network {

	extern const char* const sGuidRegistryService;

	// SIZE 0x6c
	class GuidRegistryService
		: public DescribedNonCreatable<GuidRegistryService, Instance, sGuidRegistryService>
		, public Service
	{
	public:
		const shared_ptr<GuidItem<Instance>::Registry> registry;   // 0x64 (Registry::create 0x005baea0)

		GuidRegistryService();
		~GuidRegistryService();
	};

} // namespace Network
} // namespace RBX

/* V8DataModel/MarketplaceService.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<MarketplaceService> from DataModel::
     initializeContents 0x2D3336; "MarketplaceService" string in both 2013 PEs.
   Bases (DescribedCreatable + Service) carried from March 2016 header.
   TODO: full decompile (product-info/purchase machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/MarketplaceService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sMarketplaceService;

	class MarketplaceService
		: public DescribedCreatable<MarketplaceService, Instance, sMarketplaceService, Reflection::ClassDescriptor::INTERNAL>
		, public Service
	{
	public:
		MarketplaceService() {}
	};

} // namespace RBX

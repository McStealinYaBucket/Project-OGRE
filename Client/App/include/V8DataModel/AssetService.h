/* V8DataModel/AssetService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<AssetService> from DataModel::initializeContents
     0x2D3336 (created LAST in initializeContents (function tail).); "AssetService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/AssetService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sAssetService;

	class AssetService
		: public DescribedNonCreatable<AssetService, Instance, sAssetService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		enum AccessType
		{
			ME = 0,
			FRIENDS = 1,
			EVERYONE = 2,
			INVITEONLY = 3,
		};

		AssetService() {}
	};

} // namespace RBX

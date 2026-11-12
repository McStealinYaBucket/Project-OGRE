/* V8DataModel/DebrisService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<DebrisService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "DebrisService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DebrisService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sDebrisService;

	class DebrisService
		: public DescribedNonCreatable<DebrisService, Instance, sDebrisService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		DebrisService() {}
	};

} // namespace RBX

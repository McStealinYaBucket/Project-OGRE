/* V8DataModel/ScriptService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<ScriptService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "ScriptService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ScriptService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sScriptService;

	class ScriptService
		: public DescribedNonCreatable<ScriptService, Instance, sScriptService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		ScriptService() {}
	};

} // namespace RBX

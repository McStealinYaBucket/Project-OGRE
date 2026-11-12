/* V8DataModel/ServerScriptService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: referenced from DataModel::clearContents 0x2CF1D0 / doCloseDataModel
     0x2D3952 (clearContents 0x2CF1D0 references it.); "ServerScriptService" string check TODO per service.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ServerScriptService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sServerScriptService;

	class ServerScriptService
		: public DescribedNonCreatable<ServerScriptService, Instance, sServerScriptService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		ServerScriptService() {}
	};

} // namespace RBX

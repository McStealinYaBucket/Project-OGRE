/* V8DataModel/SpawnerService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: referenced from DataModel::clearContents 0x2CF1D0 / doCloseDataModel
     0x2D3952 (clearContents 0x2CF1D0 references it.); "SpawnerService" string check TODO per service.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SpawnerService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sSpawnerService;

	class SpawnerService
		: public DescribedNonCreatable<SpawnerService, Instance, sSpawnerService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		SpawnerService() {}

		void ClearContents();  // 0x2CF1D0 calls this (capital C per binary). TODO: body.
	};

} // namespace RBX

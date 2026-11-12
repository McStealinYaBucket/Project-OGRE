/* V8DataModel/ReplicatedStorage.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: referenced from DataModel::clearContents 0x2CF1D0 / doCloseDataModel
     0x2D3952 (clearContents 0x2CF1D0 references it.); "ReplicatedStorage" string check TODO per service.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ReplicatedStorage.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sReplicatedStorage;

	class ReplicatedStorage
		: public DescribedNonCreatable<ReplicatedStorage, Instance, sReplicatedStorage, Reflection::ClassDescriptor::PERSISTENT>
		, public Service
	{
	public:
		ReplicatedStorage() {}
	};

} // namespace RBX

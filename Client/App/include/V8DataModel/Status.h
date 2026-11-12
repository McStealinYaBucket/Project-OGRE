/* V8DataModel/Status.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<StatusInstance, ModelInstance> 0x60822C (Creator 0x607DDC); sStatusInstance="Status";
     sStatusInstance = "Status".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Status.h */

#pragma once

#include "V8DataModel/ModelInstance.h"

namespace RBX {

	extern const char* const sStatusInstance;

	class Status
		: public DescribedCreatable<Status, ModelInstance, sStatusInstance>
	{
	public:
		Status() {}
	};

} // namespace RBX

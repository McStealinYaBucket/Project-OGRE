/* V8DataModel/ParallelRampInstance.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<ParallelRampInstance, PartInstance> 0x36643E; s-var @0xEDF950 -> "ParallelRampPart";
     sParallelRampInstance = "ParallelRampPart" (s-var VALUE is the Lua name; C++ name stays short).
   TODO: full decompile (geometry/param surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ParallelRampInstance.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sParallelRamp;

	class ParallelRampInstance
		: public DescribedNonCreatable<ParallelRampInstance, PartInstance, sParallelRamp>
	{
	public:
		ParallelRampInstance() {}
	};

} // namespace RBX

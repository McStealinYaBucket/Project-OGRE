/* V8DataModel/RightAngleRampInstance.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<RightAngleRampInstance, PartInstance> 0x36637A; s-var @0xEE2B90 -> "RightAngleRampPart";
     sRightAngleRampInstance = "RightAngleRampPart" (s-var VALUE is the Lua name; C++ name stays short).
   TODO: full decompile (geometry/param surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RightAngleRampInstance.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sRightAngleRamp;

	class RightAngleRampInstance
		: public DescribedNonCreatable<RightAngleRampInstance, PartInstance, sRightAngleRamp>
	{
	public:
		RightAngleRampInstance() {}
	};

} // namespace RBX

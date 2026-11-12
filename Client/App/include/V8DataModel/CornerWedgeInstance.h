/* V8DataModel/CornerWedgeInstance.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<CornerWedgeInstance, PartInstance> 0x3662B6; value by s-var pattern (sParallelRamp/sRightAngleRamp proven *Part) + metadata + PE table;
     sCornerWedgeInstance = "CornerWedgePart" (s-var VALUE is the Lua name; C++ name stays short).
   TODO: full decompile (geometry/param surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CornerWedgeInstance.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sCornerWedge;

	class CornerWedgeInstance
		: public DescribedCreatable<CornerWedgeInstance, PartInstance, sCornerWedge>
	{
	public:
		CornerWedgeInstance() {}
	};

} // namespace RBX

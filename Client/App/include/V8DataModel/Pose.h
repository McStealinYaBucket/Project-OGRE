/* V8DataModel/Pose.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Pose, Instance> 0x421694;
     sPose = "Pose".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Pose.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sPose;

	class Pose
		: public DescribedCreatable<Pose, Instance, sPose>
	{
	public:
		Pose() {}
	};

} // namespace RBX

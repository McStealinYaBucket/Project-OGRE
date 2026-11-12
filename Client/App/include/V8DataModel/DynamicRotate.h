/* V8DataModel/DynamicRotate.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<DynamicRotate, JointInstance> 0x415434; abstract joint root;
     sDynamicRotate = "DynamicRotate".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DynamicRotate.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sDynamicRotate;

	class DynamicRotate
		: public DescribedNonCreatable<DynamicRotate, JointInstance, sDynamicRotate>
	{
	public:
		DynamicRotate() {}
	};

} // namespace RBX

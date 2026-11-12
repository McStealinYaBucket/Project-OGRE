/* V8DataModel/RotateP.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<RotateP, DynamicRotate> 0x367196;
     sRotateP = "RotateP".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RotateP.h */

#pragma once

#include "V8DataModel/DynamicRotate.h"

namespace RBX {

	extern const char* const sRotateP;

	class RotateP
		: public DescribedCreatable<RotateP, DynamicRotate, sRotateP>
	{
	public:
		RotateP() {}
	};

} // namespace RBX

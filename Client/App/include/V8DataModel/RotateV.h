/* V8DataModel/RotateV.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<RotateV, DynamicRotate> 0x415294;
     sRotateV = "RotateV".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RotateV.h */

#pragma once

#include "V8DataModel/DynamicRotate.h"

namespace RBX {

	extern const char* const sRotateV;

	class RotateV
		: public DescribedCreatable<RotateV, DynamicRotate, sRotateV>
	{
	public:
		RotateV() {}
	};

} // namespace RBX

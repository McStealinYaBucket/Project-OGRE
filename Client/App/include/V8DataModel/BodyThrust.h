/* V8DataModel/BodyThrust.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyThrust, BodyMover> 0x3D8F70;
     sBodyThrust = "BodyThrust".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyThrust.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sBodyThrust;

	class BodyThrust
		: public DescribedCreatable<BodyThrust, BodyMover, sBodyThrust>
	{
	public:
		BodyThrust() {}
	};

} // namespace RBX

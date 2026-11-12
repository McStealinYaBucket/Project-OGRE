/* V8DataModel/BodyVelocity.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyVelocity, BodyMover> 0x3D9F7A;
     sBodyVelocity = "BodyVelocity".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyVelocity.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sBodyVelocity;

	class BodyVelocity
		: public DescribedCreatable<BodyVelocity, BodyMover, sBodyVelocity>
	{
	public:
		BodyVelocity() {}
	};

} // namespace RBX

/* V8DataModel/BodyAngularVelocity.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyAngularVelocity, BodyMover> 0x3D98F2;
     sBodyAngularVelocity = "BodyAngularVelocity".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyAngularVelocity.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sBodyAngularVelocity;

	class BodyAngularVelocity
		: public DescribedCreatable<BodyAngularVelocity, BodyMover, sBodyAngularVelocity>
	{
	public:
		BodyAngularVelocity() {}
	};

} // namespace RBX

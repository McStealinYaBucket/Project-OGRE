/* V8DataModel/BodyForce.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyForce, BodyMover> 0x3D9436;
     sBodyForce = "BodyForce".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyForce.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sBodyForce;

	class BodyForce
		: public DescribedCreatable<BodyForce, BodyMover, sBodyForce>
	{
	public:
		BodyForce() {}
	};

} // namespace RBX

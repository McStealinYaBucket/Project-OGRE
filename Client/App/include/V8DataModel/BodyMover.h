/* V8DataModel/BodyMover.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<BodyMover, Instance> 0x3DD2B8; abstract mover root;
     sBodyMover = "BodyMover".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyMover.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sBodyMover;

	class BodyMover
		: public DescribedNonCreatable<BodyMover, Instance, sBodyMover>
	{
	public:
		BodyMover() {}
	};

} // namespace RBX

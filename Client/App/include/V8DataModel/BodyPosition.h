/* V8DataModel/BodyPosition.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyPosition, BodyMover> 0x3DA78A;
     sBodyPosition = "BodyPosition".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyPosition.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sBodyPosition;

	class BodyPosition
		: public DescribedCreatable<BodyPosition, BodyMover, sBodyPosition>
	{
	public:
		BodyPosition() {}
	};

} // namespace RBX

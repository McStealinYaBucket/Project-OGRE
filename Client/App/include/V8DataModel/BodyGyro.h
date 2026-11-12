/* V8DataModel/BodyGyro.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyGyro, BodyMover> 0x3DB3DA;
     sBodyGyro = "BodyGyro".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyGyro.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sBodyGyro;

	class BodyGyro
		: public DescribedCreatable<BodyGyro, BodyMover, sBodyGyro>
	{
	public:
		BodyGyro() {}
	};

} // namespace RBX

/* V8DataModel/RocketPropulsion.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Rocket, BodyMover> 0x3DBDF8 (Creator 0x3DD6FA; setTarget/fire/abort semantics); sRocket="RocketPropulsion";
     sRocket = "RocketPropulsion".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RocketPropulsion.h */

#pragma once

#include "V8DataModel/BodyMover.h"

namespace RBX {

	extern const char* const sRocket;

	class RocketPropulsion
		: public DescribedCreatable<RocketPropulsion, BodyMover, sRocket>
	{
	public:
		RocketPropulsion() {}
	};

} // namespace RBX

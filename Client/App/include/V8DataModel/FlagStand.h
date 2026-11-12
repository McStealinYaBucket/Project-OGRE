/* V8DataModel/FlagStand.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<FlagStand, BasicPartInstance> 0x3697DE;
     sFlagStand = "FlagStand" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FlagStand.h */

#pragma once

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {

	extern const char* const sFlagStand;

	class FlagStand
		: public DescribedCreatable<FlagStand, BasicPartInstance, sFlagStand>
	{
	public:
		FlagStand() {}
	};

} // namespace RBX

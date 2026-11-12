/* V8DataModel/Humanoid.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Humanoid, Instance> 0x5E3F6A;
     sHumanoid = "Humanoid" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Humanoid.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sHumanoid;

	class Humanoid
		: public DescribedCreatable<Humanoid, Instance, sHumanoid>
	{
	public:
		Humanoid() {}
	};

} // namespace RBX

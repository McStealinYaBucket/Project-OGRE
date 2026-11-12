/* V8DataModel/StarterGear.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<StarterGear, Instance> 0x3EEF6C;
     sStarterGear = "StarterGear".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StarterGear.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sStarterGear;

	class StarterGear
		: public DescribedCreatable<StarterGear, Instance, sStarterGear>
	{
	public:
		StarterGear() {}
	};

} // namespace RBX

/* V8DataModel/Pants.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Pants, Clothing> 0x2A8AD4;
     sPants = "Pants".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Pants.h */

#pragma once

#include "V8DataModel/Clothing.h"

namespace RBX {

	extern const char* const sPants;

	class Pants
		: public DescribedCreatable<Pants, Clothing, sPants>
	{
	public:
		Pants() {}
	};

} // namespace RBX

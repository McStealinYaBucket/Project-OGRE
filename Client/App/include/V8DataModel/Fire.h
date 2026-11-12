/* V8DataModel/Fire.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Fire, Instance>;
     sFire = "Fire" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Fire.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sFire;

	class Fire
		: public DescribedCreatable<Fire, Instance, sFire>
	{
	public:
		Fire() {}
	};

} // namespace RBX

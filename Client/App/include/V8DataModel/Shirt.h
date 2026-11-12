/* V8DataModel/Shirt.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Shirt, Clothing> 0x2A8F10;
     sShirt = "Shirt".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Shirt.h */

#pragma once

#include "V8DataModel/Clothing.h"

namespace RBX {

	extern const char* const sShirt;

	class Shirt
		: public DescribedCreatable<Shirt, Clothing, sShirt>
	{
	public:
		Shirt() {}
	};

} // namespace RBX

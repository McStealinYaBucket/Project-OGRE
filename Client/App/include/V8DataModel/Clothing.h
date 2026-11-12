/* V8DataModel/Clothing.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Clothing, CharacterAppearance> 0x2A8E4C;
     sClothing = "Clothing".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Clothing.h */

#pragma once

#include "V8DataModel/CharacterAppearance.h"

namespace RBX {

	extern const char* const sClothing;

	class Clothing
		: public DescribedNonCreatable<Clothing, CharacterAppearance, sClothing>
	{
	public:
		Clothing() {}
	};

} // namespace RBX

/* V8DataModel/Skin.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Skin, LegacyCharacterAppearance> 0x2A8714;
     sSkin = "Skin".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Skin.h */

#pragma once

#include "V8DataModel/LegacyCharacterAppearance.h"

namespace RBX {

	extern const char* const sSkin;

	class Skin
		: public DescribedCreatable<Skin, LegacyCharacterAppearance, sSkin>
	{
	public:
		Skin() {}
	};

} // namespace RBX

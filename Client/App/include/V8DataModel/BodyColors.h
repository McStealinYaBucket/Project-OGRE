/* V8DataModel/BodyColors.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BodyColors, LegacyCharacterAppearance> 0x2A8310;
     sBodyColors = "BodyColors".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BodyColors.h */

#pragma once

#include "V8DataModel/LegacyCharacterAppearance.h"

namespace RBX {

	extern const char* const sBodyColors;

	class BodyColors
		: public DescribedCreatable<BodyColors, LegacyCharacterAppearance, sBodyColors>
	{
	public:
		BodyColors() {}
	};

} // namespace RBX

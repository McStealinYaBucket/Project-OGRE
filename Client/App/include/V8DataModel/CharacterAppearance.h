/* V8DataModel/CharacterAppearance.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<CharacterAppearance, Instance> 0x2AC438; C++-only (no metadata);
     sCharacterAppearance = "CharacterAppearance".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CharacterAppearance.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sCharacterAppearance;

	class CharacterAppearance
		: public DescribedNonCreatable<CharacterAppearance, Instance, sCharacterAppearance>
	{
	public:
		CharacterAppearance() {}
	};

} // namespace RBX

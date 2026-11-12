/* V8DataModel/CharacterMesh.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<CharacterMesh, CharacterAppearance> 0x2AC308;
     sCharacterMesh = "CharacterMesh".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CharacterMesh.h */

#pragma once

#include "V8DataModel/CharacterAppearance.h"

namespace RBX {

	extern const char* const sCharacterMesh;

	class CharacterMesh
		: public DescribedCreatable<CharacterMesh, CharacterAppearance, sCharacterMesh>
	{
	public:
		CharacterMesh() {}
	};

} // namespace RBX

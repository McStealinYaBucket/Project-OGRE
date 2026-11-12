/* V8DataModel/Texture.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Texture, Decal> 0x32D478;
     sTexture = "Texture" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Texture.h */

#pragma once

#include "V8DataModel/Decal.h"

namespace RBX {

	extern const char* const sTexture;

	class Texture
		: public DescribedCreatable<Texture, Decal, sTexture>
	{
	public:
		Texture() {}
	};

} // namespace RBX

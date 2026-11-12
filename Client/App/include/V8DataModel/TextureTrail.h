/* V8DataModel/TextureTrail.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<TextureTrail, GuiBase3d> 0x676CAC;
     sTextureTrail = "TextureTrail".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TextureTrail.h */

#pragma once

#include "V8DataModel/GuiBase3d.h"

namespace RBX {

	extern const char* const sTextureTrail;

	class TextureTrail
		: public DescribedCreatable<TextureTrail, GuiBase3d, sTextureTrail>
	{
	public:
		TextureTrail() {}
	};

} // namespace RBX

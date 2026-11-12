/* V8DataModel/Decal.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Decal, FaceInstance> 0x32DA38;
     sDecal = "Decal" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Decal.h */

#pragma once

#include "V8DataModel/FaceInstance.h"

namespace RBX {

	extern const char* const sDecal;

	class Decal
		: public DescribedCreatable<Decal, FaceInstance, sDecal>
	{
	public:
		Decal() {}
	};

} // namespace RBX

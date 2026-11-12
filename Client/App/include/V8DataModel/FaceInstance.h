/* V8DataModel/FaceInstance.h -- October 2013 STUB (minimal shell).
   Evidence: Decal parent (FactoryProduct<Decal, FaceInstance> 0x32DA38); C++-only (no metadata);
     sFaceInstance = "FaceInstance" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FaceInstance.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sFaceInstance;

	class FaceInstance
		: public DescribedNonCreatable<FaceInstance, Instance, sFaceInstance>
	{
	public:
		FaceInstance() {}
	};

} // namespace RBX

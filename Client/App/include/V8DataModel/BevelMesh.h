/* V8DataModel/BevelMesh.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<BevelMesh, DataModelMesh> 0x282C78;
     sBevelMesh = "BevelMesh".
   TODO: full decompile (mesh geometry surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BevelMesh.h */

#pragma once

#include "V8DataModel/DataModelMesh.h"

namespace RBX {

	extern const char* const sBevelMesh;

	class BevelMesh
		: public DescribedNonCreatable<BevelMesh, DataModelMesh, sBevelMesh>
	{
	public:
		BevelMesh() {}
	};

} // namespace RBX

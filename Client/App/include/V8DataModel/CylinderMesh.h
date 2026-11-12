/* V8DataModel/CylinderMesh.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<CylinderMesh, BevelMesh> 0x2CD2D0;
     sCylinderMesh = "CylinderMesh".
   TODO: full decompile (mesh geometry surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CylinderMesh.h */

#pragma once

#include "V8DataModel/BevelMesh.h"

namespace RBX {

	extern const char* const sCylinderMesh;

	class CylinderMesh
		: public DescribedCreatable<CylinderMesh, BevelMesh, sCylinderMesh>
	{
	public:
		CylinderMesh() {}
	};

} // namespace RBX

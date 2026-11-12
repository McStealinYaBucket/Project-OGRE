/* V8DataModel/SpecialMesh.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SpecialShape, FileMesh> 0x8287E8 (Creator 0x49D150; MeshType enum proves rename); sSpecialShape="SpecialMesh";
     sSpecialShape = "SpecialMesh".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SpecialMesh.h */

#pragma once

#include "V8DataModel/FileMesh.h"

namespace RBX {

	extern const char* const sSpecialShape;

	class SpecialMesh
		: public DescribedCreatable<SpecialMesh, FileMesh, sSpecialShape>
	{
	public:
		SpecialMesh() {}
	};

} // namespace RBX

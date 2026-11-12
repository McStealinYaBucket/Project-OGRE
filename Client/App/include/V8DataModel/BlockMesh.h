/* V8DataModel/BlockMesh.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BlockMesh, BevelMesh> 0x2877A0;
     sBlockMesh = "BlockMesh".
   TODO: full decompile (mesh geometry surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BlockMesh.h */

#pragma once

#include "V8DataModel/BevelMesh.h"

namespace RBX {

	extern const char* const sBlockMesh;

	class BlockMesh
		: public DescribedCreatable<BlockMesh, BevelMesh, sBlockMesh>
	{
	public:
		BlockMesh() {}
	};

} // namespace RBX

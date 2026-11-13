/* V8DataModel/DataModelMesh.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<DataModelMesh, Instance> 0x31458E; abstract mesh root (DataModelMesh C2 0x31393E);
     sDataModelMesh = "DataModelMesh".
   TODO: full decompile (mesh geometry surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DataModelMesh.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDataModelMesh;

	class DataModelMesh
		: public DescribedNonCreatable<DataModelMesh, Instance, sDataModelMesh>
	{
	public:
		enum LODType
		{
			LOW_LOD = 0,
			MEDIUM_LOD = 1,
			HIGH_LOD = 2,
		};

		DataModelMesh() {}
	};

} // namespace RBX

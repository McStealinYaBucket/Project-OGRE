/* V8DataModel/MegaCluster.h — October 2013 STUB (voxel terrain shell).
   Evidence: Workspace::createTerrain 0x514E84 creates MegaClusterInstance via
     Creatable<Instance>::create + setAndLockParent when the terrain slot
     (Workspace+110) is null; asserts "terrain" (Workspace.cpp).
     Binary hierarchy: DescribedCreatable<MegaClusterInstance, PartInstance,
     sMegaCluster> (see FactoryProduct/Creator EAs in handoff).
     TRUE parent is PartInstance — Instance stands in directly until the
     v8datamodel physics-part chain (PVInstance etc.) is reconstructed.
   TODO: full decompile (voxel grid, encode/decode, cell scripts).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/MegaCluster.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sMegaCluster;

	class MegaClusterInstance
		: public DescribedCreatable<MegaClusterInstance, PartInstance, sMegaCluster>
		// Restored TRUE parent (was Instance standing in; binary:
		// DescribedCreatable<MegaClusterInstance, PartInstance, sMegaCluster>).
	{
	public:
		MegaClusterInstance() {}
	};

} // namespace RBX

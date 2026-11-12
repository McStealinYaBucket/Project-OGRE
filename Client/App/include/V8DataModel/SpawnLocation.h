/* V8DataModel/SpawnLocation.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SpawnLocation, BasicPartInstance> 0x366E72;
     sSpawnLocation = "SpawnLocation" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SpawnLocation.h */

#pragma once

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {

	extern const char* const sSpawnLocation;

	class SpawnLocation
		: public DescribedCreatable<SpawnLocation, BasicPartInstance, sSpawnLocation>
	{
	public:
		SpawnLocation() {}
	};

} // namespace RBX

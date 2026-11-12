/* V8DataModel/Lighting.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::find<Lighting> from DataModel::getLightingDeprecated
     0x2D1636; "Lighting" string in both 2013 PEs.
   Bases (DescribedCreatable + Service) carried from March 2016 header.
   TODO: full decompile (sky/fog/time-of-day machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Lighting.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sLighting;

	class Lighting
		: public DescribedCreatable<Lighting, Instance, sLighting>
		, public Service
	{
	public:
		Lighting() {}
	};

} // namespace RBX

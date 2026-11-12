/* V8DataModel/SpotLight.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SpotLight, Light> 0x6EE884;
     sSpotLight = "SpotLight".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SpotLight.h */

#pragma once

#include "V8DataModel/Light.h"

namespace RBX {

	extern const char* const sSpotLight;

	class SpotLight
		: public DescribedCreatable<SpotLight, Light, sSpotLight>
	{
	public:
		SpotLight() {}
	};

} // namespace RBX

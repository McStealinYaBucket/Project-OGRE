/* V8DataModel/PointLight.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<PointLight, Light> 0x36612A;
     sPointLight = "PointLight".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PointLight.h */

#pragma once

#include "V8DataModel/Light.h"

namespace RBX {

	extern const char* const sPointLight;

	class PointLight
		: public DescribedCreatable<PointLight, Light, sPointLight>
	{
	public:
		PointLight() {}
	};

} // namespace RBX

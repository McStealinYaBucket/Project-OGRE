/* V8DataModel/Light.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Light, Instance> 0x366066; abstract light root (Light.cpp 0-byte stub replaced);
     sLight = "Light".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Light.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sLight;

	class Light
		: public DescribedNonCreatable<Light, Instance, sLight>
	{
	public:
		Light() {}
	};

} // namespace RBX

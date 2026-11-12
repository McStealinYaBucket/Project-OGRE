/* V8DataModel/Platform.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED (true Described<Platform, sPlatform, PlatformImpl<BasicPartInstance>>); PlatformImpl TODO;
     sPlatform = "Platform".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Platform.h */

#pragma once

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {

	extern const char* const sPlatform;

	class Platform
		: public DescribedCreatable<Platform, BasicPartInstance, sPlatform>
	{
	public:
		Platform() {}
	};

} // namespace RBX

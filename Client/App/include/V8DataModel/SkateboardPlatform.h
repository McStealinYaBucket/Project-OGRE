/* V8DataModel/SkateboardPlatform.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED (true FactoryProduct<SkateboardPlatform, PlatformImpl<BasicPartInstance>>); PlatformImpl TODO;
     sSkateboardPlatform = "SkateboardPlatform".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SkateboardPlatform.h */

#pragma once

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {

	extern const char* const sSkateboardPlatform;

	class SkateboardPlatform
		: public DescribedCreatable<SkateboardPlatform, BasicPartInstance, sSkateboardPlatform>
	{
	public:
		SkateboardPlatform() {}
	};

} // namespace RBX

/* V8DataModel/PluginMouse.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<PluginMouse, Mouse> 0x6A75EC; abstract (no creator by design);
     sPluginMouse = "PluginMouse".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PluginMouse.h */

#pragma once

#include "V8DataModel/Mouse.h"

namespace RBX {

	extern const char* const sPluginMouse;

	class PluginMouse
		: public DescribedNonCreatable<PluginMouse, Mouse, sPluginMouse>
	{
	public:
		PluginMouse() {}
	};

} // namespace RBX

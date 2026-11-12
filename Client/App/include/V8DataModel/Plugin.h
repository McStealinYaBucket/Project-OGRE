/* V8DataModel/Plugin.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Plugin, Instance> 0x6A25F4;
     sPlugin = "Plugin".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Plugin.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sPlugin;

	class Plugin
		: public DescribedCreatable<Plugin, Instance, sPlugin>
	{
	public:
		Plugin() {}
	};

} // namespace RBX

/* V8DataModel/PluginManager.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<PluginManager, Instance> 0x6A2278;
     sPluginManager = "PluginManager".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PluginManager.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sPluginManager;

	class PluginManager
		: public DescribedNonCreatable<PluginManager, Instance, sPluginManager>
	{
	public:
		PluginManager() {}
		static boost::shared_ptr<PluginManager> singleton();
	};

} // namespace RBX

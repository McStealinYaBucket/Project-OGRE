/* V8DataModel/Settings.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Settings, ServiceProvider> 0x392178; C++-only (no metadata);
     sSettings = "Settings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Settings.h */

#pragma once

#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sSettings;

	class Settings
		: public DescribedNonCreatable<Settings, ServiceProvider, sSettings>
	{
	public:
		Settings() {}
	};

} // namespace RBX

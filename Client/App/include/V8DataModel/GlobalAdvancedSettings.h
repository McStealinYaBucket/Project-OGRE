/* V8DataModel/GlobalAdvancedSettings.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<GlobalAdvancedSettings, Settings> 0x369402; C++-only (no metadata);
     sGlobalAdvancedSettings = "GlobalAdvancedSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GlobalAdvancedSettings.h */

#pragma once

#include "V8DataModel/Settings.h"

namespace RBX {

	extern const char* const sGlobalAdvancedSettings;

	class GlobalAdvancedSettings
		: public DescribedNonCreatable<GlobalAdvancedSettings, Settings, sGlobalAdvancedSettings>
	{
	public:
		GlobalAdvancedSettings() {}
		static boost::shared_ptr<GlobalAdvancedSettings> singleton();
	};

} // namespace RBX

/* V8DataModel/GlobalBasicSettings.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<GlobalBasicSettings, Settings> 0x36933A; C++-only (no metadata);
     sGlobalBasicSettings = "GlobalBasicSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GlobalBasicSettings.h */

#pragma once

#include "V8DataModel/Settings.h"

namespace RBX {

	extern const char* const sGlobalBasicSettings;

	class GlobalBasicSettings
		: public DescribedNonCreatable<GlobalBasicSettings, Settings, sGlobalBasicSettings>
	{
	public:
		GlobalBasicSettings() {}
		static boost::shared_ptr<GlobalBasicSettings> singleton();
	};

} // namespace RBX

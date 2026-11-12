/* V8DataModel/NetworkSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<NetworkSettings, GlobalAdvancedSettings::Item> 0x8703B2 (Item 0x86DF76, Creator 0x8396A0; Item template TODO);
     sNetworkSettings = "NetworkSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NetworkSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {

	extern const char* const sNetworkSettings;

	class NetworkSettings
		: public DescribedCreatable<NetworkSettings, GlobalAdvancedSettings, sNetworkSettings>
	{
	public:
		NetworkSettings() {}
	};

} // namespace RBX

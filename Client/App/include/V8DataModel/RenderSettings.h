/* V8DataModel/RenderSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<CRenderSettingsItem, GlobalAdvancedSettings::Item> (player RTTI: VCRenderSettingsItem Creator + sRenderSettings Described; Item template TODO; CRenderSettings C++ split, see studio 0x82A6A0);
     sRenderSettings = "RenderSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RenderSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {

	extern const char* const sRenderSettings;

	class RenderSettings
		: public DescribedCreatable<RenderSettings, GlobalAdvancedSettings, sRenderSettings>
	{
	public:
		RenderSettings() {}
	};

} // namespace RBX

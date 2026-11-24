/* V8DataModel/RenderSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<CRenderSettingsItem, GlobalAdvancedSettings::Item> (player RTTI: VCRenderSettingsItem Creator + sRenderSettings Described; Item template TODO; CRenderSettings C++ split, see studio 0x82A6A0);
     sRenderSettings = "RenderSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RenderSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {

	extern const char* const sRenderSettings;

	// No 2013 class: the 2013 RenderSettings item is the shell's CRenderSettingsItem. Kept for
	// GfxBase/RenderSettings.cpp's registrar. It derives the item template (no ServiceProvider/Settings
	// chain), so that /Od TU no longer emits ~ServiceProvider or the Settings descriptors, which
	// STUDIO links ahead of App's copies (2013 keeps ~ServiceProvider from DataModel.obj).
	class RenderSettings
		: public GlobalAdvancedSettingsItem<RenderSettings, sRenderSettings>
	{
	public:
		RenderSettings() {}
	};

} // namespace RBX

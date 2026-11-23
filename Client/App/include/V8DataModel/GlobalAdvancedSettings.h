/* V8DataModel/GlobalAdvancedSettings.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<GlobalAdvancedSettings, Settings> 0x369402; C++-only (no metadata);
     sGlobalAdvancedSettings = "GlobalSettings" (PLAYER 0x00b31084).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GlobalAdvancedSettings.h */

#pragma once

#include "V8DataModel/Settings.h"
#include "RbxFormat.h"

namespace RBX {

	extern const char* const sGlobalAdvancedSettings;

	extern const char* const sSettingsItem;	// NULL: no class-name string (RTTI NonFactoryProduct<Instance,sSettingsItem>, no "SettingsItem" string in PLAYER)

	class GlobalAdvancedSettings
		: public DescribedNonCreatable<GlobalAdvancedSettings, Settings, sGlobalAdvancedSettings>
	{
	public:
		GlobalAdvancedSettings() {}
		static boost::shared_ptr<GlobalAdvancedSettings> singleton();

		// The base of every settings item (RTTI FactoryProduct<LuaSettings, GlobalAdvancedSettings::Item,
		// sLuaSettings, Instance>). Instance-sized; its virtual overrides are not modelled yet.
		class Item : public NonFactoryProduct<Instance, sSettingsItem>
		{
		};
	};

	// 2013 layout (GlobalAdvancedSettingsItem<LuaSettings,sLuaSettings> ctor PLAYER 0x0085e220):
	// the item (Instance, 0x60), then Service's isPublic byte at +0x60 (stored 1). The ctor names the
	// instance after the class and throws RBX::runtime_error("singleton %s already exists") when the
	// singleton slot is taken (PLAYER 0x00de68a8 for LuaSettings).
	template<class Class, const char* const& sClassName>
	class GlobalAdvancedSettingsItem
		: public DescribedCreatable<Class, GlobalAdvancedSettings::Item, sClassName>
		, public Service
	{
		static GlobalAdvancedSettingsItem* sing;
	protected:
		GlobalAdvancedSettingsItem()
		{
			this->setName(sClassName);
			if (sing)
				throw RBX::runtime_error("singleton %s already exists", sClassName);
			sing = this;
		}
		~GlobalAdvancedSettingsItem()
		{
			sing = NULL;
		}
	};

	template<class Class, const char* const& sClassName>
	GlobalAdvancedSettingsItem<Class, sClassName>* GlobalAdvancedSettingsItem<Class, sClassName>::sing;

} // namespace RBX

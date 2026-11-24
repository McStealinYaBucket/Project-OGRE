/* V8DataModel/GlobalSettings.h -- October 2013.
   The 2013 asserts name this file: "s.get()==sing" at GlobalSettings.h:132
   (GlobalAdvancedSettingsItem<T>::singleton, PLAYER 0x00654600) and :176 (the basic-settings
   twin, not modelled yet). GlobalAdvancedSettings.h only forwards here.

   GlobalAdvancedSettings (ctor PLAYER 0x00658580): Settings (0xac), then a boost::mutex at
   +0xac/+0xb0 (both zeroed in the ctor; singleton() locks it through lea ecx,[gs+0xac]).

   GlobalAdvancedSettings::Item is NonFactoryProduct<Instance, sSettingsItem> (RTTI
   .?AV?$NonFactoryProduct@VInstance@RBX@@$E?sSettingsItem@2@3QBDB@RBX@@). Its askAddChild override
   fills slot 0x3c of every item vftable (PLAYER 0x00b7cf94+0x3c = 0x00455840, the shell's COMDAT).

   GlobalAdvancedSettingsItem<Class, sClassName> (ctor e.g. PLAYER 0x0085e220): the item, then
   Service::isPublic at +0x60 (stored 1), size 0x64. The ctor names the instance after the class
   and throws RBX::runtime_error("singleton %s already exists") when the slot is taken; the dtor
   clears the slot. The ctor pushes the class-name literal because 2013 instantiates it only in
   the item's own .cpp (the item ctors are out of line there). */

#pragma once

#include "V8DataModel/Settings.h"
#include "RbxFormat.h"

#include <boost/thread/mutex.hpp>

namespace RBX {

	extern const char* const sGlobalAdvancedSettings;

	// NULL: no "SettingsItem" string exists in PLAYER.
	extern const char* const sSettingsItem;

	// SIZE 0xb4
	class GlobalAdvancedSettings
		: public DescribedNonCreatable<GlobalAdvancedSettings, Settings, sGlobalAdvancedSettings>
	{
	public:
		GlobalAdvancedSettings() {}
		static boost::shared_ptr<GlobalAdvancedSettings> singleton();

		class Item : public NonFactoryProduct<Instance, sSettingsItem>
		{
		protected:
			// vtable+0x3c (PLAYER 0x00455840)
			/*override*/ bool askAddChild(const Instance* instance) const
			{
				return dynamic_cast<const Item*>(instance) != NULL;
			}
		};

		boost::mutex mutex;	// 0xac
	};

	template<class Class, const char* const& sClassName>
	class GlobalAdvancedSettingsItem
		: public DescribedCreatable<Class, GlobalAdvancedSettings::Item, sClassName>
		, public Service
	{
		static GlobalAdvancedSettingsItem* sing;

	protected:
		GlobalAdvancedSettingsItem()
		{
			Instance::setName(sClassName);
			if (sing)
				throw RBX::runtime_error("singleton %s already exists", sClassName);
			sing = this;
		}
		~GlobalAdvancedSettingsItem()
		{
			sing = NULL;
		}

	public:
		// One out-of-line copy per item (PLAYER: Debug 0x00407700, Game 0x00439860,
		// Network 0x00521d60, Physics 0x00654470, Lua 0x00654600). The fast path returns the slot;
		// otherwise it takes GlobalAdvancedSettings' mutex, creates the item and parents it there
		// (setParentInternal(gs, false) 0x00620440), and asserts the ctor filled the slot.
		static Class& singleton()
		{
			if (sing)
				return *static_cast<Class*>(sing);

			GlobalAdvancedSettings* gs = GlobalAdvancedSettings::singleton().get();
			boost::mutex::scoped_lock lock(gs->mutex);
			if (!sing)
			{
				shared_ptr<Class> s = Class::createInstance();
				s->setParent(gs);
				// (Spacing: the assert sits on 2013 line 132.)









































				RBXASSERT(s.get()==sing);
			}
			return *static_cast<Class*>(sing);
		}
	};

	template<class Class, const char* const& sClassName>
	GlobalAdvancedSettingsItem<Class, sClassName>* GlobalAdvancedSettingsItem<Class, sClassName>::sing;

} // namespace RBX

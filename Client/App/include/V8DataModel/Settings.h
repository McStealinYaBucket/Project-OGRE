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
		Settings(const std::string& settingsFile);	// PLAYER 0x00657f60, out of line

	private:
		typedef DescribedNonCreatable<Settings, ServiceProvider, sSettings> Super;

		// One byte: operator() ORs into it (`or byte [esi],al`, PLAYER 0x006566d0). invalid() is a cdecl
		// static of three RTTI tests (0x006562c0).
		struct InvalidDescendentDetector
		{
			bool anyInvalid;
			InvalidDescendentDetector() : anyInvalid(false) {}
			static bool invalid(const Instance* instance);
			void operator()(shared_ptr<Instance> descendant);
		};

	protected:
		// Instance vtable+0x38 (GlobalAdvancedSettings vftable slot 0x38).
		virtual void verifyAddDescendant(const Instance* newParent, const Instance* instanceGettingNewParent) const;

	protected:
		// 0x8c..0xac: GlobalAdvancedSettings' mutex sits at +0xac (ctor PLAYER 0x00658580), so Settings
		// is 0xac. The Settings ctors take a std::string by value (PLAYER 0x00658480 chain).
		std::string settingsFile;	// 0x8c  src2016 name
		bool settingsErased;		// 0xa8  src2016 name
	};

} // namespace RBX

// Slot 0 of the ServiceProvider, DataModel and Settings vftables (PLAYER 0x00b2eb90, 0x00b3008c,
// 0x00b311f4) is one folded thunk into ~ServiceProvider. Since Settings gained its string member,
// GlobalBasicSettings' own thunk is no longer that twin, so the pairing is pinned here.
// clang-format off
// SYNTHETIC: PLAYER 0x00644360
// SYNTHETIC: STUDIO 0x0073fce0
// ??_E?$DescribedNonCreatable@VDataModel@RBX@@VServiceProvider@2@$E?sDataModel@2@3QBDB$0BL@$0A@@RBX@@WBM@AEPAXI@Z
// clang-format on

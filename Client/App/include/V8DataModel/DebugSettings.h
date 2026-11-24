/* V8DataModel/DebugSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<DebugSettings, GlobalAdvancedSettings::Item> 0x31F5EC (Item 0x2E5DB8, Creator 0x2FBED0; Item template TODO);
     sDebugSettings = "DebugSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DebugSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"
#include "Util/Limits.h"

namespace RBX {

	extern const char* const sDebugSettings;

	// 2013 base: GlobalAdvancedSettingsItem<DebugSettings, sDebugSettings> (0x64 bytes; ctor PLAYER
	// 0x0065ebf0). Layout from the 2013 ctor 0x0065fa10; size 0x7c (Creator::create 0x0040d690).
	// SIZE 0x7c
	class DebugSettings
		: public GlobalAdvancedSettingsItem<DebugSettings, sDebugSettings>
	{
	public:
		enum ErrorReporting
		{
			DontReport = 0,
			Prompt = 1,
			Report = 2,
		};

		DebugSettings();

		// printCallStack reads this byte at +0x64 (ScriptContext 2013); the ctor stores 1.
		// Name from the 2016 "IsScriptStackTracingEnabled" property.
		bool stackTracingEnabled;	// 0x64
		bool getStackTracingEnabled() const { return stackTracingEnabled; }

		float m_unk0x68;			// 0x68  ctor movss 0
		float m_unk0x6c;			// 0x6c  ctor movss 0
		bool m_unk0x70;				// 0x70
		bool m_unk0x71;				// 0x71
		// openFmod reads it (Mac +0x6a; the Mac bytes 0x68..0x6b are these four flags). Name from
		// the settings string "IsFmodProfilingEnabled" (0xB46641).
		bool isFmodProfilingEnabled;	// 0x72
		bool getIsFmodProfilingEnabled() const { return isFmodProfilingEnabled; }
		bool m_unk0x73;				// 0x73
		int m_unk0x74;				// 0x74  ctor stores 2
		bool m_unk0x78;				// 0x78

		// mac: 0x31A710 / 0x31A73A: instance-count limiter setters.
		void setInstanceCountLimit(int n) { RBX::Limits::Counter::countLimit = n; }
		int getInstanceCountLimit() const { return RBX::Limits::Counter::countLimit; }
		void setEnforceInstanceCountLimit(bool b) { RBX::Limits::Counter::globallyEnforcedLimit = b; }
		bool getEnforceInstanceCountLimit() const { return RBX::Limits::Counter::globallyEnforcedLimit; }
		// (Spacing: getRobloxVersion's assert sits on 2013 line 85.)





















		// Pushed by ScriptContext::version (0x18E942, section 249).
		// 2016-carry ("?" default); the RobloxVersion property queues
		// behind the reflection era.
		static std::string robloxVersion;
		// 2016-carry (binary EA 0x31BEE4 exists; body unobserved).
		// STUB: PLAYER 0x00658b40
		// STUB: STUDIO 0x007735a0
		std::string getRobloxVersion() const
		{
			RBXASSERT(!robloxVersion.empty());
			return robloxVersion;
		}
	};

} // namespace RBX

// The FactoryProduct base's out-of-line members; their names spell the 2013 base
// GlobalAdvancedSettings::Item (RTTI), so they pair through these markers.
// clang-format off
// TEMPLATE: PLAYER 0x004091b0
// TEMPLATE: STUDIO 0x00421060
// ?getClassName@Creator@?$FactoryProduct@VDebugSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sDebugSettings@2@3QBDBVInstance@2@@RBX@@QBEABVName@3@XZ

// TEMPLATE: PLAYER 0x0065a010
// TEMPLATE: STUDIO 0x007749a0
// ?static_getCreator@?$FactoryProduct@VDebugSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sDebugSettings@2@3QBDBVInstance@2@@RBX@@KAABVCreator@12@XZ

// TEMPLATE: PLAYER 0x0065a840
// TEMPLATE: STUDIO 0x00775190
// ?getClassName@?$FactoryProduct@VDebugSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sDebugSettings@2@3QBDBVInstance@2@@RBX@@UBEABVName@2@XZ

// SYNTHETIC: PLAYER 0x0065fac0
// SYNTHETIC: STUDIO 0x00778a40
// ??_E?$GlobalAdvancedSettingsItem@VDebugSettings@RBX@@$E?sDebugSettings@2@3QBDB@RBX@@OBM@AEPAXI@Z
// clang-format on

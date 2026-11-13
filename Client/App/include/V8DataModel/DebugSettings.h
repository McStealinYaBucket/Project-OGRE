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

	class DebugSettings
		: public DescribedCreatable<DebugSettings, GlobalAdvancedSettings, sDebugSettings>
	{
	public:
		enum ErrorReporting
		{
			DontReport = 0,
			Prompt = 1,
			Report = 2,
		};

		DebugSettings()
			: stackTracingEnabled(true),
			  isFmodProfilingEnabled(false)
		{
		}
		// printCallStack gate (batch714/716: 0x19578C reads the singleton
		// byte at +0x5D; 2016-carry "IsScriptStackTracingEnabled"
		// property, category "Errors", ctor default true).  LAYOUT DELTA:
		// the binary routes through GlobalAdvancedSettingsItem<
		// DebugSettings, sDebugSettings>::singleton() and pins the byte at
		// +0x5D; the port's settings-item machinery is still a stub, so
		// the member is reached by name -- the +0x5D offset is not
		// reproducible yet.
		bool stackTracingEnabled;
		bool getStackTracingEnabled() const { return stackTracingEnabled; }

		// openFmod 0x2410F0 reads the GlobalAdvancedSettingsItem<
		// DebugSettings, sDebugSettings>::singleton() byte at +0x6A
		// (+106 decimal); on -> FMOD init flags |= FMOD_INIT_ENABLE_PROFILE
		// (32).  Name from the settings-contract string
		// "IsFmodProfilingEnabled" (0xB46641).  LAYOUT DELTA: reached by
		// name through this port singleton (the binary byte offset is not
		// reproduced).
		bool isFmodProfilingEnabled;
		bool getIsFmodProfilingEnabled() const { return isFmodProfilingEnabled; }

		// mac: 0x31A710 / 0x31A73A: instance-count limiter setters.
		void setInstanceCountLimit(int n) { RBX::Limits::Counter::countLimit = n; }
		int getInstanceCountLimit() const { return RBX::Limits::Counter::countLimit; }
		void setEnforceInstanceCountLimit(bool b) { RBX::Limits::Counter::globallyEnforcedLimit = b; }
		bool getEnforceInstanceCountLimit() const { return RBX::Limits::Counter::globallyEnforcedLimit; }
		// Same-route singleton for the unported GlobalAdvancedSettingsItem
		// (house pattern per LuaSettings::singleton; DELTA as above).
		static DebugSettings& singleton()
		{
			static DebugSettings instance;
			return instance;
		}
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

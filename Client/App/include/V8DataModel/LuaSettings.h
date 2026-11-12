/* V8DataModel/LuaSettings.h -- October 2013 (Lua GC tuning item).
   Evidence: C2 0x17D2C4 (331B, batch322: base item C2 + vtbls +
     +96 gcPause 200 + +100 gcStepMul 200 + +104/+108 double +
     +112 2 + +116 0 + +120 0 + +124 0.1f + name "Lua"; item C2
     0x17D43E). Simplified like RenderSettings (FactoryProduct Item
   template + singleton mechanics queue with the settings-item era);
   the C2 values and +96/+100 layout are binary-exact, which is what
   openState consumes. */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

#include <cstring>

namespace RBX {

	extern const char* const sLuaSettings;

	// Bitwise-exact double init for C2-pinned mystery floats.
	inline double bitsToDouble(unsigned long long bits)
	{
		double value;
		memcpy(&value, &bits, sizeof(value));
		return value;
	}

	class LuaSettings
		: public DescribedCreatable<LuaSettings, GlobalAdvancedSettings, sLuaSettings>
	{
	public:
		// STUB: PLAYER 0x0085e360
		// STUB: STUDIO 0x00948980
		LuaSettings()
			: gcPause(200)
			, gcStepMul(200)
			, unknown104(bitsToDouble(0x3F9EB851EB851EB8ull))
			, unknown112(2)
			, unknown116(0)
			, unknown120(false)
			, waitingThreadsBudget(0.1f)
		{
			Instance::setName("Lua");
		}

		// Singleton for openState's gc tuning (binary routes through
		// GlobalAdvancedSettingsItem<LuaSettings>::singleton -- DELTA).
		static LuaSettings& singleton()
		{
			static LuaSettings instance;
			return instance;
		}

		int getGcPause() const { return gcPause; }      // +96.
		int getGcStepMul() const { return gcStepMul; }  // +100.
		// Wait floor for sub_193123 (section 249: timeouts below this,
		// or NaN/Inf, clamp up to it). Role-pinned by use; the member
		// keeps its binary-exact C2 init (2016 calls this defaultWaitTime).
		double getDefaultWaitTime() const { return unknown104; }
	float getWaitingThreadsBudget() const { return waitingThreadsBudget; }  // +124.
	// +0x78 byte: registerLibrary's printf gate (0x630C9C reads it via
	// the GlobalAdvancedSettingsItem<LuaSettings> singleton).
	bool getUnknown120() const { return unknown120; }

	private:
		int gcPause;        // +96 (C2 200).
		int gcStepMul;      // +100 (C2 200).
		// +104/+108 double (C2 bytes EB851EB8/3F9EB851 = ~0.03; role TODO).
		// Initialized bitwise-exact via the helper below.
		double unknown104;
		int unknown112;     // +112 (C2 2; TODO).
		int unknown116;     // +116 (C2 0; TODO).
		bool unknown120;    // +120 (C2 false; TODO).
		float waitingThreadsBudget;   // +124 (C2 0.1f; 2016 name/role:
		// "0..1 A percentage", "10% duty cycle" budget for
		// WaitingScriptsJob::C2, which reads this float and divides by
		// 60.0 (0x1A9D40).
	};

} // namespace RBX

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

	// 2013 base: GlobalAdvancedSettingsItem<LuaSettings, sLuaSettings> (ctor PLAYER 0x0085e220), 0x64
	// bytes. Deriving GlobalAdvancedSettings (a ServiceProvider) put every member 0x28 too high: the
	// ScriptContext ctor 0x0068ad20 reads +0x78/+0x7c, clampWaitTimeout reads +0x70.
	class LuaSettings
		: public GlobalAdvancedSettingsItem<LuaSettings, sLuaSettings>
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
	int getUnknown112() const { return unknown112; }	// +0x78: ScriptContext ctor 0x0068b0f2
	int getUnknown116() const { return unknown116; }	// +0x7c: ScriptContext ctor 0x0068b100

	private:
		// 0x60: Service::isPublic, from the base (ctor 0x0085e220 stores 1); the 2013 ctor 0x0085e360
		// stores gcPause at +0x64, so everything below sits 4 bytes higher than in the Mach-O notes
		// (size 0x88).
		int gcPause;        // +0x64 (C2 200).
		int gcStepMul;      // +0x68 (C2 200).
		// +104/+108 double (C2 bytes EB851EB8/3F9EB851 = ~0.03; role TODO).
		// Initialized bitwise-exact via the helper below.
		double unknown104;
		int unknown112;     // +0x78 (C2 2; the ScriptContext ctor 0x0068b0ed reads it as gcFullStepThreshold).
		int unknown116;     // +0x7c (C2 0; read as gcFrequencyBase).
		bool unknown120;    // +0x80 (C2 false; TODO).
		float waitingThreadsBudget;   // +0x84 (C2 0.1f; 2016 name/role:
		// "0..1 A percentage", "10% duty cycle" budget for
		// WaitingScriptsJob::C2, which reads this float and divides by
		// 60.0 (0x1A9D40).
	};

} // namespace RBX

// The FactoryProduct base's out-of-line members. Their 2013 names spell the base
// GlobalAdvancedSettings::Item (RTTI .?AVCreator@?$FactoryProduct@VLuaSettings@RBX@@VItem@GlobalAdvancedSettings@2@...).
// clang-format off
// 2013's copy pushes the "LuaSettings" literal (sLuaSettings folded, so it was instantiated where the
// definition is visible); ours reads sLuaSettings. The kept COMDAT comes from LuaLibrary.obj.
// STUB: PLAYER 0x0085e220
// STUB: STUDIO 0x00948850
// ??0?$GlobalAdvancedSettingsItem@VLuaSettings@RBX@@$E?sLuaSettings@2@3QBDB@RBX@@IAE@XZ

// TEMPLATE: PLAYER 0x0085db70
// TEMPLATE: STUDIO 0x009481a0
// ?static_getCreator@?$FactoryProduct@VLuaSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sLuaSettings@2@3QBDBVInstance@2@@RBX@@KAABVCreator@12@XZ

// TEMPLATE: PLAYER 0x0085dc20
// TEMPLATE: STUDIO 0x00948250
// ?getClassName@?$FactoryProduct@VLuaSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sLuaSettings@2@3QBDBVInstance@2@@RBX@@UBEABVName@2@XZ

// The deleting-destructor thunk is GlobalAdvancedSettingsItem's: its destructor clears the singleton
// slot (PLAYER 0x0085e310 stores 0 to 0x00de68a8 before the FactoryProduct vptrs).
// SYNTHETIC: PLAYER 0x0085e300
// SYNTHETIC: STUDIO 0x00948920
// ??_E?$GlobalAdvancedSettingsItem@VLuaSettings@RBX@@$E?sLuaSettings@2@3QBDB@RBX@@OBM@AEPAXI@Z

// The thunks above jump here: ~GlobalAdvancedSettingsItem (singleton slot cleared), then the
// FactoryProduct vptrs and ~Instance.
// SYNTHETIC: PLAYER 0x0085e310
// SYNTHETIC: STUDIO 0x00948930
// RBX::LuaSettings::`scalar deleting destructor'
// clang-format on

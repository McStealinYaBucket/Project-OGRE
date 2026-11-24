/* V8DataModel/PhysicsSettings.h -- October 2013 (the "Physics" settings item).
   Layout from the PLAYER ctor 0x0085da60 (STUDIO 0x009424e0) and Creatable::create<PhysicsSettings>
   0x00653fd0 (`new 0x70`): GlobalAdvancedSettingsItem<PhysicsSettings, sPhysicsSettings> (item ctor
   0x0085d920, 0x64 bytes with Service::isPublic at 0x60), then one double at 0x68 that the ctor sets
   to 1.0 and PhysicsInstructions' ctor 0x0084d2f0 reads (2016: throttleAdjustTime). The 2013
   properties (PropDescriptor vftables 0x00b7ca44...) keep their values outside the object; they are
   not written yet. */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {

	extern const char* const sPhysicsSettings;

	// SIZE 0x70
	class PhysicsSettings
		: public GlobalAdvancedSettingsItem<PhysicsSettings, sPhysicsSettings>
	{
		double throttleAdjustTime;	// 0x68

	public:
		PhysicsSettings();

		double getThrottleAdjustTime() const { return throttleAdjustTime; }
	};

} // namespace RBX

// clang-format off
// TEMPLATE: PLAYER 0x00654470
// TEMPLATE: STUDIO 0x007cce30
// ?singleton@?$GlobalAdvancedSettingsItem@VPhysicsSettings@RBX@@$E?sPhysicsSettings@2@3QBDB@RBX@@SAAAVPhysicsSettings@2@XZ

// The item ctor pushes the "PhysicsSettings" literal: it is instantiated only in PhysicsSettings.obj,
// where the out-of-line PhysicsSettings ctor sees sPhysicsSettings' definition.
// TEMPLATE: PLAYER 0x0085d920
// TEMPLATE: STUDIO 0x00942410
// ??0?$GlobalAdvancedSettingsItem@VPhysicsSettings@RBX@@$E?sPhysicsSettings@2@3QBDB@RBX@@IAE@XZ
// clang-format on

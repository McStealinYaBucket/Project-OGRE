/* App/v8datamodel/PhysicsInstructions.cpp -- October 2013 (constructor only).
   See include/V8DataModel/PhysicsInstructions.h for evidence. */

#include "stdafx.h"

#include "V8DataModel/PhysicsInstructions.h"

namespace RBX {

// 2013 initializes throttleAdjustTime from GlobalAdvancedSettingsItem<PhysicsSettings,
// sPhysicsSettings>::singleton() (PLAYER 0x00654470) +0x68; PhysicsSettings is not in the tree yet.
// STUB: PLAYER 0x0084d2f0
// STUB: STUDIO 0x009a4fe0
PhysicsInstructions::PhysicsInstructions()
	: averageDt(30, 1.0)
	, averageDutyDt(30, 1.0)
	, timeSinceLastRadiusChange(0.0)
	, throttleTimer(0.0)
	, throttleAdjustTime(0.0)
	, requestedDutyPercent(0.0)
	, bandwidthExceeded(false)
	, networkBufferHealth(1.0)
{
}

} // namespace RBX

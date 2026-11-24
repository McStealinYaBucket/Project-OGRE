/* App/v8datamodel/PhysicsInstructions.cpp -- October 2013 (constructor only; changeSimulationRadius
   and changeMaxSimulationRadius wait on Network::Player's radius members).
   See include/V8DataModel/PhysicsInstructions.h for evidence. */

#include "stdafx.h"

#include "V8DataModel/PhysicsInstructions.h"
#include "V8DataModel/PhysicsSettings.h"

namespace RBX {

// FUNCTION: PLAYER 0x0084d2f0
// FUNCTION: STUDIO 0x009a4fe0
PhysicsInstructions::PhysicsInstructions()
	: averageDt(30, 1.0)
	, averageDutyDt(30, 1.0)
	, timeSinceLastRadiusChange(0.0)
	, throttleTimer(0.0)
	, throttleAdjustTime(PhysicsSettings::singleton().getThrottleAdjustTime())
	, requestedDutyPercent(0.0)
	, bandwidthExceeded(false)
	, networkBufferHealth(1.0)
{
}

} // namespace RBX

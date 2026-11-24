/* App/v8datamodel/PhysicsInstructions.cpp -- October 2013. See include/V8DataModel/PhysicsInstructions.h
   for evidence. The two radius changers assert on lines 55 and 63 (strings PLAYER 0x00b79d90/0x00b79e68);
   VC9 emits them before the constructor. */

#include "stdafx.h"

#include "V8DataModel/PhysicsInstructions.h"
#include "V8DataModel/PhysicsSettings.h"
#include "Network/Player.h"

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
























// FUNCTION: PLAYER 0x0084cb70
// FUNCTION: STUDIO 0x009a4b40
void PhysicsInstructions::changeSimulationRadius(Network::Player* dPhysPlayer, float change)
{
	timeSinceLastRadiusChange = 0.0;
	RBXASSERT(dPhysPlayer);
	dPhysPlayer->setSimulationRadius(dPhysPlayer->simulationRadius * change);
}

// FUNCTION: PLAYER 0x0084cbe0
// FUNCTION: STUDIO 0x009a4bb0
void PhysicsInstructions::changeMaxSimulationRadius(Network::Player* dPhysPlayer, float change)
{
	RBXASSERT(dPhysPlayer);
	dPhysPlayer->setMaxSimulationRadius(dPhysPlayer->maxSimulationRadius * change);
}

} // namespace RBX

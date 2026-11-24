/* App/v8datamodel/PhysicsSettings.cpp -- October 2013 (the "Physics" settings item): the ctor only.
   See include/V8DataModel/PhysicsSettings.h for evidence. No __FILE__ is recorded for this TU; the
   path is the 2016 tree's. */

#include "stdafx.h"

#include "V8DataModel/PhysicsSettings.h"
#include "decomp.h"

namespace RBX {
	const char* const sPhysicsSettings = "PhysicsSettings";
}

using namespace RBX;

namespace RBX {

// FUNCTION: PLAYER 0x0085da60
// FUNCTION: STUDIO 0x009424e0
PhysicsSettings::PhysicsSettings()
	: throttleAdjustTime(1.0)
{
	Instance::setName("Physics");
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::PhysicsSettings, 0x70);

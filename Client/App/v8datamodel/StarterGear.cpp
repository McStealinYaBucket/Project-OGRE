/* App/v8datamodel/StarterGear.cpp -- October 2013. The 2013 constructor sits in Hopper's .obj
   (PLAYER 0x006cb3e0, between HopperBin and StarterPackService), as 2016 keeps StarterGear in
   Hopper.cpp; it stays here until Hopper.cpp is laid out. */

#include "stdafx.h"

#include "V8DataModel/StarterGear.h"

namespace RBX {
	const char* const sStarterGear = "StarterGear";

	// FUNCTION: PLAYER 0x006cb3e0
	// FUNCTION: STUDIO 0x007b33e0
	StarterGear::StarterGear()
	{
		setName(sStarterGear);
	}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::StarterGear);
}

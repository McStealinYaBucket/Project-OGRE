/* App/v8datamodel/UserController.cpp -- October 2013, ControllerService only so far. */

#include "stdafx.h"

#include "V8DataModel/UserController.h"

namespace RBX {
	const char* const sControllerService = "ControllerService";
}

// PLAYER keeps this .obj's classDescriptor<ControllerService> copy (0x006e6e90 pushes the literal), so the
// registrar is defined here; STUDIO defines it in FactoryRegistration.cpp (see there). The address is the
// operand of the `inc` in DescribedNonCreatable<ControllerService>'s ctor (PLAYER 0x006e73b3).
#ifndef OGRE_TARGET_STUDIO
namespace RBX {
namespace Reflection {
// GLOBAL: PLAYER 0x00e0c968
template<> ClassRegistrar<ControllerService> ClassRegistrar<ControllerService>::registrar(0);
}
}
#endif

#include "decomp.h"

namespace RBX {

	// Not written yet (base ctor, vftables, m_unk0x64 = 0, setName). STUB body so callers pair.
	// STUB: PLAYER 0x006ea020
	// STUB: STUDIO 0x00817010
	ControllerService::ControllerService()
	{
		STUB(0x006ea020);
	}

} // namespace RBX

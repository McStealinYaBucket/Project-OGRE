/* App/v8datamodel/UserController.cpp -- October 2013, ControllerService only so far. */

#include "stdafx.h"

#include "V8DataModel/UserController.h"

namespace RBX {
	const char* const sControllerService = "ControllerService";
}

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

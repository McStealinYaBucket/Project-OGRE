/* App/v8world/AssemblyHistory.cpp -- October 2013 (assembly motion history).
   wakeUp from the 2013 bytes; the ctor and dtor (an Average<PhysicsCoord> of samples) are still STUBs. */

#include "stdafx.h"

#include "V8World/AssemblyHistory.h"
#include "decomp.h"

namespace RBX {

	// FUNCTION: PLAYER 0x0095d500
	// FUNCTION: STUDIO 0x00a22d30
	void AssemblyHistory::wakeUp()
	{
		wakeCounter = 15;
	}

	// STUB: PLAYER 0x0095e330
	// STUB: STUDIO 0x00a23b60
	AssemblyHistory::~AssemblyHistory()
	{
		STUB(0x0095e330);
	}

	// STUB: PLAYER 0x0095e450
	// STUB: STUDIO 0x00a23c80
	AssemblyHistory::AssemblyHistory(Assembly* assembly)
		: m_unk0x18(0)
		, wakeCounter(15)
	{
		STUB(0x0095e450);
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::AssemblyHistory, 0x20);

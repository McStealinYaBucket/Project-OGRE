/* App/v8world/Edge.cpp -- October 2013 (edge primitive slots).
   Body: setPrimitive 0x57226A (129B, batch218). */

#include "stdafx.h"

#include "V8World/Edge.h"

namespace RBX {

	// mac: 0x57226A (129B).
	// STUB: PLAYER 0x00967ca0
	// STUB: STUDIO 0x00a2c450
	void Edge::setPrimitive(int i, Primitive* p)
	{
		RBXASSERT(i == 0 || i == 1);  // Edge.cpp:21 "(i == 0) || (i == 1)"
		if (i != 0)
			prim1 = p;
		else
			prim0 = p;
	}

} // namespace RBX

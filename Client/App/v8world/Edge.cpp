/* App/v8world/Edge.cpp -- October 2013. Edge::Edge out of line (Joint::Joint 0x008215c0 calls it); assert :21. */
#include "stdafx.h"
#include "V8World/Edge.h"

namespace RBX {

	// FUNCTION: PLAYER 0x00967c70
	// FUNCTION: STUDIO 0x00a2c420
	Edge::Edge(Primitive* p0, Primitive* p1)
		: edgeState(Sim::UNDEFINED)
		, throttleType(Sim::UNDEFINED_THROTTLE)
		, prim0(p0)
		, prim1(p1)
		, index0(-1)
		, index1(-1)
	{}
	// FUNCTION: PLAYER 0x00967ca0
	// FUNCTION: STUDIO 0x00a2c450
	void Edge::setPrimitive(int i, Primitive* p)
	{
		RBXASSERT((i == 0) || (i == 1));
		if (i == 0) {
			prim0 = p;
		}
		else {
			prim1 = p;
		}
	}

} // namespace RBX

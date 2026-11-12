/* App/v8kernel/Link.cpp -- October 2013 (link state).
   Body: C2 0x531126 (batch288). */

#include "stdafx.h"

#include "V8Kernel/Link.h"
#include "V8Kernel/Body.h"

namespace RBX {

	// mac: 0x531126 (batch288: vptr + child NULL + 4x CF C2 + serial read).
	// G3D::CoordinateFrame defaults to identity (same net state as the
	// binary's four explicit CF constructions).
	Link::Link()
		: child(NULL)
	{
		serial = Body::getNextStateIndex();
	}

} // namespace RBX

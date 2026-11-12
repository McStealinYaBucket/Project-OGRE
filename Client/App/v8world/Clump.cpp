/* App/v8world/Clump.cpp -- October 2013 (group lookups).
   Bodies: getPrimitiveClump 0x55D23E (batch258/269). */

#include "stdafx.h"

#include "V8World/Clump.h"
#include "V8World/Primitive.h"

namespace RBX {

	// mac: 0x55D23E (batch258/269: single computedUpper hop from prim+8, no
	// -8 adjust -- the upper IS the Clump object (base at +0)).
	// No NULL guard in the binary (production prims always meshed).
	Clump* Clump::getPrimitiveClump(Primitive* prim)
	{
		return static_cast<Clump*>(prim->getMesh()->getComputedUpper());
	}

	// Const mirror (same shape).
	const Clump* Clump::getConstPrimitiveClump(const Primitive* prim)
	{
		return static_cast<const Clump*>(prim->getMesh()->getConstComputedUpper());
	}

} // namespace RBX

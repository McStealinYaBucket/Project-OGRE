/* V8World/BasicSpatialHashPrimitive.h -- October 2013.
   The per-primitive spatial-hash state, a base of Primitive at +0x34 (RTTI BCA 0x00c250b4).
   2013 path from the Primitive dtor 0x00755540: "spatialNodeLevel == -1 file: ...\include\
   V8World/BasicSpatialHashPrimitive.h line: 45"; the inlined base dtor then stores -2.
   Layout: an ExtentsInt32 (Primitive ctor calls ExtentsInt32() 0x004e5df0 on +0x34), then the
   level at +0x4c (-1 = not in any hash).  SpatialHash methods read the level through
   getSpatialNodeLevel(). */

#pragma once

#include "Util/ExtentsInt32.h"
#include "rbx/Debug.h"

namespace RBX {

	struct BasicSpatialHashPrimitive
	{
		ExtentsInt32 spatialExtents;
		int spatialNodeLevel;

		BasicSpatialHashPrimitive()
			: spatialNodeLevel(-1)
		{}



















		~BasicSpatialHashPrimitive()
		{
			RBXASSERT(spatialNodeLevel == -1);
			spatialNodeLevel = -2;
		}



		// STUB: PLAYER 0x00501790
		// STUB: STUDIO 0x00591a70
		int getSpatialNodeLevel() const
		{
			RBXASSERT(spatialNodeLevel >= -1);
			return spatialNodeLevel;
		}
	};

} // namespace RBX

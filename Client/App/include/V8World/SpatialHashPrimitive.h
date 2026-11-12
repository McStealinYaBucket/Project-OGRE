/* V8World/SpatialHashPrimitive.h -- October 2013
 * RBX::BasicSpatialHashPrimitive (the per-prim spatial-hash state).
 *
 * EVIDENCE: every SpatialHash method addresses it at prim+52 --
 * getSpatialNodeLevel(prim+52) in addNode 0x5696C2 / findNode 0x569CB0 /
 * onPrimitiveAdded 0x567C88 / primitiveAdded 0x568E38 / insertNodeToPrimitive
 * 0x56A6B6, and primitiveAdded writes prim+52..+79 wholesale:
 *   +52  Vector3int32 minCell   (12B)
 *   +64  Vector3int32 maxCell   (12B)
 *   +76  int level             (-1 = not inserted; asserted on insert :585)
 * getSpatialNodeLevel() returns the level (:585/:599 asserts).
 *
 * Kept in its own header (not in SpatialHashMultiRes.h) so Primitive.h can
 * contain it WITHOUT pulling in the SpatialHash template (which needs
 * Primitive complete -- cycle otherwise).  Our Primitive stores it as a
 * member wherever it fits OUR layout (documented model-relative) and
 * forwards getSpatialNodeLevel().
 *
 * Original file: unknown (BasicSpatialHashPrimitive is referenced from
 * include/v8World/SpatialHashMultiRes.inl throughout; header not recovered). */

#pragma once

#include "Util/Vector3int32.h"

namespace RBX {

	struct BasicSpatialHashPrimitive
	{
		Vector3int32 minCell;
		Vector3int32 maxCell;
		int spatialNodeLevel;   // -1 = not in any hash

		BasicSpatialHashPrimitive()
			: minCell(0, 0, 0)
			, maxCell(0, 0, 0)
			, spatialNodeLevel(-1)
		{}

		int getSpatialNodeLevel() const { return spatialNodeLevel; }
	};

} // namespace RBX

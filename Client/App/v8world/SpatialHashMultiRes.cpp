/* App/v8world/SpatialHashMultiRes.cpp -- October 2013
 * RBX::SpatialHashStatic cell math.  See include/V8World/SpatialHashMultiRes.h
 * for the evidence block.
 *
 * mac: 0x5A3BCC (342B), SpatialHashMultiRes.cpp:79.  invCellSize is computed as 1.0f / (float)(8 << level) -- a DIVIDE by the cell size (grid 8<<level world units), not a multiply -- then each bound is scaled and floored independently. DELTA, million cube: the binary clamps the query into Tolerance::maxExtents().millionCube (a static the tree does not have; name evidenced, value not recovered).  The port clamps into Extents::maxExtents() (+/-maxFinite), which is a no-op for every physical coordinate (|c| < 1e6 behaves identically under either cube) and differs only for inputs already outside the million cube.
 *
 * Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/SpatialHashMultiRes.cpp */

#include "stdafx.h"

#include "V8World/SpatialHashMultiRes.h"
#include "rbx/Debug.h"

namespace RBX {
















































	// mac: 0x5A3AE0 (236B), SpatialHashMultiRes.cpp:72-73.  The imul instructions
	// wrap mod 2^32 exactly as the binary's do (same 32-bit int arithmetic),
	// so negative cell coordinates hash identically -- the final AND with
	// (0xFFFFu >> level) clears any sign bit the XOR sets.
	// FUNCTION: PLAYER 0x007925a0
	// FUNCTION: STUDIO 0x0089c7c0
	int SpatialHashStatic::getHash(int level, const Vector3int32& cell)
	{
		int hash = (0xFFFF >> level) & ((73856093 * cell.x) ^ (19349663 * cell.y) ^ (83492791 * cell.z));
		RBXASSERT(hash >= 0);
		RBXASSERT(hash < (int)numBuckets(level));
		return hash; }
	// STUB: PLAYER 0x00792670
	// STUB: STUDIO 0x0089c890
	void SpatialHashStatic::computeMinMax(int level, const Extents& extents, Vector3int32& minOut, Vector3int32& maxOut)
	{
		RBXASSERT(level != -1);

		Extents clamped = extents.clampInsideOf(Extents::maxExtents());
		float invCellSize = 1.0f / (float)(8 << level);
		minOut = Vector3int32::floor(clamped.min() * invCellSize);
		maxOut = Vector3int32::floor(clamped.max() * invCellSize);
	}

} // namespace RBX

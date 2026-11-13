/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */
#include "stdafx.h"

#include "Util/Extents.h"
#include "Util/Math.h"
#include "rbx/Debug.h"

namespace RBX {

// mac: 0x1FFB64
Extents Extents::clampInsideOf(const Extents& other) const { //		Extents& operator&= (const Extents& other) {
	Extents answer;
	answer.low = low.clamp(other.low, other.high);
	answer.high = high.clamp(other.low, other.high);
	return answer;
}

// mac: 0x1FFCA2
NormalId Extents::closestFace(const Vector3& point)
{
	float x = std::abs(point.x - high.x);
	float y = std::abs(point.y - high.y);
	float z = std::abs(point.z - high.z);
	float nx = std::abs(point.x - low.x);
	float ny = std::abs(point.y - low.y);
	float nz = std::abs(point.z - low.z);

	NormalId best = NORM_X;
	float bestV = x;
	if (y < bestV)	{best = NORM_Y; bestV = y;}
	if (z < bestV)	{best = NORM_Z; bestV = z;}
	if (nx < bestV)	{best = NORM_X_NEG; bestV = nx;}
	if (ny < bestV)	{best = NORM_Y_NEG; bestV = ny;}
	if (nz < bestV)	{best = NORM_Z_NEG;}
	return best;
}

// Looking along -Z axis
// +x to the right
//	Front (Z)			Back (-Z)
/*
				^
				+y
	3	7				2	6
									+x -->
	1	5				0	4
*/

// mac: 0x1FFD46 - asserts at Extents.cpp:53 ("i >= 0") and :54 ("i <= 7")
// STUB: PLAYER 0x00796070
// STUB: STUDIO 0x0089f230
Vector3int16 Extents::getCornerIndex(int i) const
{
	RBXASSERT(i >= 0);
	RBXASSERT(i <= 7);
	int x_id = i / 4;			// 0,0,0,0,1,1,1,1
	int y_id = (i / 2) % 2;		// 0,0,1,1,0,0,1,1
	int z_id = i % 2;			// 0,1,0,1,0,1,0,1
	return Vector3int16(x_id, y_id, z_id);
}

// mac: 0x1FFE42
Vector3 Extents::getCorner(int i) const
{
	const Vector3int16 index = getCornerIndex(i);
	float x = (&low)[index.x].x;
	float y = (&low)[index.y].y;
	float z = (&low)[index.z].z;
	return Vector3(x, y, z);
}

// mac: 0x1FFE9E - default case asserts at Extents.cpp:104
// STUB: PLAYER 0x00796190
// STUB: STUDIO 0x0089f350
void Extents::getFaceCorners(
	NormalId			faceId,
	Vector3&            v0,
	Vector3&            v1,
	Vector3&            v2,
	Vector3&            v3) const
{
    switch (faceId) {
    case 0:
        v0 = getCorner(4); v1 = getCorner(6); v2 = getCorner(7); v3 = getCorner(5);
        break;

    case 1:
        v0 = getCorner(2); v1 = getCorner(3); v2 = getCorner(7); v3 = getCorner(6);
        break;

    case 2:
        v0 = getCorner(1); v1 = getCorner(5); v2 = getCorner(7); v3 = getCorner(3);
        break;

    case 3:
        v0 = getCorner(0); v1 = getCorner(1); v2 = getCorner(3); v3 = getCorner(2);
        break;

    case 4:
        v0 = getCorner(0); v1 = getCorner(4); v2 = getCorner(5); v3 = getCorner(1);
        break;

    case 5:
        v0 = getCorner(0); v1 = getCorner(2); v2 = getCorner(6); v3 = getCorner(4);
        break;

    default:
		RBXASSERT(0);
		break;
    }
}

// mac: 0x2001E0 - the 2013 binary loops the 8 corners, transforming each
// myFrame.pointToWorldSpace(corner) then expressInFrame.pointToObjectSpace(world)
// FUNCTION: PLAYER 0x00796920
// FUNCTION: STUDIO 0x0089fae0
Extents Extents::express(const CoordinateFrame& myFrame,
						 const CoordinateFrame& expressInFrame) const
{
	Vector3 minC(Math::inf(), Math::inf(), Math::inf());
	Vector3 maxC(-Math::inf(), -Math::inf(), -Math::inf());

	for (int i = 0; i < 8; ++i) {
		Vector3 corner = this->getCorner(i);
		Vector3 world = myFrame.pointToWorldSpace(corner);
		Vector3 inOther = expressInFrame.pointToObjectSpace(world);
		minC = minC.min(inOther);
		maxC = maxC.max(inOther);
	}
	return Extents::vv(minC, maxC);
}

// mac: 0x20050E - the 2013 binary loops the 8 corners of *this through
// localCoord.pointToWorldSpace and takes min/max (no early-out on isNull)
// FUNCTION: PLAYER 0x00796d00
// FUNCTION: STUDIO 0x0089fec0
Extents Extents::toWorldSpace(const CoordinateFrame& localCoord) const
{
	Vector3 minC(Math::inf(), Math::inf(), Math::inf());
	Vector3 maxC(-Math::inf(), -Math::inf(), -Math::inf());

	for (int i = 0; i < 8; ++i) {
		Vector3 corner = this->getCorner(i);
		Vector3 world = localCoord.pointToWorldSpace(corner);
		minC = minC.min(world);
		maxC = maxC.max(world);
	}
	return Extents::vv(minC, maxC);
}

Plane Extents::getPlane(NormalId normalId) const
{
	Vector3 point = faceCenter(normalId);
	Vector3 normal = normalIdToVector3(normalId);
	return Plane(normal, point);
}

// mac: 0x2007D8
Vector3 Extents::faceCenter(NormalId faceId) const
{
	Vector3 answer = center();
	int xyz = faceId % 3;
	if (faceId < 3)
	{						// i.e., positive face x, y, z
		answer[xyz] = high[xyz];
	}
	else
	{						// i.e., negative face -x, -y, -z
		answer[xyz] = low[xyz];
	}
	return answer;
}

Vector3 Extents::clamp(const Extents& innerExtents) const
{
	Vector3 answer(Vector3::zero());

	// for x..z
	for (int i = 0; i < 3; i++) {
		RBXASSERT(innerExtents.size()[i] <= this->size()[i]);
		if (innerExtents.max()[i] > this->max()[i]) {
			answer[i] = this->max()[i] - innerExtents.max()[i];
		}
		else if (innerExtents.min()[i] < this->min()[i]) {
			answer[i] = this->min()[i] - innerExtents.min()[i];
		}
	}
	return answer;
}

// mac: 0x200860 - per-axis squared overshoot of point beyond the box
float Extents::computeClosestSqDistanceToPoint(const Vector3& point) const
{
	Vector3 center = (high + low) / 2;
	Vector3 extents = (high - low) / 2;

	return ClosestSqDistanceToAABB(point, center, extents);
}

// mac: 0x200970 - asserts at Extents.cpp:190 ("distance > 0.0")
// VERIFIED 2026-09-27: the binary builds a fresh 24-byte Extents with
//   low  = this->low  - (d,d,d)     (ssub on all three lanes)
//   high = this->high + (d,d,d)     (sadd on all three lanes)
// and returns `overlapsOrTouches(grown, other) ^ 1`.  expand() is exactly that
// growth, so this body is semantically identical.
// STUB: PLAYER 0x00796580
// STUB: STUDIO 0x0089f740
bool Extents::separatedByMoreThan(const Extents& other, float distance) const
{
	RBXASSERT(distance > 0.0);
	Extents thisExpanded(*this);
	thisExpanded.expand(distance);
	return !(thisExpanded.overlapsOrTouches(other));
}

} // namespace

// TODO(other modules): the source export for this module also contained
// out-of-module consumers of Extents, to be reconstructed with their own
// headers: RBX::AdvMoveToolBase::getExtentsAndLocation/getExtents (0x1B74E6,
// mac: 0x1B7F1C), RBX::AxisToolBase::getExtents (0x1BFBA2), RBX::Dragger
// search/intersect/moveExtents family (0x1C3552..0x1C58A4),
// RBX::Face::fromExtentsSide (0x201476 -> Util/Face),
// RBX::HandleHitTest::hitTestHandleWorld/Local/MoveHandleWorld
// (0x2053FE/0x205532/0x2056F6), RBX::Camera::lerpToExtents/tryZoomExtents/
// zoomExtents (0x28BB6C/0x28C306/0x28C62A), RBX::GeometryService::
// getPartsTouchingExtents(WithIgnore) (0x38DE9E/0x38E18A),
// RBX::ContactManager::getPrimitivesTouchingExtents(IgnoreAncestor)/
// anyExtentsOverlapsOrTouchesPrimitives (0x563F30/0x563FEA/0x567942),
// RBX::SpatialHash/ExtentsInt32/SpatialHashStatic members
// (0x567188..0x5A3BCC), RBX::TerrainPartition::*Extents (0x6AD9E2..0x6ADFF8),
// RBX::BuoyancyContact::computeExtentsWaterBand (0x6B9CDE),
// RBX::DrawAdorn::handlePosInObject (0x81C3A4),
// RBX::Frustum::containsAABB (0x81FBCC), and
// RBX::Network::Replicator::isInStreamedRegions (0x915616/0x94F770).
// The std::vector<RBX::Extents> instantiations (0x1C58A4..0x1C5F22) are
// libc++ template expansions - not reconstructed.

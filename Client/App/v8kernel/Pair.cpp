/* App/v8kernel/Pair.cpp -- October 2013 (pair parameter block).
   Body: GeoPair C2 (batch276: zeroes dwords 1..8, leaves type,
   edgeLengths and pairData to the setXxx builders). */

#include "stdafx.h"

#include "V8Kernel/Pair.h"
#include "V8Kernel/Body.h"
#include "Util/Math.h"
#include "G3D/Vector3.h"

#include <cmath>

namespace RBX {

	// Batch276: explicit zeroing per the binary (no wholesale init).
	GeoPair::GeoPair()
		: offset0(G3D::Vector3::zero())
		, offset1(G3D::Vector3::zero())
		, body0(NULL)
		, body1(NULL)
	{
		// geoPairType/edgeLength0/edgeLength1/pairData deliberately
		// uninitialized (binary stores dwords 1..8 only; the setXxx
		// builders own them in the generate-data era).
	}

	// mac: 0x53179E (550B, batch308: body0-frame * offset0 with binary
	// assoc (m2*z)+((m0*x)+(m1*y)); normal = -sign * body1-frame
	// column[normalID1 % 3] (sign from /3); length = (pPlane - pos)
	// dot normal, same assoc. Matches 2016 computePointPlane text op
	// for op).
	void GeoPair::computePointPlane(PairParams& params)
	{
		const G3D::CoordinateFrame& c0 = body0->getCoordinateFrameFast();
		const G3D::CoordinateFrame& c1 = body1->getCoordinateFrameFast();
		const float* r0 = c0.rotation;
		const float* r1 = c1.rotation;
		const G3D::Vector3& o0 = offset0;
		const G3D::Vector3& o1 = offset1;
		G3D::Vector3 pos(
			((r0[2] * o0.z) + ((r0[0] * o0.x) + (r0[1] * o0.y))) + c0.translation.x,
			((r0[5] * o0.z) + ((r0[3] * o0.x) + (r0[4] * o0.y))) + c0.translation.y,
			((r0[8] * o0.z) + ((r0[6] * o0.x) + (r0[7] * o0.y))) + c0.translation.z);
		params.position = pos;
		int axis = pairData.normalID1 % 3;
		float sign = (float)((-2 * (pairData.normalID1 / 3)) | 1);
		G3D::Vector3 normal(
			-(sign * r1[axis]),
			-(sign * r1[axis + 3]),
			-(sign * r1[axis + 6]));
		params.normal = normal;
		G3D::Vector3 plane(
			((r1[2] * o1.z) + ((r1[0] * o1.x) + (r1[1] * o1.y))) + c1.translation.x,
			((r1[5] * o1.z) + ((r1[3] * o1.x) + (r1[4] * o1.y))) + c1.translation.y,
			((r1[8] * o1.z) + ((r1[6] * o1.x) + (r1[7] * o1.y))) + c1.translation.z);
		G3D::Vector3 diff = plane - pos;
		// Binary lanes verbatim: ((dx*nx) - (dy*v16)) - (dz*v17), where
		// v16/v17 are the pre-negation lanes (normal = (v9,-v16,-v17)).
		params.length = ((diff.x * normal.x) - (diff.y * -normal.y)) - (diff.z * -normal.z);
		// (kept unsimplified: -normal.y == v16, -normal.z == v17).
	}

	// mac: 0x5325AA (993B, batch308). See header for the lane note.
	void GeoPair::computeEdgeEdge(PairParams& params)
	{
		const G3D::CoordinateFrame& ca = body0->getCoordinateFrameFast();
		const G3D::CoordinateFrame& cb = body1->getCoordinateFrameFast();
		const float* ra = ca.rotation;
		const float* rb = cb.rotation;
		int axis0 = pairData.normalID0 % 3;
		float sign0 = (float)((-2 * (pairData.normalID0 / 3)) | 1);
		int axis1 = pairData.normalID1 % 3;
		float sign1 = (float)((-2 * (pairData.normalID1 / 3)) | 1);
		G3D::Vector3 ua(sign0 * ra[axis0], sign0 * ra[axis0 + 3], sign0 * ra[axis0 + 6]);
		G3D::Vector3 ub(sign1 * rb[axis1], sign1 * rb[axis1 + 3], sign1 * rb[axis1 + 6]);
		const G3D::Vector3& oa = offset0;
		const G3D::Vector3& ob = offset1;
		G3D::Vector3 pa(
			((ra[2] * oa.z) + ((ra[0] * oa.x) + (ra[1] * oa.y))) + ca.translation.x,
			((ra[5] * oa.z) + ((ra[3] * oa.x) + (ra[4] * oa.y))) + ca.translation.y,
			((ra[8] * oa.z) + ((ra[6] * oa.x) + (ra[7] * oa.y))) + ca.translation.z);
		G3D::Vector3 pb(
			((rb[2] * ob.z) + ((rb[0] * ob.x) + (rb[1] * ob.y))) + cb.translation.x,
			((rb[5] * ob.z) + ((rb[3] * ob.x) + (rb[4] * ob.y))) + cb.translation.y,
			((rb[8] * ob.z) + ((rb[6] * ob.x) + (rb[7] * ob.y))) + cb.translation.z);
		G3D::Vector3 p = pb - pa;
		float uaub = ((ua.z * ub.z) + ((ua.x * ub.x) + (ua.y * ub.y)));
		float q1 = ((ua.z * p.z) + ((ua.x * p.x) + (ua.y * p.y)));
		float q2n = ((ub.z * p.z) + ((ub.x * p.x) + (ub.y * p.y)));
		float d = 1.0f - uaub * uaub;
		if (d <= 1e-6f)
		{
			params.position = pa;
			params.normal = ua;
			params.length = 0.0f;
			return;
		}
		float dinv = 1.0f / d;
		float sA = ((q1 * uaub) - q2n) * dinv;   // scales the ub/pb side.
		float sB = (q1 - (q2n * uaub)) * dinv;   // scales the ua/pa side.
		G3D::Vector3 pa2 = pa + sB * ua;
		G3D::Vector3 pb2 = pb + sA * ub;
		params.position = 0.5f * (pa2 + pb2);
		params.normal = pa2 - pb2;
		params.length = -params.normal.unitize(1e-6f);
	}

	// File-local lane select (G3D Vector3 has no operator[] in-tree).
	static float laneOf(const G3D::Vector3& v, int i)
	{
		return i == 0 ? v.x : (i == 1 ? v.y : v.z);
	}

	// mac: 0x5319C4 (3046B, batch308; see header lane DELTA).
	void GeoPair::computeEdgeEdgePlane2(PairParams& params)
	{
		const G3D::CoordinateFrame& ca = body0->getCoordinateFrameFast();
		const G3D::CoordinateFrame& cPlane = body1->getCoordinateFrameFast();

		G3D::Vector3 p1 = offset0;
		G3D::Vector3 p2 = ca.pointToObjectSpace(cPlane.pointToWorldSpace(offset1));

		G3D::Vector3 u1 = ca.vectorToObjectSpace(Math::getWorldNormal((NormalId)pairData.normalID0, ca));
		G3D::Vector3 u2 = ca.vectorToObjectSpace(Math::getWorldNormal((NormalId)pairData.normalID1, cPlane));

		params.normal = -Math::getWorldNormal((NormalId)pairData.planeID, cPlane);

		if (fabsf(fabsf(u1.dot(u2)) - 1.0f) > 1e-5f)
		{
			float a1 = (p1 - p2).dot(u1);
			float a2 = (p1 - p2).dot(u2);
			float b = u1.dot(u2);

			float t1 = (b * a2 - a1 * b * b) / (1 - b * b) - a1;
			float t2 = (a2 - a1 * b) / (1 - b * b);

			G3D::Vector3 body1ClosestPointInBody1 = p1 + t1 * u1;

			G3D::Vector3 body2ClosestPointInBody2 =
				cPlane.pointToObjectSpace(ca.pointToWorldSpace(p2 + t2 * u2));
			float body1ClosestPointEdgeLocation = laneOf(body1ClosestPointInBody1, pairData.normalID0 % 3);
			float body2ClosestPointEdgeLocation = laneOf(body2ClosestPointInBody2, pairData.normalID1 % 3);

			float eps = 0.05f;
			if ((fabsf(body1ClosestPointEdgeLocation) > 0.5f * edgeLength0 + eps) ||
				(fabsf(body2ClosestPointEdgeLocation) > 0.5f * edgeLength1 + eps))
			{
				params.position = ca.pointToWorldSpace(offset0);
				params.length = 0.0f;
			}
			else
			{
				params.position = ca.pointToWorldSpace(body1ClosestPointInBody1);
				params.length = params.normal.dot(cPlane.pointToWorldSpace(offset1) - params.position);
			}
		}
		else
		{
			params.position = ca.pointToWorldSpace(offset0);
			params.length = 0.0f;
		}
	}

	// mac: 0x562E24 (289B, batch308: +28/+32 bodies with +44/+48 ids,
	// except POINT_PLANE which keys +52/+48; other types assert
	// EDGE_EDGE (Pair.h:133) then match either orientation, with the
	// swapped arm cross-comparing +48==id0 && +44==id1, verbatim).
	// STUB: PLAYER 0x00850480
	// STUB: STUDIO 0x009a8170
	bool GeoPair::match(Body* b0, Body* b1, GeoPairType pairType, int id0, int id1)
	{
		if (pairType == EDGE_EDGE_PLANE_PAIR)
		{
			return body0 == b0 && body1 == b1
				&& pairData.normalID0 == id0 && pairData.normalID1 == id1;
		}
		if (pairType == POINT_PLANE_PAIR)
		{
			return body0 == b0 && body1 == b1
				&& pairData.planeID == id0 && pairData.normalID1 == id1;
		}
		RBXASSERT(pairType == EDGE_EDGE_PAIR);  // Pair.h:133
		if (body0 == b0 && body1 == b1
			&& pairData.normalID0 == id0 && pairData.normalID1 == id1)
			return true;
		return body1 == b0 && body0 == b1
			&& pairData.normalID1 == id0 && pairData.normalID0 == id1;
	}

} // namespace RBX

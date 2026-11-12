/* App/v8world/RotateJoint.cpp -- October 2013 RBX::RotateJoint statics.
 *
 * 0x59833E  RotateJoint::surfaceTypeToJoint(SurfaceType, Primitive*,
 *             Primitive*, CF const&, CF const&)                        542B
 * 0x598DF0  sub_598DF0 (file-static frame-coincidence helper)          287B
 *
 * surfaceTypeToJoint, verbatim:
 *     if (surfaceType == ROTATE) {                       // cmp ebx, 6
 *         RotateJoint* j = new(0xD0) RotateJoint();      // 0D0h at 0x598358
 *         MultiJoint(p0, p1, c0, c1, 2);                 // 14h slot = 2
 *         j->vptr[0] = off_EEEF88; j->vptr[+32] = off_EEEFF0;
 *         return j;
 *     }
 *     Vector3 zero = Vector3::zero();                    // 12-byte memset
 *     float angle = RotateConnector::computeJointAngle(  // st0 via fstp
 *         p0->getCoordinateFrame(), p1->getCoordinateFrame(),
 *         c0, c1, zero);                                // zero never re-read
 *     if (surfaceType == ROTATE_V) {                     // cmp eax, 7
 *         RotateVJoint* j = new(0xDC) RotateVJoint();    // 0DCh
 *         MultiJoint(p0, p1, c0, c1, 2);
 *         j->angle = angle; j->int212 = 0; j->int216 = 0;
 *         j->vptr[0] = off_EEF168; j->vptr[+32] = off_EEF1D0;
 *         return j;
 *     }
 *     if (surfaceType == ROTATE_P) {                     // cmp eax, 8
 *         ... identical, vtables off_EEF0C8 / off_EEF130 ...
 *     }
 *     RBXASSERT(false);  // RotateJoint.cpp:75 ("0 file: ... line: 75")
 *     return NULL;
 *
 * The vtable fixups are the ctors' own work in the port (our
 * RotateJoint/RotatePJoint/RotateVJoint ctors already chain into
 * MultiJoint(p0,p1,c0,c1,2)), so they do not appear as statements here.
 * The `angle` store is `movss [esi+0D0h]` -- a float, not a double: the
 * QWORD-sized hexrays store is just the zero-extension of the 32-bit
 * LODWORD(v12) write, which is also where the +212 zero comes from.
 *
 * sub_598DF0, verbatim (a file-static: the symbol is literally `sub_598DF0`,
 * so the NAME below is invented and the address is the identity):
 *     bool jointFramesCoincide(centerA, centerB, normalA, normalB):
 *         if (|centerB - centerA|^2 > 0.0025) return false;
 *         for (v in {+1.0, -1.0}):   // rodata 0xCA2470 = {BF800000,3F800000}
 *             if (|(centerB + v*normalB) - (centerA - v*normalA)|^2 > 0.0025)
 *                 return false;       // threshold rodata 0xC852E4 = 0.0025
 *         return true;
 * The table order is +1 first (i==0 selects table[1]) then -1; the order is
 * irrelevant since every sample must pass.  Physically this is a lever-arm
 * test: with the centers coincident and the normals near-opposite, stepping
 * a full stud along each normal must land on the same point, which only
 * holds when the two frames' axes genuinely line up.
 *
 * DELTA, canBuildJoint: RotateJoint::canBuildJoint 0x59855C (2196B) is NOT
 * ported here -- its guard/contains/swap prologue is decoded (batch106) and
 * its two-attempt grid-snapped tail calls exactly these two functions, but
 * the full body is a session of its own.  It will live in this file.
 *
 * Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/RotateJoint.cpp */

#include "stdafx.h"

#include "V8World/MultiJoint.h"
#include "V8World/RotateConnector.h"
#include "V8World/Primitive.h"
#include "Util/Math.h"
#include "Util/Face.h"
#include "rbx/Debug.h"

namespace RBX {

	// mac: 0x598DF0 (287B).  See the file header: the name is invented (the binary
	// has only `sub_598DF0`), the address and the body are the identity.
	static bool jointFramesCoincide(
		const G3D::Vector3& centerA,
		const G3D::Vector3& centerB,
		const G3D::Vector3& normalA,
		const G3D::Vector3& normalB)
	{
		// 0xC852E4: 0.0025f == 0.05^2, the same 0.05 extrusion tolerance the
		// rest of the joint code speaks in.
		static const float kCoincideToleranceSq = 0.0025f;
		// 0xCA2470: { -1.0f, +1.0f }.  Sampled table[1] first, then
		// table[0] -- i.e. +1.0, then -1.0.
		static const float kLeverArm[2] = { -1.0f, +1.0f };

		if ((centerB - centerA).squaredLength() > kCoincideToleranceSq)
			return false;

		for (int i = 0; i < 2; ++i)
		{
			float v = (i == 0) ? kLeverArm[1] : kLeverArm[0];
			G3D::Vector3 steppedB = centerB + normalB * v;
			G3D::Vector3 steppedA = centerA - normalA * v;
			if ((steppedB - steppedA).squaredLength() > kCoincideToleranceSq)
				return false;
		}
		return true;
	}

	// mac: 0x59833E (542B).
	// STUB: PLAYER 0x00974d90
	// STUB: STUDIO 0x009c06a0
	Joint* RotateJoint::surfaceTypeToJoint(
		SurfaceType surfaceType,
		Primitive* p0,
		Primitive* p1,
		const G3D::CoordinateFrame& c0,
		const G3D::CoordinateFrame& c1)
	{
		if (surfaceType == ROTATE)
		{
			return new RotateJoint(p0, p1, c0, c1);
		}

		// The zero vector is passed by reference and written (column 2 of
		// the m0 frame) but never read afterwards -- kept because dropping
		// the parameter would change computeJointAngle's mangling.
		G3D::Vector3 unused = G3D::Vector3::zero();
		float angle = RotateConnector::computeJointAngle(
			p0->getCoordinateFrame(), p1->getCoordinateFrame(),
			c0, c1, unused);

		if (surfaceType == ROTATE_V)
		{
			RotateVJoint* joint = new RotateVJoint(p0, p1, c0, c1);
			joint->angle = angle;
			joint->int212 = 0;
			joint->int216 = 0;
			return joint;
		}

		if (surfaceType == ROTATE_P)
		{
			RotatePJoint* joint = new RotatePJoint(p0, p1, c0, c1);
			joint->angle = angle;
			joint->int212 = 0;
			joint->int216 = 0;
			return joint;
		}

		// RotateJoint.cpp:75.  canBuildJoint only ever calls this with a
		// surface its guard already proved IsRotate, so this is unreachable
		// from the joint builder -- but the binary has the assert, so the
		// port has it too.
		RBXASSERT(false);
		return NULL;
	}

	// mac: 0x59855C (2196B).  Factory returning Joint* (NULL when the pair cannot
	// carry a rotate joint).  Structure, in source order:
	//
	//   1. Guard: at least one of the two touching surfaces is a rotate
	//      surface (s-6 < 3 unsigned, i.e. IsRotate).  Otherwise NULL.
	//   2. The two face world-normals must be near-opposite:
	//      angle(N0, -N1) <= 0.025.  The normals are sign*column(n%3) with
	//      sign = +1 for n<3, -1 for n>=3 (the (-2*(n/3))|1 idiom).
	//   3. hasOverlap(f0, f1, 0.35) && overlapWithinPlanes(f0, f1, 0.05).
	//   4. contains1 = f1 contains center(f0) in 0.05 extrusion;
	//      contains0 = IsRotate(s1) && f0 contains center(f1) in 0.05.
	//      either = IsRotate(s0) && contains1.  NULL if neither.
	//   5. A/B assignment: no swap (A=p0) iff either && (!contains0 ||
	//      sizeSum(p1) <= sizeSum(p0)); else swap (A=p1).  sizeSum is
	//      x+y+z of getSize().  sA (the surface passed to surfaceTypeToJoint)
	//      is the rotate surface the taken branch proved: s0 on no-swap
	//      (either requires IsRotate(s0)), s1 on swap (contains0 requires
	//      IsRotate(s1)).
	//   6. c0 = A's face frame in A's object space; the world face center;
	//      the center in B's object space (rotation-transpose by hand);
	//      c1 = (normalIdToMatrix3(opposite(nB)), toGrid(relInB, 0.1)).
	//      Two attempts: grid-snapped first, then raw.  Each attempt passes
	//      (worldCenterA, c1World, NA_world, NB_world) through
	//      jointFramesCoincide; the first pass builds via surfaceTypeToJoint.
	//
	// DELTA, v57: the prologue computes a dead ±1.0 float from n0
	// ((-2*(n0/3))|1, stored then never read).  It was likely a leftover
	// angle local; the port omits it.
	static float sizeSum(const G3D::Vector3& s)
	{
		return s.x + s.y + s.z;
	}

	// STUB: PLAYER 0x00974f60
	// STUB: STUDIO 0x009c0870
	Joint* RotateJoint::canBuildJoint(
		Primitive* p0,
		Primitive* p1,
		NormalId n0,
		NormalId n1)
	{
		SurfaceType s0 = p0->getSurfaceType(n0);
		SurfaceType s1 = p1->getSurfaceType(n1);
		if (!IsRotate(s0) && !IsRotate(s1))
			return NULL;

		G3D::CoordinateFrame cf0 = p0->getCoordinateFrame();
		G3D::CoordinateFrame cf1 = p1->getCoordinateFrame();
		int a0 = (int)n0;
		int a1 = (int)n1;
		float sign0 = (a0 < 3) ? 1.0f : -1.0f;
		float sign1 = (a1 < 3) ? 1.0f : -1.0f;
		G3D::Vector3 N0 = sign0 * cf0.rotation.column(a0 % 3);
		G3D::Vector3 N1 = sign1 * cf1.rotation.column(a1 % 3);
		if (Math::angle(N0, -N1) > 0.025f)
			return NULL;

		Face f0 = p0->getFaceInWorld(n0);
		Face f1 = p1->getFaceInWorld(n1);
		if (!Face::hasOverlap(f0, f1, 0.35f))
			return NULL;
		if (!Face::overlapWithinPlanes(f0, f1, 0.05f))
			return NULL;

		G3D::Vector3 center0 = f0.center();
		G3D::Vector3 center1 = f1.center();
		bool contains1 = f1.fuzzyContainsInExtrusion(center0, 0.05f);
		bool contains0 = IsRotate(s1) && f0.fuzzyContainsInExtrusion(center1, 0.05f);
		bool either = IsRotate(s0) && contains1;
		if (!either && !contains0)
			return NULL;

		Primitive* A;
		Primitive* B;
		NormalId nA;
		NormalId nB;
		SurfaceType sA;
		G3D::Vector3 NA;
		G3D::Vector3 NB;
		if (either && (!contains0 || sizeSum(p1->getSize()) <= sizeSum(p0->getSize())))
		{
			A = p0; B = p1; nA = n0; nB = n1; sA = s0;
			NA = N0; NB = N1;
		}
		else
		{
			A = p1; B = p0; nA = n1; nB = n0; sA = s1;
			NA = N1; NB = N0;
		}

		G3D::CoordinateFrame c0 = A->getFaceCoordInObject(nA);
		G3D::CoordinateFrame cfA = A->getCoordinateFrame();
		G3D::CoordinateFrame cfB = B->getCoordinateFrame();
		// pointToWorldSpace, not operator*: our (frozen, vendor) G3D has no
		// CF*Vector3 overload, and the binary does this transform as flat
		// lane arithmetic anyway (0x5989A7: elt-by-elt multiply-adds).
		G3D::Vector3 worldCenter = cfA.pointToWorldSpace(c0.translation);
		G3D::Vector3 rel = worldCenter - cfB.translation;
		const G3D::Matrix3& RB = cfB.rotation;
		G3D::Vector3 relInB(
			RB.column(0).dot(rel),
			RB.column(1).dot(rel),
			RB.column(2).dot(rel));
		G3D::Matrix3 Rj(normalIdToMatrix3(normalIdOpposite(nB)));

		// Attempt 1: the B-local center snapped to a 0.1 grid.
		G3D::CoordinateFrame c1snapped(Rj, Math::toGrid(relInB, 0.1f));
		if (jointFramesCoincide(worldCenter, cfB.pointToWorldSpace(c1snapped.translation), NA, NB))
			return surfaceTypeToJoint(sA, A, B, c0, c1snapped);

		// Attempt 2: the raw B-local center.
		G3D::CoordinateFrame c1raw(Rj, relInB);
		if (jointFramesCoincide(worldCenter, cfB.pointToWorldSpace(c1raw.translation), NA, NB))
			return surfaceTypeToJoint(sA, A, B, c0, c1raw);

		return NULL;
	}

} // namespace RBX

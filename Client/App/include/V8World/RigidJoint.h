/* V8World/RigidJoint.h -- October 2013 (rigid auto-joint base: Weld/Snap).
   Evidence: Weld/Snap sizes new(0x98) (NO new members; D1 -> Joint::D2);
     Weld vtables off_F463A8/F46410, Snap off_F46438/F464A0;
     Weld/Snap -> getJointType + isBroken/isAligned/align = RigidJoint's;
     RigidJoint adds isAligned/fuzzyEq(1e-5)/align/getChildInParent/
     faceIdToCoords (faceIdToCoords 0x597E42 (554B) TODO).
   DELTAS: isBroken/isAligned/align/getChildInParent/faceIdToCoords bodies
     TODO (called only from unported builder/stepper paths); base getJointType
     asserts "0" (Joint.h:106) per binary (never called in tree — concrete
     types override it).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/RigidJoint.h (path inferred) */

#pragma once

#include "V8World/Joint.h"
#include "V8World/Primitive.h"

namespace RBX {

	class RigidJoint : public Joint
	{
	public:
		RigidJoint()
		{}
		RigidJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: Joint(p0, p1, c0, c1)
		{}

		// TODO bodies (builder/stepper paths only; see header note).
		virtual bool isBroken() const { return false; }
		// Non-const: overrides Joint::isAligned (vtable+0x28); PLAYER body
		// 0x008ee6f0 is real (TODO), not the base `return true`.
		virtual bool isAligned() { return true; }
		// mac: 0x597CEA (batch271): child frame in parent from the joint
		// coords (2016-verbatim; manual inverse in the binary matches
		// CF::inverse + compose + orthonormalize exactly).
		G3D::CoordinateFrame getChildInParent(Primitive* parent, Primitive* child);
		// mac: 0x597E42 (n1 unread): c0 = face coord, c0world = p0CF*c0,
		// c1 = inv(p1CF)*c0world (inverse LEFT per stack slots).
		static void faceIdToCoords(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1,
			G3D::CoordinateFrame& c0, G3D::CoordinateFrame& c1)
		{
			(void)n1;
			c0 = p0->getFaceCoordInObject(n0);
			G3D::CoordinateFrame c0world = p0->getCoordinateFrame() * c0;
			c1 = p1->getCoordinateFrame().inverse() * c0world;
		}
	};

	class WeldJoint : public RigidJoint
	{
	public:
		WeldJoint()
		{}
		WeldJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: RigidJoint(p0, p1, c0, c1)
		{}

		virtual int getJointType() const { return WELD_JOINT; }

		// mac: 0x5A7E64 (247 bytes) -- NOT a predicate: this is the auto-joint
		// FACTORY for the weld family and it returns a heap WeldJoint
		// (operator new(0x98) + Joint::Joint(p0,p1,c0,c1) with the
		// off_F463A8/off_F46410 vtable pair), or NULL.  Two-stage guard:
		//   (p0->getSurfaceType(n0) == WELD || p1->getSurfaceType(n1) == WELD)
		//   && Joint::canBuildJointTight(p0,p1,n0,n1)
		// The surface reads are `cmp dword [p0+edx*4+0x10C], 2` == WELD.
		static Joint* canBuildJoint(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1)
		{
			if ((p0->getSurfaceType(n0) == WELD || p1->getSurfaceType(n1) == WELD) &&
				Joint::canBuildJointTight(p0, p1, n0, n1))
			{
				G3D::CoordinateFrame c0, c1;
				RigidJoint::faceIdToCoords(p0, p1, n0, n1, c0, c1);
				return new WeldJoint(p0, p1, c0, c1);
			}
			return NULL;
		}
	};

	class SnapJoint : public RigidJoint
	{
	public:
		SnapJoint()
		{}
		SnapJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: RigidJoint(p0, p1, c0, c1)
		{}

		virtual int getJointType() const { return SNAP_JOINT; }

		// mac: 0x5A1C68.
		static bool compatibleSurfaces(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1);

		// mac: 0x5A1CCC (258 bytes) -- snap counterpart of
		// WeldJoint::canBuildJoint, same two-stage guard but the surface
		// test is the SnapJoint::compatibleSurfaces helper.
		static Joint* canBuildJoint(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1)
		{
			if (SnapJoint::compatibleSurfaces(p0, p1, n0, n1) &&
				Joint::canBuildJointTight(p0, p1, n0, n1))
			{
				G3D::CoordinateFrame c0, c1;
				RigidJoint::faceIdToCoords(p0, p1, n0, n1, c0, c1);
				return new SnapJoint(p0, p1, c0, c1);
			}
			return NULL;
		}
	};

} // namespace RBX

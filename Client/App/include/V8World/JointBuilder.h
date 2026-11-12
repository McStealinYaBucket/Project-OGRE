/* V8World/JointBuilder.h -- October 2013 (auto-joint factory namespace).
   Evidence: the 2013 binary carries exactly ONE JointBuilder member, and the
     mangled name
       __ZN3RBX12JointBuilder7canJoinEPNS_9PrimitiveES2_
     is `JointBuilder::canJoin(Primitive*, Primitive*)` with NO `this`
     parameter -- i.e. a STATIC member (hexrays renders p0 as `this`).
   JointBuilder owns no state in the binary (no vtable, no ctor, no
   instance symbols), so it is a pure static holder.
   Original file: (2013 path TODO -- header-only holder). */

#pragma once

#include "G3D/CoordinateFrame.h"
#include "Util/NormalId.h"

namespace RBX {

	class Primitive;
	class Joint;

	class JointBuilder
	{
	public:
		// mac: 0x57B164 (362 bytes).  NOT a predicate: this is the AUTO-JOINT
		// FACTORY.  It returns the first Joint* produced by the
		// RotateJoint / WeldJoint / SnapJoint / GlueJoint canBuildJoint
		// statics for the first viable face of p0, or NULL.
		//
		//   if (p0->getFastFuzzyExtents()
		//           .separatedByMoreThan(p1->getFastFuzzyExtents(), 0.05f))
		//       return NULL;
		//   for (int i = 0; i < 6; ++i)
		//   {
		//       // 0x57B1xx: s = (float)((-2 * (i / 3)) | 1)  -> +1 for i<3, -1 for i>=3
		//       float s = (i < 3) ? 1.0f : -1.0f;
		//       // the world direction of p0's local axis (i % 3), negated
		//       Vector3 n = -(s * p0->getCoordinateFrame().rotation.column(i % 3));
		//       NormalId n1 = Math::getClosestObjectNormalId(n, p1->getCoordinateFrame().rotation);
		//       Joint* j = RotateJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
		//       if (!j) j = WeldJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
		//       if (!j) j = SnapJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
		//       if (!j) j = GlueJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
		//       if (j)  return j;
		//   }
		//   return NULL;
		//
		// DELTA (documented, session 105 resolved session 110): the GlueJoint
		// arm is GlueJoint::canBuildJoint 0x57382E, ported in
		// v8world/GlueJoint.cpp.  Chain is Rotate -> Weld -> Snap -> Glue.
		static Joint* canJoin(Primitive* p0, Primitive* p1);
	};

} // namespace RBX

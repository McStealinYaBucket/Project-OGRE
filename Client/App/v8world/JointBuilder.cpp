/* App/v8world/JointBuilder.cpp -- October 2013 (auto-joint factory).
   See V8World/JointBuilder.h for the disassembly-derived body sketch and the
   documented Rotate/Glue DELTA. */

#include "stdafx.h"

#include "V8World/JointBuilder.h"
#include "V8World/Primitive.h"
#include "V8World/RigidJoint.h"
#include "V8World/MultiJoint.h"
#include "Util/Math.h"

namespace RBX {

	// mac: 0x57B164.  The separation gate runs FIRST and uses the CACHED fuzzy
	// extents of both prims (0x59003A), growing this side by 0.05f
	// (0x3D4CCCCD) and negating the overlap test.
	//
	// The face loop then walks i = 0..5 (i.e. all six NormalIds in order
	// NORM_X, NORM_Y, NORM_Z, NORM_X_NEG, NORM_Y_NEG, NORM_Z_NEG) and for
	// each one projects the world direction of p0's local axis
	// (i % 3) onto p1's axes to pick the facing side:
	//
	//   s   = (i < 3) ? +1.0f : -1.0f          // 0x57B1A8: (-2*(i/3))|1
	//   n   = -s * cf0.rotation.column(i % 3)  // flat lanes {k, 3+k, 6+k}
	//   n1  = Math::getClosestObjectNormalId(n, cf1.rotation)
	//
	// `column(i)` in G3D is {elt[0][i], elt[1][i], elt[2][i]} -- the three
	// floats at flat offsets 4*i, 4*i+12 and 4*i+24, exactly what the
	// disasm reads.
	// STUB: PLAYER 0x008ef4f0
	// STUB: STUDIO 0x00a0d270
	Joint* JointBuilder::canJoin(Primitive* p0, Primitive* p1)
	{
		if (p0->getFastFuzzyExtents().separatedByMoreThan(p1->getFastFuzzyExtents(), 0.05f))
			return NULL;

		for (int i = 0; i < 6; ++i)
		{
			float s = (i < 3) ? 1.0f : -1.0f;
			G3D::Vector3 n = -(s * p0->getCoordinateFrame().rotation.column(i % 3));
			NormalId n1 = Math::getClosestObjectNormalId(n, p1->getCoordinateFrame().rotation);

			// RotateJoint first in the binary (0x57B1F4), then Weld, then Snap.
			// (GlueJoint last -- not ported yet, see the header DELTA.)
			Joint* joint = RotateJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
			if (!joint)
				joint = WeldJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
			if (!joint)
				joint = SnapJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
			if (!joint)
				joint = GlueJoint::canBuildJoint(p0, p1, (NormalId)i, n1);
			if (joint)
				return joint;
		}
		return NULL;
	}

} // namespace RBX

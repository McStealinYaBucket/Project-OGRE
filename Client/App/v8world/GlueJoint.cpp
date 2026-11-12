/* App/v8world/GlueJoint.cpp -- October 2013 RBX::GlueJoint::canBuildJoint.
 *
 * 0x57382E, 1775 bytes, verbatim (variable names follow the disasm's stack
 * temps; the Face copies are named for what they hold):
 *
 *   if ((s0 == GLUE || s1 == GLUE)                      // prim+0x10C[n]==1
 *       && Joint::canBuildJointLoose(p0, p1, n0, n1))
 *   {
 *       Face f0 = p0->getFaceInWorld(n0);               // sret var_C8
 *       Face f1 = p1->getFaceInWorld(n1);               // sret var_F8
 *       Face proj0 = f0.projectOverlapOnMe(f1);         // sret var_128
 *       Face proj1 = f1.projectOverlapOnMe(f0);         // sret var_158
 *       Face fObj0 = proj0.toObjectSpace(CF(p0));       // sret var_188
 *       Face fObj1 = proj1.toObjectSpace(CF(p1));       // sret var_1B8
 *       Face fSnap0 = fObj0; fSnap0.snapToGrid(0.1);    // copy var_1E8
 *       Face fSnap1 = fObj1; fSnap1.snapToGrid(0.1);    // copy var_218
 *       Face wSnap0 = fSnap0.toWorldSpace(CF(p0));      // sret var_248
 *       Face wSnap1 = fSnap1.toWorldSpace(CF(p1));      // sret var_278
 *       bool aligned;
 *       if (Face::cornersAligned(wSnap0, wSnap1, 0.05))
 *           aligned = true;                             // jnz loc_573CA6
 *       else {
 *           // Second attempt on the RAW (unsnapped) object faces; the
 *           // results are memcpy'd OVER wSnap0/wSnap1 in place.
 *           Face wRaw0 = fObj0.toWorldSpace(CF(p0));
 *           Face wRaw1 = fObj1.toWorldSpace(CF(p1));
 *           aligned = Face::cornersAligned(wRaw0, wRaw1, 0.05);
 *       }
 *       if (!aligned)
 *           return NULL;
 *       CoordinateFrame c0 = p0->getFaceCoordInObject(n0);   // var_308
 *       // c1 = c0 carried into p1's frame, snapped to a 0.1 grid.  The
 *       // binary hand-builds CF(p1).inverse() (transposed rotation +
 *       // negated projected translation, 0x573CF9-0x573DDB) and multiplies;
 *       // the port spells the same value with inverse() -- identical math,
 *       // last-ulp float order documented as DELTA.
 *       CoordinateFrame c1 = Math::snapToGrid(
 *           p1->getCoordinateFrame().inverse()
 *               * (p0->getCoordinateFrame() * c0),
 *           0.1f);
 *       GlueJoint* joint = new GlueJoint(p0, p1, c0, c1);   // MultiJoint(4)
 *       Face fp = fObj0.toObjectSpace(c0);   // overlap in the joint frame
 *       joint->f0 = fp[0]; joint->f1 = fp[1];               // +208/+220
 *       joint->f2 = fp[2]; joint->f3 = fp[3];               // +232/+244
 *       return joint;
 *   }
 *   return NULL;
 *
 * Sret note: every Face return above goes through a hidden sret pointer in
 * the binary ([esp] = output temp, then this, then args).  MSVC does the
 * same transformation automatically for by-value returns, so the port just
 * writes the natural calls.
 *
 * The f0..f3 stores are direct member writes (a static member of GlueJoint
 * has private access) -- the binary does the same movsd/mov pairs at
 * 0x573E86-0x573EE2, not setFacePoint calls.
 *
 * Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/GlueJoint.cpp */

#include "stdafx.h"

#include "V8World/MultiJoint.h"
#include "V8World/Primitive.h"
#include "Util/Math.h"
#include "Util/Face.h"

namespace RBX {

	// STUB: PLAYER 0x00975e10
	// STUB: STUDIO 0x009c1750
	Joint* GlueJoint::canBuildJoint(
		Primitive* p0,
		Primitive* p1,
		NormalId n0,
		NormalId n1)
	{
		if ((p0->getSurfaceType(n0) == GLUE || p1->getSurfaceType(n1) == GLUE)
			&& Joint::canBuildJointLoose(p0, p1, n0, n1))
		{
			Face f0 = p0->getFaceInWorld(n0);
			Face f1 = p1->getFaceInWorld(n1);
			Face proj0 = f0.projectOverlapOnMe(f1);
			Face proj1 = f1.projectOverlapOnMe(f0);
			Face fObj0 = proj0.toObjectSpace(p0->getCoordinateFrame());
			Face fObj1 = proj1.toObjectSpace(p1->getCoordinateFrame());

			Face fSnap0 = fObj0;
			fSnap0.snapToGrid(0.1f);
			Face fSnap1 = fObj1;
			fSnap1.snapToGrid(0.1f);
			Face wSnap0 = fSnap0.toWorldSpace(p0->getCoordinateFrame());
			Face wSnap1 = fSnap1.toWorldSpace(p1->getCoordinateFrame());

			bool aligned;
			if (Face::cornersAligned(wSnap0, wSnap1, 0.05f))
			{
				aligned = true;
			}
			else
			{
				Face wRaw0 = fObj0.toWorldSpace(p0->getCoordinateFrame());
				Face wRaw1 = fObj1.toWorldSpace(p1->getCoordinateFrame());
				aligned = Face::cornersAligned(wRaw0, wRaw1, 0.05f);
			}
			if (!aligned)
				return NULL;

			G3D::CoordinateFrame c0 = p0->getFaceCoordInObject(n0);
			G3D::CoordinateFrame c1 = Math::snapToGrid(
				p1->getCoordinateFrame().inverse()
					* (p0->getCoordinateFrame() * c0),
				0.1f);

			GlueJoint* joint = new GlueJoint(p0, p1, c0, c1);
			Face fp = fObj0.toObjectSpace(c0);
			joint->f0 = fp[0];
			joint->f1 = fp[1];
			joint->f2 = fp[2];
			joint->f3 = fp[3];
			return joint;
		}
		return NULL;
	}

} // namespace RBX

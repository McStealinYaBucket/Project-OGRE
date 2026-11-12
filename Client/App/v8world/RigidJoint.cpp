/* App/v8world/RigidJoint.cpp -- October 2013 (rigid auto-joint statics).
   See V8World/RigidJoint.h for the layout/vtable evidence.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/RigidJoint.h (path inferred) */

#include "stdafx.h"

#include "V8World/RigidJoint.h"

namespace RBX {

	// mac: 0x597CEA (batch271; 2016-verbatim, binary-identical).
	G3D::CoordinateFrame RigidJoint::getChildInParent(Primitive* parent, Primitive* child)
	{
		const G3D::CoordinateFrame& cParent = (parent == getPrimitive(0)) ? getJointCoord0() : getJointCoord1();
		const G3D::CoordinateFrame& cChild = (child == getPrimitive(0)) ? getJointCoord0() : getJointCoord1();
		G3D::CoordinateFrame answer = cParent * cChild.inverse();
		Math::orthonormalizeIfNecessary(answer.rotation);
		return answer;
	}

	// mac: 0x5A1C68 (100 bytes, no callees) -- pure surface-type compatibility
	// table.  The two reads are `mov ecx,[p0+n1*4+0x10Ch]` and
	// `mov eax,[p1+n0*4+0x10Ch]` in the disasm (the argument order in the
	// decompilation is swapped; both slots are the same SurfaceType array
	// read, one per primitive).  Values: 3 = STUDS, 4 = INLET,
	// 5 = UNIVERSAL.
	//
	// The disasm chain, in order:
	//   if (s0 == STUDS    && s1 == INLET)   return 1;   // 0x5A1C85
	//   if (s0 == INLET    && s1 == STUDS)   return 1;   // 0x5A1C8F
	//   if (s0 == UNIVERSAL&& s1 == UNIVERSAL)return 1;  // 0x5A1C99
	//   if (s0 == UNIVERSAL&& (s1 - STUDS) < 2) return 1; // 0x5A1CA3 (unsigned)
	//   // 0x5A1CB7 tail
	//   return (s1 == UNIVERSAL) && ((s0 - STUDS) < 2);
	bool SnapJoint::compatibleSurfaces(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1)
	{
		SurfaceType s0 = p0->getSurfaceType(n0);
		SurfaceType s1 = p1->getSurfaceType(n1);
		if (s0 == STUDS && s1 == INLET)
			return true;
		if (s0 == INLET && s1 == STUDS)
			return true;
		if (s0 == UNIVERSAL && s1 == UNIVERSAL)
			return true;
		if (s0 == UNIVERSAL && (s1 == STUDS || s1 == INLET))
			return true;
		return (s1 == UNIVERSAL) && (s0 == STUDS || s0 == INLET);
	}

} // namespace RBX

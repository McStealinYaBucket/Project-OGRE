/* V8World/RotateConnector.h -- October 2013 RBX::RotateConnector (MINIMAL).
 *
 * EVIDENCE (symbolled 0.126.0.39503 Studio, idalib; batch116):
 *   0x5266AA  RotateConnector::computeJointAngle(CF const&,CF const&,
 *               CF const&,CF const&,Vector3 &)                        275B
 *   0xEED700  `vtable for'RBX::RotateConnector
 *   0x5264C0  RotateConnector::RotateConnector(Body*,Body*,CF const&,CF const&,
 *               float,float,float)
 *   0x5265EC  reset / 0x526626 computeNormalRotationFromBase /
 *   0x5267BE  computeNormalRotationFromBaseFast / 0x526890 setRotationalGoal /
 *   0x5269B8  setVelocityGoal / 0x526A00 stepGoals / 0x526A6A computeForce /
 *   0x526E34  dtor
 *
 * computeJointAngle is a STATIC member (hexrays prints it __cdecl with no
 * hidden this -- the `this` in the decompilation is just arg_0, the first
 * CF).  That is the ONLY member ported here, because it is the only one
 * surfaceTypeToJoint needs: its st0 return value becomes the RotateP/V
 * `angle` at +208.
 *
 * DELTA: the class is real and polymorphic (vtable at 0xEED700) with a
 * Body/Connector stepper behind it, none of which is ported.  Declaring ONLY
 * the static emits no vtable and no ctor, so nothing here can conflict with
 * the future full class -- but the future class MUST keep this exact static
 * signature.  Do not instantiate this class.
 *
 * Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/RotateConnector.h */

#pragma once

#include "G3D/CoordinateFrame.h"
#include "G3D/Matrix3.h"
#include "G3D/Vector3.h"

namespace RBX {

	class RotateConnector
	{
	public:
		// mac: 0x5266AA (275B).  Returns the joint angle (radians) between the
		// two joint frames; also writes one column vector to `out`, which
		// the only caller (surfaceTypeToJoint) leaves as a dead store.
		// See v8world/RotateConnector.cpp for the body.
		static float computeJointAngle(
			const G3D::CoordinateFrame& cf0,
			const G3D::CoordinateFrame& cf1,
			const G3D::CoordinateFrame& c0,
			const G3D::CoordinateFrame& c1,
			G3D::Vector3& out);
	};

} // namespace RBX

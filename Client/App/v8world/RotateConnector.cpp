/* App/v8world/RotateConnector.cpp -- October 2013
   RBX::RotateConnector::computeJointAngle (the ONLY ported member; see the
   header for what is deliberately missing).
 *
 * 0x5266AA, 275 bytes, verbatim:
 *
 *     Matrix3 m0 = cf0.rotation * c0.rotation;      // sret temp var_40
 *     Matrix3 m1 = cf1.rotation * c1.rotation;      // sret temp var_68
 *     Matrix3 rel = m0.transpose() * m1;            // sret temp var_90
 *     out = m0.column(2);                           // the dead store
 *     Vector3 c = rel.column(0);
 *     return (float)atan2((double)c.y, (double)c.x);
 *
 * Notes on the disasm:
 * - The CF* arguments are passed straight into Matrix3::operator* (which
 *   takes `Matrix3 const&`): legal because a CoordinateFrame's first member
 *   IS its rotation Matrix3 (the same 48-byte Face/CF layout pun the joint
 *   code relies on everywhere).
 * - `column(out, 2)` / `column(tmp, 0)` are Matrix3::column(int) const
 *   (0x75958A), selected by the immediates 2 and 0 at 0x526731 / 0x52676C.
 * - The return is `cvtss2sd` on both components, `call _atan2`, `fstp` to a
 *   temp, `movss` back through xmm, `fld` for the st0 return -- i.e. a float
 *   computed in double precision and narrowed, which is exactly what
 *   `(float)atan2((double)y, (double)x)` compiles to.
 * - The `out` write (column 2 of m0) is confirmed dead: surfaceTypeToJoint
 *   passes a zeroed stack Vector3 and never reads it afterwards.  It is kept
 *   because dropping a parameter would change the mangling.
 *
 * Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/RotateConnector.cpp */

#include "stdafx.h"

#include "V8World/RotateConnector.h"

#include <cmath>

namespace RBX {

	float RotateConnector::computeJointAngle(
		const G3D::CoordinateFrame& cf0,
		const G3D::CoordinateFrame& cf1,
		const G3D::CoordinateFrame& c0,
		const G3D::CoordinateFrame& c1,
		G3D::Vector3& out)
	{
		G3D::Matrix3 m0 = cf0.rotation * c0.rotation;
		G3D::Matrix3 m1 = cf1.rotation * c1.rotation;
		G3D::Matrix3 rel = m0.transpose() * m1;

		out = m0.column(2);

		G3D::Vector3 c = rel.column(0);
		return (float)atan2((double)c.y, (double)c.x);
	}

} // namespace RBX

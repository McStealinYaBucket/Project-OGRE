/* V8Kernel/Link.h -- October 2013 (joint-body link state).
   Evidence: C2 0x531126 (batch288: vptr + child NULL + 4x CF C2 +
   serial = Body::getNextStateIndex()) / D2 0x531194 (empty) (batch288).
   Layout: vptr +0 / child Body* +4 / 4x CoordinateFrame +8/+56/+104/+152
   / serial int +200 (total 204B).
   DELTAS: vtable has only D1/D0 here (the vtbl+0 refresh + reset +
   getChildInParent queue with the motor-joint work -- Link.h/C2 are
   complete for construction + storage; behavior lands with motors).
   Original file: Client/App/include/V8Kernel/Link.h (path inferred). */

#pragma once

#include "G3D/CoordinateFrame.h"

namespace RBX {

	class Body;

	class Link
	{
	public:
		Link();
		virtual ~Link() {}

	private:
		Body* child;                    // +4 (C2 NULL).
		G3D::CoordinateFrame cf08;      // +8 (C2 identity).
		G3D::CoordinateFrame cf56;      // +56 (C2 identity).
		G3D::CoordinateFrame cf104;     // +104 (C2 identity).
		G3D::CoordinateFrame cf152;     // +152 (C2 identity).
		int serial;                     // +200 (C2 getNextStateIndex).
	};

} // namespace RBX

/* App/tool/Dragger.cpp -- October 2013 (drag-safe placement, shell).
   Only Dragger::safeMoveNoDrop (0x1C300A) is declared so far; see
   include/Tool/Dragger.h for the full EA list.  The body is a BRING-UP NO-OP
   (not decompiled; needs the ContactManager broadphase). */

#include "stdafx.h"

#include "Tool/Dragger.h"
#include "V8World/ContactManager.h"

namespace RBX {

	// mac: 0x1C300A.  TODO(decompile): real body walks prims along `delta`
	// calling searchUpFine/searchDownFine against `contactManager`.
	// STUB: PLAYER 0x008e1ac0
	// STUB: STUDIO 0x009cd190
	void Dragger::safeMoveNoDrop(const G3D::Array<Primitive*, 10, 32>& prims,
		const G3D::Vector3& delta, ContactManager& contactManager)
	{
		(void)prims;
		(void)delta;
		(void)contactManager;
	}

} // namespace RBX

/* App/v8world/MutilJoint.cpp -- October 2013 (MultiJoint bodies; the file name's typo is the
   2013 assert path, see V8World/MultiJoint.h).
   PLAYER's MultiJoint::putInKernel body is identical to Contact::putInKernel, and the 2013 linker
   folded the two at 0x007b8eb0 (STUDIO keeps its own copy at 0x009bf480). */

#include "stdafx.h"

#include "V8World/MultiJoint.h"
#include "decomp.h"

namespace RBX {

	// STUB: PLAYER 0x007b8eb0
	// STUB: STUDIO 0x009bf480
	void MultiJoint::putInKernel(Kernel* kernel)
	{
		STUB(0x007b8eb0);
	}

	// D2 0x587A2A (+ Joint D2 via base). STUDIO emits it before putInKernel
	// (0x009bf480); placed here for PLAYER's order, where putInKernel is folded onto Contact's copy.
	// STUB: PLAYER 0x009741c0
	// STUB: STUDIO 0x009bf3a0
	MultiJoint::~MultiJoint()
	{
		RBXASSERT(connectors[0] == NULL);
		RBXASSERT(numConnector == 0);
	}

	// STUB: PLAYER 0x00974340
	// STUB: STUDIO 0x009bf530
	void MultiJoint::removeFromKernel()
	{
		STUB(0x00974340);
	}

	// STUB: PLAYER 0x009744c0
	// STUB: STUDIO 0x009bf6b0
	bool MultiJoint::isBroken() const
	{
		STUB(0x009744c0);
		return false;
	}

} // namespace RBX

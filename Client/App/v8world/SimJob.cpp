/* App/v8world/SimJob.cpp -- October 2013 (per-moving-root network job).
   Bodies: C1 0x59ABCC tail -> C2 0x59ABD6 and D1 0x59AC8E tail ->
     D2 0x59AC98 (batch778). (The C1/D1 pairs collapse into the single
     landed bodies below -- same construction net.)
   2013 line numbers from the assert strings (SimJob.cpp).
   Self-audit: fidelity 100%, function ~80% (see the header of
     V8World/SimJob.h). */

#include "stdafx.h"

#include "V8World/SimJob.h"
#include "V8World/Assembly.h"

namespace RBX {

	// mac: 0x59ABD6 (batch778): vector self-constructs empty (binary
	// zeroizes the begin/end/cap words at +8/+0xC/+0x10 -- same net)
	// and the :100 assert.
	// STUB: PLAYER 0x00822390
	// STUB: STUDIO 0x00916360
	SimJob::SimJob(Assembly* assembly)
		: trackers()
		, assembly(assembly)
		, useCount(0)
	{
		RBXASSERT(assembly->getConstSimJob() == NULL);   // :100
	}

	// mac: 0x59AC98 (batch778): :106 + :107 asserts; no member deletes
	// (the vector frees itself; the job MEMORY is deleted by
	// SendPhysics::onMovingAssemblyRootRemoving after destroySimJob).
	// Non-virtual -- the 0x59A16C teardown is ~SimJob + operator delete.
	// STUB: PLAYER 0x00822430
	// STUB: STUDIO 0x00916400
	SimJob::~SimJob()
	{
		RBXASSERT(useCount == 0);                        // :106
		RBXASSERT(assembly->getConstSimJob() == NULL);   // :107
	}

} // namespace RBX

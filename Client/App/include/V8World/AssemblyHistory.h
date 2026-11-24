/* V8World/AssemblyHistory.h -- October 2013 (motion-history shell).
   Only wakeUp() is ported (batch261: stores 15 at +24). C2/D2/sample/
   average (Average<PhysicsCoord> member, getAssemblyPhysicsCoord,
   maxDeviationSquared, preventNeighborSleep, sampleAndNotMoving --
   all batch261-banked) queue with the Body-branch sampling work: the
   history pointer (Assembly +60) is always NULL in-tree (C2 NULL, D2
   asserts NULL), so no construction site exists yet.
   +24 models the wake counter dword the binary zeroes-to-15; the rest
   of the footprint is unmapped padding (same precedent as the contact
   solver pads).
   Original file: Client/App/include/V8World/AssemblyHistory.h (path
   inferred). */

#pragma once

#include "decomp.h"

namespace RBX {

	class Assembly;

	// SIZE 0x20 (Assembly::reset PLAYER 0x007b15b0 news 0x20; the ctor 0x0095e450 stores 0 at +0x18 and
	// 15 at +0x1c, the wake counter AssemblyHistory::wakeUp 0x0095d500 resets).
	class AssemblyHistory
	{
	public:
		AssemblyHistory(Assembly* assembly);
		~AssemblyHistory();

		void wakeUp();

	private:
		undefined m_unk0x00[0x18 - 0x00];	// 0x00 (the ctor builds an Average<PhysicsCoord> here)
		int m_unk0x18;						// 0x18 (ctor 0)
		int wakeCounter;					// 0x1c (ctor and wakeUp 15)
	};

} // namespace RBX

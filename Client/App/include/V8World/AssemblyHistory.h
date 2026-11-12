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

namespace RBX {

	class AssemblyHistory
	{
	public:
		void wakeUp();

	private:
		char reserved[24];  // +0..+20 (unmapped; C2 builds Average here).
		int wakeCounter;    // +24 (wakeUp stores 15).
	};

} // namespace RBX

/* V8Kernel/Cofm.h -- October 2013 (composite center-of-mass node).
   Evidence: C1 0x5259DE (batch292: Allocator base + body store +0 /
   dirty=1 +4 / cofmInBody zero +8) / updateIfDirty 0x525A10
   (batch293: leaf vs multi-child branches + Cofm.cpp:35 fuzzyEq +
   :57 dirty asserts). Layout: body +0 / dirty +4 / cofmInBody +8 /
   mass +20 / moment +24 (60B total -- Allocator news 60).
   Member names carried from 2016 Cofm.h (kind+role agree; binary
   offsets confirm the layout: +8 cofmInBody reads in
   getBranchCofmOffset/CofmPos, +20 mass in getBranchIWorldAtPoint,
   +24 moment in getBranchIBody).
   DELTAS: C1 leaves mass/moment uninitialized in the binary --
   zeroed here (unread while dirty: every accessor runs
   updateIfDirty first, which recomputes both before any read).
   Original file: Client/App/include/V8Kernel/Cofm.h (path from
   assert strings: Cofm.cpp). */

#pragma once

#include "G3D/Vector3.h"
#include "G3D/Matrix3.h"
#include "rbx/memory.h"

namespace RBX {

	class Body;

	class Cofm : public Allocator<Cofm>
	{
	private:
		Body* body;                 // +0.
		bool dirty;                 // +4.
		G3D::Vector3 cofmInBody;    // +8.
		float mass;                 // +20.
		G3D::Matrix3 moment;        // +24.

		// mac: 0x525A10 (batch293 -- 2016 Cofm.cpp text, binary-identical
		// control flow including the IndexArray index asserts and the
		// :35/:57 asserts).
		void updateIfDirty();

	public:
		// mac: 0x5259DE (batch292).
		Cofm(Body* body);

		bool getIsDirty() const { return dirty; }

		void makeDirty() { dirty = true; }

		const G3D::Vector3& getCofmInBody() {
			updateIfDirty();
			return cofmInBody;
		}

		float getMass() {
			updateIfDirty();
			return mass;
		}

		const G3D::Matrix3& getMoment() {
			updateIfDirty();
			return moment;
		}
	};

} // namespace RBX

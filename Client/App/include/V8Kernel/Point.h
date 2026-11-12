/* V8Kernel/Point.h -- October 2013 (kernel attach point).
   Evidence: C2 0x532AB8 (batch244: vptr + kernelIndex -1 + numOwners 1 +
     body (arg or worldBody) + 3 zeroed Vector3s) / step/forceToBody/
     accumulate fns queue with the stepper (section 123+).
   Members (binary offsets; 2016 names first-party):
     KernelIndex base (+0 vptr / +4 kernelIndex -1)
     +8 numOwners (C2 1) / +12 body / +16 localPos / +28 worldPos /
     +40 force (all C2 zero except noted)
   Step/accumulate/setters: stepper flow, deferred (Kernel::insertPoint
   + deletePoint link against C2/D2 only).
   Original file: Client/App/include/V8Kernel/Point.h (path inferred) */

#pragma once

#include "V8Kernel/KernelIndex.h"
#include "G3D/Vector3.h"

namespace RBX {

	class Body;

	class Point : public KernelIndex
	{
		friend class KernelData;
		friend class Kernel;

	public:
		Point(Body* body = NULL);
		virtual ~Point() {}

		Body* getBody() { return body; }
		const G3D::Vector3& getWorldPos() const { return worldPos; }
		// 2016 shapes (first-party; EAs queued with the stepper).
		void step();
		void forceToBody();
		void setLocalPos(const G3D::Vector3& value);
		void setWorldPos(const G3D::Vector3& value);

		// 2016 shape (first-party; searchForDuplicatePoint predicate).
		static bool sameBodyAndOffset(const Point& p0, const Point& p1) {
			return (p0.body == p1.body) && (p0.localPos == p1.localPos);
		}

	private:
		int numOwners;            // +8 (C2 1).
		Body* body;               // +12.
		G3D::Vector3 localPos;    // +16.
		G3D::Vector3 worldPos;    // +28.
		G3D::Vector3 force;       // +40.
	};

} // namespace RBX

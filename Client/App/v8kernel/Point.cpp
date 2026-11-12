/* App/v8kernel/Point.cpp -- October 2013 (kernel attach point).
   Body: C2 0x532AB8 (batch244: kernelIndex -1 + numOwners 1 + body arg
   or worldBody + zeroed vectors). Step/accumulate/setters: stepper flow,
   deferred (Kernel::insertPoint/deletePoint link against C2/D2 only). */

#include "stdafx.h"

#include "V8Kernel/Point.h"
#include "V8Kernel/Body.h"

namespace RBX {

	// mac: 0x532AB8 (batch244).
	Point::Point(Body* bodyArg)
		: numOwners(1)
		, body(bodyArg != NULL ? bodyArg : Body::getWorldBody())
	{
		// localPos/worldPos/force zero via Vector3 defaults.
	}

	// 2016 shapes (first-party one-liners; EAs queued with the stepper).
	void Point::step()
	{
		worldPos = body->getCoordinateFrame().pointToWorldSpace(localPos);
		force = G3D::Vector3::zero();
	}

	void Point::forceToBody()
	{
		body->accumulateForce(force, worldPos);
	}

	void Point::setLocalPos(const G3D::Vector3& value)
	{
		localPos = value;
		worldPos = body->getCoordinateFrame().pointToWorldSpace(localPos);
	}

	void Point::setWorldPos(const G3D::Vector3& value)
	{
		worldPos = value;
		localPos = body->getCoordinateFrame().pointToObjectSpace(worldPos);
	}

} // namespace RBX

/* App/v8kernel/Link.cpp -- October 2013. Laid out on dirty's assert (Link.cpp:24); .obj order differs. */
#include "stdafx.h"
#include "V8Kernel/Link.h"
#include "V8Kernel/Body.h"
#include "Util/Math.h"

namespace RBX {

	Link::Link()
		: body(NULL)
		, stateIndex(Body::getNextStateIndex())
	{
	}

	Link::~Link()
	{
	}

	// FUNCTION: PLAYER 0x008e3740
	// FUNCTION: STUDIO 0x009cee10
	void Link::dirty()
	{
		if (body) {
			RBXASSERT(body->getLink() == this);
			body->makeStateDirty();
		}
	}

	// FUNCTION: PLAYER 0x008e37a0
	// FUNCTION: STUDIO 0x009cee70
	void Link::reset(const G3D::CoordinateFrame& parentC, const G3D::CoordinateFrame& childC)
	{
		parentCoord = parentC;
		childCoord = childC;
		childCoordInverse = childC.inverse();
	}

	// FUNCTION: PLAYER 0x008e3580
	// FUNCTION: STUDIO 0x009cec50
	void RevoluteLink::computeChildInParent(G3D::CoordinateFrame& answer) const
	{
		G3D::CoordinateFrame rotatedParentCoord(Math::rotateAboutZ(parentCoord.rotation, jointAngle),
			parentCoord.translation);

		G3D::CoordinateFrame::mul(rotatedParentCoord, childCoordInverse, answer);
	}

	// FUNCTION: PLAYER 0x008e3670
	// FUNCTION: STUDIO 0x009ced40
	void D6Link::computeChildInParent(G3D::CoordinateFrame& answer) const
	{
		G3D::CoordinateFrame rotatedParentCoord = parentCoord * offsetCFrame;

		G3D::CoordinateFrame::mul(rotatedParentCoord, childCoordInverse, answer);
	}

} // namespace RBX

/* App/v8kernel/Cofm.cpp -- October 2013 (composite center of mass).
   Bodies: C1 0x5259DE (batch292) / updateIfDirty 0x525A10 (batch293;
   2016 Cofm.cpp text -- binary-identical control flow). */

#include "stdafx.h"
#include "V8Kernel/Cofm.h"
#include "V8Kernel/Body.h"
#include "Util/Math.h"
#include "rbx/Debug.h"
namespace RBX {
	// mac: 0x5259DE (batch292: Allocator base + body store + dirty=1 +
	// cofmInBody zero; mass/moment zeroed here vs uninit in the binary
	// -- see header DELTA).
	Cofm::Cofm(Body* _body)
		: body(_body)
		, dirty(true)
		, cofmInBody(G3D::Vector3::zero())
		, mass(0.0f)
	{
		moment = G3D::Matrix3::identity();
	}
	// mac: 0x525A10 (batch293).
	// STUB: PLAYER 0x0099a650
	// STUB: STUDIO 0x00a39960
	void Cofm::updateIfDirty()
	{
		if (dirty) {
			mass = body->getMass();
			if (body->numChildren() == 0) {
				cofmInBody = body->getCofmOffset();
				moment = body->getIBodyAtPoint(cofmInBody);
				RBXASSERT(Math::fuzzyEq(moment,
							Math::momentToObjectSpace(body->getIWorldAtPoint(body->getCoordinateFrame().pointToWorldSpace(cofmInBody)),
								body->getCoordinateFrame().rotation),
							moment.l1Norm() * 0.01));
			}
			else {
				G3D::Vector3 bodyCofmOffset = body->getCofmOffset();
				G3D::Vector3 bodyCofmInWorld = body->getCoordinateFrame().pointToWorldSpace(bodyCofmOffset);

				G3D::Vector3 cofmWorld = bodyCofmInWorld * body->getMass();
				for (int i = 0; i < body->numChildren(); ++i) {
					Body* b = body->getTypedChild<Body>(i);
					mass += b->getBranchMass();
					cofmWorld += b->getBranchCofmPos() * b->getBranchMass();
				}
				cofmWorld = cofmWorld / mass;
				cofmInBody = body->getCoordinateFrame().pointToObjectSpace(cofmWorld);

				G3D::Matrix3 iWorldSum = body->getIWorldAtPoint(cofmWorld);
				for (int i = 0; i < body->numChildren(); ++i) {
					iWorldSum = iWorldSum + body->getTypedChild<Body>(i)->getBranchIWorldAtPoint(cofmWorld);
				}
				moment = Math::momentToObjectSpace(iWorldSum, body->getCoordinateFrame().rotation);
			}

			RBXASSERT(dirty);		// concurrency issues?
			dirty = false;
		}
	}

} // namespace RBX

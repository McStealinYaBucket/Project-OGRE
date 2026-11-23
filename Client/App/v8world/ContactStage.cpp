/* App/v8world/ContactStage.cpp -- October 2013 (contact stage node bodies).
   Bodies: C2 0x56F48C / D1 0x56F860 / D0 0x56F88E / onPrimitiveAdded
     0x56F4F6 / onPrimitiveRemoving 0x56F526 / onEdgeAdded 0x56F556 /
     onEdgeRemoving 0x56F6DE (batches 208/213/214).
   getKernel: inherited IStage::getKernel (no override -- binary slot +3
     holds the base body for every stage). */

#include "stdafx.h"

#include "V8World/ContactStage.h"
#include "V8World/TreeStage.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/Contact.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x56F48C (105B, batch213).
	ContactStage::ContactStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new TreeStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	// D0 0x009aa810 (SYNTHETIC in ContactStage.h)
	//
	ContactStage::~ContactStage()
	{
		// D1 0x56F860 / D0 0x56F88E (batch214).
		// downstream: deleted by ~IStage.
	}




	// FUNCTION: PLAYER 0x009aa880
	// FUNCTION: STUDIO 0x00a618d0
	void ContactStage::onEdgeAdded(Edge* e)
	{
		RBXASSERT(!e->inKernel());
		RBXASSERT(!e->inOrDownstreamOfStage(this));
		e->putInStage(this);
		if (e->getEdgeType() == Edge::CONTACT)
		{
			Primitive* p0 = e->getPrim0(); Primitive* p1 = e->getPrim1();
			Joint* joint = Primitive::getJoint(p0, p1, 0);
			if (joint == NULL || !joint->downstreamOfStage(this))
			{
				getDownstreamWS()->onEdgeAdded(e);
				return;
			}
			return;
		}
		if (!Joint::isGroundJoint(e))
		{
			Contact* contact = Primitive::getContact(e->getPrim0(), e->getPrim1());
			if (contact != NULL && contact->downstreamOfStage(this)) getDownstreamWS()->onEdgeRemoving(contact);
		}
		getDownstreamWS()->onEdgeAdded(e);


	}
	// STUB: PLAYER 0x009aa9d0
	// STUB: STUDIO 0x00a61a20
	void ContactStage::onEdgeRemoving(Edge* e)
	{
		if (e->downstreamOfStage(this))
			getDownstreamWS()->onEdgeRemoving(e);

		RBXASSERT(e->inStage(this));

		if (e->getEdgeType() != Edge::CONTACT)
		{
			int jointType = 13;
			if (e->getEdgeType() == Edge::JOINT)
				jointType = rbx_static_cast<Joint*>(e)->getJointType();
			if (e->getEdgeType() != Edge::JOINT
				|| (jointType != Joint::ANCHOR_JOINT && jointType != Joint::FREE_JOINT))
			{
				Contact* contact = Primitive::getContact(e->getPrim0(), e->getPrim1());
				if (contact != NULL && contact->inStage(this))
					getDownstreamWS()->onEdgeAdded(contact);
			}
		}







		e->removeFromStage(this);
		RBXASSERT(!e->inOrDownstreamOfStage(this));
	}

	// mac: 0x56F4F6 (48B, batch213).
	// FUNCTION: PLAYER 0x009aa840
	// FUNCTION: STUDIO 0x00a61890
	void ContactStage::onPrimitiveAdded(Primitive* prim)
	{
		prim->putInStage(this);
		static_cast<TreeStage*>(downstream)->onPrimitiveAdded(prim);
	}

	// mac: 0x56F526 (48B, batch214).
	// FUNCTION: PLAYER 0x009aa860
	// FUNCTION: STUDIO 0x00a618b0
	void ContactStage::onPrimitiveRemoving(Primitive* prim)
	{
		static_cast<TreeStage*>(downstream)->onPrimitiveRemoving(prim);
		prim->removeFromStage(this);
	}

} // namespace RBX

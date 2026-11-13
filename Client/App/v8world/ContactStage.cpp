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

	// STUB: PLAYER 0x009aa810
	// STUB: STUDIO 0x00a61860
	ContactStage::~ContactStage()
	{
		// D1 0x56F860 / D0 0x56F88E (batch214).
		delete static_cast<TreeStage*>(downstream);
	}

	// mac: 0x56F556 (392B, batch214). Disasm setups win: the CONTACT branch
	// calls getJoint(p0, p1, 0) -- our member getJoint(p1, n) (same EA
	// mac: 0x590916; the "extra" decompiler args are phantom stack slots,
	// verified at 0x56F643-0x56F658); getContact is the 2-word static
	// (0x56F690-0x56F69D). Joint-found-but-not-downstream forwards;
	// downstream joints are swallowed.
	// STUB: PLAYER 0x009aa880
	// STUB: STUDIO 0x00a618d0
	void ContactStage::onEdgeAdded(Edge* edge)
	{
		RBXASSERT(!edge->inStage(IStage::KERNEL_STAGE));  // :42
		RBXASSERT(!edge->inOrDownstreamOfStage(this));  // :43
		edge->putInStage(this);
		if (edge->getEdgeType() == Edge::CONTACT)
		{
			Primitive* p0 = edge->getPrim0();
			Primitive* p1 = edge->getPrim1();
			Joint* joint = p0->getJoint(p1, 0);
			if (joint == NULL || !joint->downstreamOfStage(this))
			{
				getDownstreamWS()->onEdgeAdded(edge);
				return;
			}
			return;
		}
		int jointType = 13;
		if (edge->getEdgeType() == Edge::JOINT)
			jointType = rbx_static_cast<Joint*>(edge)->getJointType();
		if (edge->getEdgeType() != Edge::JOINT
			|| (jointType != Joint::ANCHOR_JOINT && jointType != Joint::FREE_JOINT))
		{
			Contact* contact = Primitive::getContact(edge->getPrim0(), edge->getPrim1());
			if (contact != NULL && contact->downstreamOfStage(this))
				getDownstreamWS()->onEdgeRemoving(contact);
		}
		getDownstreamWS()->onEdgeAdded(edge);
	}

	// mac: 0x56F6DE (381B, batch214).
	// STUB: PLAYER 0x009aa9d0
	// STUB: STUDIO 0x00a61a20
	void ContactStage::onEdgeRemoving(Edge* edge)
	{

		if (edge->downstreamOfStage(this))
			getDownstreamWS()->onEdgeRemoving(edge);
		RBXASSERT(edge->inStage(this));  // :72
		if (edge->getEdgeType() != Edge::CONTACT)
		{
			int jointType = 13;
			if (edge->getEdgeType() == Edge::JOINT)
				jointType = rbx_static_cast<Joint*>(edge)->getJointType();
			if (edge->getEdgeType() != Edge::JOINT
				|| (jointType != Joint::ANCHOR_JOINT && jointType != Joint::FREE_JOINT))
			{
				Contact* contact = Primitive::getContact(edge->getPrim0(), edge->getPrim1());
				if (contact != NULL && contact->inStage(this))
					getDownstreamWS()->onEdgeAdded(contact);
			}
		}
		edge->removeFromStage(this);
		RBXASSERT(!edge->inOrDownstreamOfStage(this));  // :95
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

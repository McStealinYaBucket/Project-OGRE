/* App/v8world/GroundStage.cpp -- October 2013 (ground-joint stage node; mac batches 208-225).
   getKernel: inherited IStage::getKernel. */
#include "stdafx.h"

#include "V8World/GroundStage.h"
#include "V8World/EdgeStage.h"
#include "V8World/KernelJoint.h"
#include "V8World/RigidJoint.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x574E3C (105B, batch213).
	GroundStage::GroundStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new EdgeStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	// STUB: PLAYER 0x008eaa70
	// STUB: STUDIO 0x00a08c70
	GroundStage::~GroundStage()
	{
		// D2 pattern (uniform 0x10 node: delete +8; D1 0x574EF0 / D0
		// mac: 0x574EA6 per the batch225 vtable -- bodies uncaptured, the delete
		// is certain, members: none).
		delete static_cast<EdgeStage*>(downstream);
	}

	// mac: 0x574F1E (190B, batch214).
	// STUB: PLAYER 0x008eadf0
	// STUB: STUDIO 0x00a08ff0
	void GroundStage::onPrimitiveAdded(Primitive* p)
	{
		RBXASSERT(p->getNumEdges() == 0);
		p->putInStage(this);
		static_cast<EdgeStage*>(downstream)->onPrimitiveAdded(p);
		addGroundJoint(p, p->getAnchoredProperty() || p->getDragging());
	}


	// mac: 0x575118 (293B, batch214).
	// STUB: PLAYER 0x008eae90
	// STUB: STUDIO 0x00a09090
	void GroundStage::onPrimitiveRemoving(Primitive* p)
	{
		RBXASSERT(p->getNumEdges() == 1);
		removeGroundJoint(p, p->getAnchoredProperty() || p->getDragging());
		static_cast<EdgeStage*>(downstream)->onPrimitiveRemoving(p);
		p->removeFromStage(this);


		RBXASSERT(p->getNumEdges() == 0);
	}




























	// mac: 0x574FDC (315B, batch217).
	// FUNCTION: PLAYER 0x008eaaa0
	// FUNCTION: STUDIO 0x00a08ca0
	void GroundStage::addGroundJoint(Primitive* p, bool anchored)
	{
		RBXASSERT(!Joint::findConstJoint(p, Joint::ANCHOR_JOINT));
		RBXASSERT(!Joint::findConstJoint(p, Joint::FREE_JOINT));
		Joint* joint;
		if (anchored)
			joint = new AnchorJoint(p);
		else
			joint = new FreeJoint(p);
		joint->putInPipeline(this);
		getDownstreamWS()->onEdgeAdded(joint);
	}










	// mac: 0x57523E (424B, batch217).
	// FUNCTION: PLAYER 0x008eabd0
	// FUNCTION: STUDIO 0x00a08dd0
	void GroundStage::removeGroundJoint(Primitive* p, bool anchored)
	{
		Joint* j = Joint::getJoint(p, anchored ? Joint::ANCHOR_JOINT : Joint::FREE_JOINT);
		RBXASSERT(j);
		if (j != NULL)
		{
			getDownstreamWS()->onEdgeRemoving(j);
			j->removeFromPipeline(this);
			j->setPrimitive(0, NULL);
			j->setPrimitive(1, NULL);
			delete j;
		}

		RBXASSERT(!Joint::findConstJoint(p, Joint::ANCHOR_JOINT));
		RBXASSERT(!Joint::findConstJoint(p, Joint::FREE_JOINT));
	}











































	// mac: 0x575680 (293B, batch219a): asserts only (:178/:179/:180).
	// FUNCTION: PLAYER 0x008ea7a0
	// FUNCTION: STUDIO 0x00a089a0
	void GroundStage::onKernelJointAdded(KernelJoint* k)
	{
		RBXASSERT(k->getPrimitive(0));
		RBXASSERT(k->getPrimitive(1));
		RBXASSERT(k->getPrimitive(0)->downstreamOfStage(this));
	}
	// FUNCTION: PLAYER 0x008ea890
	// FUNCTION: STUDIO 0x00a08a90
	void GroundStage::onKernelJointRemoving(KernelJoint* k)
	{
		RBXASSERT(k->getPrimitive(0));
		RBXASSERT(k->getPrimitive(1));
		RBXASSERT(k->getPrimitive(0)->downstreamOfStage(this));
	}
	// STUB: PLAYER 0x008eafb0
	// STUB: STUDIO 0x00a091b0
	void GroundStage::checkForFreeGroundJoint(RigidJoint* joint)
	{
		Primitive* p0 = joint->getPrim0();
		Primitive* p1 = joint->getPrim1();
		RBXASSERT(p0->downstreamOfStage(this));  // :196
		RBXASSERT(p1->downstreamOfStage(this));  // :197
		bool r0 = p0->getAnchoredProperty() || p0->getDragging();
		if (p1->getAnchoredProperty())
		{
			if (r0)
				return;
		}
		else
		{
			if ((r0 ^ p1->getDragging()) != 1)
				return;
			if (r0)
			{
				rebuildFreeGround(p1);
				return;
			}
		}
		rebuildFreeGround(p0);
	}
























	// mac: 0x575492 (291B, batch220).
	// STUB: PLAYER 0x008ead20
	// STUB: STUDIO 0x00a08f20
	void GroundStage::rebuildFreeGround(Primitive* p)
	{
		if (p->getAnchoredProperty() || p->getDragging())
		{
			RBXASSERT(Joint::findConstJoint(p, Joint::ANCHOR_JOINT));
		}
		else if (heaviestRigidToGround(p) != NULL)
		{
			if (Joint::findConstJoint(p, Joint::ANCHOR_JOINT) == NULL)
			{
				removeGroundJoint(p, false);
				addGroundJoint(p, true);
			}
		}
		else if (Joint::findConstJoint(p, Joint::FREE_JOINT) == NULL)
		{
			removeGroundJoint(p, true);
			addGroundJoint(p, false);
		}
	}
	// mac: 0x575A7E (217B, batch222).
	// STUB: PLAYER 0x008ea980
	// STUB: STUDIO 0x00a08b80
	RigidJoint* GroundStage::heaviestRigidToGround(Primitive* p)
	{
		RBXASSERT(!p->requestFixed());
		RigidJoint* best = NULL;
		for (RigidJoint* joint = p->getFirstRigid(); joint != NULL;
			joint = p->getNextRigid(joint))
		{
			Primitive* other = joint->getPrim0() == p ? joint->getPrim1() : joint->getPrim0();
			if (!(other->getAnchoredProperty() || other->getDragging()))
				continue;
			if (best == NULL
				|| static_cast<SpanningEdge*>(joint)->isHeavierThan(
					static_cast<SpanningEdge*>(best)))
				best = joint;
		}
		return best;
	}

	// mac: 0x575604 (123B, batch209).
	// STUB: PLAYER 0x008eb150
	// STUB: STUDIO 0x00a09350
	void GroundStage::onEdgeAdded(Edge* edge)
	{
		if (edge->getEdgeType() == Edge::JOINT
			&& rbx_static_cast<Joint*>(edge)->getJointType() == Joint::KERNEL_JOINT)
			onKernelJointAdded(static_cast<KernelJoint*>(edge));
		IWorldStage::onEdgeAdded(edge);
		int type = 13;
		if (edge->getEdgeType() == Edge::JOINT)
			type = rbx_static_cast<Joint*>(edge)->getJointType();
		if ((type & ~4) == 1 || type == 2)
			checkForFreeGroundJoint(static_cast<RigidJoint*>(edge));
	}

	// mac: 0x5758DC (123B, batch209).
	// STUB: PLAYER 0x008eb1c0
	// STUB: STUDIO 0x00a093c0
	void GroundStage::onEdgeRemoving(Edge* edge)
	{

		IWorldStage::onEdgeRemoving(edge);
		int type = 13;
		if (edge->getEdgeType() == Edge::JOINT)
			type = rbx_static_cast<Joint*>(edge)->getJointType();
		if ((type & ~4) == 1 || type == 2)
			checkForFreeGroundJoint(static_cast<RigidJoint*>(edge));
		if (edge->getEdgeType() == Edge::JOINT
			&& rbx_static_cast<Joint*>(edge)->getJointType() == Joint::KERNEL_JOINT)
			onKernelJointRemoving(static_cast<KernelJoint*>(edge));
	}

} // namespace RBX

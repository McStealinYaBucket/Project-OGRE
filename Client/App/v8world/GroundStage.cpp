/* App/v8world/GroundStage.cpp -- October 2013 (ground-joint stage node).
   Bodies: C2 0x574E3C / D2 (pattern: delete +8; D1 0x574EF0 / D0 0x574EA6
     per batch225 vtable) / onPrimitiveAdded 0x574F1E / onPrimitiveRemoving
     0x575118 / addGroundJoint 0x574FDC / removeGroundJoint 0x57523E /
     onKernelJointAdded 0x575680 / onKernelJointRemoving 0x575958
     (batches 208/213/214/217/219a/225).
   onEdgeAdded 0x575604 / onEdgeRemoving 0x5758DC (123B each, batch209 --
   kernel-joint arms + base tails + rigid/ground sustain) + rebuild chain
   (checkForFreeGroundJoint 0x5757A6 / rebuildFreeGround 0x575492 /
   heaviestRigidToGround 0x575A7E -- the Joint+32 compare spells
   static_cast since the section-130 MI fix).
   getKernel: inherited IStage::getKernel (no override -- binary slot +3
     holds the base body for every stage). */

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
	void GroundStage::onPrimitiveAdded(Primitive* prim)
	{
		RBXASSERT(prim->getNumEdges() == 0);  // :38
		prim->putInStage(this);
		static_cast<EdgeStage*>(downstream)->onPrimitiveAdded(prim);
		addGroundJoint(prim, prim->getAnchoredProperty() || prim->getDragging());
	}

	// mac: 0x575118 (293B, batch214).
	// STUB: PLAYER 0x008eae90
	// STUB: STUDIO 0x00a09090
	void GroundStage::onPrimitiveRemoving(Primitive* prim)
	{
		RBXASSERT(prim->getNumEdges() == 1);  // :50
		removeGroundJoint(prim, prim->getAnchoredProperty() || prim->getDragging());
		static_cast<EdgeStage*>(downstream)->onPrimitiveRemoving(prim);
		prim->removeFromStage(this);
		RBXASSERT(prim->getNumEdges() == 0);  // :56
	}

	// mac: 0x574FDC (315B, batch217).
	// STUB: PLAYER 0x008eaaa0
	// STUB: STUDIO 0x00a08ca0
	void GroundStage::addGroundJoint(Primitive* prim, bool anchored)
	{
		RBXASSERT(Joint::findConstJoint(prim, ANCHOR_JOINT) == NULL);  // :91
		RBXASSERT(Joint::findConstJoint(prim, FREE_JOINT) == NULL);  // :92
		Joint* joint;
		if (anchored)
			joint = new AnchorJoint(prim);
		else
			joint = new FreeJoint(prim);
		joint->putInPipeline(this);
		getDownstreamWS()->onEdgeAdded(joint);
	}

	// mac: 0x57523E (424B, batch217).
	// STUB: PLAYER 0x008eabd0
	// STUB: STUDIO 0x00a08dd0
	void GroundStage::removeGroundJoint(Primitive* prim, bool anchored)
	{
		Joint* joint = Joint::getJoint(prim, anchored ? ANCHOR_JOINT : FREE_JOINT);
		RBXASSERT(joint != NULL);  // :117 "j"
		if (joint != NULL)
		{
			getDownstreamWS()->onEdgeRemoving(joint);
			joint->removeFromPipeline(this);
			joint->setPrimitive(0, NULL);
			joint->setPrimitive(1, NULL);
			delete joint;
		}
		RBXASSERT(Joint::findConstJoint(prim, ANCHOR_JOINT) == NULL);  // :127
		RBXASSERT(Joint::findConstJoint(prim, FREE_JOINT) == NULL);  // :128
	}

	// mac: 0x575680 (293B, batch219a): asserts only (:178/:179/:180).
	// STUB: PLAYER 0x008ea7a0
	// STUB: STUDIO 0x00a089a0
	void GroundStage::onKernelJointAdded(KernelJoint* joint)
	{
		RBXASSERT(joint->getPrim0() != NULL);  // :178
		RBXASSERT(joint->getPrim1() != NULL);  // :179
		RBXASSERT(joint->getPrim0()->downstreamOfStage(this));  // :180
	}

	// mac: 0x575958 (293B, batch219a): asserts only (:186/:187/:188).
	// STUB: PLAYER 0x008ea890
	// STUB: STUDIO 0x00a08a90
	void GroundStage::onKernelJointRemoving(KernelJoint* joint)
	{
		RBXASSERT(joint->getPrim0() != NULL);  // :186
		RBXASSERT(joint->getPrim1() != NULL);  // :187
		RBXASSERT(joint->getPrim0()->downstreamOfStage(this));  // :188
	}

	// mac: 0x575604 (123B, batch209).
	// STUB: PLAYER 0x008eb150
	// STUB: STUDIO 0x00a09350
	void GroundStage::onEdgeAdded(Edge* edge)
	{
		if (edge->getEdgeType() == Edge::JOINT
			&& rbx_static_cast<Joint*>(edge)->getJointType() == KERNEL_JOINT)
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
			&& rbx_static_cast<Joint*>(edge)->getJointType() == KERNEL_JOINT)
			onKernelJointRemoving(static_cast<KernelJoint*>(edge));
	}

	// mac: 0x5757A6 (309B, batch219a).
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
	void GroundStage::rebuildFreeGround(Primitive* prim)
	{
		if (prim->getAnchoredProperty() || prim->getDragging())
		{
			RBXASSERT(Joint::findConstJoint(prim, ANCHOR_JOINT) != NULL);  // :247
		}
		else if (heaviestRigidToGround(prim) != NULL)
		{
			if (Joint::findConstJoint(prim, ANCHOR_JOINT) == NULL)
			{
				removeGroundJoint(prim, false);
				addGroundJoint(prim, true);
			}
		}
		else if (Joint::findConstJoint(prim, FREE_JOINT) == NULL)
		{
			removeGroundJoint(prim, true);
			addGroundJoint(prim, false);
		}
	}

	// mac: 0x575A7E (217B, batch222).
	// STUB: PLAYER 0x008ea980
	// STUB: STUDIO 0x00a08b80
	RigidJoint* GroundStage::heaviestRigidToGround(Primitive* prim)
	{
		RBXASSERT(!(prim->getAnchoredProperty() || prim->getDragging()));  // :254
		RigidJoint* best = NULL;
		for (RigidJoint* joint = prim->getFirstRigid(); joint != NULL;
			joint = prim->getNextRigid(joint))
		{
			Primitive* other = joint->getPrim0() == prim ? joint->getPrim1() : joint->getPrim0();
			if (!(other->getAnchoredProperty() || other->getDragging()))
				continue;
			if (best == NULL
				|| static_cast<SpanningEdge*>(joint)->isHeavierThan(
					static_cast<SpanningEdge*>(best)))
				best = joint;
		}
		return best;
	}

} // namespace RBX

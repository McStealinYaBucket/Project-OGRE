/* App/v8world/MovingAssemblyStage.cpp -- October 2013 (moving-assembly bodies).
   Bodies: onEdgeAdded 0x587026 / onEdgeRemoving 0x587086 (batch221b) /
     removeJoint 0x586EE6 (batch222). C2/D2/getStageType: C2 news 0x20
     StepJointsStage (batch220); D2 in batch223 (:24 assert + set-erase +
     link walk + delete +8) -- C2/D2 DEFERRED (need StepJointsStage members
     read + StepJointsStage concrete for `new`; header declares them).
   Hook mechanics DELTA (documented in header): binary threads edges through
   typeless link addrs (edge+44/+48); ours uses the typed SpanningEdge hook
   links with NULL-unlinked convention -- same membership and order.
   getKernel: inherited IStage::getKernel. */

#include "stdafx.h"
#include "decomp.h"

#include "V8World/MovingAssemblyStage.h"
#include "V8World/StepJointsStage.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/Assembly.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x586C98 (165B, batch220).
	// STUB: PLAYER 0x008eb610
	// STUB: STUDIO 0x00a09810
	MovingAssemblyStage::MovingAssemblyStage(IStage* upstreamArg, World* worldArg)
		: numJoints(0)
		, listHead(NULL)
		, listTail(NULL)
	{
		downstream = new StepJointsStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	MovingAssemblyStage::~MovingAssemblyStage()
	{
		// D2 (batch223): :24 uiStepJoints.empty + set-erase + link walk +
		// delete +8. Members clean themselves; links nulled by walk.
		RBXASSERT(uiStepJoints.empty());  // :24
		uiStepJoints.clear();
		for (Joint* joint = listHead; joint != NULL; )
		{
			Joint* next = static_cast<Joint*>(joint->moveNext);
			joint->moveNext = NULL;
			joint->movePrev = NULL;
			joint = next;
		}
		listHead = NULL;
		listTail = NULL;
		numJoints = 0;
		delete static_cast<StepJointsStage*>(downstream);
	}

	// mac: 0x586EC0 (batch278: erase from the +28 set, return count).
	size_t MovingAssemblyStage::removeMovingGroundedAssembly(Assembly* assembly)
	{
		return uiStepJoints.erase(assembly);
	}

	// mac: 0x587026 (95B, batch221b).
	// STUB: PLAYER 0x008eb250
	// STUB: STUDIO 0x00a09450
	void MovingAssemblyStage::onEdgeAdded(Edge* edge)
	{
		edge->putInStage(this);
		if (edge->getEdgeType() == Edge::JOINT
			&& rbx_static_cast<Joint*>(edge)->canStepUi())
		{
			Joint* joint = rbx_static_cast<Joint*>(edge);
			joint->moveNext = listHead;
			joint->movePrev = NULL;
			if (listHead != NULL)
				listHead->movePrev = joint;
			else
				listTail = joint;
			listHead = joint;
			++numJoints;
		}
		getDownstreamWS()->onEdgeAdded(edge);
	}

	// mac: 0x587086 (88B, batch221b).
	// STUB: PLAYER 0x008eb4d0
	// STUB: STUDIO 0x00a096d0
	void MovingAssemblyStage::onEdgeRemoving(Edge* edge)
	{
		if (edge->downstreamOfStage(this))
			getDownstreamWS()->onEdgeRemoving(edge);
		if (edge->getEdgeType() == Edge::JOINT)
			removeJoint(rbx_static_cast<Joint*>(edge));
		edge->removeFromStage(this);
	}

	// mac: 0x586EE6 (201B, batch222).
	// STUB: PLAYER 0x008eb3f0
	// STUB: STUDIO 0x00a095f0
	void MovingAssemblyStage::removeJoint(Joint* joint)
	{
		if (!joint->canStepUi())
		{
			// :51 -- unlinked hook state (both links NULL in our model).
			RBXASSERT(joint->moveNext == NULL && joint->movePrev == NULL);
			return;
		}
		Joint* next = static_cast<Joint*>(joint->moveNext);
		if (next != NULL)
			next->movePrev = joint->movePrev;
		else
			listTail = static_cast<Joint*>(joint->movePrev);
		if (joint->movePrev != NULL)
			static_cast<Joint*>(joint->movePrev)->moveNext = next;
		else
			listHead = next;
		joint->moveNext = NULL;
		joint->movePrev = NULL;
		--numJoints;
		Assembly* assembly = joint->getPrim0()->getAssembly();
		if (assembly != NULL)
			uiStepJoints.erase(assembly);
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::MovingAssemblyStage, 0x28);

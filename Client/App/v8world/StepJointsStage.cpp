/* App/v8world/StepJointsStage.cpp -- October 2013 (step-joints bodies).
   Bodies here: D2 (pattern: delete +8; D1 0x5A41A8 per batch232 sweep) /
     onEdgeAdded 0x5A440C / onEdgeRemoving 0x5A4486 / removeJoint 0x5A4302
     (batches 208/228/232).
   DEFERRED: C2 (needs HumanoidStage members, 0x180 -- batch220 body banked,
   mapping queued). getKernel: inherited IStage::getKernel.
   Hook mechanics DELTA (documented in header): binary threads edges through
   typeless link addrs (edge+36/+40); ours uses the typed SpanningEdge hook
   links with NULL-unlinked convention -- same membership and order. */

#include "stdafx.h"

#include "V8World/StepJointsStage.h"
#include "V8World/HumanoidStage.h"
#include "V8World/Primitive.h"
#include "V8World/Assembly.h"
#include "V8World/Joint.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x5A4070 (batch220, uniform node).
	// STUB: PLAYER 0x008ef410
	// STUB: STUDIO 0x00a0d190
	StepJointsStage::StepJointsStage(IStage* upstreamArg, World* worldArg)
		: numJoints(0)
		, listHead(NULL)
		, listTail(NULL)
		, profilingJointUpdate(new Profiling::CodeProfiler("Joint Update"))
	{
		downstream = new HumanoidStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	StepJointsStage::~StepJointsStage()
	{
		// 2013 D1 0x008ef340 asserts worldStepJoints.empty() (:26), deletes the profiler and clears the
		// boost::intrusive list before ~IStage; ours keeps a raw list, so only the emptiness check stays.
		RBXASSERT(numJoints == 0);
	}

	// mac: 0x5A440C (121B, batch228).
	// STUB: PLAYER 0x008ef1a0
	// STUB: STUDIO 0x00a0cf20
	void StepJointsStage::onEdgeAdded(Edge* edge)
	{
		edge->putInStage(this);
		if (edge->getEdgeType() == Edge::JOINT
			&& rbx_static_cast<Joint*>(edge)->canStepWorld())
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
		if (edge->getEdgeType() != Edge::JOINT)
		{
			getDownstreamWS()->onEdgeAdded(edge);
			return;
		}
		if ((rbx_static_cast<Joint*>(edge)->getJointType() - 1) >= 5)
		{
			getDownstreamWS()->onEdgeAdded(edge);
			return;
		}
	}

	// mac: 0x5A4486 (98B, batch228).
	// FUNCTION: PLAYER 0x008ef210
	// FUNCTION: STUDIO 0x00a0cf90
	void StepJointsStage::onEdgeRemoving(Edge* edge)
	{
		if (!Joint::isKinematicJoint(edge))
		{
			getDownstreamWS()->onEdgeRemoving(edge);
		}

		if (edge->getEdgeType() == Edge::JOINT)
			removeJoint(rbx_static_cast<Joint*>(edge));
		edge->removeFromStage(this);
	}

	// mac: 0x5A4302 (169B, batch232).
	// STUB: PLAYER 0x008ef110
	// STUB: STUDIO 0x00a0ce90
	void StepJointsStage::removeJoint(Joint* joint)
	{
		if (!joint->canStepWorld())
		{
			// :43 -- unlinked hook state (both links NULL in our model).
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
	}

	// FUNCTION: PLAYER 0x008ef0d0
	// FUNCTION: STUDIO 0x00a0ce50
	void StepJointsStage::onSimulateAssemblyAdded(Assembly* a)
	{
		a->putInStage(this);

		rbx_static_cast<HumanoidStage*>(getDownstreamWS())->onAssemblyAdded(a);
	}

	// FUNCTION: PLAYER 0x008ef0f0
	// FUNCTION: STUDIO 0x00a0ce70
	void StepJointsStage::onSimulateAssemblyRemoving(Assembly* a)
	{
		rbx_static_cast<HumanoidStage*>(getDownstreamWS())->onAssemblyRemoving(a);

		a->removeFromStage(this);
	}

} // namespace RBX

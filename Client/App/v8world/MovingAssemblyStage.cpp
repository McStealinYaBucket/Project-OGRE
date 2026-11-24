/* App/v8world/MovingAssemblyStage.cpp -- October 2013, from the 2013 bytes; asserts on their 2013 lines (:24, :51, :103). */

#include "stdafx.h"
#include "decomp.h"
#include "V8World/MovingAssemblyStage.h"
#include "V8World/StepJointsStage.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/Assembly.h"
#include "V8World/World.h"
#include "V8World/Mechanism.h"

namespace RBX {
	// FUNCTION: PLAYER 0x008eb610
	// FUNCTION: STUDIO 0x00a09810
	MovingAssemblyStage::MovingAssemblyStage(IStage* upstream, World* world)
		: IWorldStage(upstream, new StepJointsStage(this, world), world)
	{}

	// FUNCTION: PLAYER 0x008eb520
	// FUNCTION: STUDIO 0x00a09720
	MovingAssemblyStage::~MovingAssemblyStage()
	{
		RBXASSERT(uiStepJoints.empty());
	}

	// FUNCTION: PLAYER 0x008eb3e0
	// FUNCTION: STUDIO 0x00a095e0
	size_t MovingAssemblyStage::removeMovingGroundedAssembly(Assembly* assembly)
	{
		return movingGroundedAssemblies.erase(assembly);
	}









	// FUNCTION: PLAYER 0x008eb3f0
	// FUNCTION: STUDIO 0x00a095f0
	void MovingAssemblyStage::removeJoint(Joint* j)
	{
		if (j->canStepUi()) {
			uiStepJoints.erase(uiStepJoints.iterator_to(*j));
			removeMovingGroundedAssembly(j->getPrimitive(0)->getAssembly());
		}
		else {
			RBXASSERT(!j->MovingAssemblyStageHook::is_linked());
		}
	}

	// FUNCTION: PLAYER 0x008eb230
	// FUNCTION: STUDIO 0x00a09430
	void MovingAssemblyStage::onSimulateAssemblyAdded(Assembly* a)
	{
		a->putInStage(this);
		rbx_static_cast<StepJointsStage*>(getDownstreamWS())->onSimulateAssemblyAdded(a);
	}

	// FUNCTION: PLAYER 0x008eb4a0
	// FUNCTION: STUDIO 0x00a096a0
	void MovingAssemblyStage::onSimulateAssemblyRemoving(Assembly* a)
	{
		rbx_static_cast<StepJointsStage*>(getDownstreamWS())->onSimulateAssemblyRemoving(a);
		a->removeFromStage(this);

		removeMovingGroundedAssembly(a);
	}
	// FUNCTION: PLAYER 0x008eb250
	// FUNCTION: STUDIO 0x00a09450
	void MovingAssemblyStage::onEdgeAdded(Edge* e)
	{
		e->putInStage(this);

		if (Joint::isJoint(e)) {
			Joint* j = rbx_static_cast<Joint*>(e);
			if (j->canStepUi()) {
				uiStepJoints.push_back(*j);
			}
		}

		getDownstreamWS()->onEdgeAdded(e);
	}
	// FUNCTION: PLAYER 0x008eb4d0
	// FUNCTION: STUDIO 0x00a096d0
	void MovingAssemblyStage::onEdgeRemoving(Edge* e)
	{
		if (e->downstreamOfStage(this))
			getDownstreamWS()->onEdgeRemoving(e);
		if (e->getEdgeType() == Edge::JOINT)
			removeJoint(rbx_static_cast<Joint*>(e));
		e->removeFromStage(this);
	}
	// STUB: PLAYER 0x008eb2a0
	// STUB: STUDIO 0x00a094a0
	void MovingAssemblyStage::jointsStepUi(double distributedGameTime)
	{
		for (Joints::iterator it = uiStepJoints.begin(); it != uiStepJoints.end(); ++it)
		{
			RBXASSERT(it->canStepUi());
			Assembly* a0 = NULL;
			bool a0IsGrounded = false;
			if (Joint::isKinematicJoint(&*it)) {
				a0 = it->getPrimitive(0)->getAssembly();

				const Assembly* root = Mechanism::getConstMovingAssemblyRoot(a0);
				if (root->getFilterPhase() == Assembly::Sim_BufferZone)
					continue;

				if (a0->computeIsGrounded())
					a0IsGrounded = true;
			}

			if (it->stepUi(distributedGameTime) && a0IsGrounded)
				movingGroundedAssemblies.insert(a0);
		}
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::MovingAssemblyStage, 0x28);

/* App/v8world/StepJointsStage.cpp -- October 2013 (step-joints stage, 0x20), from the 2013 bytes.
   worldStepJoints is a boost::intrusive list over Joint's StepJointsStageHook (Joint+0x20).
   Asserts on their 2013 lines: ~StepJointsStage :26, removeJoint :43, jointsStepWorld :104
   (C:\BuildAgent\work\<hash>\Client\App\v8world\StepJointsStage.cpp). VC9 emits the ctor and dtor
   after jointsStepWorld (0x008ef340/0x008ef410), so the descent below is the 2013 one. */

#include "stdafx.h"

#include "V8World/StepJointsStage.h"
#include "V8World/HumanoidStage.h"
#include "V8World/Primitive.h"
#include "V8World/Assembly.h"
#include "V8World/Joint.h"
#include "V8World/World.h"
namespace RBX {
	// FUNCTION: PLAYER 0x008ef410
	// FUNCTION: STUDIO 0x00a0d190
	StepJointsStage::StepJointsStage(IStage* upstream, World* world)
		: IWorldStage(upstream, new HumanoidStage(this, world), world)
		, profilingJointUpdate(new Profiling::CodeProfiler("Joint Update"))
	{}
	// FUNCTION: PLAYER 0x008ef340
	// FUNCTION: STUDIO 0x00a0d0c0
	StepJointsStage::~StepJointsStage()
	{
		RBXASSERT(worldStepJoints.empty());
	}







	// FUNCTION: PLAYER 0x008ef110
	// FUNCTION: STUDIO 0x00a0ce90
	void StepJointsStage::removeJoint(Joint* j)
	{
		if (j->canStepWorld()) {
			worldStepJoints.erase(worldStepJoints.iterator_to(*j));
		}
		else {
			RBXASSERT(!j->StepJointsStageHook::is_linked());
		}
	}

	// FUNCTION: PLAYER 0x008ef1a0
	// FUNCTION: STUDIO 0x00a0cf20
	void StepJointsStage::onEdgeAdded(Edge* e)
	{
		e->putInStage(this);

		if (Joint::isJoint(e)) {
			Joint* j = rbx_static_cast<Joint*>(e);
			if (j->canStepWorld()) {
				worldStepJoints.push_back(*j);
			}
		}

		if (!Joint::isKinematicJoint(e)) {
			getDownstreamWS()->onEdgeAdded(e);
		}
	}

	// FUNCTION: PLAYER 0x008ef210
	// FUNCTION: STUDIO 0x00a0cf90
	void StepJointsStage::onEdgeRemoving(Edge* e)
	{
		if (!Joint::isKinematicJoint(e)) {
			getDownstreamWS()->onEdgeRemoving(e);
		}

		if (Joint::isJoint(e)) {
			removeJoint(rbx_static_cast<Joint*>(e));
		}
		e->removeFromStage(this);
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

	// FUNCTION: PLAYER 0x008ef270
	// FUNCTION: STUDIO 0x00a0cff0
	void StepJointsStage::jointsStepWorld()
	{
		Profiling::Mark mark(*profilingJointUpdate, false);
		for (Joints::iterator it = worldStepJoints.begin(); it != worldStepJoints.end(); ++it)
		{
			RBXASSERT(it->canStepWorld());
			it->stepWorld();
		}
	}

} // namespace RBX

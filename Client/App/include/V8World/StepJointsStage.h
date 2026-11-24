/* V8World/StepJointsStage.h -- October 2013 (step-joints stage node, 0x20).
   Evidence: C1 0x5A4066 / C2 0x5A4070 (batch220: news 0x28 HumanoidStage) /
     D1 0x5A41A8 / D0 (batch232 sweep) / getStageType 0x5A45E6 -> 11 /
     onEdgeAdded 0x5A440C (121B) / onEdgeRemoving 0x5A4486 (98B) /
     removeJoint 0x5A4302 (169B, :43) (batches 208/220/228/232).
   Members: count16 + edge hook list (binary edge+36/+40 links, ours
   SpanningEdge stepNext/stepPrev -- mechanics DELTA, membership/order
   exact) + profiler28 (void*: binary news 0xA01C CodeProfiler("Joint
   Update") -- profiler module absent, always NULL, delete skipped).
   Layout: IStage base (+4 upstream / +8 downstream=HumanoidStage / +12 world).
   Original file: Client/App/v8world/StepJointsStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"
#include "V8World/Joint.h"
#include "Util/Profiling.h"
#include "boost/scoped_ptr.hpp"

namespace RBX {

	class World;
	class Edge;
	class Assembly;

	// SIZE 0x20
	class StepJointsStage : public IWorldStage
	{
	public:
		StepJointsStage(IStage* upstream, World* world);
		virtual ~StepJointsStage();

		// mac: 0x5A45E6 (10B): STEP_JOINTS_STAGE.
		// FUNCTION: PLAYER 0x008ea720
		// FUNCTION: STUDIO 0x00a2bae0
		virtual StageType getStageType() const { return STEP_JOINTS_STAGE; }
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x5A4302 (169B, batch232).
		void removeJoint(Joint* joint);

		void onSimulateAssemblyAdded(Assembly* a);
		void onSimulateAssemblyRemoving(Assembly* a);

	public:
		// PLAYER 0x008ef270: the World's step loop calls it.
		void jointsStepWorld();

	private:
		// 2013: a constant-size boost::intrusive list over Joint's StepJointsStageHook (+0x20):
		// size 0x10, root 0x14/0x18 (ctor 0x008ef410; onEdgeAdded 0x008ef1a0 links before the root).
		typedef boost::intrusive::list<Joint, boost::intrusive::base_hook<StepJointsStageHook> > Joints;
		Joints worldStepJoints;                                         // 0x10
		boost::scoped_ptr<Profiling::CodeProfiler> profilingJointUpdate;	// 0x1c ("Joint Update")

		friend class World;	// World::loadProfilers reads the profiler
	};

} // namespace RBX

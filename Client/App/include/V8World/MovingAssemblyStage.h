/* V8World/MovingAssemblyStage.h -- October 2013 (moving-assembly stage, 0x28), from the 2013 bytes.
   IWorldStage (upstream 0x04, downstream = StepJointsStage 0x08, world 0x0c), uiStepJoints (boost::intrusive
   list over Joint's MovingAssemblyStageHook) 0x10, movingGroundedAssemblies (std::set) 0x1c.
   Original file: Client/App/v8world/MovingAssemblyStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"
#include "V8World/Joint.h"

#include <set>

namespace RBX {

	class World;
	class Edge;
	class Assembly;

	// 2013 size: `new 0x28` in AssemblyStage::AssemblyStage PLAYER 0x008ea638. The Mac
	// reading's "second list head" at +32 does not exist on Windows.
	// SIZE 0x28
	class MovingAssemblyStage : public IWorldStage
	{
	public:
		MovingAssemblyStage(IStage* upstream, World* world);
		virtual ~MovingAssemblyStage();
		// mac: 0x586EC0 (batch278: erase from the +28 set, return count).
		size_t removeMovingGroundedAssembly(Assembly* assembly);

		// mac: 0x587216 (10B): MOVING_ASSEMBLY_STAGE.
		// FUNCTION: PLAYER 0x00994f60
		// FUNCTION: STUDIO 0x00861b10
		virtual StageType getStageType() const { return MOVING_ASSEMBLY_STAGE; }
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x586EE6 (201B, batch222).
		void removeJoint(Joint* joint);

		void onSimulateAssemblyAdded(Assembly* a);
		void onSimulateAssemblyRemoving(Assembly* a);

		// PLAYER 0x008eb2a0 (2013 has no animated-joint set: one loop over uiStepJoints).
		void jointsStepUi(double distributedGameTime);

	private:
		// 2013: a constant-size boost::intrusive list over Joint's MovingAssemblyStageHook (+0x28):
		// size 0x10, root 0x14/0x18 (ctor 0x008eb610, onEdgeAdded 0x008eb250).
		typedef boost::intrusive::list<Joint, boost::intrusive::base_hook<MovingAssemblyStageHook> > Joints;
		Joints uiStepJoints;                                 // 0x10
		std::set<Assembly*> movingGroundedAssemblies;        // 0x1c
	};

} // namespace RBX

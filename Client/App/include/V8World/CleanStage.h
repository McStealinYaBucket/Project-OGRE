/* V8World/CleanStage.h -- October 2013 (World's edge stage, 16B uniform).
   Evidence: C2 0x55CBAE (105B, batch210: news 0x64 JointStage, uniform) /
     D1 0x55CFD0 / D0 0x55CFFE / onEdgeAdded 0x55CE76 (66B) /
     onEdgeRemoving 0x55CEB8 (268B) (batch210) / onPrimitiveAdded 0x55CC18 /
     onPrimitiveRemoving 0x55CC48 / onJointPrimitiveNulling 0x55CC78 /
     onJointPrimitiveSet 0x55CDD8 (batches 213/219a).
   Layout: IStage base only (16B -- uniform node; the 120-shape telemetry
   was bring-up observation, retired with the chain wiring: nothing in the
   tree or harness reads it). +4 upstream(NULL per World C2 0x5A82FB) /
   +8 downstream=JointStage / +12 world.
   getKernel: inherited IStage::getKernel (chain walk -- the NULL stub is
   retired with the wiring).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/CleanStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

namespace RBX {

	class Primitive;
	class World;
	class Joint;

	class CleanStage : public IWorldStage
	{
	public:
		CleanStage(void* stageArg, World* world);
		virtual ~CleanStage();

		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);
		// mac: 0x55D048 (7B): CLEAN_STAGE.
		// STUB: PLAYER 0x0096bde0
		virtual StageType getStageType() const { return CLEAN_STAGE; }

		// Called by World::World 0x5A8308 with the mega-cluster placeholder.
		void onPrimitiveAdded(Primitive* prim);
		// Called by World::removePrimitive 0x5AB856 (pipeline-exit point).
		void onPrimitiveRemoving(Primitive* prim);

		// mac: 0x55CC78 (352B) / 0x55CDD8 (158B, batch219a): joint-prim unlink /
		// relink notifications (Joint::setPrimitive world path).
		void onJointPrimitiveNulling(Joint* joint, Primitive* prim);
		void onJointPrimitiveSet(Joint* joint, Primitive* prim);
	};

} // namespace RBX

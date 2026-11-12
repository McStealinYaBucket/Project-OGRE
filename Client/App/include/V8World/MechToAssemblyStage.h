/* V8World/MechToAssemblyStage.h -- October 2013 (mech-to-assembly node, 16B).
   Evidence: C2 (batch220: news 0x28 AssemblyStage) / D1 0x57E438 / D0
     0x57E3EE (batch224) / getStageType 0x57E8A6 -> 8.
   Layout: IStage base only (16B, no members -- uniform node). onEdge pair:
   inherited IWorldStage base bodies (batch225 vtable proof -- slots hold
   0x577450/0x5774DE, no override). TU (C2/D2 bodies) lands with
   AssemblyStage concrete (its members need C2-read mapping, section 123+).
   Original file: Client/App/v8world/MechToAssemblyStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

namespace RBX {

	class World;
	class Assembly;

	class MechToAssemblyStage : public IWorldStage
	{
	public:
		MechToAssemblyStage(IStage* upstream, World* world);
		virtual ~MechToAssemblyStage();

		// mac: 0x57E8A6 (10B): MECH_TO_ASSEMBLY_STAGE.
		// FUNCTION: PLAYER 0x00992a80 FOLDED
		// FUNCTION: STUDIO 0x007a06a0 FOLDED
		virtual StageType getStageType() const { return MECH_TO_ASSEMBLY_STAGE; }
		// onEdge pair: inherited base bodies (see MovingStage.h note).
		virtual void onEdgeAdded(Edge* edge) { IWorldStage::onEdgeAdded(edge); }
		virtual void onEdgeRemoving(Edge* edge) { IWorldStage::onEdgeRemoving(edge); }
		// Assembly dispatch (0x57E83E onFixedAssemblyAdded, batch297;
		// the Simulate/NoSimulate quartet queues with the stepper --
		// LOUD STUBS, link-only for moveInto's unreached phases).
		void onFixedAssemblyAdded(Assembly* assembly);
		void onSimulateAssemblyRootAdded(Assembly* assembly);
		void onNoSimulateAssemblyRootAdded(Assembly* assembly);
		void onSimulateAssemblyRootRemoving(Assembly* assembly);
		void onNoSimulateAssemblyRootRemoving(Assembly* assembly);
		void onFixedAssemblyRemoving(Assembly* assembly);
	};

} // namespace RBX

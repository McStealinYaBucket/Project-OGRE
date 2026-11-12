/* V8World/MovingStage.h -- October 2013 (moving stage node, 16B).
   Evidence: C2 0x58740E (105B, batch216: news 0x10 SpatialFilter) /
     D1 0x5874C2 / D0 0x587478 (batch224) / getStageType 0x5877E6 -> 6 /
     onMechanismAdded 0x587560 / onMechanismRemoving 0x5876A8 (batch220).
   Layout: IStage base only (16B, no members -- uniform node). onEdge pair:
   inherited IWorldStage base bodies (batch225 vtable proof -- slots hold
   0x577450/0x5774DE, no override). onMechanism pair (0x587560/0x5876A8,
   batch220) is a different dispatch family (mechanisms, called from
   TreeStage::cleanMechanism) and lands with the assembly flow, not here.
   Original file: Client/App/v8world/MovingStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

namespace RBX {

	class World;
	class Mechanism;
	class SpatialFilter;

	class MovingStage : public IWorldStage
	{
	public:
		MovingStage(IStage* upstream, World* world);
		virtual ~MovingStage();

		// 2016 shape (binary calls onAssemblyRootRemoving on the +8
		// downstream directly; out-of-line to avoid the include cycle).
		SpatialFilter* getSpatialFilter();

		// mac: 0x5877E6 (10B): MOVING_STAGE.
		// FUNCTION: PLAYER 0x007550d0
		// FUNCTION: STUDIO 0x00a5a740
		virtual StageType getStageType() const { return MOVING_STAGE; }
		// onEdge pair: inherited IWorldStage base bodies (batch225 vtable
		// proof). Explicit forwarders (MSVC needs an overrider for
		// concreteness; behavior == base body, same as binary slot-sharing).
		virtual void onEdgeAdded(Edge* edge) { IWorldStage::onEdgeAdded(edge); }
		virtual void onEdgeRemoving(Edge* edge) { IWorldStage::onEdgeRemoving(edge); }
		// mac: 0x5876A8 (batch295: root + child assembly-root removals +
		// removeFromStage + noAssemblies assert). Reached only for
		// pipelined mechanisms (never in-tree: fresh mechanisms skip
		// via inPipeline-false); linked for removeFromPipeline.
		void onMechanismRemoving(Mechanism* mechanism);
		// mac: 0x587560 (batch220: putInStage + grounded dispatch to
		// Fixed/Moving root-added + children). Reached from
		// cleanMechanism (assemble era).
		void onMechanismAdded(Mechanism* mechanism);
	};

} // namespace RBX

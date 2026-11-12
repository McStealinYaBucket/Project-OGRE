/* V8World/SpatialFilter.h -- October 2013 (spatial filter node, 0xA8).
   Evidence: C2 0x5A1F06 (361B, batch220: news 0x10 MechToAssemblyStage +
     members below) / D2 (batch223: assemblies-empty asserts + erases +
     link walk + delete +8) / getStageType 0x5A3298 -> 7.
   Members (binary offsets from C2/D2; ours model-relative): mode16,
   phase20, flags24, center28, points40 (WeightedPoint array), 4 assembly
   sets (D2-erased), toMove152 (MoveInstructions array). MoveInstructions +
   AssemblySet carried from 2016 SpatialFilter.h (first-party, 12B PODs);
   FilterPhase carried from 2016 Assembly.h into our Assembly.h. SimSendFilter
   (2016 member) is ABSENT from the binary C2 (no construction) -- omitted.
   onEdge pair: inherited IWorldStage base bodies (no override in vtable --
   verified absent across sweeps 208/213/221b/222). onAssemblyRoot* family:
   different dispatch (assemblies, not prims/edges); declared with that flow.
   Layout: IStage base (+4 upstream / +8 downstream=MechToAssemblyStage /
     +12 world).
   Original file: Client/App/v8world/SpatialFilter.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"
#include "V8World/Assembly.h"
#include "Util/Region2.h"
#include "G3D/Array.h"

#include <set>
#include <vector>

namespace RBX {

	class World;
	class MechToAssemblyStage;

	class SpatialFilter : public IWorldStage
	{
	public:
		typedef std::set<Assembly*> AssemblySet;

		// Carried from 2016 SpatialFilter.h (first-party POD, 12B).
		class MoveInstructions {
		public:
			Assembly* a;
			Assembly::FilterPhase from;
			Assembly::FilterPhase to;

			MoveInstructions() : a(NULL), from((Assembly::FilterPhase)0), to((Assembly::FilterPhase)0) {}
			MoveInstructions(Assembly* _a, Assembly::FilterPhase _from, Assembly::FilterPhase _to)
				: a(_a), from(_from), to(_to) {}
		};

		SpatialFilter(IStage* upstream, World* world);
		virtual ~SpatialFilter();

		// mac: 0x5A3298 (10B): SPATIAL_FILTER.
		// FUNCTION: PLAYER 0x00755180
		// FUNCTION: STUDIO 0x00a29ce0
		virtual StageType getStageType() const { return SPATIAL_FILTER; }
		// onEdge pair: inherited base bodies (see MovingStage.h note).
		virtual void onEdgeAdded(Edge* edge) { IWorldStage::onEdgeAdded(edge); }
		virtual void onEdgeRemoving(Edge* edge) { IWorldStage::onEdgeRemoving(edge); }
		// changePhase/moveInto era stub (SendPhysics-gated; see TU). Only
		// reached for pipelined assemblies (never in-tree); linked for
		// MovingStage::onMechanismRemoving.
		void onAssemblyRootRemoving(Assembly* assembly);
		// Assembly-root adds (0x5A2FF8 Fixed / 0x5A2EEE Moving,
		// batch297; Moving + filterAssembly queue behind SendPhysics).
		void onFixedAssemblyRootAdded(Assembly* assembly);
		void onMovingAssemblyRootAdded(Assembly* assembly);
		// mac: 0x5A2212 (batch297: :43/:44/:49/:58 asserts; `<= 3`
		// list gate; setFilterPhase + moveInto).
		void changePhase(MoveInstructions& mi);
		// mac: 0x5A23E0 (batch297: direct phase comparisons; :86/:90/:94
		// + :106/:110/:114 downstream asserts).
		void moveInto(MoveInstructions& mi);
		// 2016 shape (binary addresses downstream +8 directly).
		MechToAssemblyStage* getMechToAssemblyStage();
		// filterAssembly STUB (SimSendFilter + network + Humanoid;
		// SendPhysics era -- see TU).
		Assembly::FilterPhase filterAssembly(Assembly* assembly, bool simulating);

	private:
		int mode;                        // +16 (C2 1).
		int phase;                       // +20 (C2 -1).
		short flags;                     // +24 (C2 -1).
		G3D::Vector2 center;             // +28 (C2 zero).
		G3D::Array<Region2::WeightedPoint> points;  // +40.
		AssemblySet assemblies[4];       // +56.. (D2-erased).
		G3D::Array<MoveInstructions> toMove;  // +152.
	};

} // namespace RBX

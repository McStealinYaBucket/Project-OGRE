/* V8Kernel/IStage.h -- October 2013 (pipeline stage node shell; getKernel's assert is IStage.h:69).
   Uniform 16B stage nodes [vptr][+4 upstream][+8 downstream][+12 world] (CleanStage C2 0x55CBAE,
     JointStage 0x57B4CC, EdgeStage 0x573442, GroundStage 0x574E3C, ContactStage 0x56F48C, TreeStage
     0x5A46E4); IPipelined reads +4/+8 directly, downstream forwards hit vtbl+16/+20.
   Slots (batch225): IStage [2]D1 [3]D0 [4]getStageType (pure) [5]getKernel (body 0x55D050, never
     overridden); IWorldStage adds [6]onEdgeAdded [7]onEdgeRemoving [8]getMetric (0x5A6240, not modeled).
   StageType: the 16 2016 values, binary-verified (batch213). DELTA vs 2016: members public (every stage
     C2 writes +4/+8 directly); D2 does not delete (each stage D1/D2 deletes its +8 child). */

#pragma once

#include "rbx/Debug.h"

namespace RBX {

	class Edge;
	class Kernel;
	class World;
	class IWorldStage;

	class IStage
	{
	public:
		typedef enum {
			CLEAN_STAGE,
			JOINT_STAGE,
			GROUND_STAGE,
			EDGE_STAGE,
			CONTACT_STAGE,
			TREE_STAGE,
			MOVING_STAGE,
			SPATIAL_FILTER,
			MECH_TO_ASSEMBLY_STAGE,
			ASSEMBLY_STAGE,
			MOVING_ASSEMBLY_STAGE,
			STEP_JOINTS_STAGE,
			HUMANOID_STAGE,
			SLEEP_STAGE,
			SIMULATE_STAGE,
			KERNEL_STAGE
		} StageType;

		IStage()
			: upstream(NULL)
			, downstream(NULL)
			, world(NULL)
		{}
		virtual ~IStage() {}

		// +2 (vtbl+8). Pure: every banked stage returns its const above.
		virtual StageType getStageType() const = 0;






		// mac: 0x55D050 (123B, batch218): assert downstream (IStage.h:69) +
		// forward. VIRTUAL slot +3 (World::getKernel dispatches vtbl+12) --
		// non-pure with body (no stage overrides it -- every vtable in
		// batch225 holds this body at [5]). Defined here as in 2013 (the
		// assert cites IStage.h:69; the prior tree had it in IWorldStage.cpp).
		// (C4717 "recursive" warning here is benign: dispatch is virtual and
		// terminates at Kernel::getKernel / the CleanStage stub.)
		// FUNCTION: PLAYER 0x008e4710
		// FUNCTION: STUDIO 0x009cf8c0
		virtual Kernel* getKernel()
		{
			RBXASSERT(downstream);
			return downstream->getKernel();
		}

		// Inline accessors (binary reads the members directly).
		IStage* getUpstream() const { return upstream; }
		IStage* getDownstream() const { return downstream; }
		// +12 (binary findWorld 0x57A224 reads it off any stage).
		World* getWorld() const { return world; }

		// Written directly by every stage C2 and read directly by
		// IPipelined methods (see header note). Public: the binary has no
		// access control at runtime (same precedent as Primitive members).
		// downstream is IStage*: the binary +8 child slot is untyped (2016
		// IStage carries IStage* upstream/downstream; Simulate holds a
		// Kernel* here, which is not an IWorldStage). Forward sites use
		// IWorldStage::getDownstreamWS() (binary name -- IWorldStage.cpp
		// :11/:18 assert strings) for edge dispatch; getKernel() dispatches
		// through IStage::getKernel (Kernel terminal override).
		IStage* upstream;            // +4
		IStage* downstream;          // +8
		World* world;        // +12 (findWorld 0x57A224; every stage C2 stores
		                     // its World* here -- section 120 moves it up from
		                     // CleanStage, matching the binary uniformity).
	};

} // namespace RBX

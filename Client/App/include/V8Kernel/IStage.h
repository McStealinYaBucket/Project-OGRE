/* V8Kernel/IStage.h -- October 2013 (pipeline stage node shell).
   Evidence: uniform 16B stage nodes ([vptr][+4 upstream-arg][+8 child]
     [+12 world]) from CleanStage C2 0x55CBAE / JointStage C2 0x57B4CC /
     EdgeStage C2 0x573442 / GroundStage C2 0x574E3C / ContactStage C2
     0x56F48C / TreeStage C2 0x5A46E4 (batches 210/213/214); IPipelined
     methods read a2+4 (upstream) and currentStage+8 (downstream) DIRECTLY
     (0x576F19/0x576F76); downstream forwards hit vtbl+16/+20
     (IWorldStage::onEdgeAdded 0x5774D3 / onEdgeRemoving 0x577555,
     CleanStage::onEdgeAdded 0x55CEAE, JointStage::moveEdgeToDownstream
     0x57B883, TreeStage::onEdgeAdded 0x5A5E85).
     Slot map, batch225 vtable proof (Itanium [0]=offset, [1]=typeinfo):
     IStage: [2]D1, [3]D0, [4]getStageType(pure), [5]getKernel(body 0x55D050).
     IWorldStage adds [6]onEdgeAdded / [7]onEdgeRemoving (pure + base bodies
     0x577450/0x5774DE) and [8]getMetric (base body 0x5A6240 -- NOT modeled:
     no in-tree callers; TreeStage/SleepStage override it). Every stage
     vtable holds IStage::getKernel at [5] (no stage overrides it).
     Our MSVC layout differs by ABI (as everywhere); dispatch is by name
     and compiler-consistent.
     StageType enum carried from 2016 v8kernel/IStage.h -- all 16 values
     binary-verified (batch213: Clean 0 / Joint 1 / Ground 2 / Edge 3 /
     Contact 4 / Tree 5 / Moving 6 / SpatialFilter 7 / MechToAssembly 8 /
     AssemblyStage 9 / MovingAssembly 10 / StepJoints 11 / Humanoid 12 /
     Sleep 13 / Simulate 14 / Kernel 15).
     DELTA vs 2016 (private members, C2 takes links, D2 deletes downstream,
     no edge virtuals): members are public here (every stage C2 writes +4/+8
     directly and IPipelined methods read them directly, matching the binary;
     same no-access-control precedent as Primitive members); D2 does NOT
     delete (each binary D1/D2 deletes its +8 child explicitly).
   Original file: Client/App/v8world/IStage.h (path inferred from
     IPipelined.cpp assert strings) */

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
		// STUB: PLAYER 0x008e4710
		// STUB: STUDIO 0x009cf8c0
		virtual Kernel* getKernel()
		{
			RBXASSERT(downstream != NULL);  // IStage.h:69 "downstream"
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

/* App/v8world/SpatialFilter.cpp -- October 2013 (spatial filter node; mac C2 0x5A1F06, D2 batch223).
   onEdge pair and getKernel are inherited. */
#include "stdafx.h"

#include "V8World/SpatialFilter.h"
#include "V8World/MechToAssemblyStage.h"
#include "V8World/Mechanism.h"
#include "V8World/SendPhysics.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x5A1F06 (361B, batch220).
	// STUB: PLAYER 0x008e4e20
	// STUB: STUDIO 0x009cffd0
	SpatialFilter::SpatialFilter(IStage* upstreamArg, World* worldArg)
		: mode(1)
		, phase(-1)
		, flags(-1)
	{
		downstream = new MechToAssemblyStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}


	SpatialFilter::~SpatialFilter()
	{
		for (int i = 0; i < 4; ++i)
			RBXASSERT(assemblies[i].empty());
		delete static_cast<MechToAssemblyStage*>(downstream);
	}





	// mac: 0x5A2212 (batch297).
	// FUNCTION: PLAYER 0x008e47b0
	// FUNCTION: STUDIO 0x009cf960
	void SpatialFilter::changePhase(MoveInstructions& mi)
	{
		RBXASSERT(mi.to != mi.from);  // :43
		RBXASSERT(mi.a->getFilterPhase() == mi.from);  // :44

		if (mi.from < Assembly::NUM_PHASES)
		{
			int num = (int)assemblies[mi.from].erase(mi.a);
			RBXASSERT(num == 1);  // :49
		}

		mi.a->setFilterPhase(mi.to);
		moveInto(mi);

		if (mi.to < Assembly::NUM_PHASES)
		{
			bool ok = assemblies[mi.to].insert(mi.a).second;
			RBXASSERT(ok);  // :58
		}
	}

	// mac: 0x5A23E0 (batch297: direct phase comparisons -- 2013 has no
	// sendingPhase/simulatingPhase out-of-lines; :86/:90/:94 +
	// :106/:110/:114 downstream asserts).
	// STUB: PLAYER 0x008e3c70
	// STUB: STUDIO 0x009cf2e0
	void SpatialFilter::moveInto(MoveInstructions& mi)
	{
		if (mi.from == Assembly::NoSim_Send)
		{
			getWorld()->getSendPhysics()->onMovingAssemblyRootRemoving(mi.a);
		}
		if (mi.from > Assembly::NoSim_Send)
		{
			if (mi.from - Assembly::NoSim_Send > 1)
			{
				if (mi.from == Assembly::Fixed)
				{
					RBXASSERT(mi.a->downstreamOfStage(this));  // :94
					getMechToAssemblyStage()->onFixedAssemblyRemoving(mi.a);
				}
			}
			else
			{
				RBXASSERT(mi.a->downstreamOfStage(this));  // :90
				getMechToAssemblyStage()->onNoSimulateAssemblyRootRemoving(mi.a);
			}
		}
		else
		{
			RBXASSERT(mi.a->downstreamOfStage(this));  // :86
			getMechToAssemblyStage()->onSimulateAssemblyRootRemoving(mi.a);
		}
		if (mi.to == Assembly::NoSim_Send)
		{
			getWorld()->getSendPhysics()->onMovingAssemblyRootAdded(mi.a);
		}
		if (mi.to > Assembly::NoSim_Send)
		{
			if (mi.to - Assembly::NoSim_Send > 1)
			{
				if (mi.to == Assembly::Fixed)
				{
					getMechToAssemblyStage()->onFixedAssemblyAdded(mi.a);
					RBXASSERT(mi.a->downstreamOfStage(this));  // :114
				}
			}
			else
			{
				getMechToAssemblyStage()->onNoSimulateAssemblyRootAdded(mi.a);
				RBXASSERT(mi.a->downstreamOfStage(this));  // :110
			}
		}
		else
		{
			getMechToAssemblyStage()->onSimulateAssemblyRootAdded(mi.a);
			RBXASSERT(mi.a->downstreamOfStage(this));  // :106
		}
	}

	// 2016 shape (binary addresses downstream +8 directly).
	MechToAssemblyStage* SpatialFilter::getMechToAssemblyStage()
	{
		return static_cast<MechToAssemblyStage*>(downstream);
	}


















































































































































	// mac: 0x5A2EEE (batch297: :279/:280 asserts + putInPipeline +
	// changePhase(NOT_ASSIGNED -> filterAssembly)). The filter
	// half queues behind SendPhysics (see filterAssembly stub).
	// STUB: PLAYER 0x008e4ba0
	// STUB: STUDIO 0x009cfd50
	void SpatialFilter::onMovingAssemblyRootAdded(Assembly* a)
	{
		RBXASSERT(Mechanism::isMovingAssemblyRoot(a));
		RBXASSERT(!a->computeIsGrounded());
		a->putInPipeline(this);
		MoveInstructions mi(a, Assembly::NOT_ASSIGNED,
			filterAssembly(a, false));
		changePhase(mi);
	}



	// mac: 0x5A2FF8 (batch297: :295/:296 asserts + putInPipeline +
	// changePhase(NOT_ASSIGNED -> Fixed)).
	// FUNCTION: PLAYER 0x008e4c80
	// FUNCTION: STUDIO 0x009cfe30
	void SpatialFilter::onFixedAssemblyRootAdded(Assembly* a)
	{
		RBXASSERT(!Mechanism::isMovingAssemblyRoot(a));
		RBXASSERT(a->computeIsGrounded());
		a->putInPipeline(this);
		MoveInstructions mi(a, Assembly::NOT_ASSIGNED, Assembly::Fixed);
		changePhase(mi);
	}

	// changePhase/moveInto era STUB (2016 body needs the SendPhysics +
	// Simulate dispatch, unported). Reached only when a PIPELINED
	// assembly root is removed (inPipeline-true); in-tree mechanisms
	// are never pipelined, so this is link-only today. Loud if the
	// pipeline era ever reaches it.
	void SpatialFilter::onAssemblyRootRemoving(Assembly* assembly)
	{
		(void)assembly;
		RBXASSERT(false && "onAssemblyRootRemoving: pipeline era (changePhase)");
	}

	// filterAssembly STUB (needs SimSendFilter + network + Humanoid;
	// SendPhysics era). Reached only for moving assemblies (none
	// in-tree: nothing assembles a moving root yet). Loud if reached.
	Assembly::FilterPhase SpatialFilter::filterAssembly(Assembly* assembly, bool simulating)
	{
		(void)assembly;
		(void)simulating;
		RBXASSERT(false && "filterAssembly: SendPhysics era");
		return Assembly::NOT_ASSIGNED;
	}

} // namespace RBX

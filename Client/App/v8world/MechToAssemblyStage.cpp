/* App/v8world/MechToAssemblyStage.cpp -- October 2013 (node bodies).
   C2 (batch220: news 0x28 AssemblyStage, uniform) / D2 (pattern: delete +8;
   D1 0x57E438 / D0 0x57E3EE per batch224). getStageType header-inline.
   onEdge pair: inherited IWorldStage base bodies (batch225 vtable proof).
   getKernel: inherited IStage::getKernel. */

#include "stdafx.h"

#include "V8World/MechToAssemblyStage.h"
#include "V8World/AssemblyStage.h"
#include "V8World/Assembly.h"
#include "V8World/World.h"

namespace RBX {

	// Uniform 0x10 node C2 (batch220).
	MechToAssemblyStage::MechToAssemblyStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new AssemblyStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	// STUB: PLAYER 0x00997240
	// STUB: STUDIO 0x00a49fd0
	MechToAssemblyStage::~MechToAssemblyStage()
	{
		// D2 pattern (uniform 0x10 node: delete +8; D1/D0 per batch224;
		// members: none).
		delete static_cast<AssemblyStage*>(downstream);
	}

	// mac: 0x57E83E (batch297: putInStage + downstream onFixedAssemblyRootAdded).
	void MechToAssemblyStage::onFixedAssemblyAdded(Assembly* assembly)
	{
		assembly->putInStage(this);
		static_cast<AssemblyStage*>(getDownstreamWS())->onFixedAssemblyRootAdded(assembly);
	}

	// Stepper-era LOUD STUBS (EAs queued; reached only for
	// simulating/nosimulating assemblies -- none in-tree yet).
	void MechToAssemblyStage::onSimulateAssemblyRootAdded(Assembly* assembly)
	{
		(void)assembly;
		RBXASSERT(false && "onSimulateAssemblyRootAdded: stepper era");
	}

	void MechToAssemblyStage::onNoSimulateAssemblyRootAdded(Assembly* assembly)
	{
		(void)assembly;
		RBXASSERT(false && "onNoSimulateAssemblyRootAdded: stepper era");
	}

	void MechToAssemblyStage::onSimulateAssemblyRootRemoving(Assembly* assembly)
	{
		(void)assembly;
		RBXASSERT(false && "onSimulateAssemblyRootRemoving: stepper era");
	}

	void MechToAssemblyStage::onNoSimulateAssemblyRootRemoving(Assembly* assembly)
	{
		(void)assembly;
		RBXASSERT(false && "onNoSimulateAssemblyRootRemoving: stepper era");
	}

	void MechToAssemblyStage::onFixedAssemblyRemoving(Assembly* assembly)
	{
		(void)assembly;
		RBXASSERT(false && "onFixedAssemblyRemoving: stepper era");
	}

} // namespace RBX

/* App/v8world/AssemblyStage.cpp -- October 2013 (assembly stage), from the 2013 bytes; onEngineChanged asserts on :40. */

#include "stdafx.h"

#include "V8World/AssemblyStage.h"
#include "V8World/MovingAssemblyStage.h"
#include "V8World/World.h"
#include "V8World/Assembly.h"
#include "V8World/Primitive.h"

namespace RBX {








	// FUNCTION: PLAYER 0x008ea4b0
	// FUNCTION: STUDIO 0x00a086c0
	Assembly* AssemblyStage::onEngineChanging(Primitive* p)
	{
		if (Assembly::isAssemblyRootPrimitive(p)) {
			Assembly* a = p->getAssembly();
			if (a->inPipeline() && a->downstreamOfStage(this)) {
				beforeAssemblyRemoving(a);
				rbx_static_cast<MovingAssemblyStage*>(getDownstreamWS())->onSimulateAssemblyRemoving(a);
				a->removeFromPipeline(this);
				return a;
			}
		}
		return NULL;
	}

	// FUNCTION: PLAYER 0x008ea510
	// FUNCTION: STUDIO 0x00a08720
	void AssemblyStage::onEngineChanged(Assembly* a)
	{
		RBXASSERT(a);
		a->putInPipeline(this);
		rbx_static_cast<MovingAssemblyStage*>(getDownstreamWS())->onSimulateAssemblyAdded(a);
		afterAssemblyAdded(a);
	}

	// mac: 0x557E08 (149B, batch220).
	// FUNCTION: PLAYER 0x008ea620
	// FUNCTION: STUDIO 0x00a08830
	AssemblyStage::AssemblyStage(IStage* upstreamArg, World* worldArg)
		: EdgeBuffer(upstreamArg, new MovingAssemblyStage(this, worldArg), worldArg)
	{
	}

	// Inlined into the deleting destructor (see AssemblyStage.h); no out-of-line copy in PLAYER.
	AssemblyStage::~AssemblyStage()
	{
	}

	// mac: 0x55807E (batch297: putInStage + afterAssemblyAdded).
	void AssemblyStage::onFixedAssemblyRootAdded(Assembly* assembly)
	{
		assembly->putInStage(this);
		afterAssemblyAdded(assembly);
	}

} // namespace RBX

/* App/v8world/AssemblyStage.cpp -- October 2013 (assembly node).
   C2 0x557E08 (149B, batch220: news 0x34 MovingAssemblyStage; the +20 set
   member is EdgeBuffer::assemblyEdges, default-constructed). D2: implicit
   (EdgeBuffer::~EdgeBuffer owns the teardown -- binary D1 0x557ECE calls
   the base dtor). getStageType header-inline. getKernel: inherited. */

#include "stdafx.h"

#include "V8World/AssemblyStage.h"
#include "V8World/MovingAssemblyStage.h"
#include "V8World/World.h"
#include "V8World/Assembly.h"

namespace RBX {

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

/* App/v8world/SimulateStage.cpp -- October 2013 (simulate node; mac batches 221b/772/776/777).
   DELTA: the assembly lists port to std::list<Assembly*> (the binary threads the assemblies' own
   +48/+52 ints as {next, prev}). */
#include "stdafx.h"
#include "V8World/SimulateStage.h"
#include "V8World/Assembly.h"
#include "V8World/Edge.h"
#include "V8World/Mechanism.h"
#include "V8World/Primitive.h"
#include "V8World/SendPhysics.h"
#include "V8World/World.h"
#include "V8Kernel/Kernel.h"
#include "Util/FastFlags.h"
namespace RBX {
	// STUB: PLAYER 0x00998e10
	// STUB: STUDIO 0x00a593a0
	SimulateStage::SimulateStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new Kernel(this);
		upstream = upstreamArg;
		world = worldArg;
	}
	// STUB: PLAYER 0x00998ca0
	// STUB: STUDIO 0x00a59220
	SimulateStage::~SimulateStage()
	{
		RBXASSERT(movingDynamicAssemblies.size() == 0);  // :27
		RBXASSERT(realTimeAssemblies.size() == 0);       // :28
		RBXASSERT(movingAssemblyRoots.size() == 0);      // :29
		delete static_cast<Kernel*>(downstream);
	}
	// STUB: PLAYER 0x00998ac0
	// STUB: STUDIO 0x00a59040
	void SimulateStage::onAssemblyAdded(Assembly* a)
	{
		RBXASSERT(!a->getAssemblyPrimitive()->requestFixed());   // :36
		RBXASSERT(!a->computeIsGrounded());                      // :37
		// The throttle predicate is re-read at REMOVE on the exact same expression -- count follows the CURRENT predicate (a mid-stage flip would unbalance there as well).
		if (!a->getCanThrottle() && FFlag::SkipBroadPhaseWhenThrottling)
		{
			realTimeAssemblies.push_back(a);
		}
		else
		{
			movingDynamicAssemblies.push_back(a);
		}
		// (binary chains the assembly's own +48/+52 ints as the list node {next, prev}; ours store those in list heap nodes -- anchor algebra identical, see header.)
		a->putInStage(this);
		a->putInKernel(getKernel());
		// The binary re-issues the getKernel vcall (GCC no-CSE); one call here -- no observable (IStage::getKernel forwards only).
		getKernel()->insertBody(a->getAssemblyPrimitive()->getBody());
		putFirstMovingRootInSendPhysics(a);
		world->onAssemblyInSimluationStage(a);   // the binary takes the
		                                         // hook's int -- nothing consumes it.
	}










	// mac: 0x59B91E (batch776).
	// FUNCTION: PLAYER 0x00998920
	// FUNCTION: STUDIO 0x00a58ea0
	void SimulateStage::putFirstMovingRootInSendPhysics(Assembly* a)
	{
		Assembly* movingRoot = Mechanism::getMovingAssemblyRoot(a);
		RBXASSERT(Mechanism::isMovingAssemblyRoot(movingRoot));   // :72
		std::map<Assembly*, int>::iterator it = movingAssemblyRoots.find(movingRoot);
		if (it == movingAssemblyRoots.end())
		{
			movingAssemblyRoots.insert(std::make_pair(movingRoot, 1));
			world->getSendPhysics()->onMovingAssemblyRootAdded(movingRoot);
			// (the binary tails out with the SimJob* in eax -- no consumer, port drops it.)
		}
		else
		{
			++it->second;
		}
	}
	// mac: 0x59BAB4 (batch776).
	// STUB: PLAYER 0x009989f0
	// STUB: STUDIO 0x00a58f70
	bool SimulateStage::removeLastMovingRootFromSendPhysics(Assembly* a)
	{
		IndexedMesh* node = a->getMesh();
		while (node->getIndexedMeshParent() != NULL)
		{
			node = node->getIndexedMeshParent();
		}
		Assembly* owner = reinterpret_cast<Assembly*>(reinterpret_cast<char*>(node) - 8);
		if (removeFromSendPhysics(owner))
			return true;
		Assembly* oneBelowRoot = a->getMesh()->getOneBelowRoot<Assembly>();
		RBXASSERT(oneBelowRoot);
		// Include-level template (IndexedTree.h).
		bool ok = removeFromSendPhysics(oneBelowRoot);
		RBXASSERT(ok);
		return ok;
	}

	// mac: 0x59BA04 (batch772).
	// STUB: PLAYER 0x00998c10
	// STUB: STUDIO 0x00a59190
	void SimulateStage::onAssemblyRemoving(Assembly* a)
	{
		removeLastMovingRootFromSendPhysics(a);
		Kernel* kernel = getKernel();   // binary no-CSE double vcall; once.
		kernel->removeBody(a->getAssemblyPrimitive()->getBody());
		a->removeFromKernel();          // IPipelined vt+12 (kernel-exit first).
		a->removeFromStage(this);
		// Predicate re-read, exactly like the binary's unlink sequence -- both lists get their member removed and the binary's [+48]/[+52] link zero-writes are covered by the list-node mechanics.
		if (!a->getCanThrottle() && FFlag::SkipBroadPhaseWhenThrottling)
		{
			realTimeAssemblies.remove(a);
		}
		else
		{
			movingDynamicAssemblies.remove(a);
		}
	}

	// mac: 0x59BBB6 (batch776/777) -- the refcount-map operator.
	// STUB: PLAYER 0x009987f0
	// STUB: STUDIO 0x00a58d70
	bool SimulateStage::removeFromSendPhysics(Assembly* a)
	{
		std::map<Assembly*, int>::iterator it = movingAssemblyRoots.find(a);
		if (it == movingAssemblyRoots.end())
		{
			return false;
		}
		if (it->second == 1)
		{
			world->getSendPhysics()->onMovingAssemblyRootRemoving(a);
			movingAssemblyRoots.erase(it);
		}
		else
		{
			--it->second;
		}
		return true;
	}


	// mac: 0x59BC40 (57B, batch221b): stage, then kernel-link the edge
	// (putInKernel dispatches Edge vtbl+8; no downstream forward --
	// Simulate is the last IWorldStage, Kernel has no edge slots).
	// FUNCTION: PLAYER 0x00998730
	// FUNCTION: STUDIO 0x00a58a90
	void SimulateStage::onEdgeAdded(Edge* edge)
	{
		edge->putInStage(this);
		edge->putInKernel(getKernel());
	}

	// mac: 0x59BC7A (39B, batch221b): kernel-unlink, then unstage (mirror).
	// FUNCTION: PLAYER 0x00998760
	// FUNCTION: STUDIO 0x00a58ac0
	void SimulateStage::onEdgeRemoving(Edge* edge)
	{
		edge->removeFromKernel();
		edge->removeFromStage(this);
	}

} // namespace RBX

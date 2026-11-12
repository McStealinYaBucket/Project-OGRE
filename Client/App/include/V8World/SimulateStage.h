/* V8World/SimulateStage.h -- October 2013 (simulate node, 0x40).
   Evidence: C2 0x59B474 (batch220: news 0x30 Kernel + members below) /
     D2 0x59B55A (572B, batch228: :27 movingDynamicAssemblies.empty +
     :28 realTimeAssemblies.empty + :29 movingAssemblyRoots.empty + list
     walks + map erase + delete +8) / getStageType -> 14 (batch213/222) /
     onEdgeAdded 0x59BC40 (57B) / onEdgeRemoving 0x59BC7A (39B) (batch221b)
     / onAssemblyAdded 0x59B796 / onAssemblyRemoving 0x59BA04 (batch222).
   Members (binary offsets; ours model-relative):
     +16 map<Assembly*,int> movingAssemblyRoots (D2-erased; :29)
     +44 assembly list movingDynamicAssemblies (D2-walked; :27)
     +56 assembly list realTimeAssemblies (D2-walked; :28)
   Carried from 2016 SimulateStage.h (first-party names); 2016 uses
   boost::intrusive lists + std::map -- ours uses std::list + std::map
   (mechanics DELTA: binary threads Assembly+48/+52 links with sentinels;
   ours NULL-convention std::list -- same membership and order).
   onEdge pair INTERIM (documented): binary feeds edge->generate(kernel)
   (0x59BC40: putInStage + edge vtbl+8 getKernel); Edge::generate + Kernel
   insert fns need the Body module -- until then membership-only
   (putInStage/removeFromStage, no downstream forward: Simulate is the last
   IWorldStage, Kernel has no edge slots). Reversible with the stepper.
   onAssemblyAdded/Removing (different dispatch family, need Body/Kernel
   fns + sendPhysics helpers): declared with the assembly flow, not here.
   Layout: IStage base (+4 upstream / +8 downstream=Kernel terminal /
     +12 world).
   Original file: Client/App/v8world/SimulateStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

#include <list>
#include <map>

namespace RBX {

	class World;
	class Edge;
	class Assembly;
	class Kernel;

	class SimulateStage : public IWorldStage
	{
	public:
		SimulateStage(IStage* upstream, World* world);
		virtual ~SimulateStage();

		// getStageType -> 14 (batch213/222).
		virtual StageType getStageType() const { return SIMULATE_STAGE; }
		// mac: 0x59BC40/0x59BC7A (batch221b): putInStage + kernel-link
		// (Edge vtbl+8/vtbl+12 = putInKernel/removeFromKernel); no
		// downstream forward (terminal-adjacent).
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);
		// mac: 0x59B796 / 0x59BA04 (batch772; the three private helpers below
		// complete the flows with the SendPhysics + World hooks).
		void onAssemblyAdded(Assembly* a);
		void onAssemblyRemoving(Assembly* a);

	private:
		// mac: 0x59B91E (batch776): root refcount-put over getMovingAssemblyRoot
		// (assert "Mechanism::isMovingAssemblyRoot(movingRoot)" :72);
		// the first put of a root news its SimJob + fires
		// SendPhysics::onMovingAssemblyRootAdded, later ones ++ the count.
		void putFirstMovingRootInSendPhysics(Assembly* a);
		// mac: 0x59BAB4 (batch776): the assembly's mesh-root climb (owner =
		// node - 8, the mesh-at-+8 convention); removeFromSendPhysics on
		// it, else the one-below-root retry (:99 "oneBelowRoot"/
		// :102 "ok"). Bool out (both World-side callers ignore it).
		bool removeLastMovingRootFromSendPhysics(Assembly* a);
		// mac: 0x59BBB6 (batch776/777): refcount-map operator -- count==1
		// removes (fires SendPhysics::onMovingAssemblyRootRemoving) and
		// erases, >1 decrements. true = the root was mapped.
		bool removeFromSendPhysics(Assembly* a);

	private:
		std::map<Assembly*, int> movingAssemblyRoots;   // +16 (D2 :29).
		std::list<Assembly*> movingDynamicAssemblies;   // +44 (D2 :27).
		std::list<Assembly*> realTimeAssemblies;        // +56 (D2 :28).
	};

} // namespace RBX

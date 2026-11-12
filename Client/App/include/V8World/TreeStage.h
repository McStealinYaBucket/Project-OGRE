/* V8World/TreeStage.h -- October 2013 (tree stage node, 0x5C).
   Evidence: C2 0x5A46E4 (283B, batch214: news 0x10 MovingStage +
     SpanningTree secondary-base C2 at +16 + maxTreeDepth + 2 sets) /
     D0 0x5A4800 / D1 0x5A4830 (batch214; D2 pattern: set asserts +
     SpanningTree teardown + delete +8) / getStageType 0x5A62C6 -> 5 /
     onPrimitiveAdded 0x5A601A / onPrimitiveRemoving 0x5A60A2 (135B each,
     TERMINAL pair) / onEdgeAdded 0x5A5D1E (370B) / onEdgeRemoving 0x5A5E90
     (394B) / assemble 0x5A5CB8 (101B) (batches 209/211/214).
   MI: class TreeStage : public IWorldStage, public SpanningTree (2016
   shape, first-party -- binary C2 constructs the secondary at +16 with
   its own vptr; MSVC lays the secondary after the 16B IStage base, same
   access-by-name precedent as Joint:Edge+SpanningEdge).
   Members: SpanningTree secondary (+16, 24B) + maxTreeDepth +40 (C2 0) +
     dirtyMechanisms +48 + downstreamMechanisms +72 (2016 names/order,
     first-party; binary +48/+72 set-header inits).
   onEdge pair (0x5A5D1E/0x5A5E90, batch214 -- LANDED): prim gates
   :422/:423/:442/:443, putInStage, non-spanning forward, spanning
   insert/remove via the secondary tree, rigid/ground swallow gate
   ({1,2,3}/{0,11} stay tree-internal). SpanningTree hook overrides +
   assemble + mechanism/clump/destroy fns: mechanism flow, deferred.
   Layout: IStage base (+4 upstream / +8 downstream=MovingStage / +12 world).
   Original file: Client/App/v8world/TreeStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"
#include "Util/SpanningTree.h"

#include <set>

namespace RBX {

	class Primitive;
	class World;
	class Edge;
	class Mechanism;
	class SpanningEdge;
	class SpanningNode;

	// 2016 shape (NULL-safe spanning-root walk to a ground joint).
	bool chainToGround(Primitive* prim);

	class TreeStage : public IWorldStage
					, public SpanningTree
	{
	public:
		TreeStage(IStage* upstream, World* world);
		virtual ~TreeStage();

		// mac: 0x5A62C6 (10B): TREE_STAGE.
		// FUNCTION: PLAYER 0x008ec020
		virtual StageType getStageType() const { return TREE_STAGE; }
		// mac: 0x5A5D1E (370B) / 0x5A5E90 (394B, batch214): spanning joints
		// enter the tree (rigid/ground swallowed, rest forwarded); see TU.
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x5A601A / 0x5A60A2 (135B each, TERMINAL pair).
		void onPrimitiveAdded(Primitive* prim);
		void onPrimitiveRemoving(Primitive* prim);
		// mac: 0x5A5CB8 (101B, batch214: drain dirtyMechanisms through
		// cleanMechanism + clear; no trailing assert in the binary).
		void assemble();
		// cleanMechanism (2016 TreeStage.cpp text; EA queues with
		// batch297): pipeline the mechanism downstream.
		void cleanMechanism(Mechanism* mechanism);

		bool isAssembled() const { return dirtyMechanisms.empty(); }
		// Harness probe (marker-era): number of dirtied mechanisms.
		size_t dirtyMechanismCount() const { return dirtyMechanisms.size(); }

		// 2016 shape (NULL-safe root walk; binary asserts at the
		// onSpanningEdgeAdded use sites, batch271).
		// mac: 0x5A4BF4 (batch271: non-null assert + removeFromPipeline +
		// dirty-set insert).
		void dirtyMechanism(Mechanism* mechanism);
		// mac: 0x5A5A52 (batch295: in-pipeline gate + downstream erase (:327)
		// + IPipelined::removeFromPipeline + noAssemblies assert).
		// The in-pipeline half needs onMechanismRemoving (landed with
		// it); fresh mechanisms skip it (never pipelined in-tree).
		void removeFromPipeline(Mechanism* mechanism);
		// mac: 0x5A4CAE (batch271: ground/rigid/motor/spring assembly
		// cascade; :152-:156 + :195/:200/:215-:217 asserts).
		// SpanningTree hook override (fires from addEdge).
		virtual void onSpanningEdgeAdded(SpanningEdge* edge);
		// SpanningTree hook override (fires from removeEdge): tears
		// down what Added built (destroy cascade + body deparent) so a
		// later re-add parents fresh (no body cycles). 0x5A54DE /
		// mac: 0x5A5538 (batch296) + destroys 0x5A58D2/0x5A590E/0x5A5972.
		virtual void onSpanningEdgeRemoving(SpanningEdge* edge);
		virtual void onSpanningEdgeRemoved(SpanningEdge* edge, SpanningNode* child);
		// Clump/Assembly/Mechanism teardown (2016 TreeStage.cpp text;
		// EAs queue with batch296).
		void destroyClump(Primitive* prim);
		void destroyAssembly(Primitive* prim);
		void destroyMechanism(Primitive* prim);
		// 2016 shape (owner notify + rigid-children recursion; binary
		// reads prim+244 owner + joints list, batch271).
		void sendClumpChangedMessage(Primitive* childPrim);

	private:
		int maxTreeDepth;                            // +40 (C2 0).
		std::set<Mechanism*> dirtyMechanisms;        // +48.
		std::set<Mechanism*> downstreamMechanisms;   // +72.
	};

} // namespace RBX

/* Util/SpanningTree.h -- October 2013 (spanning-tree algorithm base, 24B).
   Evidence: C2 0x24EA6A (49B, batch216: vptr + Array<SpanningEdge*> at +4
     + int +20 0) / D2 (no out-of-line symbol in any sweep; 2016 asserts
     size == 0 -- the member is never mutated in either version) / full
   method family disasm-decoded (batch221a: insert 0x24EB40 /
     removeSpanningTreeEdge / swapTree 0x24EC96 / swap 0x24EEB0 /
     findLightestUpstream x2 (0x24EC2A edge + 0x24F424 static) /
     findHeaviestDownstream 0x24EDBA / buildDownstreamTree 0x24F574 /
     findAndDeactivateEdges 0x24F16C / activateEdges 0x24F2E8 / addEdge
     0x24F05E / removeEdge 0x24EF58 / empty hook bodies 0x24F6EE/0x24F6FA).
   Members (binary-exact, 2016 shapes first-party):
     +4  G3D::Array<SpanningEdge*> tempEdges (swap scratch)
     +20 int size (C2 0; never mutated -- D2 asserts 0 per 2016)
   Bodies: 2016 util/SpanningTree.cpp carried VERBATIM (first-party --
   every disasm call sequence cross-checked: getNode(0/1) + getDepth +
   isHeavierThan + side 0/1 + otherNode flips + hook slots
   [+0 Adding][+4 Added][+8 Removing][+12 Removed] + resize(0,false)).
   DELTAs vs 2016 (all disasm-pinned): swap uses tempEdges.resize(0,
   false), not fastClear; RBXASSERTs carry the binary SpanningTree.cpp
   :lines (59/74/86/106/113/119/125/136/138/145/161/165); lightParent
   omitted (2016 dead decl, no binary symbol); validating asserts kept
   via RBXASSERT_IF_VALIDATING.
   Original file: Client/App/util/SpanningTree.h (path from assert
   strings: SpanningTree.cpp) */

#pragma once

#include "G3D/Array.h"

#include <set>

namespace RBX {

	class SpanningNode;
	class SpanningEdge;

	class SpanningTree
	{
	private:
		G3D::Array<SpanningEdge*> tempEdges;  // +4 (swap scratch).
		int size;                             // +20 (C2 0).

		static SpanningNode* testEdgeToParent(int testSide, SpanningNode* child,
			SpanningEdge*& answer, int& lightSide);
		static void findLightestUpstream(SpanningNode* n0, SpanningNode* n1,
			int d0, int d1, SpanningEdge*& answer, int& lightSide);
		static void buildDownstreamTree(SpanningNode* root,
			std::set<SpanningNode*>& tree);

		void removeEdge(SpanningEdge* edge);
		void addEdge(SpanningEdge* edge, SpanningNode* newParent);

		void findAndDeactivateEdges(SpanningNode* child, SpanningEdge* deactivate,
			G3D::Array<SpanningEdge*>& toActivate);
		void activateEdges(SpanningNode* child,
			const G3D::Array<SpanningEdge*>& toActivate);

		static void findLightestUpstream(SpanningEdge* e, SpanningEdge*& answer,
			int& lightSide);
		static SpanningEdge* findHeaviestDownstream(SpanningNode* node,
			SpanningNode*& newParent);

		void swapTree(SpanningEdge* deactivate, SpanningEdge* activate,
			SpanningNode* newParent);
		void swap(SpanningEdge* deactivate, SpanningEdge* activate,
			SpanningNode* newParent);

	protected:
		virtual void onSpanningEdgeAdding(SpanningEdge* edge, SpanningNode* child) {}
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void onSpanningEdgeAdded(SpanningEdge* edge) {}
		virtual void onSpanningEdgeRemoving(SpanningEdge* edge) {}
		// STUB: PLAYER 0x008c8450
		// STUB: STUDIO 0x006125f0
		virtual void onSpanningEdgeRemoved(SpanningEdge* edge, SpanningNode* child) {}
		virtual bool validateTree(SpanningNode* root) { return true; }

	public:
		void insertSpanningTreeEdge(SpanningEdge* insertEdge);
		void removeSpanningTreeEdge(SpanningEdge* removeEdge);

		SpanningTree();
		~SpanningTree();
	};

} // namespace RBX

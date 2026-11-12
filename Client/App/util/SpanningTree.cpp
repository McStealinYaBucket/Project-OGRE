/* App/util/SpanningTree.cpp -- October 2013 (minimum-spanning-tree core).
   Carried from March 2016 util/SpanningTree.cpp VERBATIM (first-party;
   all disasm call sequences cross-checked -- see header). Bodies:
   insertSpanningTreeEdge / removeSpanningTreeEdge / swapTree / swap /
   removeEdge / addEdge / findAndDeactivateEdges / activateEdges /
   testEdgeToParent / findLightestUpstream x2 / buildDownstreamTree /
   FindHeaviest / findHeaviestDownstream (batch221a EAs in header).
   C2 0x24EA6A / D2 (size-0 assert per 2016). */

#include "stdafx.h"

#include "Util/SpanningTree.h"
#include "Util/SpanningNode.h"
#include "Util/SpanningEdge.h"

namespace RBX {

	// mac: 0x24EA6A (49B, batch216).
	SpanningTree::SpanningTree()
		: size(0)
	{
		// tempEdges default-constructs (binary Array::Array at +4).
	}

	SpanningTree::~SpanningTree()
	{
		RBXASSERT(size == 0);
	}

	// STUB: PLAYER 0x0099a1b0
	// STUB: STUDIO 0x00a5a460
	void SpanningTree::insertSpanningTreeEdge(SpanningEdge* insertEdge)
	{
		RBXASSERT(!insertEdge->inSpanningTree());  // :59

		int lightSide = 0;
		SpanningEdge* deActivate = NULL;
		SpanningTree::findLightestUpstream(insertEdge, deActivate, lightSide);

		if (!deActivate || SpanningEdge::heavierEdge(insertEdge, deActivate))
		{
			SpanningNode* insertParent = insertEdge->otherNode(lightSide);
			swapTree(deActivate, insertEdge, insertParent);
		}
	}

	void SpanningTree::removeSpanningTreeEdge(SpanningEdge* removeEdge)
	{
		RBXASSERT(removeEdge->inSpanningTree());  // :74

		SpanningNode* newParentNode = NULL;
		SpanningEdge* heaviest = findHeaviestDownstream(
			removeEdge->getChildSpanningNode(), newParentNode);

		swapTree(removeEdge, heaviest, newParentNode);
	}

	// STUB: PLAYER 0x0099a140
	// STUB: STUDIO 0x00a5a3f0
	void SpanningTree::swapTree(SpanningEdge* deactivate, SpanningEdge* activate,
		SpanningNode* newParent)
	{
		if (!activate) {
			RBXASSERT(deactivate);  // :86
		}

		if (deactivate) {
			RBXASSERT_IF_VALIDATING(validateTree(
				deactivate->getChildSpanningNode()->getRoot<SpanningNode>()));
		}

		swap(deactivate, activate, newParent);

		if (activate) {
			RBXASSERT_IF_VALIDATING(validateTree(
				activate->getChildSpanningNode()->getRoot<SpanningNode>()));
		}
	}

	// STUB: PLAYER 0x00999970
	// STUB: STUDIO 0x00a59c20
	void SpanningTree::removeEdge(SpanningEdge* edge)
	{
		RBXASSERT(edge->inSpanningTree());  // :106

		SpanningNode* child = edge->getChildSpanningNode();
		onSpanningEdgeRemoving(edge);
		edge->removeFromSpanningTree();
		onSpanningEdgeRemoved(edge, child);

		RBXASSERT(!edge->inSpanningTree());  // :113
	}

	// STUB: PLAYER 0x00999a40
	// STUB: STUDIO 0x00a59cf0
	void SpanningTree::addEdge(SpanningEdge* edge, SpanningNode* newParent)
	{
		RBXASSERT(!edge->inSpanningTree());  // :119

		onSpanningEdgeAdding(edge, edge->otherNode(newParent));
		edge->addToSpanningTree(newParent);
		onSpanningEdgeAdded(edge);

		RBXASSERT(edge->inSpanningTree());  // :125
	}

	// STUB: PLAYER 0x00999f00
	// STUB: STUDIO 0x00a5a1b0
	void SpanningTree::findAndDeactivateEdges(SpanningNode* child,
		SpanningEdge* deactivate, G3D::Array<SpanningEdge*>& toActivate)
	{
		if (SpanningNode* oldParent = child->getParent())
		{
			SpanningEdge* edge = child->getEdgeToParent();
			RBXASSERT(edge);  // :136

			RBXASSERT(edge->inSpanningTree());  // :138

			if (edge != deactivate)
			{
				toActivate.append(edge);
				removeEdge(edge);

				RBXASSERT(!edge->inSpanningTree());  // :145

				findAndDeactivateEdges(oldParent, deactivate, toActivate);
			}
		}
	}

	// STUB: PLAYER 0x00999b10
	// STUB: STUDIO 0x00a59dc0
	void SpanningTree::activateEdges(SpanningNode* child,
		const G3D::Array<SpanningEdge*>& toActivate)
	{
		SpanningNode* newParent = child;
		for (int i = 0; i < toActivate.size(); ++i)
		{
			SpanningEdge* e = toActivate[i];

			RBXASSERT(!e->inSpanningTree());  // :161

			addEdge(e, newParent);

			RBXASSERT(e->inSpanningTree());  // :165

			newParent = e->otherNode(newParent);
		}
	}

	// FUNCTION: PLAYER 0x0099a0a0
	// FUNCTION: STUDIO 0x00a5a350
	void SpanningTree::swap(SpanningEdge* deactivate, SpanningEdge* activate,
		SpanningNode* newParent)
	{
		if (activate) {
			tempEdges.resize(0, false);
			SpanningNode* child = activate->otherNode(newParent);
			findAndDeactivateEdges(child, deactivate, tempEdges);
		}

		if (deactivate) {
			removeEdge(deactivate);
		}

		if (activate) {
			addEdge(activate, newParent);
			SpanningNode* child = activate->otherNode(newParent);
			activateEdges(child, tempEdges);
		}
	}

	SpanningNode* SpanningTree::testEdgeToParent(int testSide, SpanningNode* child,
		SpanningEdge*& answer, int& lightSide)
	{
		if (SpanningEdge* edge = child->getEdgeToParent()) {
			if (!answer || edge->isLighterThan(answer)) {
				answer = edge;
				lightSide = testSide;
			}
		}
		return child->getParent();
	}

	void SpanningTree::findLightestUpstream(SpanningNode* n0, SpanningNode* n1,
		int d0, int d1, SpanningEdge*& answer, int& lightSide)
	{
		if (d0 != d1) {
			if (d0 > d1) {
				SpanningNode* n0Parent = testEdgeToParent(0, n0, answer, lightSide);
				findLightestUpstream(n0Parent, n1, d0 - 1, d1, answer, lightSide);
			}
			else {
				SpanningNode* n1Parent = testEdgeToParent(1, n1, answer, lightSide);
				findLightestUpstream(n0, n1Parent, d0, d1 - 1, answer, lightSide);
			}
		}
		else {
			if (n0 != n1) {
				SpanningNode* n0Parent = n0 ? testEdgeToParent(0, n0, answer, lightSide) : NULL;
				SpanningNode* n1Parent = n1 ? testEdgeToParent(1, n1, answer, lightSide) : NULL;
				findLightestUpstream(n0Parent, n1Parent, d0 - 1, d1 - 1, answer, lightSide);
			}
		}
	}

	void SpanningTree::findLightestUpstream(SpanningEdge* e, SpanningEdge*& answer,
		int& lightSide)
	{
		SpanningNode* n0 = e->getNode(0);
		SpanningNode* n1 = e->getNode(1);
		int d0 = SpanningNode::getDepth(n0);
		int d1 = SpanningNode::getDepth(n1);

		findLightestUpstream(n0, n1, d0, d1, answer, lightSide);
	}

	void SpanningTree::buildDownstreamTree(SpanningNode* root,
		std::set<SpanningNode*>& tree)
	{
		tree.insert(root);
		for (int i = 0; i < root->numChildren(); ++i) {
			buildDownstreamTree(root->getChild(i), tree);
		}
	}

	class FindHeaviest
	{
	public:
		std::set<SpanningNode*>& tree;
		SpanningEdge*& heaviest;
		SpanningNode*& newParent;

		FindHeaviest(
			std::set<SpanningNode*>& _tree,
			SpanningEdge*& _heaviest,
			SpanningNode*& _newParent) : tree(_tree), heaviest(_heaviest), newParent(_newParent)
		{}

		void operator()(SpanningNode* node, SpanningEdge* edge)
		{
			if (!edge->inSpanningTree()) {
				SpanningNode* other = edge->otherNode(node);
				if (!other || (tree.find(other) == tree.end())) {
					if (!heaviest || edge->isHeavierThan(heaviest)) {
						heaviest = edge;
						newParent = other;
					}
				}
			}
		}
	};

	SpanningEdge* SpanningTree::findHeaviestDownstream(SpanningNode* node,
		SpanningNode*& newParent)
	{
		std::set<SpanningNode*> downstreamTree;
		buildDownstreamTree(node, downstreamTree);

		SpanningEdge* heaviest = NULL;
		newParent = NULL;

		std::set<SpanningNode*>::const_iterator it;
		for (it = downstreamTree.begin(); it != downstreamTree.end(); ++it)
		{
			SpanningNode* n = *it;
			n->visitEdges<FindHeaviest>(FindHeaviest(downstreamTree, heaviest, newParent));
		}

		return heaviest;
	}

} // namespace RBX

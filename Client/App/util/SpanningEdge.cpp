/* App/util/SpanningEdge.cpp -- October 2013. Mac (batch221a): inSpanningTree 0x24E83C, addToSpanningTree 0x24E6A4, removeFromSpanningTree
   0x24E60C, getParentSpanningNode 0x24E5E8, getConstChildSpanningNode 0x24E54A, getChildSpanningNode 0x24E5DE. 2013 asserts :23, :41, :50-:53. */
#include "stdafx.h"
#include "Util/SpanningEdge.h"
#include "Util/SpanningNode.h"
namespace RBX {
	// mac: 0x24E54A (147B, batch221a).
	// FUNCTION: PLAYER 0x008ee8a0
	// FUNCTION: STUDIO 0x009c3800
	const SpanningNode* SpanningEdge::getConstChildSpanningNode() const
	{
		for (int i = 0; i < 2; ++i)
		{
			const SpanningNode* node = getConstNode(i);
			if (node != NULL && node->getConstEdgeToParent() == this)
				return node;
		}





		RBXASSERT(0);
		return NULL;
	}









	// mac: 0x24E60C (152B, batch221a).
	// FUNCTION: PLAYER 0x008ee910
	// FUNCTION: STUDIO 0x009c3870
	void SpanningEdge::removeFromSpanningTree()
	{
		SpanningNode* child = getChildSpanningNode();
		RBXASSERT(child);
		child->setIndexedTreeParent(NULL);
		child->setEdgeToParent(NULL);
	}
	// FUNCTION: PLAYER 0x008ee9c0
	// FUNCTION: STUDIO 0x009c3920
	void SpanningEdge::addToSpanningTree(SpanningNode* newParent)
	{
		SpanningNode* child = otherNode(newParent);
		RBXASSERT(this->otherNode(child) == newParent);
		RBXASSERT(child);
		RBXASSERT(!child->getParent());
		RBXASSERT(!child->getEdgeToParent());
		child->setEdgeToParent(this);
		child->setIndexedTreeParent(newParent);
	}

	// mac: 0x24E5DE (9B): const forward.
	SpanningNode* SpanningEdge::getChildSpanningNode()
	{
		return const_cast<SpanningNode*>(getConstChildSpanningNode());
	}
	// mac: 0x24E5E8 (36B, batch221a).
	SpanningNode* SpanningEdge::getParentSpanningNode()
	{
		return const_cast<SpanningNode*>(
			otherConstNode(getConstChildSpanningNode()));
	}
	// mac: 0x24E83C (54B, batch221a).
	bool SpanningEdge::inSpanningTree() const
	{
		for (int i = 0; i < 2; ++i)
		{
			const SpanningNode* node = getConstNode(i);
			if (node != NULL && node->getConstEdgeToParent() == this)
				return true;
		}
		return false;
	}
	// 2016 shape (first-party, no standalone EA -- header-inline there).
	SpanningNode* SpanningEdge::otherNode(int i)
	{
		RBXASSERT((i == 0) || (i == 1));
		return getNode((i + 1) % 2);
	}

} // namespace RBX

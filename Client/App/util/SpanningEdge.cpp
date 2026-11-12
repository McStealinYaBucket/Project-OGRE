/* App/util/SpanningEdge.cpp -- October 2013 (spanning-tree edge links).
   Bodies: inSpanningTree 0x24E83C (54B) / addToSpanningTree 0x24E6A4
   (408B) / removeFromSpanningTree 0x24E60C (152B) /
   getParentSpanningNode 0x24E5E8 (36B) / getConstChildSpanningNode
   0x24E54A (147B) / getChildSpanningNode 0x24E5DE (9B forwarder)
   (batch221a). Asserts via RBXASSERT + binary file:line citations.
   Virtual dispatch (otherNode/otherConstNode/getConstNode) is by name;
   the binary slot arithmetic (vtbl+12/+16/+24) is documented in the
   header. The +32 back-adjust (binary MI secondary) is spelled as
   static_cast at the use sites (Joint/Primitive modules). */

#include "stdafx.h"

#include "Util/SpanningEdge.h"
#include "Util/SpanningNode.h"

namespace RBX {

	// mac: 0x24E5DE (9B): const forward.
	SpanningNode* SpanningEdge::getChildSpanningNode()
	{
		return const_cast<SpanningNode*>(getConstChildSpanningNode());
	}

	// mac: 0x24E54A (147B, batch221a).
	// STUB: PLAYER 0x008ee8a0
	// STUB: STUDIO 0x009c3800
	const SpanningNode* SpanningEdge::getConstChildSpanningNode() const
	{
		for (int i = 0; i < 2; ++i)
		{
			const SpanningNode* node = getConstNode(i);
			if (node != NULL && node->getConstEdgeToParent() == this)
				return node;
		}
		RBXASSERT(0);  // SpanningEdge.cpp:23 "0"
		return NULL;
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

	// mac: 0x24E6A4 (408B, batch221a).
	// STUB: PLAYER 0x008ee9c0
	// STUB: STUDIO 0x009c3920
	void SpanningEdge::addToSpanningTree(SpanningNode* newParent)
	{
		SpanningNode* child = otherNode(newParent);
		RBXASSERT(otherNode(child) == newParent);  // :50
		RBXASSERT(child != NULL);  // :51 "child"
		RBXASSERT(child->getParent() == NULL);  // :52 "!child->getParent()"
		RBXASSERT(child->getEdgeToParent() == NULL);  // :53
		child->setEdgeToParent(this);
		child->setIndexedTreeParent(newParent);
	}

	// mac: 0x24E60C (152B, batch221a).
	// STUB: PLAYER 0x008ee910
	// STUB: STUDIO 0x009c3870
	void SpanningEdge::removeFromSpanningTree()
	{
		SpanningNode* child = getChildSpanningNode();
		RBXASSERT(child != NULL);  // :41 "child"
		child->setIndexedTreeParent(NULL);
		child->setEdgeToParent(NULL);
	}

	// 2016 shape (first-party, no standalone EA -- header-inline there).
	SpanningNode* SpanningEdge::otherNode(int i)
	{
		RBXASSERT((i == 0) || (i == 1));
		return getNode((i + 1) % 2);
	}

} // namespace RBX

/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

#pragma once

#include "Util/IndexedMesh.h"

namespace RBX {

	class SpanningEdge;

	// Binary evidence (Oct 2013 studio):
	//  - ~SpanningNode at 0x592D02/0x592D0C calls RBX::IndexedMesh::~IndexedMesh(this)
	//    => SpanningNode derives from IndexedMesh.
	//  - setEdgeToParent at 0x24E96A stores edge at this+40 (0x28)
	//    => SpanningEdge* edgeToParent member.
	//  - getDepth at 0x24F6BC walks the parent chain (this+4, IndexedTree::parent),
	//    matching the 2016 reference source.
	class SpanningNode : public IndexedMesh
	{
		friend class SpanningEdge;

	private:
		SpanningEdge*	edgeToParent;

	protected:
		virtual SpanningEdge* getFirstSpanningEdge() = 0;					// implemented by Primitive: 0x5920FE
		virtual SpanningEdge* getNextSpanningEdge(SpanningEdge* edge) = 0;	// implemented by Primitive: 0x592160

		void setEdgeToParent(SpanningEdge* edge);							// 0x24E96A

	public:
		SpanningNode() : edgeToParent(NULL) {}

		~SpanningNode();													// 0x592D02 (D1), 0x592D0C (D0)

		SpanningNode* getParent()					{return getTypedParent<SpanningNode>();}
		const SpanningNode* getConstParent() const	{return getConstTypedParent<SpanningNode>();}

		SpanningNode* getChild(int i) {return getTypedChild<SpanningNode>(i);}

		SpanningEdge* getEdgeToParent()							{return edgeToParent;}
		const SpanningEdge* getConstEdgeToParent() const		{return edgeToParent;}

		static int getDepth(SpanningNode* node) {				// 0x24F6BC
			if (!node) {
				return 0;
			}
			else if (!node->getParent()) {
				return 1;
			}
			else {
				return getDepth(node->getParent()) + 1;
			}
		}

		// NOTE: 2016 has `bool lessThan(const IndexedTree* other) const;` here.
		// No out-of-line SpanningNode::lessThan exists in the Oct 2013 binary
		// (studio_function_list.txt), so it is omitted.

		template<class Func>
		inline void visitEdges(Func func)
		{
			SpanningEdge* edge = getFirstSpanningEdge();
			while (edge) {
				func(this, edge);
				edge = getNextSpanningEdge(edge);
			}
		}
};

// NOTE: RBX::FindHeaviest (0x24F70A) exists as an out-of-line symbol in the binary,
// but its fields are (std::set<SpanningNode*>&, SpanningEdge*&, SpanningNode*&) -
// it is a functor local to SpanningTree.cpp (see 2016 reference), not part of this header.

} // namespace

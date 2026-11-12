/* Util/SpanningEdge.h -- October 2013 (spanning-tree edge links, 20B).
   Evidence: NO C2/D2 symbols in 5 sweeps (implicit trivial -- members are
   plain links, see below); AnchorJoint C2 sets the secondary vptr at
   Joint+32 (batch218); hook links at +36/+40 (StepJoints) and +44/+48
   (MovingAssembly) from removeJoint bodies (batches 222/228); thunks prove
   Joint::isHeavierThan/otherNode/otherConstNode live in this secondary
   vtable (batch226).
   Model: { vptr + 16B hook links }. Joint derives it secondarily
   (MSVC MI -- secondary position differs from binary by ABI, all access by
   name/static_cast, never raw offsets). Method bodies banked in raws;
   only getChildSpanningNode (9B forwarder) is defined here.
   Original file: Client/App/util/SpanningEdge.h (path inferred from
   SpanningEdge.cpp assert strings) */

#pragma once

namespace RBX {

	class Edge;
	class SpanningNode;

	class SpanningEdge
	{
	public:
		virtual ~SpanningEdge() {}

		// Secondary-vtable core (Joint overrides all five -- bodies HAVE).
		virtual bool isHeavierThan(const SpanningEdge* other) const = 0;
		virtual SpanningNode* otherNode(SpanningNode* node) = 0;
		virtual const SpanningNode* otherConstNode(const SpanningNode* node) const = 0;
		// mac: 0x578C5C / 0x578C8C (batch238: prim+8 node or NULL -- the
		// Primitive secondary is SpanningNode at +8, Itanium offset -8).
		// 2016 shapes (first-party).
		virtual SpanningNode* getNode(int i) = 0;
		virtual const SpanningNode* getConstNode(int i) const = 0;

		// 2016 shapes (first-party, header-inline there; out-of-line here
		// only where the binary has symbols).
		bool isLighterThan(const SpanningEdge* other) const {
			return other->isHeavierThan(this);
		}
		static bool heavierEdge(const SpanningEdge* test, const SpanningEdge* other) {
			return test->isHeavierThan(other);
		}
		SpanningNode* otherNode(int i);

		// Non-virtual helpers (bodies banked in raws incl. the 9B
		// getChildSpanningNode forwarder; land with the SpanningTree
		// module -- TreeStage::onEdgeAdded is their only caller).
		SpanningNode* getChildSpanningNode();
		SpanningNode* getParentSpanningNode();
		const SpanningNode* getConstChildSpanningNode() const;
		bool inSpanningTree() const;
		void addToSpanningTree(SpanningNode* node);
		void removeFromSpanningTree();

		// StepJoints intrusive links (edge+36/+40).
		Edge* stepNext;
		Edge* stepPrev;
		// MovingAssembly intrusive links (edge+44/+48).
		Edge* moveNext;
		Edge* movePrev;
	};

} // namespace RBX

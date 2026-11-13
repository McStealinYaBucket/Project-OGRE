/* V8World/Edge.h -- October 2013 (physics graph edge base).
   Evidence: Edge C2 0x572224 (+16 prim0/+20 prim1/+24 index0=-1/+28 index1=-1) /
     D2 0x572300 (Edge.h:37-40 asserts); links/getIndex/setIndex/otherPrimitive
     are header-inline in 2013 (NO standalone EAs — bodies verbatim from the
     inlined getEdge/getOther/getNext/insertEdge/removeEdge sites).
     setPrimitive slot 4 (0x57226A) TODO (partial body); Joint derives Edge
     (JointInstance+132 -> Joint*).
   2013 MSVC vftable (Joint 0x00b7252c): +0x00 dtor, +0x04 putInKernel,
   +0x08 removeFromKernel, +0x0c setPrimitive, +0x10 getEdgeType,
   +0x14 generateDataForMovingAssemblyStage; the Joint slots follow.
   VTABLE NOTE (batch227 + disasm reconciliation, Mac Itanium numbering): disasm vtbl offsets
   index from the VPTR (past offset/typeinfo: disasm+N = dump[2+N/4]).
   So disasm +20 = dump [7] = getEdgeType (pure here), +28 = dump [9]
   = getJointType, +56 = dump [16] = canStepWorld, +60 = dump [17] =
   canStepUi (AnchorJoint dump maps dump-slots to names). Our by-name
   dispatch is unaffected by slot numbering.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Edge.h (path inferred) */

#pragma once

#include "rbx/Debug.h"
#include "V8World/IPipelined.h"
#include "V8World/Enum.h"

namespace RBX {

	class Primitive;

	class Edge
		: public IPipelined
	{
	public:
		// Poor-man's dynamic-cast tag (2016 Edge.h shape; values binary-
		// proven: Joint::getEdgeType 0x485F20 returns 0, and
		// Primitive::insertEdge 0x590516 routes !=0 to the contacts lists).
		typedef enum { JOINT = 0, CONTACT = 1 } EdgeType;
		Edge(Primitive* p0, Primitive* p1)
			: edgeState(Sim::UNDEFINED)
			, throttleType(Sim::UNDEFINED_THROTTLE)
			, prim0(p0)
			, prim1(p1)
			, index0(-1)
			, index1(-1)
		{}
		// STUB: PLAYER 0x0081f840
		// STUB: STUDIO 0x00912ee0
		virtual ~Edge()
		{
			// D2 0x572300 Edge.h:37-40, index half only (prim-NULL assert +
			// poison need the owner-clear protocol — Joint module TODO).
			RBXASSERT(index0 == -1 && index1 == -1);
		}

		bool links(const Primitive* p) const { return p == prim0 || p == prim1; }
		int getIndex(const Primitive* p) const { return p == prim0 ? index0 : index1; }
		void setIndex(const Primitive* p, int i) { if (p == prim0) index0 = i; else index1 = i; }
		Primitive* otherPrimitive(const Primitive* p) const { return p == prim0 ? prim1 : prim0; }
		// Public: World::insertJoint 0x5AAD27 reads joint+16/+20 directly
		// (inlined accessors), i.e. the getters are reachable from outside.
		Primitive* getPrim0() const { return prim0; }
		Primitive* getPrim1() const { return prim1; }
		// 2016 shape (SleepStage::changeContactState 0x59Fxxx reads +8/+12
		// as DWORDs; onEdgeRemoving resets them to 0/3 -- the C2 values).
		Sim::EdgeState getEdgeState() const { return edgeState; }
		void setEdgeState(Sim::EdgeState value) { edgeState = value; }
		Sim::ThrottleType getThrottleType() const { return throttleType; }
		void setThrottleType(Sim::ThrottleType value) { throttleType = value; }
		// 2016 shape (binary Joint::setPrimitive indexes (&prim0)[i]).
		Primitive* getPrimitive(int i) { return (&prim0)[i]; }
		// mac: 0x57226A (129B): Edge.cpp:21 "(i == 0) || (i == 1)" + prim[i] set.
		// Slot 4 (see class note). DELTA (return): binary returns the prim;
		// void here -- no in-tree caller uses it (Joint::setPrimitive is the
		// only forwarder and its own return is likewise unused).
		virtual void setPrimitive(int i, Primitive* p);
		// vtable+0x10 (PLAYER Joint vftable 0x00b7252c slot 4, after setPrimitive;
		// MovingAssemblyStage::onEdgeRemoving 0x008eb4f1 dispatches +0x10).
		// DELTA vs 2016 (returns EdgeType): int keeps our Joint::getEdgeType
		// (returns 0) overriding exactly.
		virtual int getEdgeType() const = 0;
		// mac: 0x507070 (5B, batch237): stepper data feed -- base empty (the
		// Contact overrides land with the stepper; the EdgeBuffer splice
		// calls through here). Virtual per the vtbl+24 tail call in
		// pushEdgeIfOk (disasm-pinned).
		// STUB: PLAYER 0x00528720
		virtual void generateDataForMovingAssemblyStage() {}

	private:
		Sim::EdgeState edgeState;    // 0x08 byte (C2 UNDEFINED); Mac +8 dword
		Sim::ThrottleType throttleType;  // 0x09 byte (C2 UNDEFINED_THROTTLE); Mac +12 dword
		Primitive* prim0;  // 0x0c (Edge dtor 0x0081f840)
		Primitive* prim1;  // 0x10
		int index0;        // 0x14
		int index1;        // 0x18 (size 0x1c: Joint's SpanningEdge thunk 0x008eaa40 is `sub ecx,0x1c`)
	};

} // namespace RBX

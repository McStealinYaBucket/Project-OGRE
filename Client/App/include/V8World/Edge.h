/* V8World/Edge.h -- October 2013 (physics graph edge base). RBXBaseClass (novtable): ~Edge 0x0081f840 and
   Edge::Edge 0x00967c70 store no vftable. ~Edge's asserts are Edge.h:37-40, then the badMemory poison.
   2013 MSVC vftable (Joint 0x00b7252c): +0x00 dtor, +0x04 putInKernel, +0x08 removeFromKernel,
   +0x0c setPrimitive, +0x10 getEdgeType, +0x14 generateDataForMovingAssemblyStage. */
#pragma once

#include "rbx/Debug.h"
#include "V8World/IPipelined.h"
#include "V8World/Enum.h"

namespace RBX {

	class Primitive;

	class RBXBaseClass Edge : public IPipelined
	{
	private:
		Sim::EdgeState edgeState;    // 0x08 byte (C2 UNDEFINED)
		Sim::ThrottleType throttleType;  // 0x09 byte (C2 UNDEFINED_THROTTLE)
		Primitive* prim0;  // 0x0c
		Primitive* prim1;  // 0x10
		int index0;        // 0x14
		int index1;        // 0x18 (size 0x1c: Joint's SpanningEdge thunk 0x008eaa40 is `sub ecx,0x1c`)

	public:
		// mac: 0x57226A: Edge.cpp:21 "(i == 0) || (i == 1)". Slot +0x0c. Void here: no caller uses
		// the 2016 protected access or a return.
		virtual void setPrimitive(int i, Primitive* p);

		typedef enum { JOINT = 0, CONTACT = 1 } EdgeType;

		Edge(Primitive* p0, Primitive* p1);

		// FUNCTION: PLAYER 0x0081f840
		// FUNCTION: STUDIO 0x00912ee0
		virtual ~Edge() {
			RBXASSERT(index0 == -1);
			RBXASSERT(index1 == -1);
			RBXASSERT(prim0 == NULL);
			RBXASSERT(prim1 == NULL);
			index0 = -1;
			index1 = -1;
			prim0 = static_cast<Primitive*>(Debugable::badMemory());
			prim1 = static_cast<Primitive*>(Debugable::badMemory());
		}

		// vtable+0x10. DELTA vs 2016 (returns EdgeType): int keeps Joint::getEdgeType overriding exactly.
		virtual int getEdgeType() const = 0;
		// mac: 0x507070 (5B): base empty; vtable+0x14 (pushEdgeIfOk's tail call).
		// STUB: PLAYER 0x00528720
		virtual void generateDataForMovingAssemblyStage() {}

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
	};

} // namespace RBX

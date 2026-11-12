/* V8World/MovingAssemblyStage.h -- October 2013 (moving-assembly node).
   Evidence: C2 (batch220: news 0x20 StepJointsStage) / D2 (batch223:
     :24 uiStepJoints.empty + set-erase + link walk + delete +8) /
     getStageType 0x587216 -> 10 / onEdgeAdded 0x587026 (95B) /
     onEdgeRemoving 0x587086 (88B) / removeJoint 0x586EE6 (201B, :51)
     (batches 208/220/222/223).
   Members: count16 + edge hook list (binary edge+44/+48 links, ours
   SpanningEdge moveNext/movePrev -- mechanics DELTA, membership/order
   exact) + uiStepJoints set<Assembly*> (D2-erased) + second list head
   (stepper-side, untouched by captured bodies).
   Layout: IStage base (+4 upstream / +8 downstream=StepJointsStage /
     +12 world).
   Original file: Client/App/v8world/MovingAssemblyStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

#include <set>

namespace RBX {

	class World;
	class Edge;
	class Joint;
	class Assembly;

	// 2013 size: `new 0x28` in AssemblyStage::AssemblyStage PLAYER 0x008ea638. The Mac
	// reading's "second list head" at +32 does not exist on Windows.
	// SIZE 0x28
	class MovingAssemblyStage : public IWorldStage
	{
	public:
		MovingAssemblyStage(IStage* upstream, World* world);
		virtual ~MovingAssemblyStage();
		// mac: 0x586EC0 (batch278: erase from the +28 set, return count).
		size_t removeMovingGroundedAssembly(Assembly* assembly);

		// mac: 0x587216 (10B): MOVING_ASSEMBLY_STAGE.
		// FUNCTION: PLAYER 0x00994f60
		// FUNCTION: STUDIO 0x00861b10
		virtual StageType getStageType() const { return MOVING_ASSEMBLY_STAGE; }
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x586EE6 (201B, batch222).
		void removeJoint(Joint* joint);

	private:
		// 2013: these three words are a boost::intrusive::list<Joint, base_hook<
		// list_base_hook<tag<MovingAssemblyStage>>>> (size, then the root node; Joint's
		// hook is at Joint+0x28 by RTTI). Kept as plain members until Joint's hook lands.
		int numJoints;                              // 0x10  onEdgeAdded 0x008eb27a
		Joint* listHead;                            // 0x14
		Joint* listTail;                            // 0x18
		std::set<Assembly*> uiStepJoints;           // 0x1c  (D2-erased)
	};

} // namespace RBX

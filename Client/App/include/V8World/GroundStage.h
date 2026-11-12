/* V8World/GroundStage.h -- October 2013 (ground-joint stage node, 16B).
   Evidence: C2 0x574E3C (105B: news 0x10 EdgeStage) / D1+D0 (batch214) /
     getStageType 0x575B66 -> 2 / onPrimitiveAdded 0x574F1E (190B: :38 assert
     + putInStage + EdgeStage forward + addGroundJoint) / onPrimitiveRemoving
     0x575118 (293B: :50/:56 asserts + removeGroundJoint + EdgeStage forward
     + removeFromStage) / onEdgeAdded 0x575604 (123B) / onEdgeRemoving
     0x5758DC (123B) (batches 208/209/214) / addGroundJoint 0x574FDC (315B)
     / removeGroundJoint 0x57523E (424B) / rebuildFreeGround 0x575492 (291B)
     (batches 217/220) / onKernelJointAdded 0x575680 / checkForFreeGroundJoint
     0x5757A6 / onKernelJointRemoving 0x575958 (batch219a).
   Layout: IStage base (+4 upstream / +8 downstream=EdgeStage / +12 world).
   Original file: Client/App/v8world/GroundStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

namespace RBX {

	class Primitive;
	class World;
	class Edge;
	class RigidJoint;
	class KernelJoint;

	class GroundStage : public IWorldStage
	{
	public:
		GroundStage(IStage* upstream, World* world);
		virtual ~GroundStage();

		// mac: 0x575B66 (10B): GROUND_STAGE.
		// FUNCTION: PLAYER 0x00742bc0
		// FUNCTION: STUDIO 0x007a0640
		virtual StageType getStageType() const { return GROUND_STAGE; }
		// getKernel: inherited IStage::getKernel (no override -- binary
		// slot +3 holds the base body for every stage).
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x574F1E / 0x575118.
		void onPrimitiveAdded(Primitive* prim);
		void onPrimitiveRemoving(Primitive* prim);
		// mac: 0x574FDC / 0x57523E (batch217).
		void addGroundJoint(Primitive* prim, bool anchored);
		void removeGroundJoint(Primitive* prim, bool anchored);
		// mac: 0x575492 (291B, batch220).
		void rebuildFreeGround(Primitive* prim);
		// mac: 0x575A7E (217B, batch222): heaviest anchored rigid joint (NULL
		// if none). Compare spells through static_cast<SpanningEdge*>
		// (binary: raw +32 secondary-vtbl dispatch to isHeavierThan).
		RigidJoint* heaviestRigidToGround(Primitive* prim);
		// mac: 0x575680 / 0x5757A6 / 0x575958 (batch219a). KernelJoint stays
		// forward-declared (bodies touch only Edge layout + pipeline).
		void onKernelJointAdded(KernelJoint* joint);
		void checkForFreeGroundJoint(RigidJoint* joint);
		void onKernelJointRemoving(KernelJoint* joint);
	};

} // namespace RBX

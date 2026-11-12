/* V8World/ContactStage.h -- October 2013 (contact stage node, 16B).
   Evidence: C2 0x56F48C (105B: news 0x5C TreeStage) / D1 0x56F860 /
     D0 0x56F88E / getStageType 0x56F8D8 -> 4 / onPrimitiveAdded 0x56F4F6
     (48B) / onPrimitiveRemoving 0x56F526 (48B) / onEdgeAdded 0x56F556 (392B)
     / onEdgeRemoving 0x56F6DE (381B) (batches 208/213/214).
   Layout: IStage base (+4 upstream / +8 downstream=TreeStage / +12 world).
   Original file: Client/App/v8world/ContactStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

namespace RBX {

	class Primitive;
	class World;
	class Edge;

	class ContactStage : public IWorldStage
	{
	public:
		ContactStage(IStage* upstream, World* world);
		virtual ~ContactStage();

		// mac: 0x56F8D8 (10B): CONTACT_STAGE.
		// FUNCTION: PLAYER 0x00962a80
		virtual StageType getStageType() const { return CONTACT_STAGE; }
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x56F4F6 / 0x56F526 (48B each).
		void onPrimitiveAdded(Primitive* prim);
		void onPrimitiveRemoving(Primitive* prim);
	};

} // namespace RBX

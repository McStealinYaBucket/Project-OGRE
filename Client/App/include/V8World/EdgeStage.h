/* V8World/EdgeStage.h -- October 2013 (edge stage node, 16B).
   Evidence: C2 0x573442 (105B, batch213: news 0x10 ContactStage) /
     D1 0x573556 / D0 0x573584 (batch214) / getStageType 0x5735CE -> 3 /
     onEdgeAdded 0x57350C (26B: Primitive::insertEdge + base tail) /
     onEdgeRemoving 0x573526 (39B: base + Primitive::removeEdge) /
     onPrimitiveAdded 0x5734AC (48B: putInStage + ContactStage forward) /
     onPrimitiveRemoving 0x5734DC (48B: ContactStage forward + removeFromStage)
     (batch209/214/217).
   Layout: IStage base (+4 upstream / +8 downstream=ContactStage / +12 world).
   Original file: Client/App/v8world/EdgeStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

namespace RBX {

	class Primitive;
	class World;
	class ContactStage;
	class Edge;

	class EdgeStage : public IWorldStage
	{
	public:
		EdgeStage(IStage* upstream, World* world);
		virtual ~EdgeStage();

		// mac: 0x5735CE (10B): EDGE_STAGE.
		// FUNCTION: PLAYER 0x007fa990
		// FUNCTION: STUDIO 0x00a26f30
		virtual StageType getStageType() const { return EDGE_STAGE; }
		// getKernel: inherited IStage::getKernel (no override -- binary
		// slot +3 holds the base body for every stage).
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x5734AC (48B) / 0x5734DC (48B, batch217).
		void onPrimitiveAdded(Primitive* prim);
		void onPrimitiveRemoving(Primitive* prim);
	};

} // namespace RBX

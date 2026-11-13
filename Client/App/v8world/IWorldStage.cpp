/* App/v8world/IWorldStage.cpp -- October 2013 (stage base dispatch bodies).
   Bodies: onEdgeAdded 0x577450 (142B) / onEdgeRemoving 0x5774DE (142B)
   (batch209). Pure-virtual declarations live in the header (concrete stage
   required); these out-of-line definitions are what EdgeStage::onEdgeAdded
   0x573521 / onEdgeRemoving 0x57353A and GroundStage::onEdgeAdded 0x57563E /
   onEdgeRemoving 0x5758F1 tail/call into.
   Asserts via RBXASSERT + binary file:line citations. */

#include "stdafx.h"

#include "V8World/IWorldStage.h"
#include "V8World/Edge.h"

namespace RBX {

	// mac: 0x577450 (142B): assert a downstream exists, stage the edge, forward.
	// STUB: PLAYER 0x00997450
	// STUB: STUDIO 0x00a4a1e0
	void IWorldStage::onEdgeAdded(Edge* edge)
	{
		RBXASSERT(getDownstreamWS() != NULL);  // IWorldStage.cpp:11 "getDownstreamWS()"
		edge->putInStage(this);
		getDownstreamWS()->onEdgeAdded(edge);
	}

	// mac: 0x5774DE (142B): forward first, then unstage (mirror order).
	// STUB: PLAYER 0x009974c0
	// STUB: STUDIO 0x00a4a250
	void IWorldStage::onEdgeRemoving(Edge* edge)
	{
		RBXASSERT(getDownstreamWS() != NULL);  // IWorldStage.cpp:18 "getDownstreamWS()"
		getDownstreamWS()->onEdgeRemoving(edge);
		edge->removeFromStage(this);
	}

} // namespace RBX

// App/v8world/IWorldStage.cpp -- October 2013: onEdgeAdded 0x577450 / onEdgeRemoving 0x5774DE (mac).
#include "stdafx.h"
#include "V8World/IWorldStage.h"
#include "V8World/Edge.h"

namespace RBX {
	// FUNCTION: PLAYER 0x00997450
	// FUNCTION: STUDIO 0x00a4a1e0
	void IWorldStage::onEdgeAdded(Edge* edge)
	{
		RBXASSERT(getDownstreamWS());
		edge->putInStage(this);
		getDownstreamWS()->onEdgeAdded(edge);
	}
	// FUNCTION: PLAYER 0x009974c0
	// FUNCTION: STUDIO 0x00a4a250
	void IWorldStage::onEdgeRemoving(Edge* edge) {
		RBXASSERT(getDownstreamWS());
		getDownstreamWS()->onEdgeRemoving(edge);
		edge->removeFromStage(this);
	}

} // namespace RBX

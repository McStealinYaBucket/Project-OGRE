/* App/v8world/EdgeStage.cpp -- October 2013 (edge stage node bodies).
   Bodies: C2 0x573442 / D1 0x573556 / D0 0x573584 / onEdgeAdded 0x57350C /
     onEdgeRemoving 0x573526 / onPrimitiveAdded 0x5734AC /
     onPrimitiveRemoving 0x5734DC (batches 208/209/213/214/217).
   getKernel: inherited IStage::getKernel (no override -- binary slot +3
     holds the base body for every stage). */

#include "stdafx.h"

#include "V8World/EdgeStage.h"
#include "V8World/ContactStage.h"
#include "V8World/Primitive.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x573442 (105B, batch213).
	EdgeStage::EdgeStage(IStage* upstream, World* world)
	{
		downstream = new ContactStage(this, world);
		IStage::upstream = upstream;
		IStage::world = world;
	}

	// STUB: PLAYER 0x009995e0
	// STUB: STUDIO 0x00a59b70
	EdgeStage::~EdgeStage()
	{
		// D1 0x573556 / D0 0x573584 (batch214): vptr reset + delete +8
		// via vtbl+4 (D0 also operator-deletes this -- implicit).
		delete static_cast<ContactStage*>(downstream);
	}

	// mac: 0x57350C (26B, batch209): link the edge into both prims, then the
	// base tail. Disasm setup wins: single edge arg (decompiler phantom
	// second arg ignored -- same precedent as Primitive::insertEdge).
	// FUNCTION: PLAYER 0x00999610
	// FUNCTION: STUDIO 0x00a59ba0
	void EdgeStage::onEdgeAdded(Edge* edge)
	{
		Primitive::insertEdge(edge);
		IWorldStage::onEdgeAdded(edge);
	}

	// mac: 0x573526 (39B, batch209): base first, then unlink.
	// FUNCTION: PLAYER 0x00999630
	// FUNCTION: STUDIO 0x00a59bc0
	void EdgeStage::onEdgeRemoving(Edge* edge)
	{

		IWorldStage::onEdgeRemoving(edge);
		Primitive::removeEdge(edge);
	}

	// mac: 0x5734AC (48B, batch209).
	// FUNCTION: PLAYER 0x00999650
	// FUNCTION: STUDIO 0x00a59be0
	void EdgeStage::onPrimitiveAdded(Primitive* prim)
	{
		prim->putInStage(this);
		static_cast<ContactStage*>(downstream)->onPrimitiveAdded(prim);
	}

	// mac: 0x5734DC (48B, batch217).
	// FUNCTION: PLAYER 0x00999670
	// FUNCTION: STUDIO 0x00a59c00
	void EdgeStage::onPrimitiveRemoving(Primitive* prim)
	{
		static_cast<ContactStage*>(downstream)->onPrimitiveRemoving(prim);
		prim->removeFromStage(this);
	}

} // namespace RBX

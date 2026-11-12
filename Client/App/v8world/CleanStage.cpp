/* App/v8world/CleanStage.cpp -- October 2013 (World's edge stage).
   Bodies: C2 0x55CBAE (105B, batch210: news 0x64 JointStage, uniform) /
   D2 (pattern: delete +8; D1 0x55CFD0 / D0 0x55CFFE) /
   onEdgeAdded 0x55CE76 (66B) / onEdgeRemoving 0x55CEB8 (268B) (batch210) /
   onPrimitiveAdded 0x55CC18 (entry half) / onPrimitiveRemoving 0x55CC48
   (exit half) / onJointPrimitiveNulling 0x55CC78 / onJointPrimitiveSet
   0x55CDD8 (batches 213/219a).
   getStageType header-inline. getKernel: inherited IStage::getKernel. */

#include "stdafx.h"

#include "V8World/CleanStage.h"
#include "V8World/JointStage.h"
#include "V8World/Edge.h"
#include "V8World/Joint.h"
#include "V8World/World.h"
#include "V8World/ContactManager.h"
#include "V8World/Primitive.h"

namespace RBX {

	// mac: 0x55CBAE (105B, batch210).
	CleanStage::CleanStage(void* stageArg, World* worldArg)
	{
		// stageArg is NULL in every binary call site (World C2 0x5A82FB).
		(void)stageArg;
		downstream = new JointStage(this, worldArg);
		upstream = NULL;
		world = worldArg;
	}

	// STUB: PLAYER 0x008e6340
	// STUB: STUDIO 0x00a04640
	CleanStage::~CleanStage()
	{
		// D2 pattern (uniform node: delete +8; D1/D0 per batch210).
		delete static_cast<JointStage*>(downstream);
	}

	// mac: 0x55CE76 (66B, batch210): pipeline complete edges, forward them.
	// STUB: PLAYER 0x008e6550
	// STUB: STUDIO 0x00a04850
	void CleanStage::onEdgeAdded(Edge* edge)
	{
		edge->putInPipeline(this);
		Primitive* p0 = edge->getPrim0();
		Primitive* p1 = edge->getPrim1();
		if (p0 != NULL && p0 != p1 && p1 != NULL)
			getDownstreamWS()->onEdgeAdded(edge);
	}

	// mac: 0x55CEB8 (268B, batch210).
	// STUB: PLAYER 0x008e6590
	// STUB: STUDIO 0x00a04890
	void CleanStage::onEdgeRemoving(Edge* edge)
	{
		if (edge->downstreamOfStage(this))
		{
			Primitive* p0 = edge->getPrim0();
			Primitive* p1 = edge->getPrim1();
			RBXASSERT(p0 != NULL && p0 != p1 && p1 != NULL);  // :82
			getDownstreamWS()->onEdgeRemoving(edge);
			edge->removeFromPipeline(this);
			return;
		}
		{
			Primitive* p0 = edge->getPrim0();
			Primitive* p1 = edge->getPrim1();
			RBXASSERT(!(p0 != NULL && p0 != p1 && p1 != NULL));  // :86
		}
		edge->removeFromPipeline(this);
	}

	// mac: 0x55CC78 (352B, batch219a): joint-prim unlink notification.
	// STUB: PLAYER 0x008e63b0
	// STUB: STUDIO 0x00a046b0
	void CleanStage::onJointPrimitiveNulling(Joint* joint, Primitive*)
	{
		RBXASSERT(joint != NULL);  // CleanStage.cpp:49 "j"
		if (joint->downstreamOfStage(this))
		{
			// primitivesAreOk inline (both ends non-null, distinct).
			Primitive* p0 = joint->getPrim0();
			Primitive* p1 = joint->getPrim1();
			RBXASSERT(p0 != NULL && p1 != NULL && p0 != p1);  // :52
			getDownstreamWS()->onEdgeRemoving(joint);
			return;
		}
		if (joint->IPipelined::inStage(this))
		{
			Primitive* p0 = joint->getPrim0();
			Primitive* p1 = joint->getPrim1();
			RBXASSERT(!(p0 != NULL && p1 != NULL && p0 != p1));  // :56
		}
	}

	// mac: 0x55CDD8 (158B, batch219a): joint-prim relink notification.
	// STUB: PLAYER 0x008e64d0
	// STUB: STUDIO 0x00a047d0
	void CleanStage::onJointPrimitiveSet(Joint* joint, Primitive*)
	{
		RBXASSERT(!joint->downstreamOfStage(this));  // :62
		Primitive* p0 = joint->getPrim0();
		Primitive* p1 = joint->getPrim1();
		if (p0 != NULL && p1 != NULL && p0 != p1)
			getDownstreamWS()->onEdgeAdded(joint);
	}

	// STUB: PLAYER 0x008e6370
	// STUB: STUDIO 0x00a04670
	void CleanStage::onPrimitiveAdded(Primitive* prim)
	{
		// mac: 0x55CC18 (48B, batch213): putInPipeline + JointStage forward
		// (prims stage down to TreeStage terminal). The ContactManager
		// terminal call below is OURS (broadphase needs the prim; no stage
		// in the 0x5A chain feeds the CM in any captured body -- batch224
		// recons whether binary World::insertPrimitive calls CM directly;
		// until then the entry observable (pipeline + hash) is preserved
		// here, once).
		prim->putInPipeline(this);
		static_cast<JointStage*>(downstream)->onPrimitiveAdded(prim);
		ContactManager* contactManager = world->getContactManager();
		if (contactManager != NULL)
			contactManager->onPrimitiveAdded(prim);
	}

	// mac: 0x55CC48 (48B, batch213): JointStage pre-forward (mirror of the add
	// cascade), then pipeline exit.
	// FUNCTION: PLAYER 0x008e6390
	// FUNCTION: STUDIO 0x00a04690
	void CleanStage::onPrimitiveRemoving(Primitive* prim)
	{
		static_cast<JointStage*>(downstream)->onPrimitiveRemoving(prim);
		prim->removeFromPipeline(this);
	}

} // namespace RBX

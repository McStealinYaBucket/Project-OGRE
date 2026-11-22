/* App/v8world/CleanStage.cpp -- October 2013 (World's first stage). Shape as in 2016; asserts at the 2013 lines 49-86. */
#include "stdafx.h"
#include "V8World/CleanStage.h"
#include "V8World/JointStage.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"

namespace RBX {

	CleanStage::CleanStage(void* stageArg, World* worldArg)
	{
		// stageArg is NULL in every call site (World ctor).
		(void)stageArg;
		downstream = new JointStage(this, worldArg);
		upstream = NULL;
		world = worldArg;
	}
	CleanStage::~CleanStage()
	{
	}
	JointStage* CleanStage::getJointStage()
	{
		return rbx_static_cast<JointStage*>(getDownstream());
	}
	bool CleanStage::primitivesAreOk(Edge* e)
	{
		return (	e->getPrimitive(0)
				&&	e->getPrimitive(1)
				&&	(e->getPrimitive(0) != e->getPrimitive(1))	);
	}
	// FUNCTION: PLAYER 0x008e6370
	// FUNCTION: STUDIO 0x00a04670
	void CleanStage::onPrimitiveAdded(Primitive* p)
	{
		p->putInPipeline(this);
		getJointStage()->onPrimitiveAdded(p);
	}
	// FUNCTION: PLAYER 0x008e6390
	// FUNCTION: STUDIO 0x00a04690
	void CleanStage::onPrimitiveRemoving(Primitive* p)
	{
		getJointStage()->onPrimitiveRemoving(p);
		p->removeFromPipeline(this);
	}
	// FUNCTION: PLAYER 0x008e63b0
	// FUNCTION: STUDIO 0x00a046b0
	void CleanStage::onJointPrimitiveNulling(Joint* j, Primitive* nulling)
	{
		RBXASSERT(j);

		if (j->downstreamOfStage(this)) {
			RBXASSERT(primitivesAreOk(j));
			getJointStage()->onEdgeRemoving(j);
		}
		else if (j->inStage(this)) {
			RBXASSERT(!primitivesAreOk(j));
		}
	}
	// FUNCTION: PLAYER 0x008e64d0
	// FUNCTION: STUDIO 0x00a047d0
	void CleanStage::onJointPrimitiveSet(Joint* j, Primitive* p) {
		RBXASSERT(!j->downstreamOfStage(this));

		if (primitivesAreOk(j))	{
			getJointStage()->onEdgeAdded(j);
		}
	}
	// FUNCTION: PLAYER 0x008e6550
	// FUNCTION: STUDIO 0x00a04850
	void CleanStage::onEdgeAdded(Edge* e)
	{
		e->putInPipeline(this);
		if (primitivesAreOk(e)) {
			getJointStage()->onEdgeAdded(e);
		}
	}
	// FUNCTION: PLAYER 0x008e6590
	// FUNCTION: STUDIO 0x00a04890
	void CleanStage::onEdgeRemoving(Edge* e)
	{
		if (e->downstreamOfStage(this)) {
			RBXASSERT(primitivesAreOk(e));
			getJointStage()->onEdgeRemoving(e);
		}
		else {
			RBXASSERT(!primitivesAreOk(e));
		}

		e->removeFromPipeline(this);
	}

} // namespace RBX

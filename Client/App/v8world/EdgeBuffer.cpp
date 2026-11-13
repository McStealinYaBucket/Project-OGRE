/* App/v8world/EdgeBuffer.cpp -- October 2013 (assembly edge buffer; mac batches 234/235/240).
   pushEdgeIfOk returns the binary bool verdict (2016 spells void). */
#include "stdafx.h"
#include "V8World/EdgeBuffer.h"
#include "V8World/Assembly.h"
#include "V8World/Edge.h"
#include "V8World/Joint.h"
#include "V8World/Primitive.h"
#include "V8World/Contact.h"
#include "Util/FastFlags.h"
#include <boost/bind.hpp>

namespace RBX {

	// D2 0x57255A (batch240).
	// STUB: PLAYER 0x009994a0
	// STUB: STUDIO 0x00a59a30
	EdgeBuffer::~EdgeBuffer()
	{
		RBXASSERT(assemblyEdges.empty());
		delete downstream;
	}

	// 2016 shape (first-party).
	bool EdgeBuffer::assemblyIsHere(Assembly* assembly)
	{
		return assembly != NULL
			&& assembly->inPipeline()
			&& assembly->inOrDownstreamOfStage(this);
	}

	// mac: 0x572D36 (batch240: downstream-first, then unstage).
	// FUNCTION: PLAYER 0x009990c0
	// FUNCTION: STUDIO 0x00a59650
	void EdgeBuffer::onEdgeRemoving(Edge* edge)
	{
		if (edge->downstreamOfStage(this))
			getDownstreamWS()->onEdgeRemoving(edge);
		edge->removeFromStage(this);
	}

































	// mac: 0x5728E2 (289B, batch234).
	// STUB: PLAYER 0x00999170
	// STUB: STUDIO 0x00a59700
	bool EdgeBuffer::pushEdgeIfOk(Edge* e)
	{
		if (!e->inStage(this))
			return false;
		RBXASSERT(Joint::isKinematicJoint(e) == Joint::isMotorJoint(e));
		bool ok;
		if (Joint::isKinematicJoint(e)) ok = pushKinematicOk(e);
		else ok = pushSpringOk(e);
		if (ok) {
			e->generateDataForMovingAssemblyStage();  // vtbl+24
			getDownstreamWS()->onEdgeAdded(e);
		}
		return ok;
	}
	// STUB: PLAYER 0x00998ef0
	// STUB: STUDIO 0x00a59480
	bool EdgeBuffer::pushSpringOk(Edge* e)
	{
		RBXASSERT(e->inStage(this));
		Assembly* a0 = e->getPrim0()->getAssembly();
		Assembly* a1 = e->getPrim1()->getAssembly();
		return assemblyIsHere(a0) && assemblyIsHere(a1)
			&& (a0->downstreamOfStage(this) || a1->downstreamOfStage(this))
			&& (a0 != a1);
	}












	// mac: 0x572A04 (298B, batch235).
	// FUNCTION: PLAYER 0x00998fb0
	// FUNCTION: STUDIO 0x00a59540
	bool EdgeBuffer::pushKinematicOk(Edge* e)
	{
		RBXASSERT(e->inStage(this));
		RBXASSERT(Joint::isMotorJoint(e));
		Assembly* a0 = e->getPrim0()->getAssembly();
		bool grounded = a0->computeIsGrounded();
		bool downstream = assemblyIsHere(a0) && a0->downstreamOfStage(this);
		return grounded || downstream;
	}


















	// mac: 0x572C2A (batch240).
	// FUNCTION: PLAYER 0x00999270
	// FUNCTION: STUDIO 0x00a59800
	void EdgeBuffer::onEdgeAdded(Edge* e)
	{
		RBXASSERT(!Joint::isGroundJoint(e));
		RBXASSERT(!Joint::isRigidJoint(e));
		e->putInStage(this);
		pushEdgeIfOk(e);
	}

	// 2016 shape (see header note re the contactManager arm).
	void EdgeBuffer::afterAssemblyAdded(Assembly* assembly)
	{
		assembly->visitPrimitives(boost::bind(&EdgeBuffer::assemblyPrimitiveAdded, this, _1));
	}

	// mac: 0x5726EE (batch240).
	// STUB: PLAYER 0x00999360
	// STUB: STUDIO 0x00a598f0
	void EdgeBuffer::assemblyPrimitiveAdded(Primitive* prim)
	{
		if (FFlag::DeferredContacts)
		{
			// DELTA (flag-on arm): binary routes via SpatialHash::
			// onPrimitiveAssembled (unported) + prim+204 world link; the
			// flag defaults off (shipped 2013) so this arm is dead in
			// every green path -- spelled as the no-op it is.
		}
		for (int i = 0; i < prim->getNumContacts(); ++i)
			pushEdgeIfOk(prim->getContact(i));
		for (int i = 0; i < prim->getNumJoints(); ++i)
			pushEdgeIfOk(prim->getJoint(i));
	}

	// (batch240: downstream forward per downstream edge).
	void EdgeBuffer::assemblyPrimitiveRemoved(Primitive* prim)
	{
		for (int i = 0; i < prim->getNumContacts(); ++i)
		{
			Edge* contact = prim->getContact(i);
			if (contact->downstreamOfStage(this))
				getDownstreamWS()->onEdgeRemoving(contact);
		}
		for (int i = 0; i < prim->getNumJoints(); ++i)
		{
			Edge* joint = prim->getJoint(i);
			if (joint->downstreamOfStage(this))
				getDownstreamWS()->onEdgeRemoving(joint);
		}
	}

} // namespace RBX

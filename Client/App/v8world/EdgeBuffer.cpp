/* App/v8world/EdgeBuffer.cpp -- October 2013 (assembly edge buffer).
   Bodies: D2 0x57255A (batch240: :20 empty assert + erase + delete +8) /
   onEdgeAdded 0x572C2A / onEdgeRemoving 0x572D36 / pushEdgeIfOk 0x5728E2
   (batch234) / pushSpringOk 0x572B2E / pushKinematicOk 0x572A04
   (batch235) / assemblyPrimitiveAdded 0x5726EE /
   assemblyPrimitiveRemoved (batch240). Shapes: 2016 EdgeBuffer.cpp
   carried (first-party), disasm-verified per function (see notes).
   pushEdgeIfOk returns the binary bool verdict (2016 spells void).
   generateDataForMovingAssemblyStage is the vtbl+24 tail call (Edge
   base empty + Contact overrides land with the stepper -- splice feeds
   it, behavior invisible until then). */

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
		RBXASSERT(assemblyEdges.empty());  // :20
		// Multimap erase is the member dtor's job (same net state as the
		// explicit _M_erase -- cf. SpatialFilter D2 precedent).
		delete downstream;
	}

	// 2016 shape (first-party).
	bool EdgeBuffer::assemblyIsHere(Assembly* assembly)
	{
		return assembly != NULL
			&& assembly->inPipeline()
			&& assembly->inOrDownstreamOfStage(this);
	}

	// mac: 0x572C2A (batch240).
	// STUB: PLAYER 0x00999270
	// STUB: STUDIO 0x00a59800
	void EdgeBuffer::onEdgeAdded(Edge* edge)
	{
		RBXASSERT(!Joint::isGroundJoint(edge));  // :149
		RBXASSERT(!Joint::isRigidJoint(edge));  // :150
		edge->putInStage(this);
		pushEdgeIfOk(edge);
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
	bool EdgeBuffer::pushEdgeIfOk(Edge* edge)
	{
		if (!edge->inStage(this))
			return false;
		// :81 -- kinematic and motor coincide (motors route kinematic).
		RBXASSERT(Joint::isKinematicJoint(edge) == Joint::isMotorJoint(edge));
		bool ok;
		if (Joint::isKinematicJoint(edge))
			ok = pushKinematicOk(edge);
		else
			ok = pushSpringOk(edge);
		if (ok)
		{
			// vtbl+24 tail (disasm-pinned): stepper data feed (base
			// empty today) + downstream forward.
			edge->generateDataForMovingAssemblyStage();
			getDownstreamWS()->onEdgeAdded(edge);
		}
		return ok;
	}

	// mac: 0x572B2E (252B, batch235).
	// STUB: PLAYER 0x00998ef0
	// STUB: STUDIO 0x00a59480
	bool EdgeBuffer::pushSpringOk(Edge* edge)
	{
		RBXASSERT(edge->inStage(this));  // :95
		Assembly* a0 = edge->getPrim0()->getAssembly();
		Assembly* a1 = edge->getPrim1()->getAssembly();
		return assemblyIsHere(a0) && assemblyIsHere(a1)
			&& (a0->downstreamOfStage(this) || a1->downstreamOfStage(this))
			&& (a0 != a1);
	}

	// mac: 0x572A04 (298B, batch235).
	// STUB: PLAYER 0x00998fb0
	// STUB: STUDIO 0x00a59540
	bool EdgeBuffer::pushKinematicOk(Edge* edge)
	{
		RBXASSERT(edge->inStage(this));  // :119
		RBXASSERT(Joint::isMotorJoint(edge));  // :120
		Assembly* a0 = edge->getPrim0()->getAssembly();
		bool grounded = a0->computeIsGrounded();
		bool downstream = assemblyIsHere(a0) && a0->downstreamOfStage(this);
		return grounded || downstream;
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

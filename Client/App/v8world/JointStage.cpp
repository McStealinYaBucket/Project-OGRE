/* App/v8world/JointStage.cpp -- October 2013 (joint stage node; mac bodies C2 0x57B4CC / D2 0x57B614,
   batches 210-219a). onPrimitiveAdded's boost-bind range walk is BiMultiMap::visitEachLeft. */
#include "stdafx.h"
#include "V8World/JointStage.h"
#include "V8World/GroundStage.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/World.h"

#include <vector>

namespace RBX {

	// mac: 0x57B4CC (269B, batch211).
	JointStage::JointStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new GroundStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	// FUNCTION: PLAYER 0x00997df0
	// FUNCTION: STUDIO 0x00a58150
	JointStage::~JointStage()
	{
		RBXASSERT(jointMap.empty());
		RBXASSERT(incompleteJoints.empty());
		RBXASSERT(primitivesHere.empty());
		// downstream: deleted by ~IStage.
	}



	// mac: 0x57B802 (140B, batch216).
	// FUNCTION: PLAYER 0x009979b0
	// FUNCTION: STUDIO 0x00a57d10
	void JointStage::moveEdgeToDownstream(Edge* e)
	{
		RBXASSERT(edgeHasPrimitivesHere(e));
		getDownstreamWS()->onEdgeAdded(e);
	}
	// FUNCTION: PLAYER 0x00997a30
	// FUNCTION: STUDIO 0x00a57d90
	void JointStage::removeEdgeFromDownstream(Edge* e)
	{
		RBXASSERT(edgeHasPrimitivesHere(e));
		getDownstreamWS()->onEdgeRemoving(e);
	}















	// mac: 0x57B960 (181B, batch218/219a).
	// STUB: PLAYER 0x00997730
	// STUB: STUDIO 0x00a57a90
	bool JointStage::edgeHasPrimitiveHere(Edge* e, Primitive* p)
	{
		RBXASSERT(e->links(p));
		if (p == NULL)
			return false;
		return primitivesHere.find(p) != primitivesHere.end();
	}



	// mac: 0x57BA16 (184B, batch216).
	// STUB: PLAYER 0x009982a0
	// STUB: STUDIO 0x00a58600
	void JointStage::visitAddedPrimitive(Primitive* p, Joint* j, std::vector<Joint*>& out)
	{
		RBXASSERT(edgeHasPrimitiveHere(j, p));
		Primitive* other = j->getPrim0() == p ? j->getPrim1() : j->getPrim0();
		if (edgeHasPrimitiveHere(j, other))
			out.push_back(j);
	}


	// mac: 0x57BACE (437B, batch214).
	// STUB: PLAYER 0x00998360
	// STUB: STUDIO 0x00a586c0
	void JointStage::onPrimitiveAdded(Primitive* p)
	{
		bool ok = primitivesHere.insert(p).second;
		RBXASSERT(ok);
		p->putInStage(this);
		static_cast<GroundStage*>(downstream)->onPrimitiveAdded(p);
		std::vector<Joint*> found;
		// OGRE vc9: was a C++11 lambda; boost::bind is the C++03 form (the 2016 source uses it too).
		jointMap.visitEachLeft(p,
			boost::bind(&JointStage::visitAddedPrimitive, this, _1, _2, boost::ref(found)));
		for (size_t i = 0; i < found.size(); ++i)
		{
			removeJointFromHere(found[i]);
			moveEdgeToDownstream(found[i]);
		}
	}



	// mac: 0x57BD4A (727B, batch214).
	// STUB: PLAYER 0x00998480
	// STUB: STUDIO 0x00a587e0
	void JointStage::onPrimitiveRemoving(Primitive* p)
	{
		RBXASSERT(p->getNumContacts() == 0);
		std::vector<Joint*> found;
		for (Joint* j = p->getFirstJoint(); j != NULL; j = p->getNextJoint(j))
		{
			if (j->getJointType() != Joint::ANCHOR_JOINT
				&& j->getJointType() != Joint::FREE_JOINT
				&& j->downstreamOfStage(this))
			{
				RBXASSERT(edgeHasPrimitivesHere(j));
				found.push_back(j);
			}
		}
		for (size_t i = 0; i < found.size(); ++i)
		{
			removeEdgeFromDownstream(found[i]);
			putJointHere(found[i]);
		}








		RBXASSERT(p->getNumJoints() == 1);
		static_cast<GroundStage*>(downstream)->onPrimitiveRemoving(p);
		p->removeFromStage(this);
		int num = primitivesHere.erase(p);

		RBXASSERT(num == 1);
	}
	// FUNCTION: PLAYER 0x00997fb0
	// FUNCTION: STUDIO 0x00a58310
	void JointStage::putJointHere(Joint* j)
	{
		bool ok = incompleteJoints.insert(j).second;
		RBXASSERT(ok);
		jointMap.internalMap.insert(std::make_pair(j->getPrim0(), j));
		jointMap.internalMap.insert(std::make_pair(j->getPrim1(), j));
	}
	// FUNCTION: PLAYER 0x00997c10
	// FUNCTION: STUDIO 0x00a57f70
	void JointStage::removeJointFromHere(Joint* j) {
		int num = incompleteJoints.erase(j);
		RBXASSERT(num == 1);
		jointMap.removePair(j->getPrim0(), j);
		jointMap.removePair(j->getPrim1(), j);
	}
	// mac: 0x57C0DC (284B, batch214).
	// FUNCTION: PLAYER 0x00998060
	// FUNCTION: STUDIO 0x00a583c0
	void JointStage::onEdgeAdded(Edge* e)
	{
		RBXASSERT(e->getPrimitive(0) && e->getPrimitive(1) && (e->getPrimitive(0) != e->getPrimitive(1)));

		e->putInStage(this);

		if (Joint::isJoint(e)) {
			if (edgeHasPrimitivesHere(e)) {
				moveEdgeToDownstream(e);
			}
			else {
				Joint* j = rbx_static_cast<Joint*>(e);
				putJointHere(j);
			}
		}
		else {
			RBXASSERT(edgeHasPrimitivesHere(e));
			moveEdgeToDownstream(e);
		}
	}


	// FUNCTION: PLAYER 0x00997cb0
	// FUNCTION: STUDIO 0x00a58010
	void JointStage::onEdgeRemoving(Edge* e)
	{
		RBXASSERT(e->getPrimitive(0) && e->getPrimitive(1) && (e->getPrimitive(0) != e->getPrimitive(1)));

		if (e->downstreamOfStage(this)) {
			RBXASSERT(edgeHasPrimitivesHere(e));
			removeEdgeFromDownstream(e);
		}
		else {
			RBXASSERT(Joint::isJoint(e));
			Joint* j = rbx_static_cast<Joint*>(e);
			removeJointFromHere(j);
		}

		e->removeFromStage(this);
	}

	// mac: 0x57B88E (70B, batch216).
	bool JointStage::edgeHasPrimitivesHere(Edge* e)
	{
		return edgeHasPrimitiveHere(e, e->getPrim0())
			&& edgeHasPrimitiveHere(e, e->getPrim1());
	}

} // namespace RBX

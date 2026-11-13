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

	// STUB: PLAYER 0x00997df0
	// STUB: STUDIO 0x00a58150
	JointStage::~JointStage()
	{
		RBXASSERT(jointMap.empty());
		RBXASSERT(incompleteJoints.empty());
		RBXASSERT(primitivesHere.empty());
		delete static_cast<GroundStage*>(downstream);
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

	// mac: 0x57BACE (437B, batch214).
	// STUB: PLAYER 0x00998360
	// STUB: STUDIO 0x00a586c0
	void JointStage::onPrimitiveAdded(Primitive* prim)
	{
		RBXASSERT(primitivesHere.insert(prim).second);  // :95 "ok"
		prim->putInStage(this);
		static_cast<GroundStage*>(downstream)->onPrimitiveAdded(prim);
		std::vector<Joint*> found;
		// OGRE vc9: was a C++11 lambda; boost::bind is the C++03 form (the 2016 source uses it too).
		incompleteJoints.visitEachLeft(prim,
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
	void JointStage::onPrimitiveRemoving(Primitive* prim)
	{
		RBXASSERT(prim->getNumContacts() == 0);  // :116
		std::vector<Joint*> found;
		for (Joint* joint = prim->getFirstJoint(); joint != NULL;
			joint = prim->getNextJoint(joint))
		{
			if (joint->getJointType() != Joint::ANCHOR_JOINT
				&& joint->getJointType() != Joint::FREE_JOINT
				&& joint->downstreamOfStage(this))
			{
				RBXASSERT(edgeHasPrimitivesHere(joint));  // :124
				found.push_back(joint);
			}
		}
		for (size_t i = 0; i < found.size(); ++i)
		{
			removeEdgeFromDownstream(found[i]);
			putJointHere(found[i]);
		}
		RBXASSERT(prim->getNumJoints() == 1);  // :141
		static_cast<GroundStage*>(downstream)->onPrimitiveRemoving(prim);
		prim->removeFromStage(this);
		RBXASSERT(primitivesHere.erase(prim) == 1);  // :146 "num == 1"
	}

	// mac: 0x57C0DC (284B, batch214).
	// STUB: PLAYER 0x00998060
	// STUB: STUDIO 0x00a583c0
	void JointStage::onEdgeAdded(Edge* edge)
	{
		RBXASSERT(edge->getPrim0() != NULL && edge->getPrim1() != NULL
			&& edge->getPrim0() != edge->getPrim1());  // :170
		edge->putInStage(this);
		if (edge->getEdgeType() != Edge::JOINT)
		{
			RBXASSERT(edgeHasPrimitivesHere(edge));  // :184 (foreign only)
		}
		else if (!edgeHasPrimitivesHere(edge))
		{
			putJointHere(rbx_static_cast<Joint*>(edge));
			return;
		}
		moveEdgeToDownstream(edge);
	}

	// mac: 0x57C1F8 (366B, batch214).
	// STUB: PLAYER 0x00997cb0
	// STUB: STUDIO 0x00a58010
	void JointStage::onEdgeRemoving(Edge* edge)
	{

		RBXASSERT(edge->getPrim0() != NULL && edge->getPrim1() != NULL
			&& edge->getPrim0() != edge->getPrim1());  // :194
		if (edge->downstreamOfStage(this))
		{
			RBXASSERT(edgeHasPrimitivesHere(edge));  // :197
			removeEdgeFromDownstream(edge);
		}
		else
		{
			if (edge->getEdgeType() != Edge::JOINT)
				RBXASSERT(Joint::isJoint(edge));  // :201
			removeJointFromHere(static_cast<Joint*>(edge));
		}
		edge->removeFromStage(this);
	}

	// mac: 0x57BA16 (184B, batch216).
	// STUB: PLAYER 0x009982a0
	// STUB: STUDIO 0x00a58600
	void JointStage::visitAddedPrimitive(Primitive* p, Joint* joint, std::vector<Joint*>& out)
	{
		RBXASSERT(edgeHasPrimitiveHere(joint, p));  // :82
		Primitive* other = joint->getPrim0() == p ? joint->getPrim1() : joint->getPrim0();
		if (edgeHasPrimitiveHere(joint, other))
			out.push_back(joint);
	}

	// mac: 0x57BC84 (197B, batch216).
	// STUB: PLAYER 0x00997c10
	// STUB: STUDIO 0x00a57f70
	void JointStage::removeJointFromHere(Joint* joint)
	{
		RBXASSERT(jointMap.erase(joint) == 1);  // :161 "num == 1"
		incompleteJoints.removePair(joint->getPrim0(), joint);
		incompleteJoints.removePair(joint->getPrim1(), joint);
	}

	// mac: 0x57B960 (181B, batch218/219a).
	// STUB: PLAYER 0x00997730
	// STUB: STUDIO 0x00a57a90
	bool JointStage::edgeHasPrimitiveHere(Edge* edge, Primitive* prim)
	{
		RBXASSERT(edge->links(prim));  // :69 "e->links(p)"
		if (prim == NULL)
			return false;
		return primitivesHere.find(prim) != primitivesHere.end();
	}

	// mac: 0x57B88E (70B, batch216).
	bool JointStage::edgeHasPrimitivesHere(Edge* edge)
	{
		return edgeHasPrimitiveHere(edge, edge->getPrim0())
			&& edgeHasPrimitiveHere(edge, edge->getPrim1());
	}

	// mac: 0x57C022 (186B, batch216).
	// STUB: PLAYER 0x00997fb0
	// STUB: STUDIO 0x00a58310
	void JointStage::putJointHere(Joint* joint)
	{
		RBXASSERT(jointMap.insert(joint).second);  // :153 "ok"
		incompleteJoints.internalMap.insert(std::make_pair(joint->getPrim0(), joint));
		incompleteJoints.internalMap.insert(std::make_pair(joint->getPrim1(), joint));
	}

} // namespace RBX

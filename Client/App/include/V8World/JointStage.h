/* V8World/JointStage.h -- October 2013 (joint stage node, 0x64).
   Evidence: C2 0x57B4CC (269B: news 0x10 GroundStage) / D2 0x57B614 (494B:
     :26 jointMap.empty / :27 incompleteJoints.empty / :28 primitivesHere
     .empty + 3x _M_erase + validator dtor + delete +8) / getStageType
     0x57C4CC -> 1 / onPrimitiveAdded 0x57BACE (437B) / onPrimitiveRemoving
     0x57BD4A (727B) / onEdgeAdded 0x57C0DC (284B) / onEdgeRemoving 0x57C1F8
     (366B) / visitAddedPrimitive 0x57BA16 (184B) / removeJointFromHere
     0x57BC84 (197B) / moveEdgeToDownstream 0x57B802 (140B) /
     edgeHasPrimitiveHere 0x57B960 (181B) / edgeHasPrimitivesHere 0x57B88E
     (70B) / putJointHere 0x57C022 (186B) / removeEdgeFromDownstream 0x57B8D4
     (140B) (batches 210/211/214/216/218/219a).
   Layout: IStage base (+4 upstream / +8 downstream=GroundStage / +12 world)
     + ConcurrencyValidator +16 (12B) + incompleteJoints +28 (BiMultiMap,
     24B) + jointMap +52 (set<Joint*>) + primitivesHere +76 (set<Primitive*>)
     = 0x64. C2 write order + D2 erase sites pin every offset.
   Original file: Client/App/v8world/JointStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"
#include "Util/ConcurrencyValidator.h"
#include "Util/BiMultiMap.h"

#include <set>
#include <vector>

namespace RBX {

	class Primitive;
	class World;
	class Edge;
	class Joint;

	class JointStage : public IWorldStage
	{
	public:
		JointStage(IStage* upstream, World* world);
		virtual ~JointStage();

		// mac: 0x57C4CC (10B): JOINT_STAGE.
		// FUNCTION: PLAYER 0x00962a50 FOLDED
		// FUNCTION: STUDIO 0x00892cf0 FOLDED
		virtual StageType getStageType() const { return JOINT_STAGE; }
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x57BACE (437B) / 0x57BD4A (727B).
		void onPrimitiveAdded(Primitive* prim);
		void onPrimitiveRemoving(Primitive* prim);
		// mac: 0x57BA16 (184B): collect joints touching p whose far end is here.
		void visitAddedPrimitive(Primitive* p, Joint* joint, std::vector<Joint*>& out);
		// mac: 0x57BC84 (197B, :161) / 0x57B802 (140B, :39) / 0x57B88E (70B) /
		// mac: 0x57B960 (181B, :69) / 0x57C022 (186B, :153) / 0x57B8D4 (140B, :46).
		void removeJointFromHere(Joint* joint);
		void moveEdgeToDownstream(Edge* edge);
		bool edgeHasPrimitivesHere(Edge* edge);
		bool edgeHasPrimitiveHere(Edge* edge, Primitive* p);
		void putJointHere(Joint* joint);
		void removeEdgeFromDownstream(Edge* edge);

	private:
		ConcurrencyValidator validator;  // +16.
		BiMultiMap<Primitive*, Joint*> incompleteJoints;  // +28.
		std::set<Joint*> jointMap;  // +52.
		std::set<Primitive*> primitivesHere;  // +76.
	};

} // namespace RBX

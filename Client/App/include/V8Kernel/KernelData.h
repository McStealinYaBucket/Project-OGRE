/* V8Kernel/KernelData.h -- October 2013 (kernel body/connector lists, 0xB0).
   Evidence: C2 0x530AE8 (334B: 11 G3D::Array inits at +0/+16/+32/+48/+64/
     +80/+96/+112/+128/+144/+160) / D2 0x530472 (1267B: 11 empty asserts,
     KernelData.h:38-48, names banked in order below) (batch221b).
   Members (binary order + G3D::Array element types from C2 template
   instantiations; assert names from D2 strings):
     +0   Array<SimBody*>   freeFallBodies    (:38)
     +16  Array<SimBody*>   realTimeBodies    (:39)
     +32  Array<SimBody*>   jointBodies       (:40)
     +48  Array<SimBody*>   contactBodies     (:41)
     +64  Array<Body*>      leafBodies        (:42)
     +80  Array<Point*>     points            (:43)
     +96  Array<Connector*> humanoidConnectors      (:44)
     +112 Array<Connector*> secondPassConnectors    (:45)
     +128 Array<Connector*> realTimeConnectors      (:46)
     +144 Array<Connector*> jointConnectors         (:47)
     +160 Array<Connector*> contactConnectors       (:48)
   DELTA vs March 2016 KernelData.h (first-party diff aid): 2016 uses
   IndexArray x12 (adds buoyancyConnectors); 2013 uses G3D::Array x11
   (no buoyancy -- absent from C2/D2). Element classes forward-declared
   (Body module absent); arrays of pointers need no complete type.
   Original file: Client/App/include/V8Kernel/KernelData.h (path from
   assert strings). 2013 defines every member here (the asserts cite
   KernelData.h:38-230 and no KernelData.cpp string exists), so the bodies
   the prior tree had in v8kernel/KernelData.cpp are inline below, after the
   class; Kernel.cpp is the only includer and emits the out-of-line copies.
   Bodies: C2 0x530AE8 / D2 0x530472 (batch221b). C2 is 11 G3D::Array
   default constructions (implicit in C++ -- explicit body empty).
   D2 asserts each list empty (KernelData.h:38-48) then array dtors run
   implicitly (same net state as the binary's explicit teardown). */

#pragma once

#include "G3D/Array.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/SimBody.h"
#include "V8Kernel/Connector.h"
#include "V8Kernel/ContactConnector.h"
#include "V8Kernel/Pair.h"
#include "V8Kernel/Constants.h"
#include "Util/FastFlags.h"

namespace RBX {

	class SimBody;
	class Body;
	class Point;
	class Connector;

	class KernelData
	{
	public:
		G3D::Array<SimBody*> freeFallBodies;      // +0
		G3D::Array<SimBody*> realTimeBodies;      // +16
		G3D::Array<SimBody*> jointBodies;         // +32
		G3D::Array<SimBody*> contactBodies;       // +48
		G3D::Array<Body*> leafBodies;             // +64
		G3D::Array<Point*> points;                // +80
		G3D::Array<Connector*> humanoidConnectors;    // +96
		G3D::Array<Connector*> secondPassConnectors;  // +112
		G3D::Array<Connector*> realTimeConnectors;    // +128
		G3D::Array<Connector*> jointConnectors;       // +144
		G3D::Array<Connector*> contactConnectors;     // +160

		KernelData();
		~KernelData();

		// List management (batch221b/254: index asserts + back-swap
		// fastRemove spellings; validate via SimBody predicates).
		void insertBody(Body* body);
		void removeBody(Body* body);
		void addBodyToNewList(SimBody* simBody);
		void removeBodyFromCurrentList(SimBody* simBody);
		void addLeafBody(Body* body);
		void addLeafBodies(Body* body);
		void removeLeafBodies(Body* body);
		// (batch221b: :88 single-list + kind routing below; returns kind).
		int addConnector(Connector* connector);
		// (batch221b: all-clear early-out + per-list fastRemove + sim
		// decrements + symmetry pair on contact + leaf arm).
		void removeConnector(Connector* connector);
	};

	// mac: 0x530AE8 (334B, batch221b).
	inline KernelData::KernelData()
	{
		// 11 G3D::Array members default-construct (binary calls
		// Array::Array at +0/+16/+32/+48/+64/+80/+96/+112/+128/+144/+160).
	}

	// mac: 0x530472 (1267B, batch221b).
	// STUB: PLAYER 0x0082f240
	// STUB: STUDIO 0x009a1780
	inline KernelData::~KernelData()
	{
		RBXASSERT(freeFallBodies.size() == 0);      // :38
		RBXASSERT(realTimeBodies.size() == 0);      // :39
		RBXASSERT(jointBodies.size() == 0);         // :40
		RBXASSERT(contactBodies.size() == 0);       // :41
		RBXASSERT(leafBodies.size() == 0);          // :42
		RBXASSERT(points.size() == 0);              // :43
		RBXASSERT(humanoidConnectors.size() == 0);   // :44
		RBXASSERT(secondPassConnectors.size() == 0);// :45
		RBXASSERT(realTimeConnectors.size() == 0);   // :46
		RBXASSERT(jointConnectors.size() == 0);     // :47
		RBXASSERT(contactConnectors.size() == 0);   // :48
	}

	// IndexArray fastRemove equivalents below spell the back-swap +
	// moved-occupant fix + index reset inline per list (World::
	// removePrimitive precedent) -- each list owns a different index
	// member, so no shared helper.

	// (batch221b: :68 root + :70 single-list + :72 validate).
	// STUB: PLAYER 0x00831a60
	// STUB: STUDIO 0x009a40c0
	inline void KernelData::insertBody(Body* body)
	{
		RBXASSERT(body->getRoot<Body>() == body);  // :68
		SimBody* simBody = body->getSimBody();
		RBXASSERT(!body->isLeafBody() && !simBody->isInKernel());  // :70
		addBodyToNewList(simBody);
		RBXASSERT(simBody->validateBodyLists());  // :72
	}

	// (batch221b: :77 root + :78 non-leaf + :83 validate).
	// STUB: PLAYER 0x00831250
	// STUB: STUDIO 0x009a38b0
	inline void KernelData::removeBody(Body* body)
	{
		RBXASSERT(body->getRoot<Body>() == body);  // :77
		RBXASSERT(!body->isLeafBody());  // :78
		SimBody* simBody = body->getSimBody();
		removeBodyFromCurrentList(simBody);
		// Clear-sym-state stores, binary-verbatim (no dt reset -- the
		// D2 :54 dt check stays armed for removed bodies by design).
		simBody->symmetricContact = true;
		simBody->verticalContact = true;
		simBody->penetrationTorque = G3D::Vector3::zero();
		simBody->penetrationForce = G3D::Vector3::zero();
		RBXASSERT(simBody->validateBodyLists());  // :83
	}

	// (batch221b: routed by canThrottle + connector counters with the
	// FreeFall/ImpulseSolver flags; dt stamped per list).
	inline void KernelData::addBodyToNewList(SimBody* simBody)
	{
		Body* body = simBody->getBody();
		if (!body->canThrottle)
		{
			if (simBody->realTimeBodyIndex >= 0)
				return;
			removeBodyFromCurrentList(simBody);
			RBXASSERT(simBody != NULL);  // :60 "item"
			RBXASSERT(simBody->realTimeBodyIndex == -1);  // :61
			simBody->realTimeBodyIndex = (int)realTimeBodies.size();
			realTimeBodies.append(simBody);
			simBody->dt = Constants::kernelDt();
			addLeafBodies(body);
			return;
		}
		if (simBody->numConnectors[4] > 0)
		{
			if (simBody->jointBodyIndex >= 0)
				return;
			removeBodyFromCurrentList(simBody);
			RBXASSERT(simBody != NULL);  // :60
			RBXASSERT(simBody->jointBodyIndex == -1);  // :61
			simBody->jointBodyIndex = (int)jointBodies.size();
			jointBodies.append(simBody);
			simBody->dt = Constants::kernelDt();
			addLeafBodies(body);
			return;
		}
		if (simBody->numConnectors[5] > 0)
		{
			if (!FFlag::ImpulseSolverForContact)
			{
				if (simBody->jointBodyIndex >= 0)
					return;
				removeBodyFromCurrentList(simBody);
				RBXASSERT(simBody != NULL);  // :60
				RBXASSERT(simBody->jointBodyIndex == -1);  // :61
				simBody->jointBodyIndex = (int)jointBodies.size();
				jointBodies.append(simBody);
				simBody->dt = Constants::kernelDt();
				addLeafBodies(body);
				return;
			}
			if (simBody->contactBodyIndex >= 0)
				return;
			removeBodyFromCurrentList(simBody);
			RBXASSERT(simBody != NULL);  // :60
			RBXASSERT(simBody->contactBodyIndex == -1);  // :61
			simBody->contactBodyIndex = (int)contactBodies.size();
			contactBodies.append(simBody);
			simBody->dt = Constants::freeFallDt();
			return;
		}
		if (!FFlag::FreeFallOptimization || simBody->numConnectors[0] != 0)
		{
			if (simBody->jointBodyIndex >= 0)
				return;
			removeBodyFromCurrentList(simBody);
			RBXASSERT(simBody != NULL);  // :60
			RBXASSERT(simBody->jointBodyIndex == -1);  // :61
			simBody->jointBodyIndex = (int)jointBodies.size();
			jointBodies.append(simBody);
			simBody->dt = Constants::kernelDt();
			addLeafBodies(body);
			return;
		}
		if (simBody->freeFallBodyIndex < 0)
		{
			removeBodyFromCurrentList(simBody);
			RBXASSERT(simBody != NULL);  // :60
			RBXASSERT(simBody->freeFallBodyIndex == -1);  // :61
			simBody->freeFallBodyIndex = (int)freeFallBodies.size();
			freeFallBodies.append(simBody);
			simBody->dt = Constants::freeFallDt();
		}
	}

	// (batch221b: list(priority realTime > joint > contact > freeFall) +
	// dt reset).
	inline void KernelData::removeBodyFromCurrentList(SimBody* simBody)
	{
		Body* body = simBody->getBody();
		if (simBody->realTimeBodyIndex >= 0)
		{
			removeLeafBodies(body);
			int idx = simBody->realTimeBodyIndex;
			realTimeBodies[idx] = realTimeBodies.back();
			realTimeBodies.pop_back();
			if (idx < (int)realTimeBodies.size())
				realTimeBodies[idx]->realTimeBodyIndex = idx;
			simBody->realTimeBodyIndex = -1;
		}
		else if (simBody->jointBodyIndex >= 0)
		{
			removeLeafBodies(body);
			int idx = simBody->jointBodyIndex;
			jointBodies[idx] = jointBodies.back();
			jointBodies.pop_back();
			if (idx < (int)jointBodies.size())
				jointBodies[idx]->jointBodyIndex = idx;
			simBody->jointBodyIndex = -1;
		}
		else if (simBody->contactBodyIndex >= 0)
		{
			int idx = simBody->contactBodyIndex;
			contactBodies[idx] = contactBodies.back();
			contactBodies.pop_back();
			if (idx < (int)contactBodies.size())
				contactBodies[idx]->contactBodyIndex = idx;
			simBody->contactBodyIndex = -1;
		}
		else if (simBody->freeFallBodyIndex >= 0)
		{
			int idx = simBody->freeFallBodyIndex;
			freeFallBodies[idx] = freeFallBodies.back();
			freeFallBodies.pop_back();
			if (idx < (int)freeFallBodies.size())
				freeFallBodies[idx]->freeFallBodyIndex = idx;
			simBody->freeFallBodyIndex = -1;
			simBody->updateAngMomentum();
		}
		simBody->dt = 0.0f;
	}

	// (batch221a/254: :220 non-leaf + :221 connectors + :60/:61
	// IndexArray protocol + :223 listed).
	// STUB: PLAYER 0x008306e0
	// STUB: STUDIO 0x009a2d40
	inline void KernelData::addLeafBody(Body* body)
	{
		RBXASSERT(!body->isLeafBody());  // :220
		RBXASSERT(body->connectorUseCount > 0);  // :221
		RBXASSERT(body != NULL);  // :60 "item"
		RBXASSERT(body->leafBodyIndex == -1);  // :61
		body->leafBodyIndex = (int)leafBodies.size();
		leafBodies.append(body);
		RBXASSERT(body->isLeafBody());  // :223
	}

	// (batch221a: child walk + :103 index + :56 non-leaf-child +
	// :57 non-root-child + connectorUse gate + recurse).
	// STUB: PLAYER 0x00831180
	// STUB: STUDIO 0x009a37e0
	inline void KernelData::addLeafBodies(Body* body)
	{
		for (int i = 0; i < body->numChildren(); ++i)
		{
			Body* child = body->getTypedChild<Body>(i);
			RBXASSERT(child->getTypedParent<Body>() == body);  // :103 shape
			RBXASSERT(!child->isLeafBody());  // :56
			RBXASSERT(child != child->getRoot<Body>());  // :57
			if (child->connectorUseCount > 0)
				addLeafBody(child);
			addLeafBodies(child);
		}
	}

	// (batch254: :230 post-remove + recurse over children).
	inline void KernelData::removeLeafBodies(Body* body)
	{
		for (int i = 0; i < body->numChildren(); ++i)
		{
			Body* child = body->getTypedChild<Body>(i);
			RBXASSERT(child->getTypedParent<Body>() == body);  // :103 shape
			if (child->isLeafBody())
			{
				int idx = child->leafBodyIndex;
				leafBodies[idx] = leafBodies.back();
				leafBodies.pop_back();
				if (idx < (int)leafBodies.size())
					leafBodies[idx]->leafBodyIndex = idx;
				child->leafBodyIndex = -1;
				RBXASSERT(!child->isLeafBody());  // :230
			}
			removeLeafBodies(child);
		}
	}

	// (batch221b: kind routing (1/2/3 admitted; 0/4+ rejected) +
	// secondPass/joint/contact/realTime/humanoid appends + body linkage
	// + :150/:151 validates; returns the kind).
	// STUB: PLAYER 0x00831ba0
	// STUB: STUDIO 0x009a4200
	inline int KernelData::addConnector(Connector* connector)
	{
		RBXASSERT(!connector->isInKernel());  // :88
		Body* b0 = connector->getBody(Connector::body0);
		Body* b1 = connector->getBody(Connector::body1);
		SimBody* sim0 = (b0 != NULL) ? b0->getRoot<Body>()->getSimBody() : NULL;
		SimBody* sim1 = (b1 != NULL) ? b1->getRoot<Body>()->getSimBody() : NULL;
		int kind = (int)connector->getConnectorKernelType();
		bool listed0 = sim0 != NULL && sim0->isInKernel();
		bool listed1 = sim1 != NULL && sim1->isInKernel();
		if (!listed0 && !listed1 && (unsigned int)(kind - 1) >= 2
			&& kind != Connector::KERNEL_JOINT)
			return kind;
		if (kind == Connector::KERNEL_JOINT)
		{
			RBXASSERT(connector != NULL);  // :60 "item"
			RBXASSERT(connector->secondPassIndex == -1);  // :61
			connector->secondPassIndex = (int)secondPassConnectors.size();
			secondPassConnectors.append(connector);
			if (sim0 != NULL)
			{
				++sim0->numConnectors[2];
				++sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				++sim1->numConnectors[2];
				++sim1->numConnectors[0];
			}
		}
		else if (kind != Connector::HUMANOID)
		{
			bool throttle0 = sim0 == NULL || sim0->getBody()->canThrottle;
			bool throttle1 = sim1 == NULL || sim1->getBody()->canThrottle;
			if (throttle0 && throttle1)
			{
				if (kind == Connector::JOINT || kind == 4
					|| !FFlag::ImpulseSolverForContact
					|| (sim0 != NULL && sim0->jointBodyIndex >= 0)
					|| (sim1 != NULL && sim1->jointBodyIndex >= 0))
				{
					RBXASSERT(connector != NULL);  // :60
					RBXASSERT(connector->jointIndex == -1);  // :61
					connector->jointIndex = (int)jointConnectors.size();
					jointConnectors.append(connector);
					if (sim0 != NULL)
					{
						++sim0->numConnectors[4];
						++sim0->numConnectors[0];
					}
					if (sim1 != NULL)
					{
						++sim1->numConnectors[4];
						++sim1->numConnectors[0];
					}
				}
				else
				{
					RBXASSERT(kind == Connector::CONTACT);  // :137
					RBXASSERT(connector != NULL);  // :60
					RBXASSERT(connector->contactIndex == -1);  // :61
					connector->contactIndex = (int)contactConnectors.size();
					contactConnectors.append(connector);
					if (sim0 != NULL)
					{
						++sim0->numConnectors[5];
						++sim0->numConnectors[0];
					}
					if (sim1 != NULL)
					{
						++sim1->numConnectors[5];
						++sim1->numConnectors[0];
					}
				}
			}
			else
			{
				RBXASSERT(connector != NULL);  // :60
				RBXASSERT(connector->realTimeIndex == -1);  // :61
				connector->realTimeIndex = (int)realTimeConnectors.size();
				realTimeConnectors.append(connector);
				if (sim0 != NULL)
				{
					++sim0->numConnectors[3];
					++sim0->numConnectors[0];
				}
				if (sim1 != NULL)
				{
					++sim1->numConnectors[3];
					++sim1->numConnectors[0];
				}
			}
		}
		else
		{
			RBXASSERT(connector != NULL);  // :60
			RBXASSERT(connector->humanoidIndex == -1);  // :61
			connector->humanoidIndex = (int)humanoidConnectors.size();
			humanoidConnectors.append(connector);
			if (sim0 != NULL)
			{
				++sim0->numConnectors[1];
				++sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				++sim1->numConnectors[1];
				++sim1->numConnectors[0];
			}
		}
		Body* bodies[2] = { b0, b1 };
		for (int b = 0; b < 2; ++b)
		{
			Body* body = bodies[b];
			if (body == NULL)
				continue;
			++body->connectorUseCount;
			SimBody* sim = body->getRoot<Body>()->getSimBody();
			if (sim->freeFallBodyIndex >= 0 || sim->realTimeBodyIndex >= 0
				|| sim->jointBodyIndex >= 0 || sim->contactBodyIndex >= 0)
			{
				addBodyToNewList(sim);
				if (body->getRoot<Body>() != body && body->leafBodyIndex < 0
					&& (sim->jointBodyIndex >= 0 || sim->realTimeBodyIndex >= 0))
					addLeafBody(body);
			}
		}
		RBXASSERT(sim0 == NULL || sim0->validateBodyLists());  // :150
		RBXASSERT(sim1 == NULL || sim1->validateBodyLists());  // :151
		return kind;
	}

	// (batch221b: all-clear early-out; per-list fastRemove + sim/body
	// decrements; symmetry pair + leaf arm on the contact path).
	// STUB: PLAYER 0x00831f80
	// STUB: STUDIO 0x009a45e0
	inline void KernelData::removeConnector(Connector* connector)
	{
		if (connector->humanoidIndex < 0 && connector->realTimeIndex < 0
			&& connector->secondPassIndex < 0 && connector->jointIndex < 0
			&& connector->contactIndex < 0)
		{
			return;
		}
		Body* b0 = connector->getBody(Connector::body0);
		Body* b1 = connector->getBody(Connector::body1);
		SimBody* sim0 = (b0 != NULL) ? b0->getRoot<Body>()->getSimBody() : NULL;
		SimBody* sim1 = (b1 != NULL) ? b1->getRoot<Body>()->getSimBody() : NULL;
		if (connector->humanoidIndex >= 0)
		{
			int idx = connector->humanoidIndex;
			humanoidConnectors[idx] = humanoidConnectors.back();
			humanoidConnectors.pop_back();
			if (idx < (int)humanoidConnectors.size())
				humanoidConnectors[idx]->humanoidIndex = idx;
			connector->humanoidIndex = -1;
			if (sim0 != NULL)
			{
				--sim0->numConnectors[1];
				--sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				--sim1->numConnectors[1];
				--sim1->numConnectors[0];
			}
		}
		else if (connector->secondPassIndex >= 0)
		{
			int idx = connector->secondPassIndex;
			secondPassConnectors[idx] = secondPassConnectors.back();
			secondPassConnectors.pop_back();
			if (idx < (int)secondPassConnectors.size())
				secondPassConnectors[idx]->secondPassIndex = idx;
			connector->secondPassIndex = -1;
			if (sim0 != NULL)
			{
				--sim0->numConnectors[2];
				--sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				--sim1->numConnectors[2];
				--sim1->numConnectors[0];
			}
		}
		else if (connector->realTimeIndex >= 0)
		{
			int idx = connector->realTimeIndex;
			realTimeConnectors[idx] = realTimeConnectors.back();
			realTimeConnectors.pop_back();
			if (idx < (int)realTimeConnectors.size())
				realTimeConnectors[idx]->realTimeIndex = idx;
			connector->realTimeIndex = -1;
			if (sim0 != NULL)
			{
				--sim0->numConnectors[3];
				--sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				--sim1->numConnectors[3];
				--sim1->numConnectors[0];
			}
		}
		else if (connector->jointIndex >= 0)
		{
			int idx = connector->jointIndex;
			jointConnectors[idx] = jointConnectors.back();
			jointConnectors.pop_back();
			if (idx < (int)jointConnectors.size())
				jointConnectors[idx]->jointIndex = idx;
			connector->jointIndex = -1;
			if (sim0 != NULL)
			{
				--sim0->numConnectors[4];
				--sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				--sim1->numConnectors[4];
				--sim1->numConnectors[0];
			}
		}
		else
		{
			RBXASSERT(connector->isContact());  // :194
			int idx = connector->contactIndex;
			contactConnectors[idx] = contactConnectors.back();
			contactConnectors.pop_back();
			if (idx < (int)contactConnectors.size())
				contactConnectors[idx]->contactIndex = idx;
			connector->contactIndex = -1;
			if (sim0 != NULL)
			{
				--sim0->numConnectors[5];
				--sim0->numConnectors[0];
			}
			if (sim1 != NULL)
			{
				--sim1->numConnectors[5];
				--sim1->numConnectors[0];
			}
			// Binary copies this connector's contactPoint (+212) to a stack
			// PairParams (v33+v34) and runs the symmetry pair on it.
			ContactConnector* cc = static_cast<ContactConnector*>(connector);
			PairParams params = cc->getContactPoint();
			SimBody* ordered0 = sim0;
			SimBody* ordered1 = sim1;
			if (cc->getReordedSimBody(ordered0, ordered1, params))
				cc->applyContactPointForSymmetryDetection(ordered0, ordered1, params, -1.0f);
		}
		for (int b = 0; b < 2; ++b)
		{
			Body* body = (b == 0) ? b0 : b1;
			if (body == NULL)
				continue;
			if (--body->connectorUseCount == 0 && body->leafBodyIndex >= 0)
			{
				int idx = body->leafBodyIndex;
				leafBodies[idx] = leafBodies.back();
				leafBodies.pop_back();
				if (idx < (int)leafBodies.size())
					leafBodies[idx]->leafBodyIndex = idx;
				body->leafBodyIndex = -1;
				RBXASSERT(!body->isLeafBody());  // :230 shape
			}
			SimBody* sim = body->getRoot<Body>()->getSimBody();
			if (sim->freeFallBodyIndex >= 0 || sim->realTimeBodyIndex >= 0
				|| sim->jointBodyIndex >= 0 || sim->contactBodyIndex >= 0)
				addBodyToNewList(sim);
		}
	}

} // namespace RBX

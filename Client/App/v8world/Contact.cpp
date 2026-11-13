/* App/v8world/Contact.cpp -- October 2013 (contact params + data).
   Bodies: calculateFriction 0x55DFEC (batch250) / generateData
   0x55E1E0 (batch275) / onPrimitiveContactParametersChanged 0x55E06E
   (batch248). */

#include "stdafx.h"

#include "V8World/Contact.h"
#include "V8World/Primitive.h"
#include "V8Kernel/Constants.h"
#include "V8Kernel/Kernel.h"
#include "Util/FastFlags.h"

namespace RBX {

	// mac: 0x55DFEC (batch250).
	float calculateFriction(float c0, float c1)
	{
		c0 = c0 < 0.0f ? 0.0f : (c0 > 2.0f ? 2.0f : c0);
		c1 = c1 < 0.0f ? 0.0f : (c1 > 2.0f ? 2.0f : c1);
		if (((c0 <= 1.0f) && (c1 <= 1.0f))
			|| ((c0 >= 1.0f) && (c1 >= 1.0f)))
		{
			return c0 < c1 ? c0 : c1;
		}
		else
		{
			return c0 + c1 - 1.0f;
		}
	}

	// D2: heap triple + solver block free (binary 0x55DD38 shape;
	// NULL-guarded; nothing allocated until generateData runs).
	Contact::~Contact()
	{
		delete heapData;
		heapData = NULL;
	}

	// mac: 0x55DDBA (108B, batch276: virtual count/getter loop calling
	// ContactConnector::reset() per connector; recounts each pass;
	// no NULL guard on the connector -- binary-identical).
	void Contact::primitiveMovedExternally()
	{
		if (numConnectors() > 0)
		{
			for (int i = 0; i < numConnectors(); ++i)
			{
				GeoPairConnector* connector = getConnector(i);
				connector->reset();
			}
		}
	}

	// mac: 0x55E1AC (51B, batch584: getKernel + Kernel::removeConnector
	// + virtual-D0 free when non-null). Staged-contacts only
	// (binary-identical AV on NULL kernel -- Kernel-stage era owns
	// live-delete tests).
	void Contact::deleteConnector(ContactConnector* connector)
	{
		Kernel* kernel = getKernel();
		kernel->removeConnector(connector);
		if (connector != NULL)
			delete connector;
	}

	// mac: 0x55E228 (42B, batch584: deleteConnector(+48) + null when live).
	// STUB: PLAYER 0x00850a50
	// STUB: STUDIO 0x009a8730
	void BallBallContact::deleteAllConnectors()
	{
		if (connector48 != NULL)
		{
			deleteConnector(connector48);
			connector48 = NULL;
		}
	}

	// mac: 0x55E4D0 (42B, batch584: identical twin of BallBall's).
	// STUB: PLAYER 0x00850a50
	// STUB: STUDIO 0x009a8730
	void BallBlockContact::deleteAllConnectors()
	{
		if (connector48 != NULL)
		{
			deleteConnector(connector48);
			connector48 = NULL;
		}
	}

	// mac: 0x562BDA (32B, batch584: FFlag dispatch).
	// STUB: PLAYER 0x00852260
	// STUB: STUDIO 0x009a9f10
	void BlockBlockContact::deleteAllConnectors()
	{
		if (FFlag::BlockBlockNarrowPhaseRefactor)
			deleteAllConnectorsFFlag();
		else
			deleteAllConnectorsOrig();
	}

	// mac: 0x55EA6C (172B, batch584: myData assert :381 + per-element
	// deleteConnector with per-pass recount + count zero; slots keep
	// stale pointers exactly like the binary (fastClear zeroes num)).
	void BlockBlockContact::deleteAllConnectorsOrig()
	{
		RBXASSERT(myData() != NULL);  // Contact.cpp:381 "myData"
		if (myData() != NULL)
		{
			if (myData()->connectors.size() > 0)
			{
				for (size_t i = 0; i < myData()->connectors.size(); ++i)
					deleteConnector(myData()->connectors[i]);
			}
			myData()->connectors.fastClear();
		}
	}

	// mac: 0x55EB18 (189B, batch584): refactor-layout twin of Orig (36B
	// stride at a rebind offset). Layout unmapped -- loud assert +
	// Orig fallthrough (same precedent as numConnectors).
	// STUB: PLAYER 0x00852050
	// STUB: STUDIO 0x009a9d00
	void BlockBlockContact::deleteAllConnectorsFFlag()
	{
		RBXASSERT(false && "BlockBlockNarrowPhaseRefactor true arm: refactor delete layout unmapped");
		deleteAllConnectorsOrig();
	}

	// mac: 0x55E1E0 (batch275).
	void Contact::generateDataForMovingAssemblyStage()
	{
		if (heapData == NULL)
		{
			heapData = new ContactData();
			heapData->elasticBase = 0.0f;
			heapData->elasticScaled = 0.0f;
			heapData->friction = 0.0f;
		}
		onPrimitiveContactParametersChanged();
	}

	// mac: 0x55EC88 (389B, batch308: cp assert :741; match-hit
	// fastRemove + body rebind; else pairMisses++ + pool-new +
	// ContactConnector C2 + kernel insert. Pool-new modeled as
	// plain new (pool DELTA, as with the D0)).
	// STUB: PLAYER 0x008537d0
	// STUB: STUDIO 0x009ab480
	GeoPairConnector* BlockBlockContactData::findGeoPairConnector(Body* b0, Body* b1,
		GeoPairType pairType, int id0, int id1)
	{
		// owner+44 is the heap triple, passed byte-blind as the
		// ContactParams (see Contact.h note; C2 copies 12B verbatim).
		const ContactData* triple = owner->getContactData();
		RBXASSERT(triple != NULL);  // Contact.cpp:741 "cp"
		GeoPairConnector* found = NULL;
		if (triple != NULL)
		{
			for (size_t n = 0; n < connectors.size(); ++n)
			{
				GeoPairConnector* c = connectors[n];
				if (c->match(b0, b1, pairType, id0, id1))
				{
					// dword_A4A786 hit stat unmapped (DELTA).
					connectors.fastRemove(n);
					c->rebindBodies(b0, b1);  // +144/+148 stores.
					return c;
				}
			}
			++BlockBlockContact::pairMisses;
			GeoPairConnector* fresh = new GeoPairConnector(b0, b1,
				reinterpret_cast<const ContactParams&>(*triple));
			Kernel* kernel = owner->getKernel();
			kernel->insertConnector(fresh);
			found = fresh;
		}
		return found;
	}

	// mac: 0x55EBD6 (178B, batch308: :414 myData && contactParams assert,
	// then the Data find; NULL when ungenerated).
	// STUB: PLAYER 0x00854510
	// STUB: STUDIO 0x009ac1c0
	GeoPairConnector* BlockBlockContact::findGeoPairConnector(Body* b0, Body* b1,
		GeoPairType pairType, int id0, int id1)
	{
		RBXASSERT(myData() != NULL && getContactData() != NULL);  // Contact.cpp:414
		if (myData() != NULL && getContactData() != NULL)
			return myData()->findGeoPairConnector(b0, b1, pairType, id0, id1);
		return NULL;
	}

	int BlockBlockContact::pairMisses = 0;

	// mac: 0x5612CC (batch302: myData null -> 0; FFlag true arm reads the
	// refactor-layout count (*(myData + 36*[myData+72] + 32)) whose
	// layout is unmapped -- loud assert + legacy fallthrough. Flag is
	// BSS-false; parsetest 256 pins it off).
	// STUB: PLAYER 0x008511e0
	// STUB: STUDIO 0x009a8ef0
	int BlockBlockContact::numConnectors() const
	{
		const BlockBlockContactData* data = myData();
		if (data == NULL)
			return 0;
		if (FFlag::BlockBlockNarrowPhaseRefactor)
		{
			RBXASSERT(false && "BlockBlockNarrowPhaseRefactor true arm: refactor count layout unmapped");
		}
		return data->numConnectors();
	}

	// STUB: PLAYER 0x00851f40
	// STUB: STUDIO 0x009a9bf0
	GeoPairConnector* BlockBlockContact::getConnector(int i)
	{
		return myData()->getConnector(i);
	}

	// Batch275: base triple + 148B solver block.
	void BlockBlockContact::generateDataForMovingAssemblyStage()
	{
		Contact::generateDataForMovingAssemblyStage();
		if (solverData == NULL)
		{
			solverData = new char[0x94]();
			int* block = reinterpret_cast<int*>(solverData);
			block[8] = 0;
			block[17] = 0;
			block[27] = 0;
			block[28] = 0;
			block[29] = 0;
			block[36] = reinterpret_cast<int>(this);
			block[30] = -1;
			block[31] = -1;
			block[18] = 0;
		}
	}

	// D2: solver block free (NULL-guarded).
	BlockBlockContact::~BlockBlockContact()
	{
		delete[] solverData;
		solverData = NULL;
	}

	// mac: 0x55E06E (317B, batch248: heap triple fill; the +324/+328/+332
	// per-prim callback triple queues with custom physical properties
	// -- flags stay false in-tree, so only the else arms run).
	void Contact::onPrimitiveContactParametersChanged()
	{
		if (heapData == NULL)
		{
			generateDataForMovingAssemblyStage();
			if (heapData == NULL)
				return;
		}
		Primitive* p0 = getPrim0();
		Primitive* p1 = getPrim1();
		heapData->friction = calculateFriction(p0->getFriction(), p1->getFriction());
		float minElastic = p0->getElasticity() < p1->getElasticity()
			? p0->getElasticity() : p1->getElasticity();
		float e0 = p0->contactElasticity316;
		float e1 = p1->contactElasticity316;
		if (p0->contactParamsDirty320)
		{
			// Callback-triple path (custom properties) -- queued; the
			// flag is never set in-tree (see Primitive.h note).
			RBXASSERT(false);
		}
		if (p1->contactParamsDirty320)
		{
			RBXASSERT(false);
		}
		float combined = e0 < e1 ? e0 : e1;
		heapData->elasticBase = combined;
		heapData->elasticScaled = combined * Constants::getElasticMultiplier(minElastic);
	}

} // namespace RBX

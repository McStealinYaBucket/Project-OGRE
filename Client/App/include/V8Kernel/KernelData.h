/* V8Kernel/KernelData.h -- October 2013: the kernel's lists, 11 IndexArrays (no buoyancy list), 0xb0. Their operator[]/fastAppend/fastRemove
   are out-of-line copies in Kernel.obj (PLAYER 0x0082e150-0x0082e9d0, 0x0082f760-0x00830620). Asserts on their 2013 lines (:38-:230). */
#pragma once

#include "Util/IndexArray.h"
#include "V8Kernel/Point.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/SimBody.h"
#include "V8Kernel/Connector.h"
#include "V8Kernel/ContactConnector.h"
#include "V8Kernel/Constants.h"
#include "Util/FastFlags.h"
namespace RBX {
	class SimBody; class Body; class Point; class Connector;
	// SIZE 0xb0
	class KernelData
	{
	public:
		IndexArray<SimBody, &SimBody::getFreeFallBodyIndex> freeFallBodies;           // 0x00
		IndexArray<SimBody, &SimBody::getRealTimeBodyIndex> realTimeBodies;           // 0x10
		IndexArray<SimBody, &SimBody::getJointBodyIndex> jointBodies;                 // 0x20
		IndexArray<SimBody, &SimBody::getContactBodyIndex> contactBodies;             // 0x30
		IndexArray<Body, &Body::getLeafBodyIndex> leafBodies;                         // 0x40
		IndexArray<Point, &Point::getKernelIndex> points;                             // 0x50
		IndexArray<Connector, &Connector::getHumanoidIndex> humanoidConnectors;       // 0x60
		IndexArray<Connector, &Connector::getSecondPassIndex> secondPassConnectors;   // 0x70
		IndexArray<Connector, &Connector::getRealTimeIndex> realTimeConnectors;       // 0x80
		IndexArray<Connector, &Connector::getJointIndex> jointConnectors;             // 0x90
		IndexArray<Connector, &Connector::getContactIndex> contactConnectors;         // 0xa0

		// FUNCTION: PLAYER 0x008310b0
		// FUNCTION: STUDIO 0x009a3710
		KernelData() {}
		// FUNCTION: PLAYER 0x0082f240
		// FUNCTION: STUDIO 0x009a1780
		~KernelData()
		{
			RBXASSERT(freeFallBodies.size() == 0);
			RBXASSERT(realTimeBodies.size() == 0);
			RBXASSERT(jointBodies.size() == 0);
			RBXASSERT(contactBodies.size() == 0);
			RBXASSERT(leafBodies.size() == 0);
			RBXASSERT(points.size() == 0);
			RBXASSERT(humanoidConnectors.size() == 0);
			RBXASSERT(secondPassConnectors.size() == 0);
			RBXASSERT(realTimeConnectors.size() == 0);
			RBXASSERT(jointConnectors.size() == 0);
			RBXASSERT(contactConnectors.size() == 0);
		}
		// FUNCTION: PLAYER 0x00831180
		// FUNCTION: STUDIO 0x009a37e0
		void addLeafBodies(Body* b)
		{
			for (int i = 0; i < b->numChildren(); ++i) {
				Body* child = b->getChild(i);
				RBXASSERT(!child->isLeafBody());
				RBXASSERT(child != child->getRoot());
				if (child->connectorUseCount > 0)
					addLeafBody(child);
				addLeafBodies(child);
			}
		}

		// FUNCTION: PLAYER 0x00831a60
		// FUNCTION: STUDIO 0x009a40c0
		void insertBody(Body* b)
		{
			RBXASSERT(b->getRoot() == b);
			SimBody* simBody = b->getSimBody();
			RBXASSERT(!b->isLeafBody() && !simBody->isInKernel());
			addBodyToNewList(simBody);
			RBXASSERT(simBody->validateBodyLists());
		}
		// FUNCTION: PLAYER 0x00831250
		// FUNCTION: STUDIO 0x009a38b0
		void removeBody(Body* b) {
			RBXASSERT(b->getRoot() == b);
			RBXASSERT(!b->isLeafBody());
			SimBody* simBody = b->getSimBody();
			removeBodyFromCurrentList(simBody);
			simBody->clearSymStateAndAccummulator();

			RBXASSERT(simBody->validateBodyLists());
		}
		// FUNCTION: PLAYER 0x00831ba0
		// FUNCTION: STUDIO 0x009a4200
		void addConnector(Connector* c) {
			RBXASSERT(!c->isInKernel());
			Body* body0 = c->getBody(Connector::body0);
			Body* body1 = c->getBody(Connector::body1);
			SimBody* simBody0 = body0 ? body0->getRootSimBody() : NULL;
			SimBody* simBody1 = body1 ? body1->getRootSimBody() : NULL;
			Connector::KernelType connectorType = c->getConnectorKernelType();
			if ((simBody0 == NULL || !simBody0->isInKernel())
				&& (simBody1 == NULL || !simBody1->isInKernel())
				&& connectorType != Connector::HUMANOID
				&& connectorType != Connector::JOINT
				&& connectorType != Connector::KERNEL_JOINT)
				return;
			if (connectorType == Connector::HUMANOID)
			{
				humanoidConnectors.fastAppend(c);
				if (simBody0)
					simBody0->updateConnectorCount(1, 1);
				if (simBody1)
					simBody1->updateConnectorCount(1, 1);
			}
			else if (connectorType == Connector::KERNEL_JOINT)
			{
				secondPassConnectors.fastAppend(c);
				if (simBody0)
					simBody0->updateConnectorCount(2, 1);
				if (simBody1)
					simBody1->updateConnectorCount(2, 1);
			}
			else if ((simBody0 && !simBody0->getBody()->getCanThrottle())
				|| (simBody1 && !simBody1->getBody()->getCanThrottle()))
			{
				realTimeConnectors.fastAppend(c);
				if (simBody0)
					simBody0->updateConnectorCount(3, 1);
				if (simBody1)
					simBody1->updateConnectorCount(3, 1);
			}
			else if (connectorType == Connector::JOINT || connectorType == Connector::BUOYANCY
				|| !FFlag::ImpulseSolverForContact
				|| (simBody0 && simBody0->isJointBody()) || (simBody1 && simBody1->isJointBody()))
			{
				jointConnectors.fastAppend(c);
				if (simBody0)
					simBody0->updateConnectorCount(4, 1);
				if (simBody1)
					simBody1->updateConnectorCount(4, 1);
			}
			else
			{
				RBXASSERT(connectorType == Connector::CONTACT);
				contactConnectors.fastAppend(c);
				if (simBody0) simBody0->updateConnectorCount(5, 1);
				if (simBody1) simBody1->updateConnectorCount(5, 1);
			}
			if (body0)
				addConnectorToBody(c, body0);
			if (body1)
				addConnectorToBody(c, body1);




			RBXASSERT(simBody0 == NULL || simBody0->validateBodyLists());
			RBXASSERT(simBody1 == NULL || simBody1->validateBodyLists());
		}


		// FUNCTION: PLAYER 0x00831f80
		// FUNCTION: STUDIO 0x009a45e0
		void removeConnector(Connector* c)
		{








			if (!c->isInKernel())
				return;
			Body* body0 = c->getBody(Connector::body0);
			Body* body1 = c->getBody(Connector::body1);
			SimBody* simBody0 = body0 ? body0->getRootSimBody() : NULL;
			SimBody* simBody1 = body1 ? body1->getRootSimBody() : NULL;
			if (c->isHumanoid()) {
				humanoidConnectors.fastRemove(c);
				if (simBody0) simBody0->updateConnectorCount(1, -1);
				if (simBody1) simBody1->updateConnectorCount(1, -1);
			}
			else if (c->isSecondPass()) {
				secondPassConnectors.fastRemove(c);
				if (simBody0) simBody0->updateConnectorCount(2, -1);
				if (simBody1) simBody1->updateConnectorCount(2, -1);
			}
			else if (c->isRealTime()) {
				realTimeConnectors.fastRemove(c);
				if (simBody0) simBody0->updateConnectorCount(3, -1);
				if (simBody1) simBody1->updateConnectorCount(3, -1);
			}
			else if (c->isJoint()) {
				jointConnectors.fastRemove(c);
				if (simBody0) simBody0->updateConnectorCount(4, -1);
				if (simBody1) simBody1->updateConnectorCount(4, -1);
			}
			else {
				RBXASSERT(c->isContact());
				contactConnectors.fastRemove(c);
				if (simBody0) simBody0->updateConnectorCount(5, -1);
				if (simBody1) simBody1->updateConnectorCount(5, -1);
				ContactConnector* conn = static_cast<ContactConnector*>(c);
				PairParams params = conn->getContactPoint();
				if (conn->getReordedSimBody(simBody0, simBody1, params))
					conn->applyContactPointForSymmetryDetection(simBody0, simBody1, params, -1.0f);
			}
			if (body0)
				removeConnectorFromBody(c, body0);
			if (body1)
				removeConnectorFromBody(c, body1);





			RBXASSERT(simBody0 == NULL || simBody0->validateBodyLists());
			RBXASSERT(simBody1 == NULL || simBody1->validateBodyLists());
		}
	private:
		// FUNCTION: PLAYER 0x008306e0
		// FUNCTION: STUDIO 0x009a2d40
		void addLeafBody(Body* b)
		{
			RBXASSERT(!b->isLeafBody());
			RBXASSERT(b->connectorUseCount > 0);
			leafBodies.fastAppend(b);
			RBXASSERT(b->isLeafBody());
		}
		// FUNCTION: PLAYER 0x008307d0
		void removeLeafBody(Body* b)
		{
			RBXASSERT(b->isLeafBody());
			leafBodies.fastRemove(b);
			RBXASSERT(!b->isLeafBody());
		}
		// FUNCTION: PLAYER 0x00830870
		// FUNCTION: STUDIO 0x009a2ed0
		void removeLeafBodies(Body* b)
		{
			for (int i = 0; i < b->numChildren(); ++i) {
				Body* child = b->getChild(i);
				if (child->isLeafBody())
					removeLeafBody(child);
				removeLeafBodies(child);
			}
		}
		// The connector's bodies join the leaf list when their root sim is a joint or realtime body.
		// FUNCTION: PLAYER 0x008314c0
		// FUNCTION: STUDIO 0x009a3b20
		void addConnectorToBody(Connector* c, Body* b)
		{
			b->connectorUseCount++;
			SimBody* simBody = b->getRootSimBody();
			if (simBody->isInKernel()) {
				addBodyToNewList(simBody);
				if (b != b->getRoot() && !b->isLeafBody()
					&& (simBody->isJointBody() || simBody->isRealTimeBody()))
					addLeafBody(b);
			}
		}
		// FUNCTION: PLAYER 0x00831530
		// FUNCTION: STUDIO 0x009a3b90
		void removeConnectorFromBody(Connector* c, Body* b)
		{
			b->connectorUseCount--;
			if (b->isLeafBody() && b->connectorUseCount == 0)
				removeLeafBody(b);
			SimBody* simBody = b->getRootSimBody();
			if (simBody->isInKernel())
				addBodyToNewList(simBody);
		}
		// List choice: not throttleable -> realTime; joint connectors -> joint; contact connectors -> contact
		// (ImpulseSolverForContact) or joint; no connectors -> freeFall (FreeFallOptimization) or joint.
		// FUNCTION: PLAYER 0x00831370
		// FUNCTION: STUDIO 0x009a39d0
		void addBodyToNewList(SimBody* simBody)
		{
			if (!simBody->getBody()->getCanThrottle()) {
				if (!simBody->isRealTimeBody()) {
					removeBodyFromCurrentList(simBody);
					realTimeBodies.fastAppend(simBody);
					simBody->setDt(Constants::kernelDt());
					addLeafBodies(simBody->getBody());
				}
			}
			else if (simBody->numConnectors[4] > 0) {
				if (!simBody->isJointBody()) {
					removeBodyFromCurrentList(simBody);
					jointBodies.fastAppend(simBody);
					simBody->setDt(Constants::kernelDt());
					addLeafBodies(simBody->getBody());
				}
			}
			else if (simBody->numConnectors[5] > 0) {
				if (FFlag::ImpulseSolverForContact) {
					if (!simBody->isContactBody()) {
						removeBodyFromCurrentList(simBody);
						contactBodies.fastAppend(simBody);
						simBody->setDt(Constants::freeFallDt());
					}
				}
				else if (!simBody->isJointBody()) {
					removeBodyFromCurrentList(simBody);
					jointBodies.fastAppend(simBody);
					simBody->setDt(Constants::kernelDt());
					addLeafBodies(simBody->getBody());
				}
			}
			else if (FFlag::FreeFallOptimization && simBody->numConnectors[0] == 0) {
				if (!simBody->isFreeFallBody()) {
					removeBodyFromCurrentList(simBody);
					freeFallBodies.fastAppend(simBody);
					simBody->setDt(Constants::freeFallDt());
					simBody->clearSymStateAndAccummulator();
				}
			}
			else if (!simBody->isJointBody()) {
				removeBodyFromCurrentList(simBody);
				jointBodies.fastAppend(simBody);
				simBody->setDt(Constants::kernelDt());
				addLeafBodies(simBody->getBody());
			}
		}
		// FUNCTION: PLAYER 0x008308b0
		// FUNCTION: STUDIO 0x009a2f10
		void removeBodyFromCurrentList(SimBody* simBody)
		{
			if (simBody->isRealTimeBody()) {
				removeLeafBodies(simBody->getBody());
				realTimeBodies.fastRemove(simBody);
				simBody->setDt(0.0f);
			}
			else if (simBody->isJointBody()) {
				removeLeafBodies(simBody->getBody());
				jointBodies.fastRemove(simBody);
				simBody->setDt(0.0f);
			}
			else if (simBody->isContactBody()) {
				contactBodies.fastRemove(simBody);
				simBody->setDt(0.0f);
			}
			else if (simBody->isFreeFallBody()) {
				freeFallBodies.fastRemove(simBody);
				simBody->updateAngMomentum();
				simBody->setDt(0.0f);
			}
		}
	};
	// The IndexArray instances KernelData and Kernel call (Util/IndexArray.h bodies; the Connector realTime ones fold
	// with IndexedTree's: operator[] const 0x0082e7d0, fastRemove 0x00830340, fastAppend 0x009997a0).
	// TEMPLATE: PLAYER 0x0082e150
	// TEMPLATE: STUDIO 0x009a0710
	// ??A?$IndexArray@VSimBody@RBX@@$1?getFreeFallBodyIndex@12@QAEAAHXZ@RBX@@QAEPAVSimBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e1d0
	// TEMPLATE: STUDIO 0x009a0790
	// ??A?$IndexArray@VSimBody@RBX@@$1?getRealTimeBodyIndex@12@QAEAAHXZ@RBX@@QAEPAVSimBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e250
	// TEMPLATE: STUDIO 0x009a0810
	// ??A?$IndexArray@VSimBody@RBX@@$1?getRealTimeBodyIndex@12@QAEAAHXZ@RBX@@QBEQAVSimBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e2d0
	// TEMPLATE: STUDIO 0x009a0890
	// ??A?$IndexArray@VSimBody@RBX@@$1?getJointBodyIndex@12@QAEAAHXZ@RBX@@QAEPAVSimBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e350
	// TEMPLATE: STUDIO 0x009a0910
	// ??A?$IndexArray@VSimBody@RBX@@$1?getJointBodyIndex@12@QAEAAHXZ@RBX@@QBEQAVSimBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e3d0
	// TEMPLATE: STUDIO 0x009a0990
	// ??A?$IndexArray@VSimBody@RBX@@$1?getContactBodyIndex@12@QAEAAHXZ@RBX@@QAEPAVSimBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e450
	// TEMPLATE: STUDIO 0x009a0a10
	// ??A?$IndexArray@VBody@RBX@@$H?getLeafBodyIndex@12@QAEAAHXZA@@RBX@@QBEQAVBody@1@H@Z
	// TEMPLATE: PLAYER 0x0082e4d0
	// TEMPLATE: STUDIO 0x009a0a90
	// ??A?$IndexArray@VPoint@RBX@@$H?getKernelIndex@12@QAEAAHXZA@@RBX@@QAEPAVPoint@1@H@Z
	// TEMPLATE: PLAYER 0x0082e550
	// TEMPLATE: STUDIO 0x009a0b10
	// ??A?$IndexArray@VPoint@RBX@@$H?getKernelIndex@12@QAEAAHXZA@@RBX@@QBEQAVPoint@1@H@Z
	// TEMPLATE: PLAYER 0x0082e5d0
	// TEMPLATE: STUDIO 0x009a0b90
	// ??A?$IndexArray@VConnector@RBX@@$1?getHumanoidIndex@12@QAEAAHXZ@RBX@@QBEQAVConnector@1@H@Z
	// TEMPLATE: PLAYER 0x0082e6d0
	// TEMPLATE: STUDIO 0x009a0c90
	// ??A?$IndexArray@VConnector@RBX@@$1?getSecondPassIndex@12@QAEAAHXZ@RBX@@QBEQAVConnector@1@H@Z
	// TEMPLATE: PLAYER 0x0082e7d0
	// TEMPLATE: STUDIO 0x00665e40
	// ??A?$IndexArray@VConnector@RBX@@$1?getRealTimeIndex@12@QAEAAHXZ@RBX@@QBEQAVConnector@1@H@Z
	// TEMPLATE: PLAYER 0x0082e8d0
	// TEMPLATE: STUDIO 0x009a0e10
	// ??A?$IndexArray@VConnector@RBX@@$1?getJointIndex@12@QAEAAHXZ@RBX@@QBEQAVConnector@1@H@Z
	// TEMPLATE: PLAYER 0x0082e950
	// TEMPLATE: STUDIO 0x009a0e90
	// ??A?$IndexArray@VConnector@RBX@@$1?getContactIndex@12@QAEAAHXZ@RBX@@QAEPAVConnector@1@H@Z
	// TEMPLATE: PLAYER 0x0082e9d0
	// TEMPLATE: STUDIO 0x009a0f10
	// ??A?$IndexArray@VConnector@RBX@@$1?getContactIndex@12@QAEAAHXZ@RBX@@QBEQAVConnector@1@H@Z
	// TEMPLATE: PLAYER 0x0082f760
	// TEMPLATE: STUDIO 0x009a1d10
	// ?fastAppend@?$IndexArray@VSimBody@RBX@@$1?getFreeFallBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082f810
	// TEMPLATE: STUDIO 0x009a1dc0
	// ?fastRemove@?$IndexArray@VSimBody@RBX@@$1?getFreeFallBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082f8e0
	// TEMPLATE: STUDIO 0x009a1e90
	// ?fastAppend@?$IndexArray@VSimBody@RBX@@$1?getRealTimeBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082f990
	// TEMPLATE: STUDIO 0x009a1f40
	// ?fastRemove@?$IndexArray@VSimBody@RBX@@$1?getRealTimeBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fa60
	// TEMPLATE: STUDIO 0x009a2010
	// ?fastAppend@?$IndexArray@VSimBody@RBX@@$1?getJointBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fb10
	// TEMPLATE: STUDIO 0x009a20c0
	// ?fastRemove@?$IndexArray@VSimBody@RBX@@$1?getJointBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fbe0
	// TEMPLATE: STUDIO 0x009a2190
	// ?fastAppend@?$IndexArray@VSimBody@RBX@@$1?getContactBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fc90
	// TEMPLATE: STUDIO 0x009a2240
	// ?fastRemove@?$IndexArray@VSimBody@RBX@@$1?getContactBodyIndex@12@QAEAAHXZ@RBX@@QAEXPAVSimBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fd60
	// TEMPLATE: STUDIO 0x009a2310
	// ?fastAppend@?$IndexArray@VBody@RBX@@$H?getLeafBodyIndex@12@QAEAAHXZA@@RBX@@QAEXPAVBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fe10
	// TEMPLATE: STUDIO 0x009a23c0
	// ?fastRemove@?$IndexArray@VBody@RBX@@$H?getLeafBodyIndex@12@QAEAAHXZA@@RBX@@QAEXPAVBody@2@@Z
	// TEMPLATE: PLAYER 0x0082fee0
	// TEMPLATE: STUDIO 0x009a2490
	// ?fastAppend@?$IndexArray@VPoint@RBX@@$H?getKernelIndex@12@QAEAAHXZA@@RBX@@QAEXPAVPoint@2@@Z
	// TEMPLATE: PLAYER 0x0082ff90
	// TEMPLATE: STUDIO 0x009a2540
	// ?fastRemove@?$IndexArray@VPoint@RBX@@$H?getKernelIndex@12@QAEAAHXZA@@RBX@@QAEXPAVPoint@2@@Z
	// TEMPLATE: PLAYER 0x00830060
	// TEMPLATE: STUDIO 0x009a2610
	// ?fastAppend@?$IndexArray@VConnector@RBX@@$1?getHumanoidIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x00830110
	// TEMPLATE: STUDIO 0x009a26c0
	// ?fastRemove@?$IndexArray@VConnector@RBX@@$1?getHumanoidIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x008301d0
	// TEMPLATE: STUDIO 0x009a2780
	// ?fastAppend@?$IndexArray@VConnector@RBX@@$1?getSecondPassIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x00830280
	// TEMPLATE: STUDIO 0x009a2830
	// ?fastRemove@?$IndexArray@VConnector@RBX@@$1?getSecondPassIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x00830400
	// TEMPLATE: STUDIO 0x009a2a60
	// ?fastAppend@?$IndexArray@VConnector@RBX@@$1?getJointIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x008304b0
	// TEMPLATE: STUDIO 0x009a2b10
	// ?fastRemove@?$IndexArray@VConnector@RBX@@$1?getJointIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x00830570
	// TEMPLATE: STUDIO 0x009a2bd0
	// ?fastAppend@?$IndexArray@VConnector@RBX@@$1?getContactIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x00830620
	// TEMPLATE: STUDIO 0x009a2c80
	// ?fastRemove@?$IndexArray@VConnector@RBX@@$1?getContactIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z
	// TEMPLATE: PLAYER 0x009997a0
	// TEMPLATE: STUDIO 0x009a28f0
	// ?fastAppend@?$IndexArray@VConnector@RBX@@$1?getRealTimeIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z

} // namespace RBX

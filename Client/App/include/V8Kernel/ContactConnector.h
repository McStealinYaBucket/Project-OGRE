/* V8Kernel/ContactConnector.h -- October 2013 (contact connector).
   Evidence: C2 0x... (batch275, full member map below) / removeConnector
   symmetry pair (0x52E2B9/0x52E2DF, batch221b: member calls) /
   getConnectorKernelType 0x529F18 / getBody 0x529F28 (batch256).
   Member map from the C2 stores (2016 names where roles agree):
     Connector base +0..+24 (vptr + 5 indices, C2 -1)
     +24  age (C2 0)
     +28..+96 impulse-cache region (C2 does NOT store here -- left
       indeterminate, binary-exact; Matrix3 members queue with the
       impulse solver -- layout preserved, names pending)
     +100 inverseMass (0.0) / +104 penetrationVelocity (0.0) /
       +108 reboundVelocity (0.0) / +112 impulseComputed (false)
     +116 GeoPair geoPair (C2; bodies INSIDE at +144/+148, 2016 shape;
       total 56B ending +172)
     +172 ContactParams contactParams (12B C2 copy)
     +184 PairParams oldContactPoint (C2 zero)
     +212 PairParams contactPoint (C2 zero; removeConnector's v33+v34
       copy source -- the +212 binary offset confirmed)
     +240 firstApproach (0.0) / +244 threshold (0.0) /
       +248 forceMagLast (0.0)
     +252 frictionOffset Vector3 (C2 zero)
   DELTA vs a fully-mapped C2: the +28..+96 Matrix3 impulse caches
   (deltaVelPerUnitImpulse/impulsePerUnitDeltaVel) are unmapped pads
   for now -- the binary C2 never stores there either. Full Matrix3
   members queue with the impulse solver (stepper era). Layout size
   preserved via explicit carbon-copy pads.
   C2 0x69C428 banked batch244 (full init queues with generate-data).
   Original file: Client/App/include/V8Kernel/ContactConnector.h (path
   from 2016). */

#pragma once

#include "V8Kernel/Connector.h"
#include "V8Kernel/Pair.h"
#include "V8Kernel/ContactParams.h"
#include "G3D/Matrix3.h"

namespace RBX {

	class SimBody;
	class Body;
	class GeoPairConnector;

	class ContactConnector : public Connector
	{
		friend class GeoPairConnector;
	public:
		// C2 takes the contact params (12B copy at +172); defaulted so
		// tests construct without material data (binary callers always
		// pass real params -- generate-data era fills them).
		ContactConnector(Body* b0, Body* b1,
			const ContactParams& params = ContactParams());

		// STUB: PLAYER 0x0096bde0
		virtual KernelType getConnectorKernelType() const { return CONTACT; }
		virtual Body* getBody(BodyIndex id);

		// mac: 0x527CDE (batch256: sims from bodies + :65 assert + reorder
		// with normal negation; returns false only when NEITHER sim
		// is in kernel).
		bool getReordedSimBody(SimBody*& simBody0, SimBody*& simBody1,
			Body*& bodyNotInKernel, PairParams& params);
		// mac: 0x527E42 (batch256: forwards with null Body*& scratch).
		bool getReordedSimBody(SimBody*& simBody0, SimBody*& simBody1,
			PairParams& params);
		// mac: 0x529422 (batch256: mass-weighted normal*length split into
		// penetrationForce (+296) + r-cross-F into penetrationTorque
		// (+284), opposite signs per sim; 1e-12 gate).
		void applyContactPointForSymmetryDetection(SimBody* simBody0,
			SimBody* simBody1, const PairParams& params, float direction);

		PairParams& getContactPoint() { return contactPoint; }
		const PairParams& getContactPoint() const { return contactPoint; }
		// mac: 0x5296B2 (batch290: reorder + symmetric apply pair + sync
		// into oldContactPoint when listed and diverged). Virtual per
		// the slot-[11] vtable entry (no overrider in-tree).
		virtual void updateContactPoint();
		// 2016 shapes (first-party; binary EAs queued unless noted).
		static float overlapGoal() { return 0.01f; }
		virtual bool canThrottle() const;
		void reset();
		float computeRelativeVelocity(const PairParams& params,
			G3D::Vector3* deltaVnormal, G3D::Vector3* perpVel);
		float computeRelativeVelocity();
		// 2016 shape (symmetric-contact reorder helper for computeImpulse).
		bool getSimBodyAndContactVelocity(SimBody*& simBody0, SimBody*& simBody1, PairParams& params,
			float& normalVel, G3D::Vector3& perpVel);
		bool isRestingContact() { return age > 4; }
		// mac: 0x528126 (batch290: friction + normal solver; 2016 text).
		virtual void computeForce(bool throttling);
		// mac: 0x528788 (batch290; 2016 computeImpulse text).
		virtual bool computeImpulse(float& residualVelocity);
		static float percentActive();

	private:
		static int inContactHit;
		static int outOfContactHit;

	private:
		int age;  // +24 (C2 0).
		// Impulse cache (binary-pinned 92B block +24..+115 with 2016
		// member set: age + 2x Matrix3 + inverseMass + penetration pair
		// + impulseComputed; the old char[68] was 4B short).
		G3D::Matrix3 deltaVelPerUnitImpulse;  // +28.
		G3D::Matrix3 impulsePerUnitDeltaVel;  // +64.
		float inverseMass;          // +100 (C2 0.0f).
		float penetrationVelocity;  // +104 (C2 0.0f).
		float reboundVelocity;      // +108 (C2 0.0f).
		bool impulseComputed;       // +112 (C2 false).
		GeoPair geoPair;            // +116 (C2; bodies at +144/+148
		                                // live INSIDE, 2016 shape).
		ContactParams contactParams;  // +172 (12B C2 copy).
		PairParams oldContactPoint;   // +184 (C2 zero).
		PairParams contactPoint;      // +212 (C2 zero).
		float firstApproach;   // +240 (C2 0.0f).
		float threshold;       // +244 (C2 0.0f).
		float forceMagLast;    // +248 (C2 0.0f).
		G3D::Vector3 frictionOffset;  // +252 (C2 zero).
	};

	// 2016 shape (first-party): no new members (D1 0x562FB4 empty);
	// Allocator<GeoPairConnector> base unmodeled (pool delete DELTA).
	// No out-of-line C1/C2 exists DB-wide (verified 2026-10-06) --
	// the trivial base-forwarding ctor is inline everywhere, as here.
	class GeoPairConnector : public ContactConnector
	{
	public:
		GeoPairConnector(Body* b0, Body* b1,
			const ContactParams& params = ContactParams())
			: ContactConnector(b0, b1, params)
		{}
		// mac: 0x562FD0 (177B, batch308: type-dispatched compute* into
		// contactPoint, then the base updateContactPoint always).
		/*override*/ void updateContactPoint();
		// 2016 shapes (first-party forwarders into geoPair).
		void setPointPlane(const G3D::Vector3& offsetPoint,
			const G3D::Vector3& offsetPlane, int pointID, int planeNormalID)
		{
			geoPair.setPointPlane(offsetPoint, offsetPlane, pointID, planeNormalID);
		}
		void setEdgeEdgePlane(const G3D::Vector3& edge0, const G3D::Vector3& edge1,
			int normal0, int normal1, int planeID, float edgeLength0Arg, float edgeLength1Arg)
		{
			geoPair.setEdgeEdgePlane(edge0, edge1, normal0, normal1, planeID,
				edgeLength0Arg, edgeLength1Arg);
		}
		void setEdgeEdge(const G3D::Vector3& edge0, const G3D::Vector3& edge1,
			int normal0, int normal1)
		{
			geoPair.setEdgeEdge(edge0, edge1, normal0, normal1);
		}
		bool match(Body* b0, Body* b1, GeoPairType pairType, int id0, int id1)
		{
			return geoPair.match(b0, b1, pairType, id0, id1);
		}
		// +144/+148 body rebind for the Data find-hit arm (binary
		// stores directly; member fn keeps the friend list narrow).
		void rebindBodies(Body* b0, Body* b1)
		{
			geoPair.body0 = b0;
			geoPair.body1 = b1;
		}
	};

} // namespace RBX

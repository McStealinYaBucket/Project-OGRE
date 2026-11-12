/* App/v8kernel/ContactConnector.cpp -- October 2013 (contact glue).
   Bodies: C2 (batch275 full-store map) / getConnectorKernelType 0x529F18 /
   getBody 0x529F28 (batch256) / getReordedSimBody pair 0x527CDE/0x527E42 /
   applyContactPointForSymmetryDetection 0x529422 (batch256). */

#include "stdafx.h"

#include "V8Kernel/ContactConnector.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/SimBody.h"
#include "V8Kernel/Pair.h"
#include "V8Kernel/ContactParams.h"
#include "V8Kernel/Constants.h"
#include "Util/Math.h"

#include <algorithm>
#include <cmath>

namespace RBX {

	// Impulse-solver tunables (2016 file-scope consts).
	static const float REST_THRESHOLD = 1.0f;
	static const float PENETRATION_BIAS_WEIGHT = 0.1f;
	static const float FRICTION_GAIN = 1.2f;
	static const float SYM_CONTACT_MAX_TANGENT_VEL = 1.0f;
	static const float SYM_CONTACT_MAX_NORMAL_VEL = 60.0f;

	// Full-store C2 (batch275: indices -1 + vptr + zero pages + bodies
	// from args + ContactParams copy; GeoPair sub-C2 landed batch276).
	ContactConnector::ContactConnector(Body* b0, Body* b1,
		const ContactParams& params)
		: age(0)
		, inverseMass(0.0f)
		, penetrationVelocity(0.0f)
		, reboundVelocity(0.0f)
		, impulseComputed(false)
		, contactParams(params)
		, firstApproach(0.0f)
		, threshold(0.0f)
		, forceMagLast(0.0f)
	{
		geoPair.body0 = b0;
		geoPair.body1 = b1;
		// contactParams/contactPoint pages default via their own ctors
		// (zero); frictionOffset zero. The +28..+96 impulse region is
		// deliberately untouched (binary C2 never stores there either).
	}

	// FUNCTION: PLAYER 0x00850c90
	// FUNCTION: STUDIO 0x009a8980
	Body* ContactConnector::getBody(BodyIndex id)
	{
		return (id == body0) ? geoPair.body0 : geoPair.body1;
	}

	// mac: 0x527CDE (batch256).
	// STUB: PLAYER 0x0084e8b0
	// STUB: STUDIO 0x009a65a0
	bool ContactConnector::getReordedSimBody(SimBody*& simBody0, SimBody*& simBody1,
		Body*& bodyNotInKernel, PairParams& params)
	{
		simBody0 = geoPair.body0->getRoot<Body>()->getSimBody();
		simBody1 = geoPair.body1->getRoot<Body>()->getSimBody();
		bodyNotInKernel = geoPair.body0;
		RBXASSERT(simBody0 && simBody1);  // :65
		bool in0 = simBody0->isInKernel();
		bool in1 = simBody1->isInKernel();
		if (in0)
		{
			if (!in1)
			{
				simBody1 = NULL;
				bodyNotInKernel = geoPair.body1;
			}
			return true;
		}
		if (in1)
		{
			simBody0 = simBody1;
			simBody1 = NULL;
			params.normal = -params.normal;
			return true;
		}
		return false;
	}

	// mac: 0x527E42 (batch256: null Body*& scratch forward).
	// FUNCTION: PLAYER 0x0084ea10
	// FUNCTION: STUDIO 0x009a6700
	bool ContactConnector::getReordedSimBody(SimBody*& simBody0, SimBody*& simBody1,
		PairParams& params)
	{
		Body* scratch = NULL;
		return getReordedSimBody(simBody0, simBody1, scratch, params);
	}

	// mac: 0x562FD0 (177B, batch308: compute* by geoPairType, then the
	// base updateContactPoint unconditionally (tail jmp -- runs on
	// every arm including default-after-assert)).
	// STUB: PLAYER 0x00852c00
	// STUB: STUDIO 0x009aa8b0
	void GeoPairConnector::updateContactPoint()
	{
		switch (geoPair.geoPairType)
		{
		case EDGE_EDGE_PAIR:
			geoPair.computeEdgeEdge(contactPoint);
			break;
		case EDGE_EDGE_PLANE_PAIR:
			geoPair.computeEdgeEdgePlane2(contactPoint);
			break;
		case POINT_PLANE_PAIR:
			geoPair.computePointPlane(contactPoint);
			break;
		default:
			RBXASSERT(false);  // Pair.h:81 "0"
			break;
		}
		ContactConnector::updateContactPoint();
	}

	// mac: 0x5296B2 (batch290).
	// STUB: PLAYER 0x0084ea30
	// STUB: STUDIO 0x009a6720
	void ContactConnector::updateContactPoint()
	{
		if (isContact() && !(contactPoint == oldContactPoint))
		{
			PairParams params = contactPoint;
			SimBody* sim0 = NULL;
			SimBody* sim1 = NULL;
			Body* scratch = NULL;
			if (getReordedSimBody(sim0, sim1, scratch, params))
				applyContactPointForSymmetryDetection(sim0, sim1, params, -1.0f);
			params = contactPoint;
			sim0 = NULL;
			sim1 = NULL;
			scratch = NULL;
			if (getReordedSimBody(sim0, sim1, scratch, params))
				applyContactPointForSymmetryDetection(sim0, sim1, params, 1.0f);
			oldContactPoint = contactPoint;
		}
	}
	void ContactConnector::applyContactPointForSymmetryDetection(SimBody* simBody0,
		SimBody* simBody1, const PairParams& params, float direction)
	{
		float m0 = simBody0->massRecip;
		float total = m0;
		if (simBody1 != NULL)
			total += simBody1->massRecip;
		float f0 = m0 / total;
		float nxL = params.normal.x * params.length;
		float nyL = params.normal.y * params.length;
		float nzL = params.length * params.normal.z;
		float w0x = nxL * f0;
		float w0y = nyL * f0;
		float w0z = f0 * nzL;
		if (w0x * w0x + w0y * w0y + w0z * w0z > 1.0e-12f)
		{
			float fx = w0x * direction;
			float fy = w0y * direction;
			float fz = w0z * direction;
			simBody0->penetrationForce.x += fx;
			simBody0->penetrationForce.y += fy;
			simBody0->penetrationForce.z += fz;
			G3D::Vector3 r = params.position - simBody0->pvFrame.translation;
			simBody0->penetrationTorque += r.cross(G3D::Vector3(fx, fy, fz));
		}
		if (simBody1 != NULL)
		{
			float m1 = simBody1->massRecip;
			float f1 = m1 / total;
			float w1x = nxL * f1;
			float w1y = nyL * f1;
			float w1z = nzL * f1;
			if (w1x * w1x + w1y * w1y + w1z * w1z > 1.0e-12f)
			{
				float fx = w1x * direction;
				float fy = w1y * direction;
				float fz = w1z * direction;
				simBody1->penetrationForce.x -= fx;
				simBody1->penetrationForce.y -= fy;
				simBody1->penetrationForce.z -= fz;
				G3D::Vector3 r = params.position - simBody1->pvFrame.translation;
				simBody1->penetrationTorque -= r.cross(G3D::Vector3(fx, fy, fz));
			}
		}
	}

	int ContactConnector::inContactHit = 0;
	int ContactConnector::outOfContactHit = 0;

	float ContactConnector::percentActive()
	{
		int denom = inContactHit + outOfContactHit;
		float answer = (denom == 0) ? -1.0f
			: (float)(inContactHit) / (float)(denom);
		inContactHit = 0;
		outOfContactHit = 0;
		return answer;
	}

	// 2016 shape (both bodies' throttle AND).
	bool ContactConnector::canThrottle() const
	{
		return geoPair.body0->getCanThrottle() && geoPair.body1->getCanThrottle();
	}

	void ContactConnector::reset()
	{
		firstApproach = 0.0f;
		threshold = 0.0f;
		forceMagLast = 0.0f;
		age = 0;
		penetrationVelocity = 0.0f;
		reboundVelocity = 0.0f;
	}

	// 2016 shape (body linear velocities at the contact point;
	// 2016 reads getPvUnsafe (updatePV + PV); spelled on members here --
	// linearVelocityAtPoint(pos) == linearVelocityAtOffset(pos - frame)).
	float ContactConnector::computeRelativeVelocity(const PairParams& params,
		G3D::Vector3* deltaVnormal, G3D::Vector3* perpVel)
	{
		Body* b0 = geoPair.body0;
		Body* b1 = geoPair.body1;
		const G3D::CoordinateFrame& cf0 = b0->getCoordinateFrame();
		const G3D::CoordinateFrame& cf1 = b1->getCoordinateFrame();
		G3D::Vector3 velocity0 = b0->getVelocity().linearVelocityAtOffset(params.position - cf0.translation);
		G3D::Vector3 velocity1 = b1->getVelocity().linearVelocityAtOffset(params.position - cf1.translation);
		G3D::Vector3 deltaV = velocity1 - velocity0;
		float normalVel = params.normal.dot(deltaV);
		if (deltaVnormal != NULL)
			*deltaVnormal = params.normal * normalVel;
		if (perpVel != NULL)
			*perpVel = deltaV - params.normal * normalVel;
		return normalVel;
	}

	// 2016 shape.
	float ContactConnector::computeRelativeVelocity()
	{
		updateContactPoint();
		return -computeRelativeVelocity(contactPoint, NULL, NULL);
	}

	// 2016 shape (reorder + relative velocity across the contact).
	bool ContactConnector::getSimBodyAndContactVelocity(SimBody*& simBody0, SimBody*& simBody1,
		PairParams& params, float& normalVel, G3D::Vector3& perpVel)
	{
		Body* bodyNotInKernel = NULL;
		if (!getReordedSimBody(simBody0, simBody1, bodyNotInKernel, params))
		{
			outOfContactHit++;
			return false;
		}

		RBXASSERT(simBody0 != NULL || simBody1 != NULL);
		inContactHit++;

		G3D::Vector3 deltaV = -simBody0->getPV().linearVelocityAtPoint(params.position);
		if (simBody1 != NULL)
			deltaV += simBody1->getPV().linearVelocityAtPoint(params.position);
		else
		{
			PV bpv(bodyNotInKernel->getCoordinateFrame(), bodyNotInKernel->getVelocity());
			deltaV += bpv.linearVelocityAtPoint(params.position);
		}

		normalVel = params.normal.dot(deltaV);
		perpVel = deltaV - params.normal * normalVel;

		return true;
	}

	// mac: 0x528126 (batch290; 2016 computeForce text -- binary-identical
	// structure including the friction cone, decay, gating and
	// per-sim force application).
	// STUB: PLAYER 0x0084f5a0
	// STUB: STUDIO 0x009a7290
	void ContactConnector::computeForce(bool throttling)
	{
		RBXASSERT(!throttling || !canThrottle());  // :119
		updateContactPoint();
		const PairParams& params = contactPoint;

		if (params.length >= 0.0f)
		{
			outOfContactHit++;
			reset();
			return;
		}

		inContactHit++;

		G3D::Vector3 deltaVnormal, perpVel;
		float normalVel = computeRelativeVelocity(params, &deltaVnormal, &perpVel);

		frictionOffset += perpVel * Constants::kernelDt();

		float frictionSpringK = contactParams.kSpring * 0.2f;
		float currentForce = frictionOffset.length() * frictionSpringK;
		float maxForce = forceMagLast * contactParams.kFriction;
		if ((currentForce > maxForce) && (currentForce > 1e-8f)) {
			frictionOffset *= (maxForce / currentForce);
		}
		frictionOffset -= params.normal * (params.normal.dot(frictionOffset));

		threshold = (threshold != 0.0f)
			? ((-overlapGoal()) + ((threshold + overlapGoal()) * 0.999f))
			: ((firstApproach == 0.0f) ? 0.0f : params.length);
		firstApproach = (firstApproach != 0.0f)
			? ((-overlapGoal()) + ((firstApproach + overlapGoal()) * 0.999f))
			: params.length;

		float kApplied = (normalVel < 0.0f) ? contactParams.kSpring : contactParams.kNeg;
		forceMagLast = kApplied * (firstApproach - params.length);
		forceMagLast = (params.length <= threshold) ? forceMagLast : 0.0f;

		G3D::Vector3 force = (forceMagLast * params.normal) - (frictionOffset * frictionSpringK);

		geoPair.body0->accumulateForce(-force, params.position);
		geoPair.body1->accumulateForce(force, params.position);
		RBXASSERT(fabs(force.x) < Math::inf());  // :160 (binary checks .x only)
	}

	// mac: 0x528788 (batch290; 2016 computeImpulse text).
	// STUB: PLAYER 0x0084fa20
	// STUB: STUDIO 0x009a7710
	bool ContactConnector::computeImpulse(float& residualVelocity)
	{
		RBXASSERT(geoPair.body0->getRootSimBody()->isInKernel()
			|| geoPair.body1->getRootSimBody()->isInKernel());

		PairParams params = contactPoint;

		if (params.length >= 0.0f)
		{
			outOfContactHit++;
			reset();
			return false;
		}

		SimBody* simBody0;
		SimBody* simBody1;
		float normalVel;
		G3D::Vector3 perpVel;

		if (!getSimBodyAndContactVelocity(simBody0, simBody1, params, normalVel, perpVel))
			return false;

		bool symetricContact = true;
		bool verticalSymetricContact = true;
		float normalVelSize = fabs(normalVel);
		bool velocityOutOfRange = (normalVelSize > SYM_CONTACT_MAX_NORMAL_VEL
			|| perpVel.squaredMagnitude() > SYM_CONTACT_MAX_TANGENT_VEL);

		if (simBody0->isSymmetricContact())
		{
			if (velocityOutOfRange)
			{
				simBody0->clearSymmetricContact();
				symetricContact = verticalSymetricContact = false;
			}
			else if (!simBody0->isVerticalContact())
				verticalSymetricContact = false;
		}
		else
			symetricContact = verticalSymetricContact = false;

		if (simBody1 != NULL)
		{
			if (simBody1->isSymmetricContact())
			{
				if (velocityOutOfRange)
				{
					simBody1->clearSymmetricContact();
					symetricContact = verticalSymetricContact = false;
				}
				else if (!simBody1->isVerticalContact())
					verticalSymetricContact = false;
			}
			else
				symetricContact = verticalSymetricContact = false;
		}

		if (normalVel >= 0.0f && (!symetricContact || age == 0))
			return false;

		if (!impulseComputed)
		{
			age++;
			inverseMass = simBody0->getMassRecip();
			G3D::Matrix3 contactPositionSkew =
				Math::toSkewSymmetric(params.position - simBody0->getPV().position.translation);
			deltaVelPerUnitImpulse = -contactPositionSkew * simBody0->getInverseInertiaInWorld() * contactPositionSkew;

			if (simBody1 != NULL)
			{
				inverseMass += simBody1->getMassRecip();
				contactPositionSkew =
					Math::toSkewSymmetric(params.position - simBody1->getPV().position.translation);
				deltaVelPerUnitImpulse -= contactPositionSkew * simBody1->getInverseInertiaInWorld() * contactPositionSkew;
			}

			deltaVelPerUnitImpulse += Math::fromDiagonal(G3D::Vector3(inverseMass, inverseMass, inverseMass));

			bool revertable = deltaVelPerUnitImpulse.inverse(impulsePerUnitDeltaVel, 1e-20);
			RBXASSERT(revertable);
		}

		float desiredDeltaVelocity = -normalVel;

		float velocityThisFrame = simBody0->getMassRecip() * simBody0->getImpulseLast().dot(params.normal);
		if (simBody1 != NULL)
			velocityThisFrame -= simBody1->getMassRecip() * simBody1->getImpulseLast().dot(params.normal);

		if (!impulseComputed)
		{
			reboundVelocity = 0.0f;
			if (-normalVel >= REST_THRESHOLD && !isRestingContact() && !symetricContact &&
				velocityThisFrame >= 0.0f && velocityThisFrame <= -normalVel)
			{
				float entranceVelocity = normalVel + velocityThisFrame;
				RBXASSERT(entranceVelocity <= 0.0f);
				reboundVelocity = -contactParams.kNeg / contactParams.kSpring * entranceVelocity;
				RBXASSERT(reboundVelocity >= 0.0f);
			}

			float penetrationBias = std::max(0.0f, -params.length - overlapGoal() * 2.0f)
				/ Constants::freeFallDt() * PENETRATION_BIAS_WEIGHT;
			if (reboundVelocity < penetrationBias)
				reboundVelocity = penetrationBias;

			penetrationVelocity = std::max(1.0f, normalVelSize);
			impulseComputed = true;
		}
		desiredDeltaVelocity += reboundVelocity;

		residualVelocity += fabs(desiredDeltaVelocity / penetrationVelocity);

		G3D::Vector3 velocityToKill = desiredDeltaVelocity * params.normal - perpVel;
		G3D::Vector3 impulse = impulsePerUnitDeltaVel * velocityToKill;

		if (!verticalSymetricContact)
		{
			float impulseNormalSize = impulse.dot(params.normal);
			G3D::Vector3 impulseTangent = impulse - impulseNormalSize * params.normal;
			velocityThisFrame = fabs(velocityThisFrame);
			impulseNormalSize = fabs(impulseNormalSize);
			float impulseThisFrame = velocityThisFrame / inverseMass;
			float impulseTangentMax = std::max(impulseNormalSize, impulseThisFrame) * contactParams.kFriction;
			if (impulseTangent.squaredMagnitude() > impulseTangentMax * impulseTangentMax)
			{
				G3D::Vector3 impulseDirection = params.normal
					+ contactParams.kFriction * FRICTION_GAIN * impulseTangent.direction();
				G3D::Vector3 VelDelta = deltaVelPerUnitImpulse * impulseDirection;
				if (desiredDeltaVelocity > 0)
					desiredDeltaVelocity = std::max(desiredDeltaVelocity, velocityThisFrame);
				else
					desiredDeltaVelocity = std::min(desiredDeltaVelocity, -velocityThisFrame);
				impulse = desiredDeltaVelocity / VelDelta.dot(params.normal) * impulseDirection;
			}
		}

		simBody0->applyImpulse(-impulse, params.position);

		if (simBody1 != NULL)
			simBody1->applyImpulse(impulse, params.position);

		return true;
	}

} // namespace RBX

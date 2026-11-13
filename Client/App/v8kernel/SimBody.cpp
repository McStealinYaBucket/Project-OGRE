/* App/v8kernel/SimBody.cpp -- October 2013 (simulated body state).
   Bodies: C2 0x53482A (batch244: member map in header) / D2 0x534A60
   (:54 dt==0 / :55-58 indices==-1) (batch243). Step/accumulate/update
   fns: stepper flow, deferred. */

#include "stdafx.h"

#include "V8Kernel/SimBody.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/Constants.h"
#include "Util/Math.h"
#include "Util/Units.h"
#include "Util/Velocity.h"

namespace RBX {

	// mac: 0x53482A (batch244).
	SimBody::SimBody(Body* bodyArg)
		: body(bodyArg)
		, dt(0.0f)
		, dirty(true)
		, massRecip(0.0f)
		, constantForceY(0.0f)
		, freeFallBodyIndex(-1)
		, realTimeBodyIndex(-1)
		, jointBodyIndex(-1)
		, contactBodyIndex(-1)
		, symmetricContact(false)
		, verticalContact(false)
	{
		// Vector3/CF/Quaternion members default (zero/identity) via their
		// own ctors, matching the C2 zeroes; pvFrame identity;
		// qOrientation identity (0,0,0,1); momentRecipWorld zero.
		momentRecipWorld = G3D::Matrix3::zero();
		for (int i = 0; i < 6; ++i)
			numConnectors[i] = 0;
	}











	// mac: 0x534A60 (batch243).
	// STUB: PLAYER 0x00976a00
	// STUB: STUDIO 0x009c4330
	SimBody::~SimBody()
	{
		RBXASSERT(dt == 0.0f);  // :54
		RBXASSERT(freeFallBodyIndex == -1);  // :55
		RBXASSERT(realTimeBodyIndex == -1);  // :56
		RBXASSERT(jointBodyIndex == -1);  // :57
		RBXASSERT(contactBodyIndex == -1);  // :58
	}
	// Batch261: zero the velocity pair (+60/+72), mirror into the body linVel/angVel (+212/+224), bump the body serial. (2016 refactored SimBody around a pv member; the binary stores are used verbatim here with our member names.)
	void SimBody::clearVelocity()
	{
		linVel = G3D::Vector3::zero();
		angVel = G3D::Vector3::zero();
		Body* b = body;
		b->linVel = linVel;
		b->angVel = angVel;
		b->advanceStateIndex();
	}
	// mac: 0x534E72 (batch298; 2016 SimBody::update text, spelled on the split 2013 PV members -- the binary reads worldFrame/linVel/angVel directly and replicates the offset lanes explicitly, as below).
	// STUB: PLAYER 0x00978150
	// STUB: STUDIO 0x009c5a80
	void SimBody::update()
	{
		RBXASSERT(dirty);  // SimBody.cpp:70
		G3D::Vector3 cofmInBody = body->getBranchCofmOffset();
		body->updatePV();
		// pv.position.rotation copies the body rotation (binary _muls by
		// identity -- *1/+0 are exact, so bitwise-identical to a copy).
		pvFrame.rotation = body->worldFrame.rotation;
		// pv.position.translation = R*cofm + T (binary lanes: row c is
		// ((R[c2]*cz) + ((R[c0]*cx) + (R[c1]*cy))) + T[c]).
		const float* R = (const float*)&body->worldFrame.rotation;
		const float* T = (const float*)&body->worldFrame.translation;
		float cx = cofmInBody.x;
		float cy = cofmInBody.y;
		float cz = cofmInBody.z;
		pvFrame.translation.x = ((R[2] * cz) + ((R[0] * cx) + (R[1] * cy))) + T[0];
		pvFrame.translation.y = ((R[5] * cz) + ((R[3] * cx) + (R[4] * cy))) + T[1];
		pvFrame.translation.z = ((R[8] * cz) + ((R[6] * cx) + (R[7] * cy))) + T[2];
		// velocities: lin = bodyLin + ang x r; ang = bodyAng (lanes):
		float rx = pvFrame.translation.x - T[0];
		float ry = pvFrame.translation.y - T[1];
		float rz = pvFrame.translation.z - T[2];
		float wx = body->angVel.x;
		float wy = body->angVel.y;
		float wz = body->angVel.z;
		linVel.x = ((wy * rz) - (ry * wz)) + body->linVel.x;
		linVel.y = ((rx * wz) - (rz * wx)) + body->linVel.y;
		linVel.z = ((ry * body->angVel.y) - (rx * body->angVel.x)) + body->linVel.z;
		angVel = body->angVel;
		qOrientation = Quaternion(pvFrame.rotation);
		// normalize (binary lanes: 1/sqrt(w^2 + (z^2 + (x^2 + y^2)))):
		{
			float nqx = qOrientation.x;
			float nqy = qOrientation.y;
			float nqz = qOrientation.z;
			float nqw = qOrientation.w;
			float inv = 1.0f / sqrtf((nqw * nqw) + ((nqz * nqz) + ((nqx * nqx) + (nqy * nqy))));
			qOrientation.x = nqx * inv;
			qOrientation.y = nqy * inv;
			qOrientation.z = nqz * inv;
			qOrientation.w = nqw * inv;
		}
		massRecip = 1.0f / body->getBranchMass();
		moment = body->getBranchIBodyV3();
		momentRecip = G3D::Vector3(1.0f, 1.0f, 1.0f) / moment;
		updateAngMomentum();
		{
			G3D::Matrix3 temp;
			Math::mulMatrixDiagVector(pvFrame.rotation, momentRecip, temp);
			Math::mulMatrixMatrixTranspose(temp, pvFrame.rotation, momentRecipWorld);
		}
		constantForceY = body->getBranchMass()
			* Units::kmsAccelerationToRbx(Constants::getKmsGravity());
		RBXASSERT(dirty);  // SimBody.cpp:89 (concurrency check)
		dirty = false;
	}

	// (batch254: iWorld = rot * moment * rot'; angMomentum = iWorld * rotVel.)
	void SimBody::updateAngMomentum()
	{
		G3D::Matrix3 scaled;
		G3D::Matrix3 world;
		Math::mulMatrixDiagVector(pvFrame.rotation, moment, scaled);
		Math::mulMatrixMatrixTranspose(scaled, pvFrame.rotation, world);
		const float* rows = world;
		float vx = angVel.x;
		float vy = angVel.y;
		float vz = angVel.z;
		angMomentum.x = rows[2] * vz + rows[0] * vx + rows[1] * vy;
		angMomentum.y = rows[5] * vz + rows[3] * vx + rows[4] * vy;
		angMomentum.z = rows[8] * vz + rows[6] * vx + rows[7] * vy;
	}

	// 2016 shape (owner-frame PV for cofm bodies).
	PV SimBody::getOwnerPV()
	{
		RBXASSERT(!dirty);
		G3D::Vector3 cofmInBody = body->getBranchCofmOffset();
		PV own(pvFrame, Velocity(linVel, angVel));
		return own.pvAtLocalOffset(-cofmInBody);
	}

	// 2016 SimBody::applyImpulse text (split members; EA queued).
	void SimBody::applyImpulse(const G3D::Vector3& value, const G3D::Vector3& worldPos)
	{
		RBXASSERT(!dirty);
		G3D::Vector3 localPosWorld = worldPos - pvFrame.translation;
		G3D::Vector3 rotImpulse = localPosWorld.cross(value);

		angMomentum *= 0.9998f;
		angMomentum += rotImpulse;
		{
			const float* mrows = momentRecipWorld;
			float ax = angMomentum.x;
			float ay = angMomentum.y;
			float az = angMomentum.z;
			const G3D::Vector3& den = Constants::denormalSmall();
			angVel.x = ((mrows[2] * az) + ((mrows[0] * ax) + (mrows[1] * ay))) + den.x;
			angVel.y = ((mrows[5] * az) + ((mrows[3] * ax) + (mrows[4] * ay))) + den.y;
			angVel.z = ((mrows[8] * az) + ((mrows[6] * ax) + (mrows[7] * ay))) + den.z;
		}
		linVel += massRecip * value + Constants::denormalSmall();

		angMomentum += Constants::denormalSmall();

		if (body->getCofm() != NULL)
		{
			PV ownerPv = getOwnerPV();
			body->worldFrame = ownerPv.position;
			body->linVel = ownerPv.velocity.linear;
			body->angVel = ownerPv.velocity.rotational;
		}
		else
		{
			body->worldFrame = pvFrame;
			body->linVel = linVel;
			body->angVel = angVel;
		}
		body->advanceStateIndex();
	}

	// 2016 SimBody::stepFreeFall/stepPosition texts (split members;
	// EAs queued with the stepper).
	void SimBody::stepFreeFall()
	{
		RBXASSERT(!hasExternalForceOrImpulse());
		angVel *= 0.99621f;
		Quaternion qDot(Quaternion(angVel) * qOrientation * 0.5f);
		qOrientation += qDot * dt;
		qOrientation.normalize();
		qOrientation.toRotationMatrix(pvFrame.rotation);

		float dyHalf = Units::kmsAccelerationToRbx(Constants::getKmsGravity()) * dt / 2.0f;
		linVel.y += dyHalf;
		pvFrame.translation += linVel * dt;
		linVel.y += dyHalf;

		linVel += Constants::denormalSmall();
		angVel += Constants::denormalSmall();

		if (body->getCofm() != NULL)
		{
			PV ownerPv = getOwnerPV();
			body->worldFrame = ownerPv.position;
			body->linVel = ownerPv.velocity.linear;
			body->angVel = ownerPv.velocity.rotational;
		}
		else
		{
			body->worldFrame = pvFrame;
			body->linVel = linVel;
			body->angVel = angVel;
		}
		body->advanceStateIndex();
	}

	// 2016 moment-world helpers (split members; EAs queued).
	void SimBody::updateMomentRecipWorld()
	{
		G3D::Matrix3 temp;
		Math::mulMatrixDiagVector(pvFrame.rotation, momentRecip, temp);
		Math::mulMatrixMatrixTranspose(temp, pvFrame.rotation, momentRecipWorld);
	}

	G3D::Vector3 SimBody::computeRotationVelocityFromMomentumFast()
	{
		return momentRecipWorld * angMomentum;
	}

	G3D::Vector3 SimBody::computeRotationVelocityFromMomentum()
	{
		updateMomentRecipWorld();
		return computeRotationVelocityFromMomentumFast();
	}

	// 2016 SimBody::stepVelocity/stepPosition texts (split members;
	// EAs queued with the stepper).
	void SimBody::stepVelocity()
	{
		RBXASSERT(!dirty);
		angMomentum *= 0.99621f;
		angMomentum += rotationalImpulse + torque * dt;
		updateMomentRecipWorld();
		angVel = computeRotationVelocityFromMomentumFast() + Constants::denormalSmall();
		impulseLast = impulse + force * dt;
		linVel += massRecip * impulseLast;
		force = getWorldGravityForce();
		torque = G3D::Vector3::zero();
		impulse = G3D::Vector3::zero();
		rotationalImpulse = G3D::Vector3::zero();

		angMomentum += Constants::denormalSmall();
		linVel += Constants::denormalSmall();

		if (body->getCofm() != NULL)
		{
			PV ownerPv = getOwnerPV();
			body->linVel = ownerPv.velocity.linear;
			body->angVel = ownerPv.velocity.rotational;
		}
		else
		{
			body->linVel = linVel;
			body->angVel = angVel;
		}
		body->advanceStateIndex();
	}

	void SimBody::updateFromSolver(const G3D::Vector3& newPosition, const G3D::Matrix3& newOrientation,
		const G3D::Vector3& newLinearVelocity, const G3D::Vector3& newAngularVelocity)
	{
		pvFrame.translation = newPosition;
		pvFrame.rotation = newOrientation;
		linVel = newLinearVelocity;
		angVel = newAngularVelocity;
		qOrientation = Quaternion(pvFrame.rotation);

		force = getWorldGravityForce();
		torque = G3D::Vector3::zero();
		impulse = G3D::Vector3::zero();
		rotationalImpulse = G3D::Vector3::zero();

		if (body->getCofm() != NULL)
		{
			PV ownerPv = getOwnerPV();
			body->worldFrame = ownerPv.position;
			body->linVel = ownerPv.velocity.linear;
			body->angVel = ownerPv.velocity.rotational;
		}
		else
		{
			body->worldFrame = pvFrame;
			body->linVel = linVel;
			body->angVel = angVel;
		}
		body->advanceStateIndex();

		dirty = true;
		update();
	}

	void SimBody::stepPosition()
	{
		RBXASSERT(!dirty);
		Quaternion qDot(Quaternion(angVel) * qOrientation * 0.5f);
		qOrientation += qDot * dt;
		qOrientation.normalize();
		qOrientation.toRotationMatrix(pvFrame.rotation);

		pvFrame.translation += linVel * dt;

		RBXASSERT(!hasExternalForceOrImpulse());

		if (body->getCofm() != NULL)
		{
			PV ownerPv = getOwnerPV();
			body->worldFrame = ownerPv.position;
			body->linVel = ownerPv.velocity.linear;
			body->angVel = ownerPv.velocity.rotational;
		}
		else
		{
			body->worldFrame = pvFrame;
			body->linVel = linVel;
			body->angVel = angVel;
		}
		body->advanceStateIndex();
	}

	// mac: 0x535534 (batch298; 2016 SimBody::step text with the binary's
	// exact summation lanes -- see notes inline).
	// STUB: PLAYER 0x00976e10
	// STUB: STUDIO 0x009c4740
	void SimBody::step()
	{
		RBXASSERT(!dirty);  // SimBody.cpp:138
		float dtStep = dt;
		// angMomentum *= damping; += rotationalImpulse + torque * dt
		// (lanes: ((torque.c * dt) + rotImp.c) + (angMom.c * 0.9998)):
		angMomentum.x = ((torque.x * dtStep) + rotationalImpulse.x)
			+ angMomentum.x * 0.9998f;
		angMomentum.y = ((torque.y * dtStep) + rotationalImpulse.y)
			+ angMomentum.y * 0.9998f;
		angMomentum.z = ((torque.z * dtStep) + rotationalImpulse.z)
			+ angMomentum.z * 0.9998f;
		// rotVel = momentRecipWorld * angMomentum + denormal (lanes:
		// (m[r2]*az) + ((m[r0]*ax) + (m[r1]*ay)) + den.c):
		{
			G3D::Matrix3 temp;
			Math::mulMatrixDiagVector(pvFrame.rotation, momentRecip, temp);
			Math::mulMatrixMatrixTranspose(temp, pvFrame.rotation, momentRecipWorld);
			const float* mrows = momentRecipWorld;
			float ax = angMomentum.x;
			float ay = angMomentum.y;
			float az = angMomentum.z;
			const G3D::Vector3& den = Constants::denormalSmall();
			angVel.x = ((mrows[2] * az) + ((mrows[0] * ax) + (mrows[1] * ay))) + den.x;
			angVel.y = ((mrows[5] * az) + ((mrows[3] * ax) + (mrows[4] * ay))) + den.y;
			angVel.z = ((mrows[8] * az) + ((mrows[6] * ax) + (mrows[7] * ay))) + den.z;
		}
		// quaternion integrate (qDot = 0.5 * wQuat * q; q += qDot * dt;
		// normalize -- binary lanes replicated verbatim):
		{
			float qx = qOrientation.x;
			float qy = qOrientation.y;
			float qz = qOrientation.z;
			float qw = qOrientation.w;
			float wx = angVel.x;
			float wy = angVel.y;
			float wz = angVel.z;
			float qdx = ((((wx * qw) + (qx * 0.0f)) + ((wy * qz) - (wz * qy))) * 0.5f) * dtStep + qx;
			float qdy = ((((wy * qw) + (qy * 0.0f)) + ((wz * qx) - (wx * qz))) * 0.5f) * dtStep + qy;
			float qdz = ((((wx * qy) - (wy * qx)) + ((wz * qw) + (qz * 0.0f))) * 0.5f) * dtStep + qz;
			float qdw = (((0.0f * qw) - ((wz * qz) + ((wx * qx) + (wy * qy)))) * 0.5f) * dtStep + qw;
			float inv = 1.0f / sqrtf((qdw * qdw) + ((qdz * qdz) + ((qdx * qdx) + (qdy * qdy))));
			qOrientation.x = qdx * inv;
			qOrientation.y = qdy * inv;
			qOrientation.z = qdz * inv;
			qOrientation.w = qdw * inv;
			qOrientation.toRotationMatrix(pvFrame.rotation);
		}
		// linVel += massRecip * (impulse + force * dt) (lanes:
		// (((f.c * dt) + i.c) * m) + v.c); translation += linVel * dt:
		{
			float m = massRecip;
			float nvx = (((force.x * dtStep) + impulse.x) * m) + linVel.x;
			float nvy = (((force.y * dtStep) + impulse.y) * m) + linVel.y;
			float nvz = (((force.z * dtStep) + impulse.z) * m) + linVel.z;
			linVel.x = nvx;
			linVel.y = nvy;
			linVel.z = nvz;
			pvFrame.translation.x = (dtStep * nvx) + pvFrame.translation.x;
			pvFrame.translation.y = (dtStep * nvy) + pvFrame.translation.y;
			pvFrame.translation.z = (dtStep * nvz) + pvFrame.translation.z;
		}
		// clearForceAccumulators (force to gravity) + clearImpulseAccumulators:
		force.x = 0.0f;
		force.y = constantForceY;
		force.z = 0.0f;
		torque = G3D::Vector3::zero();
		impulse = G3D::Vector3::zero();
		rotationalImpulse = G3D::Vector3::zero();
		// denormal guards:
		{
			const G3D::Vector3& den = Constants::denormalSmall();
			angMomentum.x = den.x + angMomentum.x;
			angMomentum.y = den.y + angMomentum.y;
			angMomentum.z = den.z + angMomentum.z;
			linVel.x = den.x + linVel.x;
			linVel.y = den.y + linVel.y;
			linVel.z = den.z + linVel.z;
		}
		// mirror into the body (2016: body->pv = cofm ? getOwnerPV() : pv):
		if (body->getCofm() != NULL)
		{
			PV ownerPv = getOwnerPV();
			body->worldFrame = ownerPv.position;
			body->linVel = ownerPv.velocity.linear;
			body->angVel = ownerPv.velocity.rotational;
		}
		else
		{
			body->worldFrame = pvFrame;
			body->linVel = linVel;
			body->angVel = angVel;
		}
		body->advanceStateIndex();
	}

} // namespace RBX

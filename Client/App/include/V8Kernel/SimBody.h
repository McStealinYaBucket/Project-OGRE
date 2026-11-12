/* V8Kernel/SimBody.h -- October 2013 (simulated body state, 308B).
   Evidence: C2 0x53482A (batch244: member map below) / D2 0x534A60
   (:54 dt==0 / :55-58 four body-index==-1) (batch243).
   Members (binary offsets from C2; certain names from D2 asserts +
   2016 SimBody.h order, confirmed by update 0x534E72 / step 0x535534
   member traffic, batch298):
     +0 body / +4 dt (C2 0.0; D2 :54) / +8 dirty (C2 true)
     +12 pvFrame CoordinateFrame (C2 identity)
     +60 linVel / +72 angVel (velocity pair)
     +84 qOrientation Quaternion (C2 identity -- the old zero84/88/92
       + phantom +96-massRecip reading was wrong: +84..+99 is the quat
       (0,0,0,1); massRecip lives at +172)
     +100 angMomentum / +112 moment (diag) / +124 momentRecip
     +136 momentRecipWorld Matrix3 (C2 zero)
     +172 massRecip (update() seeds 1/branchMass; step() + 0x529422 use)
     +176 constantForceY (C2 0.0; never cleared)
     +180 force / +192 torque / +204 impulse / +216 rotationalImpulse
       (step clears force to gravity + rest to zero)
     +228 impulseLast (written by stepVelocity, not step)
     +240 freeFallBodyIndex / +244 realTimeBodyIndex / +248 jointBodyIndex
       / +252 contactBodyIndex (C2 -1; D2 :55-58; NO buoyancy slot --
       matches the 11-array KernelData)
     +256..+276 numConnectors[6] (C2 0; 2016 six minus buoyancy)
     +280 symFlag (C2 false; symmetric/verticalContact role queued)
     +284 penetrationTorque / +296 penetrationForce (2016 names;
       clear-sym stores in removeBody match clearSymStateAndAccummulator;
       symmetry apply 0x529422 accumulates here)
   Total ends +308 (matches the SimBody 308u pool). Step/accumulate/
   update fns: stepper flow, deferred.
   Original file: Client/App/include/V8Kernel/SimBody.h (path from D2
   assert strings: SimBody.cpp:54-58) */

#pragma once

#include "rbx/memory.h"
#include "G3D/CoordinateFrame.h"
#include "G3D/Matrix3.h"
#include "G3D/Vector3.h"
#include "Util/Quaternion.h"
#include "Util/PV.h"

namespace RBX {

	class Body;

	class SimBody
		: public Allocator<SimBody>
	{
		friend class KernelData;
		friend class Kernel;
		friend class ContactConnector;

	public:
		SimBody(Body* body);
		~SimBody();

		Body* getBody() { return body; }
		float getDt() const { return dt; }
		// 2016 shape (stepWorld sets kernelDt when promoting freeFall
		// bodies to realTime).
		void setDt(float value) { dt = value; }
		// 2016 shape (Body::setVelocity marks the sim stale after
		// rewriting the body velocities).
		void makeDirty() { dirty = true; }
		// 2016 shape (Body::makeCofmDirty :95 asserts the root sim is
		// dirty on the already-dirty fast path).
		bool getDirty() const { return dirty; }
		// 2016 shape (called before step/accumulate; the body cofm is
		// assumed clean -- the binary inlines this dirty check).
		void updateIfDirty() {
			if (dirty)
				update();
		}
		// (batch254: recompute angular momentum from moment/orientation).
		void updateAngMomentum();
		// mac: 0x534E72 (batch298): refresh the sim state from the body
		// (branch cofm PV + quaternion + mass/moment + gravity).
		void update();
		// mac: 0x535534 (batch298): integrate velocities/quaternion/position,
		// clear accumulators, mirror into the body.
		void step();
		// 2016 shape (split-PV bundle; enables verbatim 2016 reads of
		// sim PVs -- DELTA: by value, 2013 has no PV member).
		PV getPV() const { return PV(pvFrame, Velocity(linVel, angVel)); }
		// 2016 shape (owner-frame PV for cofm bodies; called by step's
		// tail when body->getCofm() != NULL).
		PV getOwnerPV();
		// 2016 shapes (first-party const readers; binary EAs queued).
		const G3D::Vector3& getForce() const { return force; }
		const G3D::Vector3& getTorque() const { return torque; }
		const G3D::Vector3& getImpulse() const { return impulse; }
		const G3D::Vector3& getRotationalImpulse() const { return rotationalImpulse; }
		const float& getMassRecip() const { return massRecip; }
		const G3D::Vector3& getImpulseLast() const { return impulseLast; }
		G3D::Vector3 getWorldGravityForce() const { return G3D::Vector3(0, constantForceY, 0); }
		bool hasExternalForceOrImpulse() const {
			return force != getWorldGravityForce() || torque != G3D::Vector3::zero()
				|| impulse != G3D::Vector3::zero() || rotationalImpulse != G3D::Vector3::zero();
		}
		const G3D::Matrix3& getInverseInertiaInWorld() const { return momentRecipWorld; }
		// 2016 shapes (symmetric-contact gating for the impulse solver).
		bool isSymmetricContact() const { return symmetricContact; }
		bool isVerticalContact() const { return verticalContact; }
		void clearSymmetricContact() { symmetricContact = false; }
		// 0x5????? (EA queued; 2016 SimBody::applyImpulse text -- binary
		// calls it from computeImpulse; direct port below).
		void applyImpulse(const G3D::Vector3& impulse, const G3D::Vector3& worldPos);
		// 2016 shapes (EA queued; binary calls both from stepWorld).
		void stepFreeFall();
		void stepPosition();
		// 2016 shapes (moment-world helpers; EAs queued).
		void updateMomentRecipWorld();
		G3D::Vector3 computeRotationVelocityFromMomentum();
		G3D::Vector3 computeRotationVelocityFromMomentumFast();
		// 2016 shapes (split-step API for the impulse loop; EAs queued).
		void stepVelocity();
		void updateFromSolver(const G3D::Vector3& newPosition, const G3D::Matrix3& newOrientation,
			const G3D::Vector3& newLinearVelocity, const G3D::Vector3& newAngularVelocity);
		// 2016 shapes (parallel-accumulate suite; SLOW asserts compile
		// out here -- same net stores; binary EAs queued).
		static G3D::Vector3 computeTorqueFromOffsetForce(const G3D::Vector3& force,
			const G3D::Vector3& cofm, const G3D::Vector3& forceLocationWorld) {
			G3D::Vector3 localPosWorld = forceLocationWorld - cofm;
			return localPosWorld.cross(force);
		}
		void accumulateForceCofm(const G3D::Vector3& value) {
			updateIfDirty();
			force += value;
		}
		void accumulateForce(const G3D::Vector3& value, const G3D::Vector3& worldPos) {
			updateIfDirty();
			force += value;
			torque += computeTorqueFromOffsetForce(value, pvFrame.translation, worldPos);
		}
		void accumulatePenetrationForce(const G3D::Vector3& value, const G3D::Vector3& worldPos) {
			penetrationForce += value;
			penetrationTorque += computeTorqueFromOffsetForce(value, pvFrame.translation, worldPos);
		}
		void accumulateTorque(const G3D::Vector3& value) {
			updateIfDirty();
			torque += value;
		}
		void accumulateImpulse(const G3D::Vector3& value, const G3D::Vector3& worldPos) {
			updateIfDirty();
			impulse += value;
			G3D::Vector3 localPosWorld = worldPos - pvFrame.translation;
			rotationalImpulse += localPosWorld.cross(value);
		}
		void accumulateImpulseAtBranchCofm(const G3D::Vector3& value) {
			updateIfDirty();
			impulse += value;
		}
		void accumulateRotationalImpulse(const G3D::Vector3& value) {
			updateIfDirty();
			rotationalImpulse += value;
		}
		void resetImpulseAccumulators() {
			updateIfDirty();
			impulse = G3D::Vector3::zero();
			rotationalImpulse = G3D::Vector3::zero();
		}
		void resetForceAccumulators() {
			updateIfDirty();
			force = getWorldGravityForce();
			torque = G3D::Vector3::zero();
		}
		void clearSymStateAndAccummulator() {
			symmetricContact = true;
			verticalContact = true;
			penetrationTorque = G3D::Vector3::zero();
			penetrationForce = G3D::Vector3::zero();
		}
		// Batch261: zero velocities + mirror to body + serial bump.
		void clearVelocity();
		// 2016 shapes (first-party, minus buoyancy -- 2013 has 4 lists;
		// KernelData::insertBody asserts ride these).
		bool isFreeFallBody() const { return freeFallBodyIndex >= 0; }
		bool isRealTimeBody() const { return realTimeBodyIndex >= 0; }
		bool isJointBody() const { return jointBodyIndex >= 0; }
		bool isContactBody() const { return contactBodyIndex >= 0; }
		bool isInKernel() const {
			return isFreeFallBody() || isRealTimeBody()
				|| isJointBody() || isContactBody();
		}
		bool validateBodyLists() const {
			return (freeFallBodyIndex >= 0) + (realTimeBodyIndex >= 0)
				+ (jointBodyIndex >= 0) + (contactBodyIndex >= 0) <= 1;
		}

	private:
		Body* body;                       // +0.
		float dt;                         // +4 (C2 0.0; D2 :54).
		bool dirty;                       // +8 (C2 true).
		G3D::CoordinateFrame pvFrame;     // +12.
		G3D::Vector3 linVel;              // +60.
		G3D::Vector3 angVel;              // +72.
		Quaternion qOrientation;          // +84 (C2 identity).
		G3D::Vector3 angMomentum;         // +100.
		G3D::Vector3 moment;              // +112.
		G3D::Vector3 momentRecip;         // +124.
		G3D::Matrix3 momentRecipWorld;    // +136 (C2 zero).
		float massRecip;                  // +172 (C2 0.0; update() seeds
		                                // 1/branchMass; step() + the 0x529422
		                                // symmetry weights read it).
		float constantForceY;             // +176 (C2 0.0).
		G3D::Vector3 force;               // +180.
		G3D::Vector3 torque;              // +192.
		G3D::Vector3 impulse;             // +204.
		G3D::Vector3 rotationalImpulse;   // +216.
		G3D::Vector3 impulseLast;         // +228.
		int freeFallBodyIndex;            // +240 (C2 -1; D2 :55).
		int realTimeBodyIndex;            // +244 (C2 -1; D2 :56).
		int jointBodyIndex;               // +248 (C2 -1; D2 :57).
		int contactBodyIndex;             // +252 (C2 -1; D2 :58).
		int numConnectors[6];             // +256 (C2 0).
		bool symmetricContact;            // +280 (C2 false).
		bool verticalContact;             // +281 (C2 false).
		G3D::Vector3 penetrationTorque;   // +284 (2016 name; aggregated
		                                // contact-normal torque; C2 zero).
		G3D::Vector3 penetrationForce;    // +296 (2016 name; aggregated
		                                // contact-normal force; C2 zero).
	};

} // namespace RBX

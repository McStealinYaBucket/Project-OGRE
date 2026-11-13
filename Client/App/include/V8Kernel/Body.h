/* V8Kernel/Body.h -- October 2013 (rigid body node).
   Evidence: C2 0x523AB6 / D2 0x523C58 (:35 connectorUseCount==0 / :36
   !link / :37 simBody / :38 leafBodyIndex==-1 + Cofm/SimBody deletes)
   (batch243) / SimBody 308B pool (batch225/243 allocators).
   Members (binary offsets from C2/D2; names carried from 2016 Body.h
   where kind+role agree, positional otherwise -- see below):
     IndexedTree base (+0..+24)
     +24 unknown24 (C2-untouched) / +28 unknown28 (C2 false; TODO identity
       -- 2016 has mutex/uid/guidIndex here, unobserved in 2013)
     +32 leafBodyIndex (C2 -1; D2 :38; IndexArray<Body> slot)
     +36 connectorUseCount (C2 0; D2 :35)
     +40 root (C2 this -- 2016 `root`, top body)
     +44 cofm (C2 NULL; D2 deletes via Cofm pool)
     +48 simBody (C2 news 308B SimBody; D2 :37 + pool delete)
     +52 link (C2 NULL; D2 :36)
     +56 canThrottle (C2 true -- 2016 name)
     +60 meInParent CoordinateFrame (C2 identity)
     +108 moment Matrix3 (C2 identity)
     +144 mass (C2 0.0)
     +148 cofmOffset Vector3 (C2 zero)
     +160 stateIndex (C2 ++globalId)
     +164 worldFrame CoordinateFrame (C2 identity; TODO role -- 2016 folds
       this into PV, 2013 splits CF + vectors below)
     +212 linVel Vector3 (C2 zero; TODO role)
     +224 angVel Vector3 (C2 zero; TODO role)
   Baseline statics: globalId (C2 ++dword counter) + worldBody (Point C2
   default; initStaticData queued -- NULL until then, never deref'd in
   green paths).
   Stepper/validate/parent-Cofm fns: Body module flow, deferred (bodies
   touch SimBody stepping + Connector use-counts).
   Original file: Client/App/include/V8Kernel/Body.h (path from D2
   assert strings: Body.cpp:35-38) */

#pragma once

#include "Util/IndexedTree.h"
#include "V8Kernel/BodyPvSetter.h"
#include "V8Kernel/Cofm.h"
#include "V8Kernel/SimBody.h"
#include "Util/Math.h"
#include <tbb/spin_mutex.h>	// OGRE vc9: see unknown28
#include "Util/Velocity.h"
#include "rbx/memory.h"
#include "G3D/CoordinateFrame.h"
#include "G3D/Matrix3.h"
#include "G3D/Vector3.h"

namespace RBX {

	class SimBody;
	class Cofm;
	class Velocity;
	class PV;
	class Link;

	class Body
		: public IndexedTree
		, public Allocator<Body>
	{
		friend class KernelData;
		friend class Kernel;
		friend class SimBody;

	public:
		Body();
		virtual ~Body();

		static Body* getWorldBody() { return worldBody; }
		// 2016 shapes (first-party; KernelData::insertBody asserts).
		SimBody* getSimBody() { return simBody; }
		const SimBody* getConstSimBody() const { return simBody; }
		// Primitive::getCanThrottle (0x5914E2, batch257) + Connector::
		// computeCanThrottle (0x526464) read Body+56 directly; same
		// observable through this getter (friend pattern would sprawl:
		// Connector, Primitive, Assembly, Sleep all read it).
		bool getCanThrottle() const { return canThrottle; }
		SimBody* getRootSimBody() { return getRoot<Body>()->getSimBody(); }
		const SimBody* getConstRootSimBody() const { return getRoot<Body>()->getConstSimBody(); }
		// 2016 shapes (parallel-accumulate forwarding; binary EAs queued
		// except accumulateForceAtBranchCofm 0x5F7460, defined below).
		void accumulateImpulseAtBranchCofm(const G3D::Vector3& impulse) {
			if (SimBody* s = getRootSimBody())
				s->accumulateImpulseAtBranchCofm(impulse);
		}
		void accumulateLinearImpulse(const G3D::Vector3& impulse, const G3D::Vector3& worldPos) {
			if (SimBody* s = getRootSimBody())
				s->accumulateImpulse(impulse, worldPos);
		}
		void accumulateRotationalImpulse(const G3D::Vector3& impulse) {
			if (SimBody* s = getRootSimBody())
				s->accumulateRotationalImpulse(impulse);
		}
		// mac: 0x5F7460 (batch298: :334 root assert + rootSim updateIfDirty +
		// force add; 2016 spells the tail as accumulateForceCofm).
		// Defined here as in 2013 (assert Body.h:334; the prior tree had it in Body.cpp).
		// STUB: PLAYER 0x00920bc0
		// STUB: STUDIO 0x00948dc0
		void accumulateForceAtBranchCofm(const G3D::Vector3& force)
		{
			RBXASSERT(getRoot<Body>() == this);  // Body.h:334
			if (SimBody* s = getRootSimBody())
				s->accumulateForceCofm(force);
		}
		void accumulateForce(const G3D::Vector3& force, const G3D::Vector3& worldPos) {
			if (SimBody* s = getRootSimBody())
				s->accumulateForce(force, worldPos);
		}
		void accumulateTorque(const G3D::Vector3& torque) {
			if (SimBody* s = getRootSimBody())
				s->accumulateTorque(torque);
		}
		void resetForceAccumulators() {
			if (SimBody* s = getRootSimBody())
				s->resetForceAccumulators();
		}
		void resetImpulseAccumulators() {
			if (SimBody* s = getRootSimBody())
				s->resetImpulseAccumulators();
		}
		const G3D::Vector3& getBranchForce() const {
			RBXASSERT(getRoot<Body>() == this);
			const SimBody* s = getConstRootSimBody();
			return s != NULL ? s->getForce() : G3D::Vector3::zero();
		}
		const G3D::Vector3& getBranchTorque() const {
			RBXASSERT(getRoot<Body>() == this);
			const SimBody* s = getConstRootSimBody();
			return s != NULL ? s->getTorque() : G3D::Vector3::zero();
		}
		// 2016 shape (linear + rotational bundle for the PartInstance
		// velocity setters; 2013 binary reads the two members directly).
		Velocity getVelocity() const { return Velocity(linVel, angVel); }
		inline bool isLeafBody() const { return leafBodyIndex >= 0; }
		// 0x5A07xx (batch261: ++globalId stamp into stateIndex).
		unsigned int advanceStateIndex();
		// mac: 0x523BFE (24B, batch289: ++globalId stamp, returned (same
		// counter advanceStateIndex consumes; Link C2 seeds from it)).
		static unsigned int getNextStateIndex();
		// 0x5A07xx (batch261: parent-NULL gate (+4, typed-parent
		// accessor); linVel/angVel store (+212/+224); serial bump (+160);
		// sim dirty (+8). The setter arg is an overload tag, never read
		// -- defaulted: zeroVelocity's call site passes (body, vel) only
		// (batch260 disasm).
		void setVelocity(const Velocity& vel, const BodyPvSetter& setter = BodyPvSetter());
		// mac: 0x524B7E (batch268: parented bodies hit the :273 assert; else
		// full PV store (worldFrame/linVel/angVel) + sim dirty + serial).
		void setPv(const PV& pv, const BodyPvSetter& setter = BodyPvSetter());
		// 2016 shape (asserts parent; Link arm queues with the Link class
		// -- link is always NULL in-tree, guarded loudly below).
		// 2016 shape (getParent assert + link check).
		// Defined here as in 2013 (assert Body.h:109; the prior tree had it in Body.cpp).
		// STUB: PLAYER 0x0071ea50
		// STUB: STUDIO 0x007ba4a0
		const G3D::CoordinateFrame& getMeInParent()
		{
			RBXASSERT(getTypedParent<Body>() != NULL);
			// Link arm queues with the Link class (unported shell): link is
			// C2-NULL and only joint code sets it -- loud if ever reached.
			RBXASSERT(link == NULL);
			return meInParent;
		}
		// 2016 shape (plain store; the Link* overload queues with Link).
		void setMeInParent(const G3D::CoordinateFrame& value) { meInParent = value; }
		// 2016 shape (Link* store for motor-joint bodies).
		void setMeInParent(Link* value) { link = value; }
		// mac: 0x455320 (batch258: recursive parent refresh with worldFrame/
		// linVel/angVel rebuild + serial sync; :119/:128 asserts).
		// Return-value DELTA: binary returns char (recomputed flag) --
		// the only in-tree caller ignores it -> void here.
		// mac: 0x455320 (batch258: :119 parent/root invariant; stale-serial
		// rebuild of worldFrame/linVel/angVel from the parent (:128
		// stale-call guard); serial sync to the root).
		// Return-value DELTA: binary returns char; the only in-tree caller
		// ignores it -> void here.
		// Defined here as in 2013 (asserts Body.h:119/:128; the prior tree had it in Body.cpp).
		// STUB: PLAYER 0x0071eab0
		// STUB: STUDIO 0x007ba500
		void updatePV()
		{
			Body* parent = getTypedParent<Body>();
			RBXASSERT((parent != NULL) != (getRoot<Body>() == this));  // :119
			if (parent != NULL)
			{
				Body* root = getRoot<Body>();
				unsigned int rootSerial = root->stateIndex;
				root->updatePV();
				if (stateIndex != rootSerial)
				{
					parent->updatePV();
					const G3D::CoordinateFrame& me = getMeInParent();
					// ROBLOX fastMul == binary _mul flop-for-flop (_mul is
					// private; fastMul forwards to it directly).
					G3D::Matrix3::fastMul(parent->worldFrame.rotation, me.rotation, worldFrame.rotation);
					float tx = me.translation.x;
					float ty = me.translation.y;
					float tz = me.translation.z;
					const float* pr = (const float*)&parent->worldFrame.rotation;
					const float* pt = (const float*)&parent->worldFrame.translation;
					float lx = (tz * pr[2]) + ((tx * pr[0]) + (ty * pr[1])) + pt[0];
					float ly = (tz * pr[5]) + ((tx * pr[3]) + (ty * pr[4])) + pt[1];
					float lz = (tz * pr[8]) + ((tx * pr[6]) + (ty * pr[7])) + pt[2];
					worldFrame.translation.x = lx;
					worldFrame.translation.y = ly;
					worldFrame.translation.z = lz;
					float ox = lx - pt[0];
					float oy = ly - pt[1];
					float oz = lz - pt[2];
					float wx = parent->angVel.x;
					float wy = parent->angVel.y;
					float wz = parent->angVel.z;
					float vx = (oy * wx - ox * wy) + parent->linVel.z;
					float vy = (ox * wz - oz * wx) + parent->linVel.y;
					linVel.x = (wy * oz - oy * wz) + parent->linVel.x;
					linVel.y = vy;
					linVel.z = vx;
					angVel = parent->angVel;
					Body* rootCheck = getRoot<Body>();
					rootCheck->updatePV();
					RBXASSERT(stateIndex != rootCheck->stateIndex);  // :128
				}
				root->updatePV();
				stateIndex = root->stateIndex;
			}
		}
		// mac: 0x5927EA (batch258: +28 spinlock, updatePV refresh, unlock,
		// return worldFrame). The worldFrame/linVel/angVel trio IS the
		// PV (position + velocity), so callers read PV through it.
		// DELTA: contended-path backoff is YieldProcessor (windows.h),
		// not tbb::atomic_backoff::pause (absent from the shim) -- same
		// pause instruction, simpler backoff; unreachable single-threaded.
		PV* getPV_Spin_Lock();

		// Cofm surface (2016 shapes; out-of-line EAs below are 2013).
		Cofm* getCofm() { return cofm; }
		// 2016 shapes (public typed-parent access; TreeStage's cascade
		// and chainToGround ride these -- the binary calls
		// setIndexedTreeParent/getParent-slot directly, i.e. inlined).
		Body* getParent() { return getTypedParent<Body>(); }
		void setParent(Body* value) { setIndexedTreeParent(value); }
		float getMass() const { return mass; }
		const G3D::Vector3& getCofmOffset() { return cofmOffset; }
		Body* getChild(int i) { return getTypedChild<Body>(i); }
		G3D::Matrix3 getIBody() const { return moment; }
		// 2013 splits the 2016 PV into worldFrame + linVel/angVel; the
		// frame IS the PV position (cf. getPV_Spin_Lock).
		const G3D::Vector3& getPos() { updatePV(); return worldFrame.translation; }
		const G3D::CoordinateFrame& getCoordinateFrame() { updatePV(); return worldFrame; }
		// 2016 shape (binary reads worldFrame/+164 directly with no call
		// -- the compute* engine uses this; no updatePV by design).
		const G3D::CoordinateFrame& getCoordinateFrameFast() const { return worldFrame; }
		// mac: 0x525570 (batch294: moment copy + updatePV + momentToWorldSpace
		// off worldFrame.rotation; 2016 header-inline shape).
		G3D::Matrix3 getIWorld();
		// Absent from the symbol table -> 2016 header inlines (same
		// cofm-read pattern as the banked getBranchIBody/getBranchMass
		// disasm: cofm+20 mass / cofm+24 moment after updateIfDirty).
		float getBranchMass() { return cofm ? cofm->getMass() : mass; }
		G3D::Matrix3 getBranchIBody() { return cofm ? cofm->getMoment() : moment; }
		// Absent from symbols -> 2016 header inline (rides getBranchIBody;
		// read by SimBody::update 0x534E72).
		G3D::Vector3 getBranchIBodyV3() { return Math::toDiagonal(getBranchIBody()); }
		G3D::Matrix3 getBranchIWorld()
			{ return Math::momentToWorldSpace(getBranchIBody(), getCoordinateFrame().rotation); }
		// mac: 0x52500E (batch291: simBody :356 + leaf/cofm :357 asserts;
		// cofm ? cofmInBody(+8 after updateIfDirty) : cofmOffset).
		const G3D::Vector3& getBranchCofmOffset();
		// mac: 0x524F12 (batch292: updatePV then pointToWorldSpace/getPos).
		G3D::Vector3 getBranchCofmPos();
		// mac: 0x525140 (batch292: updatePV + branch pos + worldFrame rotation).
		G3D::CoordinateFrame getBranchCofmCoordinateFrame();
		// mac: 0x524DEE (batch293: Math::getIBodyAtPoint off moment/mass).
		G3D::Matrix3 getIBodyAtPoint(const G3D::Vector3& point);
		// mac: 0x524E38 (batch293: updatePV + getIWorld + getPos translation).
		G3D::Matrix3 getIWorldAtPoint(const G3D::Vector3& point);
		// mac: 0x524E94 (batch293: branch pos/world/mass -> getIWorldAtPoint).
		G3D::Matrix3 getBranchIWorldAtPoint(const G3D::Vector3& point);
		// mac: 0x524D5C (batch294: makeCofmDirty + store).
		// Return-value DELTA: binary threads makeCofmDirty's char
		// through; no in-tree reader -> void.
		void setMass(float _mass);
		// mac: 0x524D90 (batch294: != gate + makeCofmDirty + copy; void
		// per the same DELTA).
		void setMoment(const G3D::Matrix3& _momentInBody);
		// mac: 0x524760 (batch294: per-lane change gate + refresh + dirty).
		void setCofmOffset(const G3D::Vector3& _cofmOffset);
		// mac: 0x524632 (batch294: create/delete Cofm on need + :184 assert).
		void refreshCofm();
		// mac: 0x5242B6 (batch294: :122 calcRoot assert + store + recurse).
		void resetRoot(Body* newRoot);
		// mac: 0x524064 (batch293/294: dirty-cofm fast path with
		// validateParent + root-dirty asserts; else parent recurse /
		// root sim-dirty; then dirty own cofm + :112/:115 asserts).
		void makeCofmDirty();
		// mac: 0x523F8C (batch293: iterative parent-chain cofm-is-dirty
		// walk, :78/:79 -- assert-only, bool for RBXASSERT chaining).
		bool validateParentCofmDirty();
		// mac: 0x524628 (batch294: makeCofmDirty on child arrival).
		virtual void onChildAdding(IndexedTree* child);

	private:
		static Body* worldBody;
		static unsigned int globalId;

		int unknown24;              // +24 (C2-untouched; TODO identity).
		// OGRE vc9: was bool, spun on with _InterlockedCompareExchange8 (VC9 has no 8-bit CAS intrinsic).
		// PLAYER getPV_Spin_Lock 0x007554e0 holds a tbb::spin_mutex::scoped_lock on +28 (lock byte from
		// 0x00402240, unlock stores it back under the EH frame). The 2016 name is "mutex".
		tbb::spin_mutex unknown28;  // +28 (C2 flag 0).
		int leafBodyIndex;          // +32 (C2 -1; D2 :38).
		int connectorUseCount;      // +36 (C2 0; D2 :35).
		Body* root;                 // +40 (C2 this).
		Cofm* cofm;                 // +44 (C2 NULL).
		SimBody* simBody;           // +48 (C2 news; D2 deletes).
		Link* link;                 // +52 (C2 NULL; D2 :36).
		bool canThrottle;           // +56 (C2 true).
		G3D::CoordinateFrame meInParent;  // +60.
		G3D::Matrix3 moment;        // +108.
		float mass;                 // +144 (C2 0.0).
		G3D::Vector3 cofmOffset;    // +148.
		unsigned int stateIndex;    // +160 (C2 ++globalId).
		G3D::CoordinateFrame worldFrame;  // +164 (TODO role).
		G3D::Vector3 linVel;        // +212 (TODO role).
		G3D::Vector3 angVel;        // +224 (TODO role).
	};

} // namespace RBX

/* App/v8world/SleepStage.cpp -- October 2013 (2013 assert path Client\App\V8World\SleepStage.cpp).
   The ctor and dtor are written from the 2013 bytes (layout in the header); the dtor's asserts sit
   on their 2013 lines 64-77. The state-machine bodies further down are still the Mac reconstruction
   and carry no FUNCTION markers. */

#include "stdafx.h"

#include "V8World/SleepStage.h"
#include "decomp.h"
#include "V8World/SimulateStage.h"
#include "V8World/Assembly.h"
#include "V8World/Primitive.h"
#include "V8World/Contact.h"
#include "V8World/World.h"
#include "V8World/Edge.h"
#include "V8World/Joint.h"
#include "V8Kernel/Body.h"
#include "Util/Profiling.h"

namespace RBX {

	// FUNCTION: PLAYER 0x008e7f50
	// FUNCTION: STUDIO 0x00a06180
	SleepStage::SleepStage(IStage* upstream, World* world)
		: IWorldStage(upstream, new SimulateStage(this, world), world)
		, numContactsInStage(0)
		, numContactsInKernel(0)
		, throttling(false)
		, debugReentrant(false)
		, longStepId(-1)
		, recursivePassId(0)
		, externalRecursiveWake(false)
		, profilingCollision(new Profiling::CodeProfiler("Collision"))
		, profilingJointSleep(new Profiling::CodeProfiler("Joint Sleep"))
		, profilingWake(new Profiling::CodeProfiler("Wake"))
		, profilingSleep(new Profiling::CodeProfiler("Sleep"))
	{
	}





















	// FUNCTION: PLAYER 0x008e7160
	// FUNCTION: STUDIO 0x00a053a0
	SleepStage::~SleepStage()
	{
		RBXASSERT(numContactsInStage == 0);
		RBXASSERT(numContactsInKernel == 0);
		RBXASSERT(debugReentrant == 0);

		RBXASSERT(recursiveWakePending.empty());
		RBXASSERT(wakePending.empty());
		RBXASSERT(awake.empty());
		RBXASSERT(sleepingChecking.empty());
		RBXASSERT(sleepingDeeply.empty());
		RBXASSERT(removing.empty());
		RBXASSERT(steppingContacts[Sim::CAN_NOT_THROTTLE].size() == 0);
		RBXASSERT(touchingContacts[Sim::CAN_NOT_THROTTLE].size() == 0);
		RBXASSERT(steppingContacts[Sim::CAN_THROTTLE].size() == 0);
		RBXASSERT(touchingContacts[Sim::CAN_THROTTLE].size() == 0);
	}
















































































































































































































	// 2016 shape (throttlenioskę gate for stepJoints).
	static bool canThrottle(Edge* e)
	{
		RBXASSERT(e->getThrottleType() != Sim::UNDEFINED_THROTTLE);
		return e->getThrottleType() == Sim::CAN_THROTTLE;
	}



































































	// stepJoints (batch222: :365 empty-scratch + per-j :372 gate;
	// collect both-assemblies-unmoving joints, sleep them, clear).
	void SleepStage::stepJoints()
	{
		Profiling::Mark mark(NULL, false, false);
		RBXASSERT(toSleepingJoint.empty());
		for (JointSet::iterator it = steppingJoints.begin(); it != steppingJoints.end(); ++it)
		{
			Joint* j = *it;



			RBXASSERT(j->inOrDownstreamOfStage(this));  // :372
			Primitive* p0 = j->getPrimitive(0);
			Primitive* p1 = j->getPrimitive(1);
			if (!throttling || !canThrottle(j))
			{
				Assembly* a0 = p0->getAssembly();
				Assembly* a1 = p1->getAssembly();
				Sim::AssemblyState s0 = a0->getAssemblyState();
				Sim::AssemblyState s1 = a1->getAssemblyState();
				// Binary gate, verbatim: unmoving states are ANCHORED /
				// SLEEPING_* / REMOVING, i.e. (s & ~2) != 1 && s != 2.
				if ((((int)s0 & ~2) != 1 && s0 != Sim::WAKE_PENDING)
					&& (((int)s1 & ~2) != 1 && s1 != Sim::WAKE_PENDING))
					toSleepingJoint.push_back(j);
			}
		}
		changeJointState(toSleepingJoint, Sim::SLEEPING);
		toSleepingJoint.clear();
	}

	// 0x59Fxxx (batch259: per-prim assembly-prim PV lanes vs 1.0;
	// radius lazy through the +116 cache (+120 flag arm recomputes --
	// modeled as computeAssemblyMaxRadius directly, the +128 functor
	// indirection as DELTA). 2016 spells the same flow with
	// highVelocityVector/highVelocityContact helpers.)
	bool SleepStage::highVelocityNewTouch(Contact* contact)
	{
		for (int i = 0; i < 2; ++i)
		{
			Assembly* assembly = contact->getPrimitive(i)->getAssembly();
			const Velocity& v = assembly->getAssemblyPrimitive()->getPV()->velocity;
			if (fabsf(v.linear.x) > 1.0f || fabsf(v.linear.y) > 1.0f || fabsf(v.linear.z) > 1.0f)
				return true;
			float radius = assembly->getMaxRadius();
			if (assembly->needsMaxRadiusRecompute())
			{
				radius = assembly->computeAssemblyMaxRadius();
				assembly->storeMaxRadius(radius);
			}
			G3D::Vector3 spin = v.rotational * radius;
			if (fabsf(spin.x) > 1.0f || fabsf(spin.y) > 1.0f || fabsf(spin.z) > 1.0f)
				return true;
		}
		return false;
	}

	// Edge fan (batch251: both prims' assemblies through wakeEvent).
	void SleepStage::wakeEvent(Edge* edge)
	{
		for (int i = 0; i < 2; ++i)
			wakeEvent(edge->getPrimitive(i)->getAssembly());
	}

	// Assembly gate (batch251: state >= 4 -> WAKE_PENDING).
	void SleepStage::wakeEvent(Assembly* assembly)
	{
		if (assembly->getAssemblyState() >= Sim::SLEEPING_CHECKING)
			changeAssemblyState(assembly, Sim::WAKE_PENDING);
	}

	// Contact fan (batch259: both prims' assemblies through the
	// assembly version).
	void SleepStage::recursiveWakeEvent(Contact* contact)
	{
		for (int i = 0; i < 2; ++i)
			recursiveWakeEvent(contact->getPrimitive(i)->getAssembly());
	}

	// Assembly gate (batch259: state >= 2 -> RECURSIVE_WAKE_PENDING).
	void SleepStage::recursiveWakeEvent(Assembly* assembly)
	{
		if (assembly->getAssemblyState() >= Sim::WAKE_PENDING)
			changeAssemblyState(assembly, Sim::RECURSIVE_WAKE_PENDING);
	}

	// 2016 shape (external wake tickle; EA queued with the stepper).
	void SleepStage::onExternalTickleAssembly(Assembly* assembly, bool recursive)
	{
		if (assembly->inPipeline()) {
			if (assembly->inOrDownstreamOfStage(this)) {
				assembly->wakeUp();
				if (recursive) {
					externalRecursiveWake = true;
					recursiveWakeEvent(assembly);
				}
				else {
					wakeEvent(assembly);
				}
			}
		}
	}

















































































































































































































	// Single (batch257: stepping/touching IndexArrays + kernel + wake +
	// signal; 2016 text).
	void SleepStage::changeContactState(Contact* contact, Sim::EdgeState newState)
	{
		Sim::EdgeState currentState = contact->getEdgeState();
		Sim::ThrottleType throttleType = contact->getThrottleType();
		RBXASSERT(currentState != newState);  // :678

		bool wasInStepping = (currentState == Sim::STEPPING);
		bool wasInTouching = (currentState == Sim::CONTACTING);
		bool desiredInStepping = (newState == Sim::STEPPING);
		bool desiredInTouching = (newState == Sim::CONTACTING);
		if (wasInStepping != desiredInStepping && wasInStepping)
			steppingContacts[throttleType].fastRemove(contact);
		if (wasInTouching != desiredInTouching && wasInTouching)
			touchingContacts[throttleType].fastRemove(contact);
		if (wasInStepping != desiredInStepping && desiredInStepping)
			steppingContacts[throttleType].fastAppend(contact);
		if (wasInTouching != desiredInTouching && desiredInTouching)
			touchingContacts[throttleType].fastAppend(contact);

		bool wasInKernel = (currentState == Sim::CONTACTING);
		bool desiredInKernel = (newState == Sim::CONTACTING);
		if (wasInKernel != desiredInKernel) {
			if (wasInKernel) {
				getDownstreamWS()->onEdgeRemoving(contact);
				numContactsInKernel--;
			}
			else {
				getDownstreamWS()->onEdgeAdded(contact);
				numContactsInKernel++;
			}
		}

		if ((currentState == Sim::STEPPING) && (newState == Sim::CONTACTING)) {
			if (highVelocityNewTouch(contact))
				recursiveWakeEvent(contact);
			else
				wakeEvent(contact);
		}
		else if ((currentState == Sim::CONTACTING) && (newState == Sim::STEPPING)) {
			wakeEvent(contact);
		}

		if ((newState == Sim::CONTACTING) && contact->getNumTouchCycles() < 3)
			getWorld()->onPrimitiveCollided(contact->getPrimitive(0), contact->getPrimitive(1));

		contact->setEdgeState(newState);
	}












	// Single (batch257: :743 currentState != newState; stepping-set
	// erase (:751 num == 1) / insert (:756 ok); downstream vtbl+20/+16
	// are onEdgeRemoving/onEdgeAdded (slot reconciliation, cf. IStage.h
	// note) reached here via the binary-named getDownstreamWS()).
	// RUNTIME PROOF DEFERRED (standalone driver crashes pre-output --
	// see section 159 log; proven with the World-integrated Sleep
	// driver alongside the onEdge pair).
	void SleepStage::changeJointState(Joint* joint, Sim::EdgeState newState)
	{
		Sim::EdgeState currentState = joint->getEdgeState();
		RBXASSERT(currentState != newState);  // :743
		if ((currentState == Sim::STEPPING) != (newState == Sim::STEPPING))
		{
			if (currentState == Sim::STEPPING)
			{
				size_t num = steppingJoints.erase(joint);


				RBXASSERT(num == 1);  // :751
				getDownstreamWS()->onEdgeRemoving(joint);
			}
			else {
				bool ok = steppingJoints.insert(joint).second;
				RBXASSERT(ok);  // :756
				getDownstreamWS()->onEdgeAdded(joint);
			}
		}
		joint->setEdgeState(newState);
	}




	void SleepStage::changeAssemblyState(Assembly* a, Sim::AssemblyState newState)
	{
		RBXASSERT(!debugReentrant);  // :768
		debugReentrant = true;
		if (!a->inOrDownstreamOfStage(this))
		{
			RBXASSERT(a->inStage(IStage::HUMANOID_STAGE));
		}
		else {
			Sim::AssemblyState currentState = a->getAssemblyState();
			RBXASSERT(currentState != newState);  // :776
			RBXASSERT(currentState != Sim::ANCHORED);  // :777
			size_t num = stateToSet(currentState).erase(a);





			RBXASSERT(num == 1);  // :784
			bool ok = stateToSet(newState).insert(a).second;
			RBXASSERT(ok);  // :786
			bool currentSleeping = (currentState == Sim::SLEEPING_CHECKING
				|| currentState == Sim::SLEEPING_DEEPLY);
			bool newSleeping = (newState == Sim::SLEEPING_CHECKING
				|| newState == Sim::SLEEPING_DEEPLY);
			if (currentSleeping && newSleeping)
			{
				if (a->downstreamOfStage(this))
				{
					a->getAssemblyPrimitive()->getBody()->getRootSimBody()->clearVelocity();
					static_cast<SimulateStage*>(getDownstreamWS())->onAssemblyRemoving(a);
				}
			}
			else
			{
				if (currentSleeping)
				{
					Primitive* assemblyPrim = a->getAssemblyPrimitive();
					assemblyPrim->zeroVelocity();
					a->wakeUp();
					assemblyPrim->getBody()->resetImpulseAccumulators();
				}
				if (newState == Sim::AWAKE)
				{
					if (!a->downstreamOfStage(this))
					{
						a->getAssemblyPrimitive()->getBody()->resetForceAccumulators();
						static_cast<SimulateStage*>(getDownstreamWS())->onAssemblyAdded(a);
					}
				}
				else if (newSleeping || newState == Sim::REMOVING)
				{
					if (a->downstreamOfStage(this))
					{
						a->getAssemblyPrimitive()->getBody()->getRootSimBody()->clearVelocity();
						static_cast<SimulateStage*>(getDownstreamWS())->onAssemblyRemoving(a);
					}
				}
			}
			a->setAssemblyState(newState);
		}
		debugReentrant = false;
	}
	// STUB: PLAYER 0x008e6860
	// STUB: STUDIO 0x00a04b60
	SleepStage::AssemblySet& SleepStage::stateToSet(Sim::AssemblyState state)
	{
		switch (state)
		{
		case Sim::RECURSIVE_WAKE_PENDING: return recursiveWakePending;
		case Sim::WAKE_PENDING: return wakePending;
		case Sim::AWAKE: return awake;
		case Sim::SLEEPING_CHECKING: return sleepingChecking;
		case Sim::SLEEPING_DEEPLY: return sleepingDeeply;
		case Sim::REMOVING: return removing;
		default:
			RBXASSERT(0);
			return awake;
		}
	}

	// STUB: PLAYER 0x008e6e60
	// STUB: STUDIO 0x00a050a0
	void SleepStage::onAssemblyAdded(Assembly* assembly)
	{
		STUB(0x008e6e60);
	}

	// STUB: PLAYER 0x008e7c90
	// STUB: STUDIO 0x00a05ec0
	void SleepStage::onAssemblyRemoving(Assembly* assembly)
	{
		STUB(0x008e7c90);
	}


































































	// mac: 0x59FE68 (batch221b: :932-:934 asserts + throttle stamp +
	// stepping-array admission + joint dispatch).
	// FUNCTION: PLAYER 0x008e7e00
	// FUNCTION: STUDIO 0x00a06030
	void SleepStage::onEdgeAdded(Edge* e)
	{
		RBXASSERT(!e->inOrDownstreamOfStage(this));  // :932
		RBXASSERT(e->getEdgeState() == Sim::UNDEFINED);  // :933
		RBXASSERT(e->getThrottleType() == Sim::UNDEFINED_THROTTLE);  // :934
		e->setThrottleType(Assembly::computeCanThrottle(e)
			? Sim::CAN_THROTTLE : Sim::CAN_NOT_THROTTLE);
		e->putInStage(this);
		if (e->getEdgeType() == Edge::CONTACT)
		{
			Contact* contact = static_cast<Contact*>(e);
			steppingContacts[contact->getThrottleType()].fastAppend(contact);
			contact->setEdgeState(Sim::STEPPING);
			numContactsInStage++;
		}
		else
		{
			e->setEdgeState(Sim::SLEEPING);
			changeJointState(static_cast<Joint*>(e), Sim::STEPPING);
		}
	}

	// mac: 0x5A00D8 (batch221b: :957/:979-:981 + wake routing + 0/3 reset).
	// STUB: PLAYER 0x008e8e90
	// STUB: STUDIO 0x00a070c0
	void SleepStage::onEdgeRemoving(Edge* e)
	{
		RBXASSERT(e->inOrDownstreamOfStage(this));  // :957
		if (e->getEdgeType() == Edge::CONTACT)
		{
			Sim::EdgeState state = e->getEdgeState();
			if (state == Sim::CONTACTING || state == Sim::CONTACTING_SLEEPING)
				wakeEvent(e);
			if (state != Sim::SLEEPING)
				changeContactState(static_cast<Contact*>(e), Sim::SLEEPING);
			numContactsInStage--;
		}
		else if (e->getEdgeState() == Sim::SLEEPING)
		{
			wakeEvent(e);
		}
		else
		{
			changeJointState(static_cast<Joint*>(e), Sim::SLEEPING);
		}




		RBXASSERT(e->inStage(this));  // :979
		RBXASSERT(e->getEdgeState() == Sim::SLEEPING);  // :980
		RBXASSERT(e->getThrottleType() != Sim::UNDEFINED_THROTTLE);  // :981
		e->setThrottleType(Sim::UNDEFINED_THROTTLE);
		e->setEdgeState(Sim::UNDEFINED);
		e->removeFromStage(this);
	}

	// Vector fan-out (batch257: trivial loop).
	void SleepStage::changeContactState(const std::vector<Contact*>& contacts, Sim::EdgeState newState)
	{
		for (size_t i = 0; i < contacts.size(); ++i)
			changeContactState(contacts[i], newState);
	}

	// Vector fan-out (batch257: trivial loop).
	void SleepStage::changeJointState(const std::vector<Joint*>& joints, Sim::EdgeState newState)
	{
		for (size_t i = 0; i < joints.size(); ++i)
			changeJointState(joints[i], newState);
	}

	// Vector fan-out (batch257: trivial loop).
	void SleepStage::changeAssemblyState(const std::vector<Assembly*>& assemblies, Sim::AssemblyState newState)
	{
		for (size_t i = 0; i < assemblies.size(); ++i)
			changeAssemblyState(assemblies[i], newState);
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::SleepStage, 0x14c);

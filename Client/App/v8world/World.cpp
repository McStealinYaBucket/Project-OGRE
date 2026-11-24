/* App/v8world/World.cpp -- October 2013.
   ctor 0x0074ae30 and dtor 0x00749ae0 rebuilt from the 2013 bytes (World.h has the layout); the dtor's
   asserts sit on their 2013 lines (187, 190-193).  The bodies after the dtor are still the earlier
   Mac-based reconstruction: STUB until each is checked against PLAYER, and their asserts are not on
   their 2013 lines yet (tools/assert_lines.py). */

#include "stdafx.h"
#include "decomp.h"
#include "V8World/World.h"
#include "V8World/CleanStage.h"
#include "V8World/MovingAssemblyStage.h"
#include "V8World/SendPhysics.h"
#include "V8World/TreeStage.h"
#include "V8World/SleepStage.h"
#include "V8World/StepJointsStage.h"
#include "V8Kernel/Kernel.h"
#include "V8World/Assembly.h"
#include "V8World/Contact.h"
#include "V8World/Geometry.h"
#include "V8World/Joint.h"
#include "V8World/JointBuilder.h"
#include "V8World/Primitive.h"
#include "V8DataModel/JointInstance.h"
#include "V8Kernel/Constants.h"

#include <boost/bind.hpp>
#include <set>
#include <vector>

// The 2013 flag macro registers only the flag (initializer PLAYER 0x00a658c0: no PlaceFilter string).
namespace FFlag {
	// GLOBAL: PLAYER 0x00d30efc
	// GLOBAL: STUDIO 0x00ec17e8
	bool ConsistentThrottlingIncrements = true;
	int ConsistentThrottlingIncrementsIniter = FLog::RegisterFlag("ConsistentThrottlingIncrements", &ConsistentThrottlingIncrements, NULL, FASTVARTYPE_STATIC);
}

namespace RBX {

	// GLOBAL: PLAYER 0x00df72a8
	// GLOBAL: STUDIO 0x00f918d4
	EThrottle::EThrottleType EThrottle::globalDebugEThrottle = EThrottle::ThrottleDefaultAuto;

	// 0/8, 1/8, 1/4, 1/3, 1/2, 2/3, 3/4, 7/8, 15/16 of the steps throttled
	// GLOBAL: PLAYER 0x00b52860
	// GLOBAL: STUDIO 0x00c240f0
	const int EThrottle::throttleSetting[] = {100000000, 8, 4, 3, 2, -3, -4, -8, -16};

	// inline: World::World 0x0074ae30 carries this body and no out-of-line copy exists.
	inline EThrottle::EThrottle()
		: requestedSkip(1)
		, throttleIndex(1)
	{
		if (FFlag::ConsistentThrottlingIncrements && globalDebugEThrottle == ThrottleDefaultAuto)
			usedSkip = 100000000;
		else
			usedSkip = 1;
	}

	// FUNCTION: PLAYER 0x00745590
	// FUNCTION: STUDIO 0x008863d0
	bool EThrottle::increaseLoad(bool increase)
	{
		if (FFlag::ConsistentThrottlingIncrements && globalDebugEThrottle == ThrottleDefaultAuto)
		{
			if (increase)
			{
				if (throttleIndex > 0)
				{
					throttleIndex -= 1;
					return true;
				}
			}
			else
			{
				if (throttleIndex < 8)
				{
					throttleIndex += 1;
					return true;
				}
			}
		}
		else
		{
			if (increase)
			{
				if (requestedSkip > 1)
				{
					requestedSkip /= 2;
					return true;
				}
			}
			else
			{
				if (requestedSkip < 16)
				{
					requestedSkip *= 2;
					return true;
				}
			}
		}
		return false;
	}

	// FUNCTION: PLAYER 0x00745600
	// FUNCTION: STUDIO 0x00886440
	bool EThrottle::computeThrottle(int step)
	{
		switch (globalDebugEThrottle)
		{
		case ThrottleDefaultAuto:
			if (FFlag::ConsistentThrottlingIncrements)
			{
				usedSkip = throttleSetting[throttleIndex];
				return (usedSkip > 0) == ((step % usedSkip) == 0);
			}
			else
			{
				usedSkip = requestedSkip;
				return (step % usedSkip) != 0;
			}
		case Skip2:
			usedSkip = 2;
			return (step % 2) != 0;
		case Skip4:
			usedSkip = 4;
			return (step % 4) != 0;
		case Skip8:
			usedSkip = 8;
			return (step % 8) != 0;
		case Skip16:
			usedSkip = 16;
			return (step % 16) != 0;
		case ThrottleAlways:
			usedSkip = 100000000;
			return true;
		case ThrottleDisabled:
		default:
			usedSkip = 1;
			return false;
		}
	}

	// FUNCTION: PLAYER 0x00745720
	// FUNCTION: STUDIO 0x00886560
	float EThrottle::getEnvironmentSpeed() const
	{
		if (FFlag::ConsistentThrottlingIncrements && globalDebugEThrottle == ThrottleDefaultAuto)
			return usedSkip > 0 ? (1.0f - (1.0f / usedSkip)) : -1.0f / usedSkip;
		else
			return 1.0f / usedSkip;
	}

	// FUNCTION: PLAYER 0x0074ae30
	// STUB: STUDIO 0x0088b780
	World::World()
		: frmThrottle(1)
		, inStepCode(false)
		, inJointNotification(NULL)
		, worldStepId(0)
		, groundPrimitive(new Primitive(Primitive::GEOMETRY_BLOCK))
		, numJoints(0)
		, numContacts(0)
		, numLinkCalls(0)
		, profilingBreak(new Profiling::CodeProfiler("Break Time"))
		, profilingAssembly(new Profiling::CodeProfiler("Assembler Time"))
		, profilingFilter(new Profiling::CodeProfiler("Filter Time"))
		, profilingWorldStep(new Profiling::CodeProfiler("World Step"))
		, profilingUiStep(new Profiling::CodeProfiler("UI Step"))
		, contactManager(new ContactManager(this))
		, sendPhysics(new SendPhysics())
		, cleanStage(new CleanStage(NULL, this))
	{
		groundPrimitive->setAnchoredProperty(true);
		groundPrimitive->setWorld(this);
		cleanStage->onPrimitiveAdded(groundPrimitive);

		std::vector<Profiling::CodeProfiler*> worldProfilers;
		loadProfilers(worldProfilers);
	}

	// FUNCTION: PLAYER 0x00749ae0
	// FUNCTION: STUDIO 0x0088a430
	World::~World()
	{
		cleanStage->onPrimitiveRemoving(groundPrimitive);
		RBXASSERT(groundPrimitive->getNumEdges() == 0);
		groundPrimitive->setWorld(NULL);

		RBXASSERT(numJoints == 0);
		RBXASSERT(numContacts == 0);
		RBXASSERT(primitives.size() == 0);
		RBXASSERT(breakableJoints.empty());

		delete groundPrimitive;
		delete sendPhysics;
		delete cleanStage;
		delete contactManager;
	}

	// FUNCTION: PLAYER 0x008bb760
	SendPhysics* World::getSendPhysics()
	{
		return sendPhysics;
	}

	// FUNCTION: PLAYER 0x00745780
	// FUNCTION: STUDIO 0x008865c0
	Kernel* World::getKernel()
	{
		return cleanStage->getKernel();
	}

	const Kernel* World::getKernel() const
	{
		return cleanStage->getKernel();
	}

	// mac: 0x5ABFBD.
	bool doNotIgnore(Primitive* p, const std::set<Primitive*>* ignore, const std::set<Primitive*>* include)
	{
		if (ignore != NULL)
			return ignore->find(p) == ignore->end();
		if (include != NULL)
			return include->find(p) != include->end();
		return true;
	}

	const SleepStage* World::getSleepStage() const
	{
		return static_cast<const SleepStage*>(cleanStage->findStage(IStage::SLEEP_STAGE));
	}

	const StepJointsStage* World::getStepJointsStage() const
	{
		return static_cast<const StepJointsStage*>(cleanStage->findStage(IStage::STEP_JOINTS_STAGE));
	}

	// 2013 order (0x00747f20): World's four, the ContactManager's, the SleepStage's four, the
	// StepJointsStage's and the Kernel's two; each stage lookup walks the chain again.
	// FUNCTION: PLAYER 0x00747f20
	// FUNCTION: STUDIO 0x00888870
	void World::loadProfilers(std::vector<Profiling::CodeProfiler*>& worldProfilers) const
	{
		worldProfilers.push_back(profilingBreak.get());
		worldProfilers.push_back(profilingAssembly.get());
		worldProfilers.push_back(profilingFilter.get());
		worldProfilers.push_back(profilingUiStep.get());
		worldProfilers.push_back(contactManager->profiler.get());
		worldProfilers.push_back(getSleepStage()->profilingCollision.get());
		worldProfilers.push_back(getSleepStage()->profilingJointSleep.get());
		worldProfilers.push_back(getSleepStage()->profilingWake.get());
		worldProfilers.push_back(getSleepStage()->profilingSleep.get());
		worldProfilers.push_back(getStepJointsStage()->profilingJointUpdate.get());
		worldProfilers.push_back(getKernel()->profilingKernelBodies.get());
		worldProfilers.push_back(getKernel()->profilingKernelConnectors.get());
	}


	// mac: 0x5A9998 (44B, batch776): the stage chain walk to the
	// MovingAssemblyStage, then the grounded-assembly removal there.
	// FUNCTION: PLAYER 0x00746270
	// FUNCTION: STUDIO 0x00887090
	int World::onAssemblyInSimluationStage(Assembly* a)
	{
		IStage* stageLink = cleanStage;   // binary: [this+0xC0] (+192).
		while (stageLink->getStageType() != IStage::MOVING_ASSEMBLY_STAGE)
		{
			stageLink = stageLink->getDownstream();   // binary: [i+8].
		}
		return static_cast<int>(
			static_cast<MovingAssemblyStage*>(stageLink)->removeMovingGroundedAssembly(a));
	}

	// mac: 0x5A9AB0 / 0x5A9ADA (42B each, batch219a): forward to the +192
	// stage (Joint::setPrimitive world path).
	// FUNCTION: PLAYER 0x00745a00
	// FUNCTION: STUDIO 0x00886840
	void World::onJointPrimitiveNulling(Joint* joint, Primitive* prim)
	{
		cleanStage->onJointPrimitiveNulling(joint, prim);
	}

	// FUNCTION: PLAYER 0x00745a10
	// FUNCTION: STUDIO 0x00886850
	void World::onJointPrimitiveSet(Joint* joint, Primitive* prim)
	{
		cleanStage->onJointPrimitiveSet(joint, prim);
	}

































































	// mac: 0x5A9120 (batch299: NULL-guard + world assert + Sleep tickle).
	// STUB: PLAYER 0x00745dd0
	// STUB: STUDIO 0x00886bf0
	void World::ticklePrimitive(Primitive* p, bool recursive)
	{
		if (p == NULL)
			return;
		RBXASSERT(p->getWorld());
		Assembly* assembly = p->getAssembly();
		if (assembly != NULL)
		{
			IStage* stage = getStage();
			while (stage != NULL && stage->getStageType() != IStage::SLEEP_STAGE)
				stage = stage->getDownstream();
			if (stage != NULL)
				static_cast<SleepStage*>(stage)->onExternalTickleAssembly(assembly, recursive);
		}
	}

	// STUB: PLAYER 0x00745e50
	// STUB: STUDIO 0x00886c70
	Assembly* World::onPrimitiveEngineChanging(Primitive* p)
	{
		STUB(0x00745e50);
		return NULL;
	}

	// STUB: PLAYER 0x00745fc0
	// STUB: STUDIO 0x00886de0
	void World::onPrimitiveEngineChanged(Assembly* a)
	{
		STUB(0x00745fc0);
	}

	// STUB: PLAYER 0x00746040
	// STUB: STUDIO 0x00886e60
	void World::onPrimitiveFixedChanging(Primitive* p)
	{
		STUB(0x00746040);
	}

	// STUB: PLAYER 0x00746110
	// STUB: STUDIO 0x00886f30
	void World::onPrimitiveFixedChanged(Primitive* p)
	{
		STUB(0x00746110);
	}

	// STUB: PLAYER 0x007461e0
	// STUB: STUDIO 0x00887000
	void World::onPrimitivePreventCollideChanged(Primitive* p)
	{
		STUB(0x007461e0);
	}

	// STUB: PLAYER 0x00745820
	// STUB: STUDIO 0x00886660
	void World::onPrimitiveContactParametersChanged(Primitive* p)
	{
		STUB(0x00745820);
	}




	// mac: 0x5A9754 (223B).
	// FUNCTION: PLAYER 0x007458a0
	// FUNCTION: STUDIO 0x008866e0
	void World::onPrimitiveExtentsChanged(Primitive* p)
	{
		RBXASSERT(p->getWorld());
		RBXASSERT(!inStepCode); contactManager->onPrimitiveExtentsChanged(p);
	}
	// FUNCTION: PLAYER 0x00747620
	// FUNCTION: STUDIO 0x00887f70
	void World::onAssemblyExtentsChanged(Assembly* a)
	{
		RBXASSERT(!inStepCode);
		RBXASSERT(a->getAssemblyPrimitive()->getWorld());
		a->visitPrimitives(boost::bind(&ContactManager::onPrimitiveExtentsChanged,
			contactManager, _1));
	}

	// mac: 0x5AA9F8 (32B).  Exactly one call: append to the +32 array.
	void World::reportTouchInfo(const TouchInfo& info)
	{
		touchReporting.append(info);
	}

	// mac: 0x5AAA18 (batch259: prim+253 anchored check gates the +124
	// low-nibble bump (high nibble preserved); then the +8 signal).
	void World::onPrimitiveCollided(Primitive* p0, Primitive* p1)
	{
		if (!p0->getAnchoredProperty())
			p0->bumpCollideCount();
		if (!p1->getAnchoredProperty())
			p1->bumpCollideCount();
		primitiveCollideSignal(std::make_pair(p0, p1));
	}

	// mac: 0x5AA47E (29B).
	int World::getUiStepId()
	{
		return worldStepId / Constants::worldStepsPerUiStep();
	}

	// mac: 0x5AAD81..0x5AAE9A: the four expand-tests.  getEdgeType() is 0 for every
	// real Joint, so in practice only a foreign edge satisfies them.
	bool World::isKernelOrForeignEdge(const Joint* joint)
	{
		return joint->getEdgeType() != 0 || joint->getJointType() == Joint::KERNEL_JOINT;   // 12
	}

	bool World::isAutoFamilyOrForeignEdge(const Joint* joint)
	{
		return joint->getEdgeType() != 0 ||
			(unsigned int)(joint->getJointType() - Joint::ROTATE_JOINT) <= (Joint::MANUAL_GLUE_JOINT - Joint::ROTATE_JOINT);
	}


































































































































































































































	// mac: 0x5AACA8.
	// STUB: PLAYER 0x0074a5d0
	// STUB: STUDIO 0x0088af20
	void World::insertJoint(Joint* joint)
	{
		assertNotInStep();
		RBXASSERT(inJointNotification == NULL);      // World.cpp:705

		Primitive* p0 = joint->getPrim0();
		Primitive* p1 = joint->getPrim1();
		if (p0 != NULL && p1 != NULL && p0 != p1 &&
			p0->getGeometryType() != Primitive::GEOMETRY_MEGACLUSTER &&
			p1->getGeometryType() != Primitive::GEOMETRY_MEGACLUSTER)
		{
			Joint* other = Primitive::getJoint(p0, p1, 0);
			if (other != NULL &&
				isKernelOrForeignEdge(other) && isKernelOrForeignEdge(joint) &&
				isAutoFamilyOrForeignEdge(other) && isAutoFamilyOrForeignEdge(joint))
			{
				destroyJoint(other);
			}
		}

		cleanStage->onEdgeAdded(joint);                  // vtbl+0x10
		++numJoints;                                 // +120

		if (joint->isBreakable())
		{
			// DELTA: 2013 guards this with vtbl slot 2; isBreakable() keeps it symmetric with removeFromBreakable.
			bool ok = breakableJoints.insert(joint).second;



			RBXASSERT(ok);                           // World.cpp:732 "ok"
		}

		if (joint->getEdgeType() == 0 &&	// a user joint: type in [1,5]
			(unsigned int)(joint->getJointType() - Joint::WELD_JOINT) <= (Joint::MOTOR_6D_JOINT - Joint::WELD_JOINT))
			joint->notifyMoved();
	}
	// FUNCTION: PLAYER 0x00747e40
	// FUNCTION: STUDIO 0x00888790
	void World::removeFromBreakable(Joint* joint)
	{
		if (joint->isBreakable())
		{
			size_t num = breakableJoints.erase(joint);
			RBXASSERT(num == 1);
		}
	}

	// mac: 0x5AAFD2.
	// FUNCTION: PLAYER 0x00747eb0
	// FUNCTION: STUDIO 0x00888800
	void World::removeJoint(Joint* joint)
	{
		assertNotInStep();
		removeFromBreakable(joint);
		cleanStage->onEdgeRemoving(joint);                // vtbl+0x14
		--numJoints;
	}

	// mac: 0x5AA022 -- walk the breakable set and destroy every joint that is
	// broken AND currently in the joint-simulation stage.  The iterator is
	// advanced BEFORE the destroy (disasm 0x5AA04C calls _Rb_tree_increment
	// ahead of the 0x5AA061 inStage / 0x5AA06F isBroken pair), so erasing the
	// current node inside destroyJoint -> removeFromBreakable is safe.
	void World::doBreakJoints()
	{
		std::set<Joint*>::iterator it = breakableJoints.begin();
		while (it != breakableJoints.end())
		{
			Joint* joint = *it;
			++it;                                       // 0x5AA04C
			if (joint->inStage(IStage::KERNEL_STAGE) &&  // 0x5AA061 (binary passes literal 15)
				joint->isBroken())                     // 0x5AA06F vtbl+0x24
			{
				destroyJoint(joint);                   // 0x5AA040
			}
		}
	}

	// mac: 0x5AAEB6 -- RBXASSERT(j) at World.cpp:806.
	void World::destroyJoint(Joint* joint)
	{
		RBXASSERT(joint);
		inJointNotification = joint;
		autoDestroySignal(joint);
		inJointNotification = NULL;
	}

	// mac: 0x5AB290.
	void World::insertContact(Contact* contact)
	{
		cleanStage->onEdgeAdded(contact);                 // vtbl+0x10
		++numContacts;                                // +124
	}























	// mac: 0x5AB2B8.
	// STUB: PLAYER 0x00745a80
	// STUB: STUDIO 0x008868b0
	void World::destroyContact(Contact* contact)
	{
		RBXASSERT(!inStepCode);
		cleanStage->onEdgeRemoving(contact);               // vtbl+0x14
		--numContacts;
	}

















	// mac: 0x5AB4E0 (885B).  Asserts carried verbatim (World.cpp:856-861 + :869).
	// Calls CleanStage::onPrimitiveAdded then ContactManager::onPrimitiveAdded
	// (PLAYER 0x007473d5 / 0x007473e1); 2013's CleanStage does not touch the CM.
	//
	// The +60
	// IndexArray append + :60/:61 asserts are spelled with the primitives
	// member directly (worldIndex = size pre-append).
	// STUB: PLAYER 0x007471f0
	// STUB: STUDIO 0x00887cb0
	void World::insertPrimitive(Primitive* p)
	{
		RBXASSERT(!inStepCode);  // :856 "!inStepCode" (World.cpp:856)
		RBXASSERT(!p->getAssembly());  // :857 "!p->getAssembly()"
		RBXASSERT(!p->inPipeline());  // :858 "!p->inPipeline()"
		RBXASSERT(!p->getFirstEdge());  // :859 "!p->getFirstEdge()"
		RBXASSERT(!p->getNumEdges());  // :860 "!p->getNumEdges()"
		RBXASSERT(!p->getWorld());  // :861 "!p->getWorld()"
		primitives.fastAppend(p);
		p->setWorld(this);
		// IWorldStage lacks the hooks (see header note); same static_cast
		// the harness uses.
		cleanStage->onPrimitiveAdded(p);  contactManager->onPrimitiveAdded(p);


		RBXASSERT(!inStepCode);  // :869 "!inStepCode" (World.cpp:869)
	}
	// STUB: PLAYER 0x0074a790
	// STUB: STUDIO 0x0088b0e0
	void World::removePrimitive(Primitive* p, bool keepJoints)
	{
		RBXASSERT(!inStepCode);  // :875 "!inStepCode" (World.cpp:875)
		RBXASSERT(p->inPipeline());  // :876 "p->inPipeline()" (World.cpp:876)
		RBXASSERT(p->getWorld());  // :877 "p->getWorld()" (World.cpp:877)
		destroyAutoJoints(p, NULL, !keepJoints, !keepJoints);
		contactManager->onPrimitiveRemoved(p);
		cleanStage->onPrimitiveRemoving(p);
		primitives.fastRemove(p);
		p->setWorld(NULL);





		RBXASSERT(p->getFirstEdge() == NULL);  // :888 (World.cpp:888)
		RBXASSERT(p->getNumEdges() == 0);  // :889 (World.cpp:889)
		RBXASSERT(!inStepCode);  // :890 "!inStepCode" (World.cpp:890)
		RBXASSERT(!p->getAssembly());  // :891 "!p->getAssembly()"
		RBXASSERT(!p->inPipeline());  // :892 "!p->inPipeline()" (World.cpp:892)
		RBXASSERT(!p->getWorld());  // :893 "!p->getWorld()" (World.cpp:893)
	}

	// mac: 0x5AC168 -- destroyAutoJoints(p, NULL, b, true).  5-arg body 0x5ABBEA.
	void World::destroyAutoJoints(Primitive* prim, bool b)
	{
		destroyAutoJoints(prim, NULL, b, true);
	}

	// mac: 0x5ABBEA (979B).
	// Phase 1: for each joint edge on prim, select by (autoJoints &&
	// isAutoJoint()) or (manualJoints && edge is a manual-family JOINT:
	// getEdgeType()==0 with (getJointType() & ~8)==2, i.e. MANUAL_WELD(2)
	// or MANUAL_GLUE(10) -- both calls go through the vtable, slots +20
	// and +28, so overrides are honored exactly like the binary).  Collect
	// shared_from(owner) into tempJoints (skip null owners).
	// Phase 2: for the [startIndex, size) owners, read the Joint* back via
	// the +132 backlink, assert it links prim (:937 "destroyJ->links(p)"),
	// destroyJoint it iff the FAR prim passes doNotIgnore(other, ignore,
	// NULL).  Then remove the range from tempJoints and assert the size is
	// back to startIndex (:948).  The remaining asserts ("destroyJ" :920,
	// owner non-null :924, tempJoint non-null :936) are ported verbatim.
	void World::destroyAutoJoints(Primitive* prim,
		std::set<Primitive*>* ignorePrims, bool manualJoints, bool autoJoints)
	{
		assertNotInStep();  // World.cpp:910 "!inStepCode"
		int startIndex = (int)tempJoints.size();
		for (int i = 0; i < prim->joints.size(); ++i)
		{
			Joint* joint = static_cast<Joint*>(prim->getJoint(i));
			RBXASSERT(joint != NULL);  // World.cpp:920 "destroyJ"
			if ((autoJoints && joint->isAutoJoint())
				|| (manualJoints && joint->getEdgeType() == 0
					&& ((joint->getJointType() & ~8) == 2)))
			{
				JointInstance* owner = joint->getJointOwner();
				RBXASSERT(owner != NULL);  // World.cpp:924 "jointOwner"
				if (owner != NULL)
					tempJoints.append(shared_from(owner));
			}
		}
		for (int k = startIndex; k < (int)tempJoints.size(); ++k)
		{
			JointInstance* owner = tempJoints[k].get();
			RBXASSERT(owner != NULL);  // World.cpp:936 "destroyJ"
			Joint* joint = owner->getJoint();
			// World.cpp:937 "destroyJ->links(p)".
			RBXASSERT(joint->getPrim0() == prim || joint->getPrim1() == prim);
			Primitive* other = (joint->getPrim0() == prim)
				? joint->getPrim1() : joint->getPrim0();
			if (doNotIgnore(other, ignorePrims, NULL))
				destroyJoint(joint);
		}
		tempJoints.remove(tempJoints.begin() + startIndex,
			(int)tempJoints.size() - startIndex);
		// World.cpp:948 "startIndex == tempJoints.size()".
		RBXASSERT((int)tempJoints.size() == startIndex);
	}

	// mac: 0x5AB4B2 -- createAutoJoints(p, NULL, NULL).  The 3-arg body (0x5AC2E0,
	// 335 bytes) needs ContactManager::getPrimitivesTouchingExtents 0x563F30
	// and JointBuilder::canJoin 0x57B164, so the builder loop is still TODO.
	void World::createAutoJoints(Primitive* prim)
	{
		createAutoJoints(prim, NULL, NULL);
	}





























	// mac: 0x5AC2E0 (335B).  assertNotInStep (World.cpp:999); ++numLinkCalls;
	// tempPrimitives.resize(0, false); fill via the hash walk (flag 0 =
	// unlimited); for each survivor: doNotIgnore(ignore, include) gate,
	// skip when prim->getJoint(other, 0) already links them, else canJoin
	// and on success insertJoint + the inJointNotification-bracketed signal
	// fire (same bracket as destroyJoint).
	// STUB: PLAYER 0x0074aa80
	// STUB: STUDIO 0x0088b3d0
	void World::createAutoJoints(Primitive* prim,
		std::set<Primitive*>* ignorePrims, std::set<Primitive*>* includePrims)
	{
		RBXASSERT(!inStepCode);
		++numLinkCalls;
		tempPrimitives.resize(0, false);
		contactManager->getPrimitivesTouchingExtents(
			prim->getFastFuzzyExtents(), prim, 0, tempPrimitives);
		for (int i = 0; i < tempPrimitives.size(); ++i)
		{
			Primitive* other = tempPrimitives[i];
			if (!doNotIgnore(other, ignorePrims, includePrims))
				continue;
			if (Primitive::getJoint(prim, other, 0) != NULL)
				continue;
			if (Joint* joint = JointBuilder::canJoin(prim, other))
			{
				insertJoint(joint);
				inJointNotification = joint;
				autoJoinSignal(joint);
				inJointNotification = NULL;
			}
		}
	}

	void World::joinAll()
	{
		// mac: 0x5AB34C (357B, batch205/209): per-prim IndexArray walk (:103
		// index asserts flank the scan) + MegaCluster-with-geometry skip +
		// surface prefilter (skip prims with no joinable face) +
		// createAutoJoints(p, NULL, NULL) per survivor + assemble() tail
		// (jmp -- spelled as the call; assemble's own stage walk is TODO).
		// DELTA (+236): the binary skips only when the Geometry object at
		// prim+236 is non-null AND the type is MEGACLUSTER.  Geometry
		// objects are unported (no in-tree counterpart -- the slot reads
		// would be model-relative garbage), so only the type half is
		// spelled; in-tree the extra conjunct is dead either way.
		// DELTA (dedup): multi-prim dedup rides prim edge lists via the
		// wired chain (insertContact/insertJoint -> Clean -> ... ->
		// EdgeStage::insertEdge links into the lists), arming the hash's
		// getContact gate -- one live edge per pair, binary behavior.
		for (int i = 0; i < primitives.size(); ++i)
		{
			Primitive* prim = primitives[i];
			if (prim->getGeometryType() == Primitive::GEOMETRY_MEGACLUSTER)
				continue;
			bool hasJoinable = false;
			for (int s = 0; s < 6; ++s)
			{
				if (!IsNoSurface(prim->getSurfaceType(static_cast<NormalId>(s))))
				{
					hasJoinable = true;
					break;
				}
			}
			if (!hasJoinable)
				continue;
			createAutoJoints(prim, NULL, NULL);
		}
		assemble();
	}

	void World::assemble()
	{
		// mac: 0x5A9B04 (46B, batch209): walk the World+192 stage chain via
		// getStageType (vtbl+8) to TREE_STAGE (5), then TreeStage::assemble.
		// DELTA (NULL-tripwire): the binary chain always reaches TreeStage;
		// ours is complete (CleanStage-wired), so the tripwire is dead but
		// harmless. The assemble terminal queues behind cleanMechanism
		// (Mechanism flow -- TreeStage::assemble would link it).
		IStage* stageIt = cleanStage;
		while (stageIt != NULL && stageIt->getStageType() != IStage::TREE_STAGE)
			stageIt = stageIt->getDownstream();
		if (stageIt == NULL)
			return;
		// TreeStage::assemble 0x5A5CB8 (batch214) -- landed with the
		// assemble chain (cleanMechanism era).
		static_cast<TreeStage*>(stageIt)->assemble();
	}

	// mac: 0x5A9B32: walk to type 5, answer the tree's set-empty.
	bool World::isAssembled()
	{
		IStage* stageIt = cleanStage;
		while (stageIt != NULL && stageIt->getStageType() != IStage::TREE_STAGE)
			stageIt = stageIt->getDownstream();
		if (stageIt == NULL)
			return true;
		TreeStage* ts = static_cast<TreeStage*>(stageIt);
		fflush(stderr);
		return ts->isAssembled();
	}

} // namespace RBX

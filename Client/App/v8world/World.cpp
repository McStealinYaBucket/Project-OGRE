/* App/v8world/World.cpp -- October 2013 (edge lifecycle + world shell).
   Bodies below are transcribed from the Studio symbolled binary; the line
   numbers in the RBXASSERTs are the binary's own World.cpp lines.
   TODO: full decompile (step/assemble/joinAll/auto-joint builders),
   D2 0x5A8A28's ground teardown + :187-:193 asserts.
   Self-audit (sendPhysics slice, batch776/778): fidelity ~99% for the
     landed ctor/dtor/getSendPhysics/onAssemblyInSimluationStage shapes
     (deleting order groundPrim -> sendPhysics -> stage -> contactManager,
     +188/+192 hands, the getStageType walk); function: the class remains
     under the TODO stack above (step/assemble drives era by era). */

#include "stdafx.h"

#include "V8World/World.h"
#include "V8World/CleanStage.h"
#include "V8World/MovingAssemblyStage.h"
#include "V8World/SendPhysics.h"
#include "V8World/TreeStage.h"
#include "V8World/SleepStage.h"
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

namespace RBX {

	// mac: 0x5ABFBD.
	bool doNotIgnore(Primitive* p, const std::set<Primitive*>* ignore, const std::set<Primitive*>* include)
	{
		if (ignore != NULL)
			return ignore->find(p) == ignore->end();
		if (include != NULL)
			return include->find(p) != include->end();
		return true;
	}

	// STUB: PLAYER 0x00749ae0
	// STUB: STUDIO 0x0088a430
	World::~World()
	{
		// mac: 0x5A8A28 (batch778) deleting order: groundPrim ->
		// sendPhysics -> stage -> contactManager. (The D2's
		// CleanStage::onPrimitiveRemoving(groundPrimitive), its
		// World.cpp:187-:193 empty-state asserts and the groundPrim
		// field-clears queue with their own pass -- not modelled yet.)
		delete megaClusterPrim;
		megaClusterPrim = NULL;
		delete sendPhysics;
		sendPhysics = NULL;
		delete stage;
		stage = NULL;
		delete contactManager;
		contactManager = NULL;
	}

	// STUB: PLAYER 0x0074ae30
	// STUB: STUDIO 0x0088b780
	World::World()
		: inStepCode(false)
		, inJointNotification(NULL)
		, uiStepWorldCounter(0)
		, megaClusterPrim(new Primitive(Primitive::GEOMETRY_BLOCK))
		, numJoints(0)
		, numContacts(0)
		, int128(0)
		, contactManager(new ContactManager(this))   // +184 (C2 0x5A8230 order:
		                                             // ContactManager first).
		, sendPhysics(new SendPhysics())             // +188 (binary new(0x24))
		, stage(new CleanStage(NULL, this))          // +192 (CleanStage last).
	{
		// mac: 0x5A8308: setAnchoredProperty(true) + the world backref at
		// megaClusterPrim+World, then CleanStage::onPrimitiveAdded.
		megaClusterPrim->setAnchoredProperty(true);
		static_cast<CleanStage*>(stage)->onPrimitiveAdded(megaClusterPrim);
	}

	// mac: 0x5A902E (batch776/777): the binary reads this+188 in one
	// instruction; the member hand-out reproduces it.
	SendPhysics* World::getSendPhysics()
	{
		return sendPhysics;   // +188.
	}

	// mac: 0x5A9998 (44B, batch776): the stage chain walk to the
	// MovingAssemblyStage, then the grounded-assembly removal there.
	int World::onAssemblyInSimluationStage(Assembly* a)
	{
		IStage* stageLink = stage;   // binary: [this+0xC0] (+192).
		while (stageLink->getStageType() != IStage::MOVING_ASSEMBLY_STAGE)
		{
			stageLink = stageLink->getDownstream();   // binary: [i+8].
		}
		return static_cast<int>(
			static_cast<MovingAssemblyStage*>(stageLink)->removeMovingGroundedAssembly(a));
	}

	// mac: 0x5AC8D4 -- "!inStepCode file: include/V8World/World.h line: 142".
	void World::assertNotInStep()
	{
		RBXASSERT(!inStepCode);
	}

	// mac: 0x5A9AB0 / 0x5A9ADA (42B each, batch219a): forward to the +192
	// stage (Joint::setPrimitive world path).
	// STUB: PLAYER 0x00745a00
	// STUB: STUDIO 0x00886840
	void World::onJointPrimitiveNulling(Joint* joint, Primitive* prim)
	{
		static_cast<CleanStage*>(stage)->onJointPrimitiveNulling(joint, prim);
	}

	// STUB: PLAYER 0x00745a10
	// STUB: STUDIO 0x00886850
	void World::onJointPrimitiveSet(Joint* joint, Primitive* prim)
	{
		static_cast<CleanStage*>(stage)->onJointPrimitiveSet(joint, prim);
	}

	// mac: 0x5AAF40 -- RBXASSERT(num == 1) at World.cpp:746.
	// STUB: PLAYER 0x00747e40
	// STUB: STUDIO 0x00888790
	void World::removeFromBreakable(Joint* joint)
	{
		if (joint->isBreakable())
		{
			size_t num = breakableJoints.erase(joint);
			RBXASSERT(num == 1);
		}
	}

	// mac: 0x5AAD81..0x5AAE9A: the four expand-tests.  getEdgeType() is 0 for every
	// real Joint, so in practice only a foreign edge satisfies them.
	bool World::isKernelOrForeignEdge(const Joint* joint)
	{
		return joint->getEdgeType() != 0 || joint->getJointType() == KERNEL_JOINT;   // 12
	}

	bool World::isAutoFamilyOrForeignEdge(const Joint* joint)
	{
		return joint->getEdgeType() != 0 ||
			(unsigned int)(joint->getJointType() - ROTATE_JOINT) <= (MANUAL_GLUE_JOINT - ROTATE_JOINT);
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
			Joint* other = p0->getJoint(p1, 0);
			if (other != NULL &&
				isKernelOrForeignEdge(other) && isKernelOrForeignEdge(joint) &&
				isAutoFamilyOrForeignEdge(other) && isAutoFamilyOrForeignEdge(joint))
			{
				destroyJoint(other);
			}
		}

		stage->onEdgeAdded(joint);                  // vtbl+0x10
		++numJoints;                                 // +120

		if (joint->isBreakable())
		{
			// DELTA: the binary guards this with vtbl slot 2
			// (IPipelined::putInKernel, 0x576E4A) whose argument is not
			// recoverable from the 80-insn disasm head we have; isBreakable()
			// is used so insert/erase stay symmetric with removeFromBreakable.
			bool ok = breakableJoints.insert(joint).second;
			RBXASSERT(ok);                           // World.cpp:732 "ok"
		}

		// getEdgeType() == 0 and type in [1,5] => a user joint.
		if (joint->getEdgeType() == 0 &&
			(unsigned int)(joint->getJointType() - WELD_JOINT) <= (MOTOR_6D_JOINT - WELD_JOINT))
		{
			joint->notifyMoved();
		}
	}

	// mac: 0x5AAFD2.
	// STUB: PLAYER 0x00747eb0
	// STUB: STUDIO 0x00888800
	void World::removeJoint(Joint* joint)
	{
		assertNotInStep();
		removeFromBreakable(joint);
		stage->onEdgeRemoving(joint);                // vtbl+0x14
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
		inJointNotification = joint;                 // +52
		jointNotificationSignal(joint);              // signal at +4
		inJointNotification = NULL;
	}

	// mac: 0x5AB290.
	void World::insertContact(Contact* contact)
	{
		stage->onEdgeAdded(contact);                 // vtbl+0x10
		++numContacts;                                // +124
	}

	// mac: 0x5AB2B8.
	// STUB: PLAYER 0x00745a80
	// STUB: STUDIO 0x008868b0
	void World::destroyContact(Contact* contact)
	{
		stage->onEdgeRemoving(contact);               // vtbl+0x14
		--numContacts;
	}

	// mac: 0x5AA9F8 (32B).  Exactly one call: append to the +32 array.
	void World::reportTouchInfo(const TouchInfo& info)
	{
		touchInfoQueue.append(info);
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

	// mac: 0x5A9120 (batch299: NULL-guard + world assert + Sleep tickle).
	// STUB: PLAYER 0x00745dd0
	// STUB: STUDIO 0x00886bf0
	void World::ticklePrimitive(Primitive* prim, bool recursive)
	{
		if (prim == NULL)
			return;
		RBXASSERT(prim->getWorld() != NULL);  // :362
		Assembly* assembly = prim->getAssembly();
		if (assembly != NULL)
		{
			IStage* stage = getStage();
			while (stage != NULL && stage->getStageType() != IStage::SLEEP_STAGE)
				stage = stage->getDownstream();
			if (stage != NULL)
				static_cast<SleepStage*>(stage)->onExternalTickleAssembly(assembly, recursive);
		}
	}

	// mac: 0x5A9834 (batch299: :432/:433/Assembly.h:203 asserts + extents
	// fan. Binary calls visitPrimitivesImpl directly; the public
	// visitPrimitives walks identically from the same root).
	// STUB: PLAYER 0x00747620
	// STUB: STUDIO 0x00887f70
	void World::onAssemblyExtentsChanged(Assembly* assembly)
	{
		RBXASSERT(!inStepCode);  // :432
		Primitive* assemblyPrim = assembly->getAssemblyPrimitive();
		RBXASSERT(assemblyPrim == NULL || assemblyPrim->getWorld() != NULL);  // :433 shape
		RBXASSERT(assemblyPrim != NULL);  // Assembly.h:203 "p"
		assembly->visitPrimitives(boost::bind(&ContactManager::onPrimitiveExtentsChanged,
			contactManager, _1));
	}

	// mac: 0x5A9754 (223B).
	// STUB: PLAYER 0x007458a0
	// STUB: STUDIO 0x008866e0
	void World::onPrimitiveExtentsChanged(Primitive* prim)
	{
		RBXASSERT(prim->getWorld() != NULL);  // :425 "p->getWorld()"
		assertNotInStep();  // :426 "!inStepCode" (established equivalent).
		contactManager->onPrimitiveExtentsChanged(prim);
	}

	// mac: 0x5AB4E0 (885B).  Asserts carried verbatim (World.cpp:856-861 + :869).
	// Routing DELTA: binary calls CleanStage::onPrimitiveAdded (pipeline,
	// unported past the entry) AND ContactManager::onPrimitiveAdded; our
	// stage entry already forwards to the CM (see CleanStage.cpp), so one
	// call covers both -- same terminal state, no double-insert.  The +60
	// IndexArray append + :60/:61 asserts are spelled with the primitives
	// member directly (worldIndex = size pre-append).
	// STUB: PLAYER 0x007471f0
	// STUB: STUDIO 0x00887cb0
	void World::insertPrimitive(Primitive* prim)
	{
		RBXASSERT(!inStepCode);  // :856 "!inStepCode" (World.cpp:856)
		RBXASSERT(prim->getAssembly() == NULL);  // :857 "!p->getAssembly()"
		RBXASSERT(!prim->inPipeline());  // :858 "!p->inPipeline()"
		RBXASSERT(prim->getFirstEdge() == NULL);  // :859 "!p->getFirstEdge()"
		RBXASSERT(prim->getNumEdges() == 0);  // :860 "!p->getNumEdges()"
		RBXASSERT(prim->getWorld() == NULL);  // :861 "!p->getWorld()"
		RBXASSERT(prim != NULL);  // IndexArray.h:60 "item"
		RBXASSERT(prim->getWorldIndex() == -1);  // IndexArray.h:61 "indexOf(item) == -1"
		prim->setWorldIndex((int)primitives.size());
		primitives.append(prim);
		prim->setWorld(this);
		// IWorldStage lacks the hooks (see header note); same static_cast
		// the harness uses.
		static_cast<CleanStage*>(stage)->onPrimitiveAdded(prim);
		RBXASSERT(!inStepCode);  // :869 "!inStepCode" (World.cpp:869)
	}

	// mac: 0x5AB856 (916B).  Asserts carried verbatim (:875-:877, :888-:893).
	// The swap-remove is IndexArray::fastRemove via worldIndexFunc 0x5AB8D4
	// spelled inline (back-swap + moved-occupant fix + index reset).
	// STUB: PLAYER 0x0074a790
	// STUB: STUDIO 0x0088b0e0
	void World::removePrimitive(Primitive* prim, bool keepJoints)
	{
		RBXASSERT(!inStepCode);  // :875 "!inStepCode" (World.cpp:875)
		RBXASSERT(prim->inPipeline());  // :876 "p->inPipeline()" (World.cpp:876)
		RBXASSERT(prim->getWorld() != NULL);  // :877 "p->getWorld()" (World.cpp:877)
		destroyAutoJoints(prim, NULL, !keepJoints, !keepJoints);
		contactManager->onPrimitiveRemoved(prim);
		static_cast<CleanStage*>(stage)->onPrimitiveRemoving(prim);
		// IndexArray::fastRemove(this+60, prim) equivalent.
		RBXASSERT(prim->getWorldIndex() >= 0
			&& prim->getWorldIndex() < (int)primitives.size());
		RBXASSERT(primitives[prim->getWorldIndex()] == prim);
		{
			int idx = prim->getWorldIndex();
			primitives[idx] = primitives.back();
			primitives.pop_back();
			if (idx < (int)primitives.size())
				primitives[idx]->setWorldIndex(idx);
			prim->setWorldIndex(-1);
		}
		prim->setWorld(NULL);
		RBXASSERT(prim->getFirstEdge() == NULL);  // :888 (World.cpp:888)
		RBXASSERT(prim->getNumEdges() == 0);  // :889 (World.cpp:889)
		RBXASSERT(!inStepCode);  // :890 "!inStepCode" (World.cpp:890)
		RBXASSERT(prim->getAssembly() == NULL);  // :891 "!p->getAssembly()"
		RBXASSERT(!prim->inPipeline());  // :892 "!p->inPipeline()" (World.cpp:892)
		RBXASSERT(prim->getWorld() == NULL);  // :893 "!p->getWorld()" (World.cpp:893)
	}

	// mac: 0x5AA47E (29B).
	int World::getUiStepId()
	{
		return uiStepWorldCounter / Constants::worldStepsPerUiStep();
	}

	// mac: 0x5AB4B2 -- createAutoJoints(p, NULL, NULL).  The 3-arg body (0x5AC2E0,
	// 335 bytes) needs ContactManager::getPrimitivesTouchingExtents 0x563F30
	// and JointBuilder::canJoin 0x57B164, so the builder loop is still TODO.
	void World::createAutoJoints(Primitive* prim)
	{
		createAutoJoints(prim, NULL, NULL);
	}

	// mac: 0x5AC2E0 (335B).  assertNotInStep (World.cpp:999); ++int128;
	// scratchPrimitives.resize(0, false); fill via the hash walk (flag 0 =
	// unlimited); for each survivor: doNotIgnore(ignore, include) gate,
	// skip when prim->getJoint(other, 0) already links them, else canJoin
	// and on success insertJoint + the inJointNotification-bracketed signal
	// fire (same bracket as destroyJoint).
	// STUB: PLAYER 0x0074aa80
	// STUB: STUDIO 0x0088b3d0
	void World::createAutoJoints(Primitive* prim,
		std::set<Primitive*>* ignorePrims, std::set<Primitive*>* includePrims)
	{
		assertNotInStep();  // World.cpp:999 "!inStepCode"
		++int128;
		scratchPrimitives.resize(0, false);
		contactManager->getPrimitivesTouchingExtents(
			prim->getFastFuzzyExtents(), prim, 0, scratchPrimitives);
		for (int i = 0; i < scratchPrimitives.size(); ++i)
		{
			Primitive* other = scratchPrimitives[i];
			if (!doNotIgnore(other, ignorePrims, includePrims))
				continue;
			if (prim->getJoint(other, 0) != NULL)
				continue;
			if (Joint* joint = JointBuilder::canJoin(prim, other))
			{
				insertJoint(joint);
				inJointNotification = joint;
				jointNotificationSignal(joint);
				inJointNotification = NULL;
			}
		}
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
		for (int i = 0; i < (int)primitives.size(); ++i)
		{
			RBXASSERT(primitives[i]->getWorldIndex() == i);  // IndexArray.h:103 "indexOf(array[n]) == n"
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
			RBXASSERT(primitives[i]->getWorldIndex() == i);  // IndexArray.h:103 (dispatch gate)
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
		IStage* stageIt = stage;
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
		IStage* stageIt = stage;
		while (stageIt != NULL && stageIt->getStageType() != IStage::TREE_STAGE)
			stageIt = stageIt->getDownstream();
		if (stageIt == NULL)
			return true;
		TreeStage* ts = static_cast<TreeStage*>(stageIt);
		fflush(stderr);
		return ts->isAssembled();
	}

} // namespace RBX

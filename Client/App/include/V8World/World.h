/* V8World/World.h -- October 2013 (physics world + full member map).
   Evidence: World C2 0x5A8230 (every field), D1 0x5A8A1E, reset 0x518A3E,
     assemble 0x5A9B04 / isAssembled 0x5A9B32 / joinAll 0x5AB34C,
     insertJoint 0x5AACA8 / destroyJoint 0x5AAEB6 / removeFromBreakable
     0x5AAF40 / removeJoint 0x5AAFD2 / insertContact 0x5AB290 /
     destroyContact 0x5AB2B8 / assertNotInStep 0x5AC8D4,
     createAutoJoints 0x5AB4B2 + 0x5AC2E0, destroyAutoJoints 0x5AC168 +
     0x5ABBEA, create/destroyAutoJointsToWorld 0x5AC430 / 0x5AC19E,
     createAutoJointsToPrimitives 0x5AC56A, doNotIgnore 0x5ABFBD.
   RBX::World is NOT polymorphic in this build (no __ZTVN3RBX5WorldE, no
     typeinfo, ctor stores no vptr).

   ---- BINARY MEMBER MAP (Mach-O World* offsets, ORDER kept) ----
     +0   dword 0        signal<void(Joint*)>            [ctor zeroes]
     +4   dword 1        signal_with_args<1,void(Joint*)>  <-- the one
                          destroyJoint 0x5AAF26 fires (lea eax,[edi+4])
     +8   dword 2        signal<void(pair<Primitive*,Primitive*>)>
     +12  dword 3        signal<void(Primitive*)>
     +16  dword 4  = 1   (role TODO)
     +20  dword 5  = 1   (role TODO)
     +24  dword 6  = 1, or 100000000 when
                          (flag && *(int*)&dword_A01226 == 0)   (role TODO)
     +28  dword 7  = 1   (role TODO)
     +32  G3D::Array<World::TouchInfo,10,32>  (12B)   (ctor Array@32)
     +48  byte           inStepCode            (ctor 0, assertNotInStep
                                               World.h:142 asserts !it)
     +52  dword 13       Joint* inJointNotification (ctor 0; insertJoint
                                               asserts NULL World.cpp:705;
                                               destroyJoint brackets the fire)
     +56  dword 14 = 0   (role TODO)
     +60  G3D::Array<Primitive*,10,32>        (12B)   (ctor Array@60)
     +76  dword 19       Primitive* megaClusterPrim = new Primitive(2)
                          (ctor 0x5A82CE, geometryType 2 = GEOMETRY_BLOCK;
                          setAnchoredProperty(true) + world backref +
                          CleanStage::onPrimitiveAdded right after)
     +80  G3D::Array<Primitive*,10,32>        (12B)   (ctor Array@80)
     +100 std::set<Joint*>  (20B: _M_color=100,_M_parent=104,_M_left=108,
                          _M_right=112,_M_node_count=116 -- the ctor writes
                          exactly those five dwords with 0,0,&+100,&+100,0).
                          insertJoint _M_insert_unique + removeFromBreakable
                          erase + RBXASSERT(num==1) World.cpp:746.  NOTE IDA
                          prints the set at +96; the ctor proves +100.)
     +120 dword 30       unsigned numJoints   (++ 0x5AACE9ish / -- removeJoint)
     +124 dword 31       unsigned numContacts (++ insertContact 0x5AB2AF)
     +128 dword 32       ++ in createAutoJoints 0x5AC2E0 (role TODO)
     +132 G3D::Array<Primitive*,10,32>        (12B)   createAutoJoints
                          scratch (resize(0,0) then filled by
                          ContactManager::getPrimitivesTouchingExtents)
     +148 G3D::Array<shared_ptr<JointInstance>,10,32> (12B)  tempJoints
                          (destroyAutoJoints asserts startIndex == size())
     +164..+180          five CodeProfiler* (0xA01C each: "Broadphase",
                          <unnamed>, "Filter Time", "World Step", "UI Step")
     +184 dword 46       ContactManager* contactManager = new (0x54)
                          ContactManager(this) -- a RAW pointer, read straight
                          through by createAutoJoints (*((_DWORD*)a1+46)) and
                          by Workspace::getContactManagerIfInWorkspace 0x512568
     +188 dword 47       SendPhysics* sendPhysics = new (0x24) SendPhysics()
     +192 dword 48       IWorldStage* stage = new (0x10) CleanStage(NULL, this)
                          -- vtbl+0x10/+0x14 == onEdgeAdded/onEdgeRemoving
   End (0x196 rounded to 0x1A0 = 416 bytes).

   DELTAS: the four signals at +0/+8/+12 (only +4 is fired by any body we
     have) and the CodeProfiler/SendPhysics members are not modelled yet; the
     +16..+28 / +56 dwords keep their TODO role.  contactManager is a raw
     pointer (new in World.cpp, delete in the World dtor -- both in the lib TU,
     so the mixed-CRT rule is satisfied without a shared_ptr).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/World.h (path inferred; World.h:142 hosts the assertNotInStep assert) */

#pragma once

#include "rbx/boost.hpp"
#include "rbx/signal.h"
#include "G3D/Array.h"
#include "V8World/ContactManager.h"
#include "V8World/IWorldStage.h"

#include <set>
#include <utility>

namespace RBX {

	class Contact;
	class IWorldStage;
	class Joint;
	class JointInstance;
	class Primitive;
	class SendPhysics;
	class Assembly;

	class World
	{
	public:
		World();
		~World();

		// Binary ContactManager slot: World+0xB8 (read by
		// Workspace::getContactManagerIfInWorkspace 0x512568).
		ContactManager* getContactManager() { return contactManager; }
		// mac: 0x5A902E (batch776/777 -- returns the +188 member).
		SendPhysics* getSendPhysics();
		// mac: 0x5A9998 (44B, batch776): walks the +192 stage chain until the
		// MovingAssemblyStage, then removes the grounded assembly there.
		// (Called from SimulateStage's landed flows.) The walk never
		// bails in the binary either -- the stage chain is invariant.
		int onAssemblyInSimluationStage(Assembly* assembly);
		// World::getSpatialFilter 0x5A8FEA / getKernel 0x5A9012 walk the
		// +192 stage; only the stage hand-out is modelled so far.
		IWorldStage* getStage() { return stage; }
		// mac: 0x5A9012 / 0x5A8FCE (28B each, batch218): stage->getKernel().
		// NULL today (CleanStage stub short-circuits; base body retires it
		// with the JointStage chain).
		Kernel* getKernel() { return stage->getKernel(); }
		const Kernel* getKernel() const { return stage->getKernel(); }

		void joinAll();       // 0x5AB34C (TODO: primitive scan -- see World.cpp)
		void reset();         // 0x518A3E: assert !inStepCode
		void assemble();      // 0x5A9B04 (TODO: stage walk -- see World.cpp)
		bool isAssembled();   // 0x5A9B32 (TODO: stage walk -- see World.cpp)

		// mac: 0x5AACA8 (assert World.cpp:705 + :732).
		void insertJoint(Joint* joint);
		// mac: 0x5AAFD2.
		void removeJoint(Joint* joint);
		// mac: 0x5AAEB6 (assert World.cpp:806).
		void destroyJoint(Joint* joint);
		// mac: 0x5AA022 -- destroy every breakable-set joint that isBroken() and in
		// the joint-simulation stage (IStage::StageType 15).
		void doBreakJoints();
		// mac: 0x5AB290 / 0x5AB2B8.
		void insertContact(Contact* contact);
		void destroyContact(Contact* contact);
		// TouchInfo layout recovered from the two producers (Primitive.cpp
		// twins sub_590C8B/sub_590DE7, batch161): {reporting prim, other
		// prim, flag} with flag 0 (first twin) or 1 (second twin).  Only the
		// first 12 bytes are ever written; the G3D::Array stride (32) is
		// alignment, not size.
		struct TouchInfo
		{
			Primitive* reportingPrim;  // +0 (v10[0] = a1).
			Primitive* otherPrim;  // +4 (v10[1] = a2).
			int flag;  // +8 (v10[2] = 0/1 per twin).
			TouchInfo() : reportingPrim(NULL), otherPrim(NULL), flag(0) {}
			TouchInfo(Primitive* r, Primitive* o, int f)
				: reportingPrim(r), otherPrim(o), flag(f) {}
		};
		// mac: 0x5AA9F8 (32B): append to the +32 touch-info array.
		void reportTouchInfo(const TouchInfo& info);
		// mac: 0x5A9754 (223B): production moves entry -- asserts (World.cpp:425
		// p->getWorld, World.cpp:426 !inStepCode) then forwards to the
		// contact manager.  The :425 half is spelled pointer-truth like the
		// assert text (non-null, not == this).
		void onPrimitiveExtentsChanged(Primitive* prim);
		// mac: 0x5AB4B2 / 0x5AC168 -- the 1-/2-arg forwarders PartInstance::join
		// (0x44504C) and PartInstance::destroyJoints (0x444FE0) call.
		// mac: 0x5AB4B2 is createAutoJoints(p, NULL, NULL).
		void createAutoJoints(Primitive* prim);
		// mac: 0x5AC2E0 (335B): assert + int128 + scratch fill via the hash walk
		// + doNotIgnore gate + getJoint skip + canJoin/insert/fire.
		void createAutoJoints(Primitive* prim,
			std::set<Primitive*>* ignorePrims, std::set<Primitive*>* includePrims);
		// mac: 0x5AC168 -- destroyAutoJoints(p, NULL, b, true).
		void destroyAutoJoints(Primitive* prim, bool b);
		// mac: 0x5ABBEA (979B).  Two phases: collect owner shared_ptrs of every
		// selected joint into tempJoints (manual and/or auto per the two
		// bools), then destroy the [startIndex, size) joints whose far prim
		// passes doNotIgnore, remove them from tempJoints, and assert the
		// size is back to startIndex (:948).  See v8world/World.cpp.
		void destroyAutoJoints(Primitive* prim,
			std::set<Primitive*>* ignorePrims,
			bool manualJoints, bool autoJoints);
		// mac: 0x5AB4E0 (885B) / 0x5AB856 (916B): world-membership lifecycle.
		// insert: asserts (:856 inStep, :857 assembly-free, :858 pipeline-
		// free, :859/:860 edgeless, :861 worldless) + IndexArray append +
		// backref + stage/CM dispatch + :869 inStep re-check.  remove:
		// asserts (:875 inStep, :876 pipelined, :877 world set) +
		// destroyAutoJoints(prim, 0, !b, !b) + CM/stage dispatch + swap-
		// remove + backref clear + post asserts (:888 edgeless, :889 no
		// edges, :890 inStep, :891 assembly-free, :892 pipeline-free,
		// :893 worldless).  Void here (binary returns the Asserts flag byte;
		// no caller uses it).  See v8world/World.cpp for the per-assert and
		// routing DELTAs.
		void insertPrimitive(Primitive* prim);
		void removePrimitive(Primitive* prim, bool manualJoints);
		// mac: 0x5A9AB0 / 0x5A9ADA (42B each, batch219a): forward to the +192
		// stage (Joint::setPrimitive world path).
		void onJointPrimitiveNulling(Joint* joint, Primitive* prim);
		void onJointPrimitiveSet(Joint* joint, Primitive* prim);

		// Bring-up helpers (not in binary): read-only views of the counters
		// and the breakable set for the harness.
		unsigned int getNumJoints() const { return numJoints; }
		unsigned int getNumContacts() const { return numContacts; }
		size_t getNumPrimitives() const { return primitives.size(); }
		size_t getBreakableJointCount() const { return breakableJoints.size(); }
		bool isBreakableJointTracked(Joint* joint) const
		{
			return breakableJoints.find(joint) != breakableJoints.end();
		}
		Joint* getInJointNotification() const { return inJointNotification; }
		bool isInStepCode() const { return inStepCode; }
		// mac: 0x5AA47E (29B): world-step counter at +56 divided by the UI-step
		// rate.  Non-const like the binary (mangled __ZN..., no K) though it
		// only reads.
		int getUiStepId();
		// Bring-up views of the +32 touch-info queue (binary: write-only
		// telemetry -- producers append, no in-tree reader).
		size_t getTouchInfoCount() const { return touchInfoQueue.size(); }
		const TouchInfo& getTouchInfo(size_t i) const { return touchInfoQueue[i]; }
		rbx::signals::signal_with_args<1, void(Joint*)>& getJointNotificationSignal()
		{
			return jointNotificationSignal;
		}
		// Collision signal (binary +8; 2016 name/shape). Fired by
		// onPrimitiveCollided 0x5AAA18 (batch259).
		rbx::signal<void(std::pair<Primitive*, Primitive*>)>& getPrimitiveCollideSignal()
		{
			return primitiveCollideSignal;
		}
		// mac: 0x5AAA18 (batch259: unanchored prims bump the +124 low-nibble
		// collide counter, then the +8 signal fires).
		void onPrimitiveCollided(Primitive* p0, Primitive* p1);
		// mac: 0x5A9120 (batch299: NULL-guard + world assert + Sleep tickle).
		void ticklePrimitive(Primitive* prim, bool recursive);
		// mac: 0x5A9834 (batch299: step-guard + world asserts + extents fan).
		void onAssemblyExtentsChanged(Assembly* assembly);

	private:
		// mac: 0x5AC8D4: RBXASSERT(!inStepCode), "include/V8World/World.h line: 142".
		void assertNotInStep();
		// mac: 0x5AAF40 (RBXASSERT(num == 1), World.cpp:746).
		void removeFromBreakable(Joint* joint);
		// The two 4-term expand-tests insertJoint 0x5AAD81..0x5AAE9A runs on
		// both joints (slots [5]=getEdgeType, [7]=getJointType); they are
		// written as helpers because the compiler re-emits the virtual calls
		// four times (they cannot be CSE'd).  Private static members, not free
		// functions, so friendship/visibility stays local.
		static bool isKernelOrForeignEdge(const Joint* joint);
		static bool isAutoFamilyOrForeignEdge(const Joint* joint);

		// Binary order (see the member map above).
		rbx::signals::signal_with_args<1, void(Joint*)> jointNotificationSignal; // +4
		// Binary +8 (ctor zeroes; model-relative slot like the rest of
		// our World -- the map above documents binary addresses).
		rbx::signal<void(std::pair<Primitive*, Primitive*>)> primitiveCollideSignal;
		G3D::Array<TouchInfo, 10, 32> touchInfoQueue;  // +32
		bool inStepCode;                    // +48
		Joint* inJointNotification;         // +52
		// +56 (14th dword, ctor 0): world-step counter read by getUiStepId
		// mac: 0x5AA47E and written by the stepping path (unported).
		int uiStepWorldCounter;
		// +60: world prim list (insertPrimitive appends with worldIndex;
		// removePrimitive swap-removes).  The +80 second array's role is
		// unmapped (TODO) -- not modelled.
		G3D::Array<Primitive*, 10, 32> primitives;
		Primitive* megaClusterPrim;         // +76
		std::set<Joint*> breakableJoints;   // +100
		unsigned int numJoints;             // +120
		unsigned int numContacts;           // +124
		// +128: incremented once per createAutoJoints 3-arg call (0x5AC348).
		// No reader found in either auto-joint driver, so the NAME is
		// unknown -- plain int, placed for the documented offset.  (The
		// model-relative caveat in the header applies: our World already
		// deviates from binary offsets elsewhere.)
		int int128;
		G3D::Array<Primitive*, 10, 32> scratchPrimitives;  // +132
		// +148: destroyAutoJoints staging (phase 1 appends owner shared_ptrs,
		// phase 2 destroys their joints, tail removes + asserts size).
		// G3D::Array.h and the JointInstance fwd-decl are already in scope.
		G3D::Array<shared_ptr<JointInstance>, 10, 32> tempJoints;
		ContactManager* contactManager;     // +184 (raw; new/delete in World.cpp)
		SendPhysics* sendPhysics;           // +188 (new(0x24).news in the C2;
		                                    // deleted in the D2 order below)
		IWorldStage* stage;                 // +192
	};

	// mac: 0x5ABFBD (free fn): ignore-set excludes, include-set restricts.
	bool doNotIgnore(Primitive* p, const std::set<Primitive*>* ignore, const std::set<Primitive*>* include);

} // namespace RBX

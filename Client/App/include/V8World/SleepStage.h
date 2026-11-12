/* V8World/SleepStage.h -- October 2013 (sleep node, 0x180).
   Evidence: C2 0x59BF76 (1618B, batch220: news 0x40 SimulateStage +
     members below) / D2 (batch228: :27-32 set-empty asserts for the 6
     sets + walks + erases + delete +8) / getStageType -> 13
     (batch213/222) / onEdgeAdded 0x59FE68 (623B) / onEdgeRemoving
     0x5A00D8 (543B) (batch221b) / step fns (batch222).
   Members (binary offsets; ours model-relative):
     +16 numContactsInStage (C2 0) / +20 numContactsInKernel (C2 0;
       2016 names, first-party)
     +24..+116 reserved zeroed counters (C2 0; identities queue with the
       stepper flow -- stepSleepStage/doContacts/stepJoints touch them;
       section 123+). NOTE: +116 doubles as the contacting kernel
       counter (changeContactState 0x59Fxxx inc/dec -- ours is
       reservedCounters[23]).
     +120 flags (C2 short 0; byte +121 is the changeAssemblyState
       debugReentrant guard 0x59Fxxx :768 -- modeled as a union below,
       same bytes) / +124 longStepId (C2 -1; 2016 name)
     +128/+152/+176/+200/+224/+248 6x AssemblySet (stateToSet 0x59F8AE,
       batch259: 1->+128 recursiveWakePending, 2->+152 wakePending,
       3->+176 awake, 4->+200 sleepingChecking, 5->+224 sleepingDeeply,
       6->+248 removing; ANCHORED has no set)
     +272/+288 steppingContacts[2] + +304/+320 touchingContacts[2]
       (G3D::Array<Contact*>; 2016 ContactLists, NUM_THROTTLE_TYPE=2)
     +340 JointSet steppingJoints (2016 name)
     +360 count (C2 0; stepper-owned) / +364 flag (C2 byte 0)
     +368..+380 4x profiler (void*: binary news 0xA01C CodeProfilers
       "Collision"/"Joint Sleep"/"Wake"/"Sleep" -- module absent, NULL)
   Ours uses std::set (mechanics DELTA vs libstdc++ header shape -- same
   membership; cf. SimulateStage.h).
   onEdge pair INTERIM (fully decoded, parked): onEdgeAdded 0x59FE68
   (623B) asserts :932-:934, stamps throttleType via computeCanThrottle,
   stages, then contacts append to stepping/touching arrays (+272..+320,
   index at +36) or joints changeJointState(.,2); onEdgeRemoving 0x5A00D8
   (543B) asserts :957/:979-:981, wakeEvent/changeContact/changeJoint
   dispatch, resets state to 0/3, unstages. Blocked on: Edge edgeState/
   throttleType members (+8/+12, unmapped) + Assembly::computeCanThrottle
   (needs prim+240 Body*) + changeContact/Joint/AssemblyState trio
   (unswept) + wakeEvent assembly half (getAssemblyState). Batch221b +
   batch251 recon archived. Inherited base bodies preserve pipeline flow
   meanwhile. Step/validate/assembly fns: declared with that flow.
   Layout: IStage base (+4 upstream / +8 downstream=SimulateStage /
     +12 world).
   Original file: Client/App/v8world/SleepStage.h (path from assert
   strings: SimulateStage.cpp/SleepStage.cpp) */

#pragma once

#include "V8World/IWorldStage.h"
#include "V8World/Enum.h"
#include "V8World/Contact.h"
#include "Util/IndexArray.h"
#include "G3D/Array.h"

#include <set>
#include <vector>

namespace RBX {

	class World;
	class Edge;
	class Assembly;
	class Contact;
	class Joint;

	class SleepStage : public IWorldStage
	{
	public:
		typedef std::set<Assembly*> AssemblySet;
		typedef std::set<Joint*> JointSet;

		SleepStage(IStage* upstream, World* world);
		virtual ~SleepStage();

		// getStageType -> 13 (batch213/222).
		virtual StageType getStageType() const { return SLEEP_STAGE; }
		// onEdge pair (0x59FE68/0x5A00D8, batch221b: throttle stamp +
		// stepping/touching IndexArrays + joint dispatch + wake routing).
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		// Sleep dispatch (batch257 bodies; contact/assembly fan-outs and
		// singles + wake machinery queue in 160 with batch259):
		// vector fan-out (trivial loop over the single).
		void changeJointState(const std::vector<Joint*>& joints, Sim::EdgeState newState);
		// Single (0x59Fxxx, batch257: stepping-set membership + downstream
		// onEdge pair, :743/:751/:756 asserts).
		void changeJointState(Joint* joint, Sim::EdgeState newState);
		// Contact pair (batch257: stepping/touching IndexArrays by
		// throttle + kernel inc/dec + wake transitions + collide signal).
		void changeContactState(const std::vector<Contact*>& contacts, Sim::EdgeState newState);
		void changeContactState(Contact* contact, Sim::EdgeState newState);
		// Assembly pair (batch257: debugReentrant + set move + Simulate
		// downstream + state store).
		void changeAssemblyState(const std::vector<Assembly*>& assemblies, Sim::AssemblyState newState);
		void changeAssemblyState(Assembly* assembly, Sim::AssemblyState newState);
		// Wake machinery (batch251/259: edge/contact -> assembly fans +
		// state-gated transitions).
		void wakeEvent(Edge* edge);
		void wakeEvent(Assembly* assembly);
		void recursiveWakeEvent(Contact* contact);
		void recursiveWakeEvent(Assembly* assembly);
		// highVelocityNewTouch STUB (banked EA batch259; needs the
		// maxRadius recompute -- geometry era. Loud if a stepping→
		// contacting transition ever reaches it in-tree).
		bool highVelocityNewTouch(Contact* contact);
		// stepJoints (batch222: collect non-moving joints + sleep them).
		void stepJoints();
		// 2016 shape (external wake tickle from World::ticklePrimitive;
		// EA queued with the stepper).
		void onExternalTickleAssembly(Assembly* assembly, bool recursive);

	private:
		// mac: 0x59F8AE (batch259; exercised by changeAssemblyState in 161).
		AssemblySet& stateToSet(Sim::AssemblyState state);

		int numContactsInStage;     // +16 (C2 0).
		int numContactsInKernel;    // +20 (C2 0).
		int reservedCounters[24];   // +24..+116 (C2 0; TODO stepper).
		                            // [23] (+116) is the contacting kernel
		                            // counter (changeContactState inc/dec).
		union {
			short flags;            // +120 (C2 0).
			struct {
				char flagsLo;       // +120 (role TODO).
				bool debugReentrant;  // +121 (changeAssemblyState guard).
			};
		};
		int longStepId;             // +124 (C2 -1).
		// 2016 shape (latch for onExternalTickleAssembly recursion).
		bool externalRecursiveWake;
		AssemblySet recursiveWakePending;  // +132 (D2).
		AssemblySet wakePending;           // +156 (D2).
		AssemblySet awake;                 // +180 (D2).
		AssemblySet sleepingChecking;      // +204 (D2).
		AssemblySet sleepingDeeply;        // +228 (D2).
		AssemblySet removing;              // +252 (D2).
		// Binary-faithful IndexArrays (fastRemove(item) + steppingIndex
		// at Contact+36; a contact holds at most one list slot -- states
		// are exclusive -- so the single index suffices).
		IndexArray<Contact, &Contact::steppingIndexFunc> steppingContacts[2];  // +272/+288.
		IndexArray<Contact, &Contact::steppingIndexFunc> touchingContacts[2];   // +304/+320.
		JointSet steppingJoints;           // +340 (D2).
		// Scratch for stepJoints (binary +100 vector; model-relative
		// here -- layout after this point is ours, not binary).
		std::vector<Joint*> toSleepingJoint;
		int unknown360;             // +360 (C2 0; TODO stepper).
		bool unknown364;            // +364 (C2 false; TODO stepper).
		void* profilerCollision;    // +368 (NULL -- see note).
		void* profilerJointSleep;   // +372 (NULL).
		void* profilerWake;         // +376 (NULL).
		void* profilerSleep;        // +380 (NULL).
	};

} // namespace RBX

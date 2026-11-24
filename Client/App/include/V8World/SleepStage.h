/* V8World/SleepStage.h -- October 2013.
   Layout from the 2013 ctor (PLAYER 0x008e7f50, STUDIO 0x00a06180) and dtor (0x008e7160): the
   HumanoidStage ctor 0x0099b050 news 0x14c.
     0x10..0x8c  eight std::vector scratch lists (alloc dword left unset, three pointers zeroed; their
                 EH states 2..9 come before the first set's). Names and element types: src2016.
     0x90/0x94   numContactsInStage / numContactsInKernel (dtor asserts :64/:65)
     0x98/0x99   throttling / debugReentrant (dtor assert :66 "debugReentrant == 0")
     0x9c        longStepId (-1)
     0xa0..0xdc  six AssemblySets (dtor asserts :68-:73 name them in this order)
     0xe8/0x108  steppingContacts[2] / touchingContacts[2] (IndexArray, `eh vector constructor
                 iterator' with the 0x008e6e50 element ctor; dtor asserts :74-:77)
     0x128       steppingJoints
     0x134/0x138 recursivePassId (0) / externalRecursiveWake (false)
     0x13c..0x148 four scoped_ptr<CodeProfiler>: "Collision", "Joint Sleep", "Wake", "Sleep"
                 (World::loadProfilers 0x00747f20 reads them in that order).
   The Mac model's reserved counters and flags union are gone. Original file:
   Client/App/include/V8World/SleepStage.h (2013 assert path of the .cpp: Client\App\V8World). */

#pragma once

#include "V8World/IWorldStage.h"
#include "V8World/Enum.h"
#include "V8World/Contact.h"
#include "Util/IndexArray.h"
#include "Util/Profiling.h"
#include "G3D/Array.h"
#include "boost/scoped_ptr.hpp"

#include <set>
#include <vector>

namespace RBX {

	class World;
	class Edge;
	class Assembly;
	class Contact;
	class Joint;

	// VTABLE: PLAYER 0x00b92b24
	// SIZE 0x14c
	class SleepStage : public IWorldStage
	{
		friend class World;	// World::loadProfilers reads the four profilers

	public:
		typedef std::set<Assembly*> AssemblySet;
		typedef std::set<Joint*> JointSet;

		SleepStage(IStage* upstream, World* world);
		virtual ~SleepStage();

		virtual StageType getStageType() const { return SLEEP_STAGE; }
		virtual void onEdgeAdded(Edge* edge);
		virtual void onEdgeRemoving(Edge* edge);

		void changeJointState(const std::vector<Joint*>& joints, Sim::EdgeState newState);
		void changeJointState(Joint* joint, Sim::EdgeState newState);
		void changeContactState(const std::vector<Contact*>& contacts, Sim::EdgeState newState);
		void changeContactState(Contact* contact, Sim::EdgeState newState);
		void changeAssemblyState(const std::vector<Assembly*>& assemblies, Sim::AssemblyState newState);
		void changeAssemblyState(Assembly* assembly, Sim::AssemblyState newState);
		void wakeEvent(Edge* edge);
		void wakeEvent(Assembly* assembly);
		void onAssemblyAdded(Assembly* assembly);
		void onAssemblyRemoving(Assembly* assembly);
		void recursiveWakeEvent(Contact* contact);
		void recursiveWakeEvent(Assembly* assembly);
		bool highVelocityNewTouch(Contact* contact);
		void stepJoints();
		void onExternalTickleAssembly(Assembly* assembly, bool recursive);

	private:
		typedef IndexArray<Contact, &Contact::steppingIndexFunc> ContactList;

		AssemblySet& stateToSet(Sim::AssemblyState state);

		std::vector<Assembly*> toDeep;					// 0x10
		std::vector<Assembly*> toWake;					// 0x20
		std::vector<Assembly*> toSleepingChecking;		// 0x30
		std::vector<Contact*> toSleeping;				// 0x40
		std::vector<Contact*> toStepping;				// 0x50
		std::vector<Contact*> toContacting;				// 0x60
		std::vector<Contact*> toContactingSleeping;		// 0x70
		std::vector<Joint*> toSleepingJoint;			// 0x80
		int numContactsInStage;							// 0x90
		int numContactsInKernel;						// 0x94
		bool throttling;								// 0x98
		bool debugReentrant;							// 0x99
		int longStepId;									// 0x9c
		AssemblySet recursiveWakePending;				// 0xa0
		AssemblySet wakePending;						// 0xac
		AssemblySet awake;								// 0xb8
		AssemblySet sleepingChecking;					// 0xc4
		AssemblySet sleepingDeeply;						// 0xd0
		AssemblySet removing;							// 0xdc
		ContactList steppingContacts[Sim::NUM_THROTTLE_TYPE];	// 0xe8
		ContactList touchingContacts[Sim::NUM_THROTTLE_TYPE];	// 0x108
		JointSet steppingJoints;						// 0x128
		int recursivePassId;							// 0x134
		bool externalRecursiveWake;						// 0x138
		boost::scoped_ptr<Profiling::CodeProfiler> profilingCollision;	// 0x13c
		boost::scoped_ptr<Profiling::CodeProfiler> profilingJointSleep;	// 0x140
		boost::scoped_ptr<Profiling::CodeProfiler> profilingWake;		// 0x144
		boost::scoped_ptr<Profiling::CodeProfiler> profilingSleep;		// 0x148
	};

	// The IndexArray template SleepStage.obj carries out of line:
	// TEMPLATE: PLAYER 0x008e6cf0
	// TEMPLATE: STUDIO 0x00a04fd0
	// ?fastRemove@?$IndexArray@VContact@RBX@@$1?steppingIndexFunc@12@QAEAAHXZ@RBX@@QAEXPAVContact@2@@Z

} // namespace RBX

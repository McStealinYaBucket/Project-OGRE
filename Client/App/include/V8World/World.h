/* V8World/World.h -- October 2013 (physics world).
   2013 layout, size 0xb8, from the PLAYER ctor 0x0074ae30 and dtor 0x00749ae0 (operator new 0xb8 in
   RootInstance::RootInstance 0x007e35f0); member names from the 2016 header where the 2013 bytes
   agree (dtor asserts World.cpp:187-193 name groundPrimitive, numJoints, numContacts, primitives and
   breakableJoints; reset 0x0070d390 zeroes worldStepId at 0x38, World.h:166).
   Not polymorphic.  The four signals' ctors: 0x00/0x04 share 0x00746cc0 (same type), 0x08 0x00746d20,
   0x0c 0x00746d80.  2016's postInsertJointSignal/postRemoveJointSignal, the Bullet members, the
   frame-info doubles and primitiveIndexation do not exist yet.
   EThrottle (0x14, 0xc) is 2016's: requestedSkip/usedSkip/throttleIndex at +0/+4/+8
   (increaseLoad 0x00745590, computeThrottle 0x00745600); its ctor is inline (no out-of-line copy).
   The Mac-model member map this header used to carry is superseded.
   Original file: Client/App/include/V8World/World.h (2013 assert path "include\V8World/World.h"). */

#pragma once

#include "rbx/boost.hpp"
#include "rbx/signal.h"
#include "G3D/Array.h"
#include "Util/IndexArray.h"
#include "V8World/ContactManager.h"
#include "V8World/CleanStage.h"
#include "V8World/Primitive.h"

#include <set>
#include <utility>
#include <boost/scoped_ptr.hpp>

namespace RBX {

	class Contact;
	class Joint;
	class JointInstance;
	class SendPhysics;
	class Assembly;
	class SleepStage;
	class StepJointsStage;












	class EThrottle
	{
	public:
		typedef enum { ThrottleDefaultAuto, ThrottleDisabled, ThrottleAlways, Skip2, Skip4, Skip8, Skip16 } EThrottleType;
	private:
		int requestedSkip;	// 0x00
		int usedSkip;		// 0x04
		static const int throttleSetting[];	// 0x00b52860
		int throttleIndex;	// 0x08
	public:
		static EThrottleType globalDebugEThrottle;	// GLOBAL 0x00df72a8
		EThrottle();
		bool computeThrottle(int step);
		bool increaseLoad(bool increase);
		float getEnvironmentSpeed() const;
	};

	// SIZE 0xb8
	class World
	{
		friend class ContactManager;
	public:
		rbx::signal<void(Joint*)> autoJoinSignal;					// 0x00
		rbx::signal<void(Joint*)> autoDestroySignal;				// 0x04
		rbx::signal<void(std::pair<Primitive*, Primitive*>)> primitiveCollideSignal;	// 0x08
		rbx::signal<void(Primitive*)> m_unk0x0c;					// 0x0c

		// 12 bytes as written by the Primitive.cpp producers; destroyed as a POD array (0x007b4980).
		struct TouchInfo
		{
			Primitive* reportingPrim;
			Primitive* otherPrim;
			int flag;
			TouchInfo() : reportingPrim(NULL), otherPrim(NULL), flag(0) {}
			TouchInfo(Primitive* r, Primitive* o, int f)
				: reportingPrim(r), otherPrim(o), flag(f) {}
		};

	private:
		int frmThrottle;											// 0x10
		EThrottle eThrottle;										// 0x14
		G3D::Array<TouchInfo> touchReporting;						// 0x20
		bool inStepCode;											// 0x30
		Joint* inJointNotification;									// 0x34
		int worldStepId;											// 0x38
		IndexArray<Primitive, &Primitive::worldIndexFunc> primitives;	// 0x3c
		Primitive* groundPrimitive;									// 0x4c
		G3D::Array<Primitive*> movingPrimitives;					// 0x50
		std::set<Joint*> breakableJoints;							// 0x60
		int numJoints;												// 0x6c
		int numContacts;											// 0x70
		int numLinkCalls;											// 0x74
		G3D::Array<Primitive*> tempPrimitives;						// 0x78
		G3D::Array<shared_ptr<JointInstance> > tempJoints;			// 0x88
		boost::scoped_ptr<Profiling::CodeProfiler> profilingBreak;		// 0x98
		boost::scoped_ptr<Profiling::CodeProfiler> profilingAssembly;	// 0x9c
		boost::scoped_ptr<Profiling::CodeProfiler> profilingFilter;		// 0xa0
		boost::scoped_ptr<Profiling::CodeProfiler> profilingWorldStep;	// 0xa4
		boost::scoped_ptr<Profiling::CodeProfiler> profilingUiStep;		// 0xa8
		ContactManager* contactManager;								// 0xac
		SendPhysics* sendPhysics;									// 0xb0
		CleanStage* cleanStage;										// 0xb4

		void loadProfilers(std::vector<Profiling::CodeProfiler*>& worldProfilers) const;
		void removeFromBreakable(Joint* joint);



	public:
		// Evidence for the members above (PLAYER):
		//   ctor 0x0074ae30: signals 0x00-0x0c, frmThrottle = 1, EThrottle inline, the five arrays,
		//     the set head node, zeroed 0x30/0x34/0x38 and 0x6c/0x70/0x74, new Primitive(GEOMETRY_BLOCK)
		//     (0x170), five new CodeProfiler (0xc040: Break, Assembler, Filter, World Step, UI Step),
		//     new ContactManager (0x6c), new SendPhysics (0x3c), new CleanStage(NULL, this) (0x10),
		//     then loadProfilers 0x00747f20 into a local vector.
		//   dtor 0x00749ae0: EH states show the profilers are member objects (scoped_ptr) and the
		//     last three pointers are raw (deleted in the body: ground, sendPhysics, stage, CM).
		//   getSendPhysics 0x008bb760: mov eax,[ecx+0xb0] (ICF-folded out of World.obj).
		//   reset 0x0070d390: World.h:166, worldStepId = 0.
		//
		int onAssemblyInSimluationStage(Assembly* assembly);
		void joinAll();
		void assemble();
		bool isAssembled();
		void insertJoint(Joint* joint);
		void removeJoint(Joint* joint);
		void destroyJoint(Joint* joint);
		void doBreakJoints();
		void insertContact(Contact* contact);
		void destroyContact(Contact* contact);
		World();
		~World();

		void assertNotInStep() { RBXASSERT(!inStepCode); }
		SendPhysics* getSendPhysics();
		ContactManager* getContactManager() { return contactManager; }
		IWorldStage* getStage() { return cleanStage; }
		Kernel* getKernel();
		const Kernel* getKernel() const;
		SleepStage* getSleepStage(); const SleepStage* getSleepStage() const; const StepJointsStage* getStepJointsStage() const;
		void reportTouchInfo(const TouchInfo& info);
		void onPrimitiveExtentsChanged(Primitive* prim); void onPrimitiveGeometryChanged(Primitive* p);
		void onPrimitiveContactParametersChanged(Primitive* p);
		Assembly* onPrimitiveEngineChanging(Primitive* p);
		void onPrimitiveEngineChanged(Assembly* a);
		void onPrimitiveFixedChanging(Primitive* p);
		void onPrimitiveFixedChanged(Primitive* p);
		void onPrimitivePreventCollideChanged(Primitive* p);
		void createAutoJoints(Primitive* prim);
		void createAutoJoints(Primitive* prim, std::set<Primitive*>* ignorePrims, std::set<Primitive*>* includePrims);
		void destroyAutoJoints(Primitive* prim, bool b);
		void destroyAutoJointsToWorld(const G3D::Array<Primitive*>& primitives);
		void createAutoJointsToWorld(const G3D::Array<Primitive*>& primitives);
		void createAutoJointsToPrimitives(const G3D::Array<Primitive*>& primitives);
		void destroyAutoJoints(Primitive* prim, std::set<Primitive*>* ignorePrims, bool manualJoints, bool autoJoints);
		// FUNCTION: PLAYER 0x0070d390
		// FUNCTION: STUDIO 0x0082a960
		void reset() { RBXASSERT(!inStepCode); worldStepId = 0; }
		void insertPrimitive(Primitive* prim);
		void removePrimitive(Primitive* prim, bool manualJoints);
		void onJointPrimitiveNulling(Joint* joint, Primitive* prim);
		void onJointPrimitiveSet(Joint* joint, Primitive* prim);
		int getUiStepId();
		void onPrimitiveCollided(Primitive* p0, Primitive* p1);
		void ticklePrimitive(Primitive* prim, bool recursive);
		void onAssemblyExtentsChanged(Assembly* assembly);
		void sendClumpChangedMessage(Primitive* childPrim);
		void notifyMoved(Primitive* p);
		void jointCoordsChanged(Joint* j);
		void computeFallen(G3D::Array<Primitive*>& fallen) const;
	};

	// The member templates World.obj carries out of line (PLAYER order):
	// TEMPLATE: PLAYER 0x007465a0
	// TEMPLATE: STUDIO 0x00887370
	// ??1?$Array@V?$shared_ptr@VJointInstance@RBX@@@boost@@$09$0CA@@G3D@@QAE@XZ

	// TEMPLATE: PLAYER 0x00746cc0
	// TEMPLATE: STUDIO 0x00887780
	// ??0?$signal@$$A6AXPAVJoint@RBX@@@Z@signals@rbx@@QAE@XZ

	// TEMPLATE: PLAYER 0x00746d20
	// TEMPLATE: STUDIO 0x008877e0
	// ??0?$signal@$$A6AXU?$pair@PAVPrimitive@RBX@@PAV12@@std@@@Z@signals@rbx@@QAE@XZ

	// TEMPLATE: PLAYER 0x00746d80
	// TEMPLATE: STUDIO 0x00887840
	// ??0?$signal@$$A6AXPAVPrimitive@RBX@@@Z@signals@rbx@@QAE@XZ

	// TEMPLATE: PLAYER 0x00746f40
	// TEMPLATE: STUDIO 0x00887a00
	// ?fastRemove@?$IndexArray@VPrimitive@RBX@@$H?worldIndexFunc@12@QAEAAHXZA@@RBX@@QAEXPAVPrimitive@2@@Z

	// TEMPLATE: PLAYER 0x00749000
	// TEMPLATE: STUDIO 0x00889950
	// ??1?$signal@$$A6AXPAVJoint@RBX@@@Z@signals@rbx@@QAE@XZ

	// TEMPLATE: PLAYER 0x00749150
	// TEMPLATE: STUDIO 0x00889aa0
	// ??1?$signal@$$A6AXU?$pair@PAVPrimitive@RBX@@PAV12@@std@@@Z@signals@rbx@@QAE@XZ

	bool doNotIgnore(Primitive* p, const std::set<Primitive*>* ignore, const std::set<Primitive*>* include);

} // namespace RBX

/* V8DataModel/Workspace.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: Creatable<Instance>::create<Workspace> at +2936 in the DataModel ctor;
     getWorkspace() + "Workspace" string in both 2013 PEs.
   DIVERGENCE from March 2016 (DescribedNonCreatable<Workspace, RootInstance> +
     Service + VerbContainer + IMovingManager + IScriptFilter): the Oct 2013 DataModel
     ctor CREATES the Workspace via Creatable, so the 2013 class MUST be Creatable.
     Stubbed as DescribedCreatable<Workspace, Instance> + Service + VerbContainer
     (VerbContainer is real: initializeContents calls setVerbParent on the workspace
     container, and getWhitelistVerb-style calls need it) pending the real decompile
     (which will restore the true bases).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Workspace.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "V8Tree/Verb.h"
#include "rbx/boost.hpp"
#include "V8World/World.h"
#include "Network/Players.h"
#include "V8DataModel/MouseCommand.h"
#include "V8DataModel/RootInstance.h"

namespace RBX {

	extern const char* const sWorkspace;

	class Camera;

	class Workspace
		: public DescribedNonCreatable<Workspace, RootInstance, sWorkspace>
		// Restored TRUE chain (was Instance standing in; binary:
		// NonFactoryProduct<Workspace, RootInstance, sWorkspace> 0x519ADE).
		// NonFactory = no createByName("Workspace") (root-only, created
		// directly by DataModel); Creatable<Instance>::create<Workspace>()
		// still works (direct new, no map).
		, public Service
		, public VerbContainer
	{
	public:
		// 2013: Workspace(IDataState*) PLAYER 0x00718e10 / STUDIO 0x008357b0 (iOS
		// _ZN3RBX9WorkspaceC1EPNS_10IDataStateE), created by the DataModel ctor through
		// Creatable<Instance>::create<Workspace>(this).
		Workspace(IDataState* dataState);

		// Binary World slot: Workspace+312 (0x138), read by
		// getWorldIfInWorkspace 0x512512 (creation site TODO-verify).
		World* getWorld() { return world.get(); }

		// mac: 0x512536: nearest enclosing Workspace (self counts), else NULL.
		static Workspace* getWorkspaceIfInWorkspace(Instance* inst);
		// mac: 0x512512: World of the enclosing Workspace, else NULL.
		static World* getWorldIfInWorkspace(Instance* inst);
		// mac: 0x512568: ContactManager of that World, else NULL (also NULL
		// when the World slot itself is NULL).
		static ContactManager* getContactManagerIfInWorkspace(Instance* inst);

		// mac: 0x516402 (member, no args — was misdeclared static taking self).
		void setDefaultMouseCommand();

		// mac: 0x2CF1D0 (clearContents) callees — signatures TODO-verify (not yet decompiled):
		// real getTerrain returns Terrain* (v8world; Terrain : Instance, FW at +68).
		Instance* getTerrain();
		void setMouseCommand(shared_ptr<MouseCommand> command);
		void setNullMouseCommand();	// PLAYER 0x00710080
		void setCurrentCamera(Camera* camera);
		Workspace* getCameraOwnerModel() { return this; }

		// mac: 0x514C68 (17B, virtual, slot +0xC8): the binary body jumps
		// through vtable slot +0xCC (getConstCamera) -- GCC tail-jump;
		// landed = the plain call + const_cast.
		// DELTA: const-qualified in the landed build (binary mangle
		// __ZN3RBX9Workspace9getCameraEv is non-const) -- the Reflection
		// RefPropDescriptor plumbing (Reflection.h:128-131) calls getters
		// through `const Workspace*`, so the descriptor's getter must be
		// const-callable.  Pure read; observable behavior unchanged.
		virtual Camera* getCamera() const;
		// mac: 0x514C9E (24B, virtual, slot +0xCC): the +0x1FC current-camera
		// px, else the +0x204 fallback.
		virtual const Camera* getConstCamera() const;

		// Terrain trio (0x514E84/0x514FF2/0x515534). The terrain slot is
		// Workspace+110/111 (shared_ptr<MegaClusterInstance>); the world for
		// joinAllHack lives at World offset +78 (member TODO-verify).
		void createTerrain();
		void clearTerrain();
		void joinAllHack();

		// mac: 0x51239E: presence delegate (binary: Players::clientIsPresent(this, 0)).
		static bool clientIsPresent(const Instance* context)
		{
			return Network::Players::clientIsPresent(context, false);
		}

		// mac: 0x51237C: presence delegate (binary: Players::serverIsPresent(this, 0)).
		static bool serverIsPresent(const Instance* context)
		{
			return Network::Players::serverIsPresent(context, false);
		}

		// Run-state machine (0x515636/0x515896/0x515AF0; driven by
		// DataModel::onRunTransition, bound in initializeContents 0x2D3336).
		void start();
		void stop();
		void reset();
		// Physics assembly (0x515636 callee; TODO: body — World wiring).
		void assemble();

		// Mouse command (2016 shape: non-null in-engine; NULL-tolerant here
		// until the tool pipeline sets commands — TODO: restore assert).
		MouseCommand* getCurrentMouseCommand() { return mouseCommand.get(); }

		double getDistributedGameTime() const { return distributedGameTime; }
		void setDistributedGameTime(double value);
		void setDistributedGameTimeNoTransmit(double value) { distributedGameTime = value; }

		static const Reflection::PropDescriptor<Workspace, double> prop_DistributedGameTime;

		// Load-stats (loadContent 0x2CDBAA zeroes three doubles on the
		// workspace; 2016 names them syncHttpGet/XMLLoad/joinAll times).
		void setStatsSyncHttpGetTime(double value) { statsSyncHttpGetTime = value; }
		void setStatsXMLLoadTime(double value) { statsXMLLoadTime = value; }
		void setStatsJoinAllTime(double value) { statsJoinAllTime = value; }

	private:
		// Engine-management state (2016 shape).
		void updateDistributedGameTime();  // 0x515896 callee (ported; private -- reached via stop())
		double distributedGameTime;

		// +464 (116*4, shared_ptr slot reset by setDefaultMouseCommand; TODO verify).
		shared_ptr<MouseCommand> mouseCommand;
		// Terrain (see above) + physics World (binary slot +312; now constructed; see getWorld).
		shared_ptr<Instance> terrain;
		shared_ptr<World> world;
		double statsSyncHttpGetTime;
		double statsXMLLoadTime;
		double statsJoinAllTime;

		// Camera slots (setCurrentCamera 0x512E4A / accessors 0x514C68:
		// +0x1FC = the current-camera shared px pair, +0x204 = the raw
		// fallbackCamera px).  Order invariant; MSVC layout DELTA.
		shared_ptr<Camera> currentCamera;   // +0x1FC (px @+0x1FC)
		Camera* fallbackCamera;             // +0x204
	};

} // namespace RBX

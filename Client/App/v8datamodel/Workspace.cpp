/* App/v8datamodel/Workspace.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile.  Camera chain landed ARC-2026-10-20 (see the
   camera block below): setCurrentCamera 0x512E4A (298B) / getCamera
   0x514C68 (17B) / getConstCamera 0x514C9E (24B) + the +0x1FC/+0x204
   member pairs. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/Workspace.h"
#include "V8DataModel/Camera.h"
#include "V8DataModel/MegaCluster.h"
#include "V8DataModel/CoreGuiService.h"
#include "V8DataModel/ImageLabel.h"
#include "V8DataModel/RunService.h"
#include "util/FeatherweightDict.h"

namespace RBX {
	const char* const sWorkspace = "Workspace";
}

using namespace RBX;

REFLECTION_BEGIN();
const Reflection::PropDescriptor<Workspace, double> Workspace::prop_DistributedGameTime("DistributedGameTime", category_Data, &Workspace::getDistributedGameTime, &Workspace::setDistributedGameTime, Reflection::PropertyDescriptor::STANDARD_NO_REPLICATE);
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Workspace);
}

LOGGROUP(MegaClusterInit)

namespace RBX {

// mac: 0x512536: ServiceProvider::find<Workspace> + self-or-ancestor check
// (parent walk; NULL when no service or detached from it).
Workspace* Workspace::getWorkspaceIfInWorkspace(Instance* inst)
{
	Workspace* found = ServiceProvider::find<Workspace>(inst);
	if (found == NULL)
		return NULL;
	if (found == inst)
		return found;
	for (Instance* cur = inst->getParent(); cur != NULL; cur = cur->getParent())
	{
		if (cur == found)
			return found;
	}
	return NULL;
}

// mac: 0x512512.
World* Workspace::getWorldIfInWorkspace(Instance* inst)
{
	Workspace* ws = getWorkspaceIfInWorkspace(inst);
	return ws != NULL ? ws->getWorld() : NULL;
}

// mac: 0x512568 (NULL also when the World slot is NULL).
ContactManager* Workspace::getContactManagerIfInWorkspace(Instance* inst)
{
	Workspace* ws = getWorkspaceIfInWorkspace(inst);
	if (ws == NULL)
		return NULL;
	World* world = ws->getWorld();
	return world != NULL ? world->getContactManager() : NULL;
}


// mac: 0x2CF1D0 callees — TODO: bodies (v8world/terrain + camera wiring).
Instance* Workspace::getTerrain()
{
	return terrain.get();
}

void Workspace::setMouseCommand(shared_ptr<MouseCommand> command)
{
	(void)command;
}

// "CurrentCamera" ref property: the binary's descriptor sits at
// 0xAA49C0 (raised from setCurrentCamera 0x512EA9) but its storage
// reads as a relocated (unset) block in dbq24.  Landed registers the
// named pair per the landed RefPropDescriptor machinery (the
// BillboardGui/prop_Adornee precedent).  DELTAs: the name pinned by the
// Studio Lua reads (game.Workspace.CurrentCamera -- BuiltInPlugins/
// terrain/*.lua); the method pair per the landed accessors; category
// carried per the corpus convention; attribute bits unread.
static Reflection::RefPropDescriptor<Workspace, Camera> prop_CurrentCamera(
	"CurrentCamera", category_Data, &Workspace::getCamera, &Workspace::setCurrentCamera);

// mac: 0x514C68 (17B, virtual, class vtable slot +0xC8): the binary body is
// a tail-jump through slot +0xCC (getConstCamera) -- GCC's skip-to of
// the const call; landed = the plain call with the const strip.
Camera* Workspace::getCamera() const
{
	return const_cast<Camera*>(getConstCamera());
}

// mac: 0x514C9E (24B, virtual, slot +0xCC): the +0x1FC px when the
// current-camera pair is live, else the +0x204 fallback pointer.
const Camera* Workspace::getConstCamera() const
{
	if (currentCamera.get() != NULL)
		return currentCamera.get();
	return fallbackCamera;
}

// mac: 0x512E4A (298B, mangle __ZN3RBX9Workspace15setCurrentCameraEPNS_6CameraE):
// serverIsPresent gate -> px-compare early-out -> the pair store -> the
// raise -> the NULL clamp; then the +0x38 signal-group connect walk,
// skipped here (DELTAs at the tail).
void Workspace::setCurrentCamera(Camera* camera)
{
	// mac: 0x512E58: Network::Players::serverIsPresent(this, false) -> return.
	if (Network::Players::serverIsPresent(this, false))
		return;

	// the px compare: currentCamera.get() == camera -> early-out.
	if (currentCamera.get() == camera)
		return;

	// mac: 0x512E90: the pair store -- RBX::shared_from<Camera> (the px/copy
	// idiom writes the same +0x1FC slot; null-safe via shared_from).
	currentCamera = RBX::shared_from<Camera>(camera);

	// mac: 0x512EA9 (PIC anchor 0x512E58's offset base): raisePropertyChanged.
	// DELTA: the landed prop_CurrentCamera tripwire raise (the binary
	// descriptor at 0xAA49C0 is unread/unset in the DB).
	raisePropertyChanged(prop_CurrentCamera);

	// mac: 0x512F10: camera == NULL early-out BEFORE the +0x38 walk.
	if (camera == NULL)
		return;

	// DELTA (skipped): the +0x38 signal-group walk connects the new
	// camera's listeners through the bound functor 0x514CCE -- the
	// signal-group storage layout is unmodeled in this shell (parked
	// DELTA inside setCurrentCamera per the arc).  The raise + the pair
	// store above carry the observable surface.
}

// mac: 0x516402: reset the mouse-command slot, then setMouseCommand(null).
void Workspace::setDefaultMouseCommand()
{
	mouseCommand.reset();
	setMouseCommand(shared_ptr<MouseCommand>());
}

// mac: 0x514E84: create the voxel terrain when the slot is empty, parent it, and
// assert presence. The binary sizes/positions the part (getPartSizeXml +
// identity rotation + half-height offset) — physics-part wiring TODO.
// STUB: PLAYER 0x0070fe90
// STUB: STUDIO 0x0082d150
void Workspace::createTerrain()
{
	if (!terrain)
	{
		if (FLog::MegaClusterInit)
			FLog::FastLog(FLog::MegaClusterInit, "Terrain doesn't exist, creating new terrain", 0);
		shared_ptr<MegaClusterInstance> fresh = Creatable<Instance>::create<MegaClusterInstance>();
		fresh->setAndLockParent(this);
		terrain = fresh;
		RBXASSERT(terrain != NULL);  // "terrain" (Workspace.cpp)
	}
}

// mac: 0x514FF2: unlock (FW b21 clear), unparent, and release the terrain slot.
// Binary passes the LIVE terrain+68 slot to FWValue::set (the redirect
// lands the unlock); a local FWRef leaves the live slot locked and the
// setParent below throws (caught live as the isodm-L crash).
// STUB: PLAYER 0x0070f090
// STUB: STUDIO 0x0082c460
void Workspace::clearTerrain()
{
	if (terrain)
	{
		if (FLog::MegaClusterInit)
			FLog::FastLog(FLog::MegaClusterInit, "Clearing terrain", 0);
		if (FWInstance* f = terrain->fw())
		{
			bool unlocked = false;
			FWValue<bool>::set(*reinterpret_cast<bool*>(&f->b21), unlocked,
				terrain->fwRef());
		}
		terrain->setParent(NULL);
		terrain.reset();
	}
}

// mac: 0x515534: delegate to the physics world (World+78 in the binary).
void Workspace::joinAllHack()
{
	if (world)
		world->joinAll();
}

// mac: 0x51580E: assemble the physics world + assert assembled (Workspace.cpp:913).
// STUB: PLAYER 0x0070ebe0
// STUB: STUDIO 0x0082bfb0
void Workspace::assemble()
{
	if (world)
	{
		world->assemble();
		RBXASSERT(world->isAssembled());
	}
}

// mac: 0x515A7E: server ? transmit RunService game time : silent update.
void Workspace::updateDistributedGameTime()
{
	RunService* runService = ServiceProvider::create<RunService>(this);
	if (serverIsPresent(this))
		setDistributedGameTime(runService->gameTime());
	else
		setDistributedGameTimeNoTransmit(runService->gameTime());
}

// 2016 shape: compare + Changed event (binary inlines this at the +69 store).
void Workspace::setDistributedGameTime(double value)
{
	if (value != distributedGameTime)
	{
		distributedGameTime = value;
		raiseChanged(prop_DistributedGameTime);
	}
}

// 2016 name (DataModel::onChildAdded calls it when a network Client is added); body not
// written yet (237 bytes).
// STUB: PLAYER 0x00710080
// STUB: STUDIO 0x0082d340
void Workspace::setNullMouseCommand()
{
	STUB(0x00710080);
}

// iOS: RBX::registerNetworkStreamingProp(), run once by the DataModel ctor (call_once on
// PLAYER 0x00de4778). Body not written yet.
// STUB: PLAYER 0x007154c0
// STUB: STUDIO 0x008321d0
void registerNetworkStreamingProp()
{
	STUB(0x007154c0);
}

// The 2013 class has virtual bases (the ctor tests the most-derived flag first) and keeps
// dataState; this stand-in only gives the DataModel ctor its call.
// STUB: PLAYER 0x00718e10
// STUB: STUDIO 0x008357b0
Workspace::Workspace(IDataState* dataState)
	: VerbContainer(NULL)
	, distributedGameTime(0.0)
	, world(new World())
	, fallbackCamera(NULL)
{
}

// mac: 0x515636: assemble, assert the mouse command is free, raise the CoreGui
// "Frame" ImageLabel to ZIndex 1.
// STUB: PLAYER 0x00719210
// STUB: STUDIO 0x00835bb0
void Workspace::start()
{
	assemble();
	// Binary asserts unconditionally (Workspace.cpp:792); the command slot is
	// empty until the tool pipeline sets one (TODO: restore bare assert).
	if (MouseCommand* command = getCurrentMouseCommand())
		RBXASSERT(!command->captured());  // "!getCurrentMouseCommand()->captured()"
	if (CoreGuiService* coreGui = ServiceProvider::find<CoreGuiService>(this))
	{
		if (Instance* frame = coreGui->findFirstChildByNameRecursive("Frame"))
		{
			if (ImageLabel* label = Instance::fastDynamicCast<ImageLabel>(frame))
				label->setZIndex(1);
		}
	}
}

// mac: 0x515896: lower the CoreGui "Frame" ImageLabel to ZIndex 10, assert the
// mouse command is free, stamp game time.
// STUB: PLAYER 0x00719390
// STUB: STUDIO 0x00835d30
void Workspace::stop()
{
	if (CoreGuiService* coreGui = ServiceProvider::find<CoreGuiService>(this))
	{
		if (Instance* frame = coreGui->findFirstChildByNameRecursive("Frame"))
		{
			if (ImageLabel* label = Instance::fastDynamicCast<ImageLabel>(frame))
				label->setZIndex(10);
		}
	}
	if (MouseCommand* command = getCurrentMouseCommand())
		RBXASSERT(!command->captured());  // (Workspace.cpp:810)
	updateDistributedGameTime();
}

// mac: 0x515AF0: stop the run, reset the physics world.
// STUB: PLAYER 0x00719510
// STUB: STUDIO 0x00835eb0
void Workspace::reset()
{
	stop();
	if (world)
		world->reset();
}

} // namespace RBX

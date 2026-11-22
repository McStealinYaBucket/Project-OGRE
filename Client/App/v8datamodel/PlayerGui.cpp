/* App/v8datamodel/PlayerGui.cpp -- October 2013. BasePlayerGui, CoreGuiService and PlayerGui (and, in
   2013, StarterGuiService) live in this one file: BasePlayerGui::scriptShouldRun's assert records
   ...\Client\App\v8datamodel\PlayerGui.cpp line 60, and the three classes' functions interleave in
   one .obj range (PLAYER 0x007083d0-0x0070c450). Layouts in BasePlayerGui.h, CoreGuiService.h and
   PlayerGui.h. StarterGuiService's code is still in StarterGuiService.cpp. */

#include "stdafx.h"

#include "V8DataModel/PlayerGui.h"
#include "V8DataModel/CoreGuiService.h"
#include "Network/Player.h"
#include "Network/Players.h"
#include "decomp.h"

namespace RBX {
	const char* const sBasePlayerGui = "BasePlayerGui";
	const char* const sPlayerGui = "PlayerGui";
	const char* const sCoreGuiService = "CoreGui";  // 0.125 metadata (was "CoreGuiService")
}

using namespace RBX;

DECOMP_SIZE_ASSERT(RBX::BasePlayerGui, 0x74);
DECOMP_SIZE_ASSERT(RBX::PlayerGui, 0x74);
DECOMP_SIZE_ASSERT(RBX::CoreGuiService, 0x90);

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::BasePlayerGui);
RBX_REGISTER_CLASS(RBX::PlayerGui);
RBX_REGISTER_CLASS(RBX::CoreGuiService);
}

namespace RBX {

	// The original then passes dynamic_cast<IAdornable*>(instance) to
	// IAdornableCollector::onRenderableDescendantAdded (0x009cac50, GfxBase, not declared yet).
	// STUB: PLAYER 0x007083d0
	// STUB: STUDIO 0x007951b0
	void BasePlayerGui::onDescendantAdded(Instance* instance)
	{
		Super::onDescendantAdded(instance);
	}

	// The original first passes dynamic_cast<IAdornable*>(instance.get()) to
	// IAdornableCollector::onRenderableDescendantRemoving (0x009cadb0).
	// STUB: PLAYER 0x00708410
	// STUB: STUDIO 0x007951f0
	void BasePlayerGui::onDescendantRemoving(const shared_ptr<Instance>& instance)
	{
		Super::onDescendantRemoving(instance);
	}

	// The original tail-calls adornableCollector->append3dSortedAdornItems(sortedAdorn, camera) (0x009cb130).
	// STUB: PLAYER 0x00708460
	// STUB: STUDIO 0x00795240
	void BasePlayerGui::append3dSortedAdorn(std::vector<IAdornable*>& sortedAdorn, const Camera* camera) const
	{
	}

	// As BasePlayerGui::onDescendantAdded, after locking the new descendant.
	// STUB: PLAYER 0x007084e0
	// STUB: STUDIO 0x007952c0
	void CoreGuiService::onDescendantAdded(Instance* instance)
	{
		if (instance)
			instance->setRobloxLocked(true);
		Instance::onDescendantAdded(instance);
	}

	// FUNCTION: PLAYER 0x00708720
	// FUNCTION: STUDIO 0x00795500
	BasePlayerGui::~BasePlayerGui()
	{
		FASTLOG2(FLog::GuiTargetLifetime, "BasePlayerGui destroyed: %p, GuiTarget pointer: %p", this, static_cast<GuiTarget*>(this));
	}

	// mac: 0x45E19E: create ScreenGui + name "RobloxGui" + robloxLock + parent.
	// FUNCTION: PLAYER 0x00708d80
	// FUNCTION: STUDIO 0x00795a60
	void CoreGuiService::createRobloxScreenGui()
	{
		robloxGui = Creatable<Instance>::create<ScreenGui>();
		robloxGui->setName("RobloxGui");
		robloxGui->setRobloxLocked(true);
		robloxGui->setParent(this);
	}

	// ICF-folded with Players::askAddChild.
	// FUNCTION: PLAYER 0x0070a240
	bool PlayerGui::askSetParent(const Instance* instance) const
	{
		return Instance::fastDynamicCast<Network::Player>(instance) != NULL;
	}

	// The original resets adornableCollector to a new IAdornableCollector (operator new(0x30), inline
	// ctor 0x00708d20) before the log; GfxBase's IAdornable.h only forward-declares that class.
	// STUB: PLAYER 0x0070b480
	// STUB: STUDIO 0x00797bb0
	BasePlayerGui::BasePlayerGui()
		: mouseWasOverGui(false)
	{
		FASTLOG2(FLog::GuiTargetLifetime, "BasePlayerGui created: %p, GuiTarget pointer: %p", this, static_cast<GuiTarget*>(this));
	}

	// The original asserts isAncestorOf(script) (line 60), lets a LocalScript run only under the local
	// player's PlayerGui (Players::findLocalPlayer 0x0056baa0, then two calls 0x006d9090/0x007086c0),
	// and otherwise returns Players::backendProcessing(this, false).
	// STUB: PLAYER 0x0070b550
	// STUB: STUDIO 0x00797c80
	bool BasePlayerGui::scriptShouldRun(BaseScript* script)
	{
		return Network::Players::backendProcessing(this, false);
	}

	// FUNCTION: PLAYER 0x0070c050
	// FUNCTION: STUDIO 0x00798750
	PlayerGui::PlayerGui()
	{
		setName("PlayerGui");
	}

	// FUNCTION: PLAYER 0x0070c350
	// FUNCTION: STUDIO 0x00798a60
	CoreGuiService::CoreGuiService()
	{
		setRobloxLocked(true);
		Instance::setName(sCoreGuiService);
		onScreenMessages.resize(3);
	}

} // namespace RBX

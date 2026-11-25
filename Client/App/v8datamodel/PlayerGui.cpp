/* App/v8datamodel/PlayerGui.cpp -- October 2013. BasePlayerGui, CoreGuiService, PlayerGui and
   StarterGuiService live in this one file: BasePlayerGui::scriptShouldRun's assert records
   ...\Client\App\v8datamodel\PlayerGui.cpp line 60, and the four classes' functions interleave in
   one .obj range (PLAYER 0x007083d0-0x0070c450, STUDIO 0x007951b0-0x00798b80). Layouts in
   BasePlayerGui.h, CoreGuiService.h, PlayerGui.h and StarterGuiService.h. The StarterGuiService
   reflection block and setShowGui/setResetPlayerGui are not written yet. */

#include "stdafx.h"

#include "V8DataModel/PlayerGui.h"
#include "V8DataModel/CoreGuiService.h"
#include "V8DataModel/StarterGuiService.h"
#include "Network/Player.h"
#include "Network/Players.h"
#include "Util/StandardOut.h"
#include "reflection/enumconverter.h"
#include "decomp.h"

namespace RBX {
	const char* const sBasePlayerGui = "BasePlayerGui";
	const char* const sPlayerGui = "PlayerGui";
	const char* const sCoreGuiService = "CoreGui";  // 0.125 metadata (was "CoreGuiService")
	const char* const sStarterGuiService = "StarterGui";  // 0.125 metadata (was "StarterGuiService")
}

using namespace RBX;

DECOMP_SIZE_ASSERT(RBX::BasePlayerGui, 0x74);
DECOMP_SIZE_ASSERT(RBX::PlayerGui, 0x74);
DECOMP_SIZE_ASSERT(RBX::CoreGuiService, 0x90);
DECOMP_SIZE_ASSERT(RBX::StarterGuiService, 0x98);

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::BasePlayerGui);
RBX_REGISTER_CLASS(RBX::PlayerGui);
RBX_REGISTER_CLASS(RBX::CoreGuiService);
RBX_REGISTER_CLASS(RBX::StarterGuiService);
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

	// FUNCTION: PLAYER 0x007095e0
	// FUNCTION: STUDIO 0x007960b0
	bool StarterGuiService::getCoreGuiEnabled(CoreGuiType type)
	{
		if (!Network::Players::frontendProcessing(this))
			StandardOut::singleton()->print(MESSAGE_WARNING, "StarterGui:GetCoreGuiEnabled called from server script.  Did you mean to use a local script?");

		return coreGuiEnabledState[type];
	}

} // namespace RBX

namespace RBX {
namespace Reflection {

	// FUNCTION: PLAYER 0x00709b20
	// FUNCTION: STUDIO 0x007962b0
	template<>
	EnumDesc<StarterGuiService::CoreGuiType>::EnumDesc()
		: EnumDescriptor("CoreGuiType", typeid(StarterGuiService::CoreGuiType))
	{
		addPair(StarterGuiService::COREGUI_PLAYERLISTGUI, "PlayerList");
		addPair(StarterGuiService::COREGUI_HEALTHGUI, "Health");
		addPair(StarterGuiService::COREGUI_BACKPACKGUI, "Backpack");
		addPair(StarterGuiService::COREGUI_CHATGUI, "Chat");
		addPair(StarterGuiService::COREGUI_ALL, "All");
	}

	template<>
	const Type& Type::getSingleton<StarterGuiService::CoreGuiType>()
	{
		return EnumDesc<StarterGuiService::CoreGuiType>::singleton();
	}

	template<> EnumRegistrar<StarterGuiService::CoreGuiType> EnumRegistrar<StarterGuiService::CoreGuiType>::registrar(0);
	template<> TypeRegistrar<StarterGuiService::CoreGuiType> TypeRegistrar<StarterGuiService::CoreGuiType>::registrar(0);

} // namespace Reflection

	// FUNCTION: PLAYER 0x00709c60
	// FUNCTION: STUDIO 0x007963f0
	void StarterGuiService::setCoreGuiEnabled(CoreGuiType type, bool enabled)
	{
		if (!Network::Players::frontendProcessing(this))
			StandardOut::singleton()->print(MESSAGE_WARNING, "StarterGui:SetCoreGuiEnabled called from server script.  Did you mean to use a local script?");

		if (type == COREGUI_ALL)
		{
			// The All state overwrites every key.
			for (int i = COREGUI_PLAYERLISTGUI; i <= COREGUI_ALL; ++i)
				coreGuiEnabledState[static_cast<CoreGuiType>(i)] = enabled;
		}
		else
		{
			coreGuiEnabledState[type] = enabled;

			// All follows the other keys when they agree.
			bool lastBool = enabled;
			for (int i = COREGUI_PLAYERLISTGUI; i < COREGUI_ALL; ++i)
			{
				CoreGuiType iteratedType = static_cast<CoreGuiType>(i);
				if (lastBool != coreGuiEnabledState[iteratedType])
				{
					coreGuiEnabledState[COREGUI_ALL] = false;
					lastBool = coreGuiEnabledState[iteratedType];
					break;
				}
			}

			if (lastBool == enabled)
				coreGuiEnabledState[COREGUI_ALL] = lastBool;
		}

		coreGuiChangedSignal(type, enabled);
	}

	// ICF-folded with Players::askAddChild.
	// FUNCTION: PLAYER 0x0070a240 FOLDED
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

	// FUNCTION: PLAYER 0x0070c150
	// FUNCTION: STUDIO 0x00798860
	StarterGuiService::StarterGuiService()
		: showGui(true)
		, resetPlayerGui(true)
	{
		setName(sStarterGuiService);

		for (int i = COREGUI_PLAYERLISTGUI; i <= COREGUI_ALL; ++i)
			coreGuiEnabledState[static_cast<CoreGuiType>(i)] = true;
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

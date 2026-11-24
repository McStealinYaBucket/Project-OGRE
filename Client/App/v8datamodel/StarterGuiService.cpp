/* App/v8datamodel/StarterGuiService.cpp — October 2013. The ctor and the CoreGui state accessors from
   the PLAYER bytes; layout evidence in include/V8DataModel/StarterGuiService.h. The reflection block
   and setShowGui/setResetPlayerGui are not written yet. In 2013 these bodies sit inside PlayerGui.obj. */

#include "stdafx.h"

#include "V8DataModel/StarterGuiService.h"
#include "Network/Players.h"
#include "Util/StandardOut.h"
#include "decomp.h"

DECOMP_SIZE_ASSERT(RBX::StarterGuiService, 0x98);

namespace RBX {
	const char* const sStarterGuiService = "StarterGui";  // 0.125 metadata (was "StarterGuiService")
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::StarterGuiService);
}

namespace RBX {

	// FUNCTION: PLAYER 0x007095e0
	// FUNCTION: STUDIO 0x007960b0
	bool StarterGuiService::getCoreGuiEnabled(CoreGuiType type)
	{
		if (!Network::Players::frontendProcessing(this))
			StandardOut::singleton()->print(MESSAGE_WARNING, "StarterGui:GetCoreGuiEnabled called from server script.  Did you mean to use a local script?");

		return coreGuiEnabledState[type];
	}

} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

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
} // namespace RBX

namespace RBX {

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

	// STUB: PLAYER 0x0070c150
	// STUB: STUDIO 0x00798860
	StarterGuiService::StarterGuiService()
		: showGui(true)
		, resetPlayerGui(true)
	{
		setName(sStarterGuiService);

		for (int i = COREGUI_PLAYERLISTGUI; i <= COREGUI_ALL; ++i)
			coreGuiEnabledState[static_cast<CoreGuiType>(i)] = true;
	}

} // namespace RBX

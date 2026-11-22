/* App/v8datamodel/GuiService.cpp -- October 2013. The ctor and the dialog helpers from the PLAYER
   bytes; each assert sits on the line its 2013 string records (118, 128, 145). */

#include "stdafx.h"

#include "V8DataModel/GuiService.h"
#include "decomp.h"
#include "Util/Http.h"
#include "Util/standardout.h"
#include "Network/Players.h"

DECOMP_SIZE_ASSERT(RBX::GuiService, 0xc0);

// Registered at PLAYER 0x00a6a460 / 0x00a6a480, after the dynamic initializers above them in the 2013 .obj.
// GLOBAL: PLAYER 0x00de9ac0
// GLOBAL: STUDIO 0x00f8ea28
// FFlag::DE4508Fixed
FASTFLAGVARIABLE(DE4508Fixed, false)
// GLOBAL: PLAYER 0x00de9ac1
// GLOBAL: STUDIO 0x00f8ea29
// FFlag::OpenNativeBrowserWindowFromLua
FASTFLAGVARIABLE(OpenNativeBrowserWindowFromLua, false)

namespace RBX {
	const char* const sGuiService = "GuiService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiService);
}

// The 2013 dialog code sits at lines 110-147 (asserts at 118, 128, 145).



































































namespace RBX {

	// FUNCTION: PLAYER 0x00693d60
	// FUNCTION: STUDIO 0x00857070
	bool GuiService::shouldPreemptCurrentDialog(DialogWrapper* newDialog) const
	{
		if (currentDialog == NULL)
			return true;
		if (newDialog->dialogType > currentDialog->dialogType)
			return true;
		if (newDialog->dialogType < currentDialog->dialogType)
			return false;
		RBXASSERT(newDialog->dialogType == currentDialog->dialogType);
		switch (newDialog->dialogType)
		{
		case CENTER_DIALOG_UNSOLICITED_DIALOG:
			return false;
		case CENTER_DIALOG_PLAYER_INITIATED_DIALOG:
		case CENTER_DIALOG_MODAL_DIALOG:
		case CENTER_DIALOG_QUIT_DIALOG:
			return true;
		default:
			RBXASSERT(0);
			return false;
		}
	}
	// FUNCTION: PLAYER 0x00693e20
	// FUNCTION: STUDIO 0x00857130
	bool GuiService::getModalDialogStatus() const
	{
		if (currentDialog == NULL)
			return false;
		switch (currentDialog->dialogType)
		{
		case CENTER_DIALOG_UNSOLICITED_DIALOG: case CENTER_DIALOG_PLAYER_INITIATED_DIALOG:
			return false;
		case CENTER_DIALOG_MODAL_DIALOG: case CENTER_DIALOG_QUIT_DIALOG:
			return true;
		default:
			RBXASSERT(0);
			return false;
		}
	}

	// FUNCTION: PLAYER 0x00695ca0
	// FUNCTION: STUDIO 0x008590a0
	void GuiService::removeKey(std::string key)
	{
		if (key.size() != 1)
			throw RBX::runtime_error("GuiService:RemoveKey requires a string with a single character");
		char character = tolower(key[0]);

		std::set<char>::iterator it = subscribedChars.find(character);
		if (it != subscribedChars.end())
			subscribedChars.erase(it);
	}

	// FUNCTION: PLAYER 0x00696980
	// FUNCTION: STUDIO 0x008598d0
	void GuiService::addKey(std::string key)
	{
		if (key.size() != 1)
			throw RBX::runtime_error("GuiService:AddKey requires a string with a single character");
		char character = tolower(key[0]);

		if (subscribedChars.find(character) == subscribedChars.end())
			subscribedChars.insert(character);
	}

} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0069a080
// FUNCTION: STUDIO 0x0085bf00
template<>
EnumDesc<GuiService::SpecialKey>::EnumDesc()
	: EnumDescriptor("SpecialKey", typeid(GuiService::SpecialKey))
{
	addPair(GuiService::KEY_INSERT, "Insert");
	addPair(GuiService::KEY_HOME, "Home");
	addPair(GuiService::KEY_END, "End");
	addPair(GuiService::KEY_PAGEUP, "PageUp");
	addPair(GuiService::KEY_PAGEDOWN, "PageDown");
	addPair(GuiService::KEY_CHATHOTKEY, "ChatHotkey");
}

template<>
const Type& Type::getSingleton<GuiService::SpecialKey>()
{
	return EnumDesc<GuiService::SpecialKey>::singleton();
}

template<> EnumRegistrar<GuiService::SpecialKey> EnumRegistrar<GuiService::SpecialKey>::registrar(0);
template<> TypeRegistrar<GuiService::SpecialKey> TypeRegistrar<GuiService::SpecialKey>::registrar(0);

// FUNCTION: PLAYER 0x0069a1d0
// FUNCTION: STUDIO 0x0085c050
template<>
EnumDesc<GuiService::CenterDialogType>::EnumDesc()
	: EnumDescriptor("CenterDialogType", typeid(GuiService::CenterDialogType))
{
	addPair(GuiService::CENTER_DIALOG_UNSOLICITED_DIALOG, "UnsolicitedDialog");
	addPair(GuiService::CENTER_DIALOG_PLAYER_INITIATED_DIALOG, "PlayerInitiatedDialog");
	addPair(GuiService::CENTER_DIALOG_MODAL_DIALOG, "ModalDialog");
	addPair(GuiService::CENTER_DIALOG_QUIT_DIALOG, "QuitDialog");
}

template<>
const Type& Type::getSingleton<GuiService::CenterDialogType>()
{
	return EnumDesc<GuiService::CenterDialogType>::singleton();
}

template<> EnumRegistrar<GuiService::CenterDialogType> EnumRegistrar<GuiService::CenterDialogType>::registrar(0);
template<> TypeRegistrar<GuiService::CenterDialogType> TypeRegistrar<GuiService::CenterDialogType>::registrar(0);

} // namespace Reflection
} // namespace RBX

namespace RBX {

	// ios: RBX::GuiService::dispatchKey(RBX::GuiService::SpecialKey); processKeyDown 0x0069a360 calls it with KEY_CHATHOTKEY.
	// FUNCTION: PLAYER 0x0069a300
	// FUNCTION: STUDIO 0x0085c180
	bool GuiService::dispatchKey(SpecialKey special)
	{
		if (hasSpecial(special))
		{
			specialKeyPressed(special, "");
			return true;
		}
		return false;
	}








	// Byte-identical (probe_compare); its Http::isRobloxSite callee 0x00623da0 is not written yet.
	// STUB: PLAYER 0x0069a540
	// STUB: STUDIO 0x0085c3c0
	void GuiService::openBrowserWindow(std::string url)
	{
		if (!FFlag::OpenNativeBrowserWindowFromLua)
		{
			StandardOut::singleton()->print(MESSAGE_WARNING, "GuiService::OpenBrowserWindow() is disabled.");
			return;
		}
		if (!RBX::Http::isRobloxSite(url.c_str()))
		{
			StandardOut::singleton()->print(MESSAGE_WARNING, "GuiService::OpenBrowserWindow() was called on non-Roblox url.");
			return;
		}
		if (!RBX::Network::Players::frontendProcessing(this))
		{
			StandardOut::singleton()->print(MESSAGE_WARNING, "GuiService::OpenBrowserWindow() was called on not a client (use local scripts on this call).");
			return;
		}
		openUrlWindow(url);
	}

	// FUNCTION: PLAYER 0x0069b310
	// FUNCTION: STUDIO 0x0085cfb0
	bool GuiService::showWaitingDialog(CenterDialogType type)
	{
		RBXASSERT(currentDialog == NULL);

		if (!dialogQueue[type].empty())
		{
			DialogWrapper* result = dialogQueue[type].front();
			dialogQueue[type].pop_front();

			currentDialog = result;

			return true;
		}
		return false;
	}

	// STUDIO 99.0%: reccmp/studio-stl.csv's FLIRT row at 0x00858310 (??1strstreambuf..._42) hides the
	// signal<void(std::string, std::string)> ctor it calls.
	// FUNCTION: PLAYER 0x0069ed90
	// STUB: STUDIO 0x00860ae0
	GuiService::GuiService()
		: Super(sGuiService)
		, showLegacyPlayerList(true)
		, currentDialog(NULL)
	{
		useLuaChat = false;
	}

} // namespace RBX

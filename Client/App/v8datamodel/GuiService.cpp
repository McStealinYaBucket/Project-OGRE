/* App/v8datamodel/GuiService.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GuiService.h"

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

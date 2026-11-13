/* App/v8datamodel/Hopper.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Hopper.h"

namespace RBX {
	const char* const sHopper = "Hopper";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Hopper);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"
#include "V8DataModel/HopperBin.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006c9c40
// FUNCTION: STUDIO 0x007b1fa0
template<>
EnumDesc<HopperBin::BinType>::EnumDesc()
	: EnumDescriptor("BinType", typeid(HopperBin::BinType))
{
	addPair(HopperBin::SCRIPT_BIN, "Script");
	addPair(HopperBin::GAME_TOOL, "GameTool");
	addPair(HopperBin::GRAB_TOOL, "Grab");
	addPair(HopperBin::CLONE_TOOL, "Clone");
	addPair(HopperBin::HAMMER_TOOL, "Hammer");
	addLegacy(5, "Slingshot", HopperBin::SCRIPT_BIN);
	addLegacy(6, "Rocket", HopperBin::SCRIPT_BIN);
	addLegacy(7, "Laser", HopperBin::SCRIPT_BIN);
}

template<>
const Type& Type::getSingleton<HopperBin::BinType>()
{
	return EnumDesc<HopperBin::BinType>::singleton();
}

template<> EnumRegistrar<HopperBin::BinType> EnumRegistrar<HopperBin::BinType>::registrar(0);
template<> TypeRegistrar<HopperBin::BinType> TypeRegistrar<HopperBin::BinType>::registrar(0);

} // namespace Reflection
} // namespace RBX

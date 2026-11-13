/* App/v8datamodel/Feature.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Feature.h"

namespace RBX {
	const char* const sFeature = "Feature";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Feature);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0091d370
// FUNCTION: STUDIO 0x00943ac0
template<>
EnumDesc<Feature::TopBottom>::EnumDesc()
	: EnumDescriptor("TopBottom", typeid(Feature::TopBottom))
{
	addPair(Feature::TOP, "Top");
	addPair(Feature::CENTER_TB, "Center");
	addPair(Feature::BOTTOM, "Bottom");
}

template<>
const Type& Type::getSingleton<Feature::TopBottom>()
{
	return EnumDesc<Feature::TopBottom>::singleton();
}

template<> EnumRegistrar<Feature::TopBottom> EnumRegistrar<Feature::TopBottom>::registrar(0);
template<> TypeRegistrar<Feature::TopBottom> TypeRegistrar<Feature::TopBottom>::registrar(0);

// FUNCTION: PLAYER 0x0091d490
// FUNCTION: STUDIO 0x00943be0
template<>
EnumDesc<Feature::LeftRight>::EnumDesc()
	: EnumDescriptor("LeftRight", typeid(Feature::LeftRight))
{
	addPair(Feature::LEFT, "Left");
	addPair(Feature::CENTER_LR, "Center");
	addPair(Feature::RIGHT, "Right");
}

template<>
const Type& Type::getSingleton<Feature::LeftRight>()
{
	return EnumDesc<Feature::LeftRight>::singleton();
}

template<> EnumRegistrar<Feature::LeftRight> EnumRegistrar<Feature::LeftRight>::registrar(0);
template<> TypeRegistrar<Feature::LeftRight> TypeRegistrar<Feature::LeftRight>::registrar(0);

// FUNCTION: PLAYER 0x0091d5b0
// FUNCTION: STUDIO 0x00943d00
template<>
EnumDesc<Feature::InOut>::EnumDesc()
	: EnumDescriptor("InOut", typeid(Feature::InOut))
{
	addPair(Feature::EDGE, "Edge");
	addPair(Feature::INSET, "Inset");
	addPair(Feature::CENTER_IO, "Center");
}

template<>
const Type& Type::getSingleton<Feature::InOut>()
{
	return EnumDesc<Feature::InOut>::singleton();
}

template<> EnumRegistrar<Feature::InOut> EnumRegistrar<Feature::InOut>::registrar(0);
template<> TypeRegistrar<Feature::InOut> TypeRegistrar<Feature::InOut>::registrar(0);

} // namespace Reflection
} // namespace RBX

/* v8datamodel/Enums.cpp -- October 2013.
   Six EnumDesc<T> ctors that share one 2013 .obj: PLAYER 0x008e5250-0x008e58d0, between
   Hash.cpp (0x008e5210) and SendPhysics.cpp (0x008e5ec0); the 2016 tree has the same set in
   App/v8datamodel/Enums.cpp (plus the later GuiObject::ImageScale). No assert names the file. */

#include "stdafx.h"

#include "V8DataModel/BasicPartInstance.h"
#include "V8DataModel/ExtrudedPartInstance.h"
#include "V8DataModel/PrismInstance.h"
#include "V8DataModel/PyramidInstance.h"
#include "V8DataModel/Handles.h"
#include "V8DataModel/GuiObject.h"


// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x008e5250
// FUNCTION: STUDIO 0x009809a0
template<>
EnumDesc<BasicPartInstance::LegacyPartType>::EnumDesc()
	: EnumDescriptor("PartType", typeid(BasicPartInstance::LegacyPartType))
{
	addPair(BasicPartInstance::BALL_LEGACY_PART, "Ball");
	addPair(BasicPartInstance::BLOCK_LEGACY_PART, "Block");
	addPair(BasicPartInstance::CYLINDER_LEGACY_PART, "Cylinder");
}

template<>
const Type& Type::getSingleton<BasicPartInstance::LegacyPartType>()
{
	return EnumDesc<BasicPartInstance::LegacyPartType>::singleton();
}

template<> EnumRegistrar<BasicPartInstance::LegacyPartType> EnumRegistrar<BasicPartInstance::LegacyPartType>::registrar(0);
template<> TypeRegistrar<BasicPartInstance::LegacyPartType> TypeRegistrar<BasicPartInstance::LegacyPartType>::registrar(0);

// FUNCTION: PLAYER 0x008e5370
// FUNCTION: STUDIO 0x00980ac0
template<>
EnumDesc<ExtrudedPartInstance::VisualTrussStyle>::EnumDesc()
	: EnumDescriptor("Style", typeid(ExtrudedPartInstance::VisualTrussStyle))
{
	addPair(ExtrudedPartInstance::FULL_ALTERNATING_CROSS_BEAM, "AlternatingSupports");
	addPair(ExtrudedPartInstance::BRIDGE_STYLE_CROSS_BEAM, "BridgeStyleSupports");
	addPair(ExtrudedPartInstance::NO_CROSS_BEAM, "NoSupports");
	addLegacyName("Alternating Supports", ExtrudedPartInstance::FULL_ALTERNATING_CROSS_BEAM);
	addLegacyName("Bridge Style Supports", ExtrudedPartInstance::BRIDGE_STYLE_CROSS_BEAM);
	addLegacyName("No Supports", ExtrudedPartInstance::NO_CROSS_BEAM);
}

template<>
const Type& Type::getSingleton<ExtrudedPartInstance::VisualTrussStyle>()
{
	return EnumDesc<ExtrudedPartInstance::VisualTrussStyle>::singleton();
}

template<> EnumRegistrar<ExtrudedPartInstance::VisualTrussStyle> EnumRegistrar<ExtrudedPartInstance::VisualTrussStyle>::registrar(0);
template<> TypeRegistrar<ExtrudedPartInstance::VisualTrussStyle> TypeRegistrar<ExtrudedPartInstance::VisualTrussStyle>::registrar(0);

// FUNCTION: PLAYER 0x008e5510
// FUNCTION: STUDIO 0x00980c60
template<>
EnumDesc<PrismInstance::NumSidesEnum>::EnumDesc()
	: EnumDescriptor("PrismSides", typeid(PrismInstance::NumSidesEnum))
{
	addPair(PrismInstance::sides3, "3");
	addPair(PrismInstance::sides5, "5");
	addPair(PrismInstance::sides6, "6");
	addPair(PrismInstance::sides8, "8");
	addPair(PrismInstance::sides10, "10");
	addPair(PrismInstance::sides20, "20");
}

template<>
const Type& Type::getSingleton<PrismInstance::NumSidesEnum>()
{
	return EnumDesc<PrismInstance::NumSidesEnum>::singleton();
}

template<> EnumRegistrar<PrismInstance::NumSidesEnum> EnumRegistrar<PrismInstance::NumSidesEnum>::registrar(0);
template<> TypeRegistrar<PrismInstance::NumSidesEnum> TypeRegistrar<PrismInstance::NumSidesEnum>::registrar(0);

// FUNCTION: PLAYER 0x008e5660
// FUNCTION: STUDIO 0x00980db0
template<>
EnumDesc<PyramidInstance::NumSidesEnum>::EnumDesc()
	: EnumDescriptor("PyramidSides", typeid(PyramidInstance::NumSidesEnum))
{
	addPair(PyramidInstance::sides3, "3");
	addPair(PyramidInstance::sides4, "4");
	addPair(PyramidInstance::sides5, "5");
	addPair(PyramidInstance::sides6, "6");
	addPair(PyramidInstance::sides8, "8");
	addPair(PyramidInstance::sides10, "10");
	addPair(PyramidInstance::sides20, "20");
}

template<>
const Type& Type::getSingleton<PyramidInstance::NumSidesEnum>()
{
	return EnumDesc<PyramidInstance::NumSidesEnum>::singleton();
}

template<> EnumRegistrar<PyramidInstance::NumSidesEnum> EnumRegistrar<PyramidInstance::NumSidesEnum>::registrar(0);
template<> TypeRegistrar<PyramidInstance::NumSidesEnum> TypeRegistrar<PyramidInstance::NumSidesEnum>::registrar(0);

// FUNCTION: PLAYER 0x008e57c0
// FUNCTION: STUDIO 0x00980f10
template<>
EnumDesc<Handles::VisualStyle>::EnumDesc()
	: EnumDescriptor("HandlesStyle", typeid(Handles::VisualStyle))
{
	addPair(Handles::RESIZE_HANDLES, "Resize");
	addPair(Handles::MOVEMENT_HANDLES, "Movement");
}

template<>
const Type& Type::getSingleton<Handles::VisualStyle>()
{
	return EnumDesc<Handles::VisualStyle>::singleton();
}

template<> EnumRegistrar<Handles::VisualStyle> EnumRegistrar<Handles::VisualStyle>::registrar(0);
template<> TypeRegistrar<Handles::VisualStyle> TypeRegistrar<Handles::VisualStyle>::registrar(0);

// FUNCTION: PLAYER 0x008e58d0
// FUNCTION: STUDIO 0x00981020
template<>
EnumDesc<GuiObject::SizeConstraint>::EnumDesc()
	: EnumDescriptor("SizeConstraint", typeid(GuiObject::SizeConstraint))
{
	addPair(GuiObject::RelativeXY, "RelativeXY");
	addPair(GuiObject::RelativeXX, "RelativeXX");
	addPair(GuiObject::RelativeYY, "RelativeYY");
}

template<>
const Type& Type::getSingleton<GuiObject::SizeConstraint>()
{
	return EnumDesc<GuiObject::SizeConstraint>::singleton();
}

template<> EnumRegistrar<GuiObject::SizeConstraint> EnumRegistrar<GuiObject::SizeConstraint>::registrar(0);
template<> TypeRegistrar<GuiObject::SizeConstraint> TypeRegistrar<GuiObject::SizeConstraint>::registrar(0);

} // namespace Reflection
} // namespace RBX

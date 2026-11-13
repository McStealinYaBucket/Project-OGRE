/* App/v8datamodel/Humanoid.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Humanoid.h"

namespace RBX {
	const char* const sHumanoid = "Humanoid";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Humanoid);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006db250
// FUNCTION: STUDIO 0x0081def0
template<>
EnumDesc<Humanoid::NameOcclusion>::EnumDesc()
	: EnumDescriptor("NameOcclusion", typeid(Humanoid::NameOcclusion))
{
	addPair(Humanoid::NAME_OCCLUSION_ALL, "OccludeAll");
	addPair(Humanoid::NAME_OCCLUSION_ENEMY, "EnemyOcclusion");
	addPair(Humanoid::NAME_OCCLUSION_NONE, "NoOcclusion");
}

template<>
const Type& Type::getSingleton<Humanoid::NameOcclusion>()
{
	return EnumDesc<Humanoid::NameOcclusion>::singleton();
}

template<> EnumRegistrar<Humanoid::NameOcclusion> EnumRegistrar<Humanoid::NameOcclusion>::registrar(0);
template<> TypeRegistrar<Humanoid::NameOcclusion> TypeRegistrar<Humanoid::NameOcclusion>::registrar(0);

// FUNCTION: PLAYER 0x006db370
// FUNCTION: STUDIO 0x0081e010
template<>
EnumDesc<Humanoid::Status>::EnumDesc()
	: EnumDescriptor("Status", typeid(Humanoid::Status))
{
	addPair(Humanoid::POISON_STATUS, "Poison");
	addPair(Humanoid::CONFUSION_STATUS, "Confusion");
}

template<>
const Type& Type::getSingleton<Humanoid::Status>()
{
	return EnumDesc<Humanoid::Status>::singleton();
}

template<> EnumRegistrar<Humanoid::Status> EnumRegistrar<Humanoid::Status>::registrar(0);
template<> TypeRegistrar<Humanoid::Status> TypeRegistrar<Humanoid::Status>::registrar(0);

} // namespace Reflection
} // namespace RBX

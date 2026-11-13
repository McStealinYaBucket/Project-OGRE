/* App/v8datamodel/NetworkSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/NetworkSettings.h"

namespace RBX {
	const char* const sNetworkSettings = "NetworkSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::NetworkSettings);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x005b6640
// FUNCTION: STUDIO 0x006087d0
template<>
EnumDesc<NetworkSettings::PhysicsSendMethod>::EnumDesc()
	: EnumDescriptor("PhysicsSendMethod", typeid(NetworkSettings::PhysicsSendMethod))
{
	addPair(NetworkSettings::ErrorComputation, "ErrorComputation");
	addPair(NetworkSettings::ErrorComputation2, "ErrorComputation2");
	addPair(NetworkSettings::RoundRobin, "RoundRobin");
	addPair(NetworkSettings::TopNErrors, "TopNErrors");
}

template<>
const Type& Type::getSingleton<NetworkSettings::PhysicsSendMethod>()
{
	return EnumDesc<NetworkSettings::PhysicsSendMethod>::singleton();
}

template<> EnumRegistrar<NetworkSettings::PhysicsSendMethod> EnumRegistrar<NetworkSettings::PhysicsSendMethod>::registrar(0);
template<> TypeRegistrar<NetworkSettings::PhysicsSendMethod> TypeRegistrar<NetworkSettings::PhysicsSendMethod>::registrar(0);

// FUNCTION: PLAYER 0x005b6770
// FUNCTION: STUDIO 0x00608900
template<>
EnumDesc<NetworkSettings::PhysicsReceiveMethod>::EnumDesc()
	: EnumDescriptor("PhysicsReceiveMethod", typeid(NetworkSettings::PhysicsReceiveMethod))
{
	addPair(NetworkSettings::Direct, "Direct");
	addPair(NetworkSettings::Interpolation, "Interpolation");
}

template<>
const Type& Type::getSingleton<NetworkSettings::PhysicsReceiveMethod>()
{
	return EnumDesc<NetworkSettings::PhysicsReceiveMethod>::singleton();
}

template<> EnumRegistrar<NetworkSettings::PhysicsReceiveMethod> EnumRegistrar<NetworkSettings::PhysicsReceiveMethod>::registrar(0);
template<> TypeRegistrar<NetworkSettings::PhysicsReceiveMethod> TypeRegistrar<NetworkSettings::PhysicsReceiveMethod>::registrar(0);

} // namespace Reflection
} // namespace RBX

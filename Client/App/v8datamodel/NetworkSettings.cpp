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

namespace RBX {

// STUB: PLAYER 0x005b7cf0
NetworkSettings::NetworkSettings()
	: m_unk0x70(0)
	, m_unk0x78(true)
	, m_unk0x80("127.0.0.1")
	, m_unk0x9c(0x94eb)
	, m_unk0xa0("")
	, m_unk0xc0(0)
	, m_unk0xc8(true)
	, m_unk0xcc(0)
	, m_unk0xd0(1)
	, m_unk0xd4(1)
	, m_unk0xd8(2)
	, m_unk0xdc(true)
	, m_unk0xe0(0)
	, m_unk0xe4(10.0f)
	, m_unk0xe8(1)
	, m_unk0xec(-1)
	, m_unk0xf0(10.0f)
	, m_unk0xf4(false)
	, m_unk0xf5(false)
	, m_unk0xfc(-200)
	, m_unk0x100(-200)
	, m_unk0x104(30.0f)
	, m_unk0x108(20.0f)
	, m_unk0x10c(20.0f)
	, m_unk0x110(60.0)
	, m_unk0x118(0)
{
	for (int i = 0; i < 0x6e - 0x64; i++)
		m_unk0x64[i] = false;
	for (int i = 0; i < 0x7e - 0x79; i++)
		m_unk0x79[i] = false;
	Instance::setName("Network");
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::NetworkSettings, 0x120);

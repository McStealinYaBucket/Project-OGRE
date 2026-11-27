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
// NetworkSettings is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
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

// FUNCTION: PLAYER 0x005b7cf0
// FUNCTION: STUDIO 0x00609e80
NetworkSettings::NetworkSettings()
	: printSplitMessages(false)
	, m_unk0x65(false)
	, m_unk0x66(false)
	, m_unk0x67(false)
	, m_unk0x68(false)
	, m_unk0x69(false)
	, m_unk0x6a(false)
	, m_unk0x6b(false)
	, m_unk0x6c(false)
#ifdef OGRE_TARGET_PLAYER	// STUDIO's ctor 0x00609e80 stores only 0x64-0x6c (older revision, 2 days apart)
	, m_unk0x6d(false)
#endif
	, m_unk0x70(0)
	, isQueueErrorComputed(true)
	, usePhysicsPacketCache(false)
	, useInstancePacketCache(false)
	, m_unk0x7b(false)
	, m_unk0x7c(false)
	, m_unk0x7d(false)
	, m_unk0x80("127.0.0.1")
	, m_unk0x9c(0x94eb)
	, m_unk0xa0("")
	, m_unk0xc0(0)
	, m_unk0xc8(true)
	, physicsSendMethod(ErrorComputation)
	, m_unk0xd0(1)
	, m_unk0xd4(1)
	, dataSendPriority(2)
	, distributedPhysicsEnabled(true)
	, m_unk0xe0(0)
	, m_unk0xe4(10.0f)
	, m_unk0xe8(1)
	, m_unk0xec(-1)
	, networkOwnerRate(10.0f)
	, m_unk0xf4(false)
	, m_unk0xf5(false)
	, physicsMtuAdjust(-200)
	, replicationMtuAdjust(-200)
	, m_unk0x104(30.0f)
	, m_unk0x108(20.0f)
	, m_unk0x10c(20.0f)
	, receiveRate(60.0)
	, m_unk0x118(0)
{
	Instance::setName("Network");
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::NetworkSettings, 0x120);

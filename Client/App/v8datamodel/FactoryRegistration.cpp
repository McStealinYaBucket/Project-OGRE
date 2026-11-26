/* App/v8datamodel/FactoryRegistration.cpp -- October 2013 (class registration TU), partial.
   The 2013 binary records no __FILE__ for this TU; the name is the 2016 tree's (factoryregistration.cpp),
   with an uppercase F so the glob links it ahead of LuaSettings.cpp, as 2013's TU sits ahead of it.
   2013 evidence (PLAYER): a run of ClassRegistrar dynamic initializers (0x00a75a20...) calls
   GlobalAdvancedSettings::classDescriptor (0x005268b0) first and LuaSettings::classDescriptor
   (0x00788570) later; the TU's own classDescriptor copies sit in one block (0x00786d70-0x0078a800:
   PhysicsSettings 0x007874f0, LuaSettings 0x00788570, TimerService 0x00789270) and read each class
   name through its extern, as a TU that does not define the s-name does. Only these four
   registrations are here so far; the rest of the block's classes still register in their own
   .cpp files. */

#include "stdafx.h"

#include "V8DataModel/GlobalAdvancedSettings.h"
#include "V8DataModel/PhysicsSettings.h"
#include "V8DataModel/LuaSettings.h"
#include "V8DataModel/TimerService.h"
#include "Network/NetworkPacketCache.h"
#include "Network/NetworkClusterPacketCache.h"

namespace RBX {
RBX_REGISTER_CLASS(RBX::GlobalAdvancedSettings);
RBX_REGISTER_CLASS(RBX::PhysicsSettings);
RBX_REGISTER_CLASS(RBX::LuaSettings);
RBX_REGISTER_CLASS(RBX::TimerService);
}

// The packet caches register here in 2013 (initializers PLAYER 0x00a76660/0x00a76680/0x00a766a0,
// between CharacterAppearance's and ChatFilter's).
namespace RBX {
namespace Reflection {
// GLOBAL: PLAYER 0x00e0cbe8
// GLOBAL: STUDIO 0x00f83598
template<> ClassRegistrar<Network::PhysicsPacketCache> ClassRegistrar<Network::PhysicsPacketCache>::registrar(0);
// GLOBAL: PLAYER 0x00e0ce14
// GLOBAL: STUDIO 0x00f837c8
template<> ClassRegistrar<Network::InstancePacketCache> ClassRegistrar<Network::InstancePacketCache>::registrar(0);
// GLOBAL: PLAYER 0x00e0c8b8
// GLOBAL: STUDIO 0x00f8325c
template<> ClassRegistrar<Network::ClusterPacketCache> ClassRegistrar<Network::ClusterPacketCache>::registrar(0);
} // namespace Reflection
} // namespace RBX

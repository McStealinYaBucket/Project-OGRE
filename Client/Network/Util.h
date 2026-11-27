#pragma once

// Network/Util.h -- October 2013, partial. No 2013 __FILE__ names this file; path and name from the
// 2016 tree (Network/Util.h). RakNetToRbxAddress: iOS/Mac symbol, PLAYER 0x005c6c50 (STUDIO 0x0064c350),
// right after ConcurrentRakPeer.obj.

#include "Util/SystemAddress.h"
#include "RakNetTypes.h"

namespace RBX {
namespace Network {

const RBX::SystemAddress RakNetToRbxAddress(const RakNet::SystemAddress& raknetAddress);

} // namespace Network
} // namespace RBX

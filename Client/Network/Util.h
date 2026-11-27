#pragma once

// Network/Util.h -- October 2013, partial. No 2013 __FILE__ names this file; path and name from the
// 2016 tree (Network/Util.h). RakNetToRbxAddress: iOS/Mac symbol, PLAYER 0x005c6c50 (STUDIO 0x0064c350),
// right after ConcurrentRakPeer.obj; RakNetAddressToString (iOS: RakNetAddressToString(RakNet::SystemAddress
// const&, bool, char)) follows it at PLAYER 0x005c6c80.

#include "Util/SystemAddress.h"
#include "RakNetTypes.h"
#include <string>

namespace RBX {
namespace Network {

const RBX::SystemAddress RakNetToRbxAddress(const RakNet::SystemAddress& raknetAddress);
std::string RakNetAddressToString(const RakNet::SystemAddress& raknetAddress, bool writePort = true, char portDelineator = '|');

} // namespace Network
} // namespace RBX

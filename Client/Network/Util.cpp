// Network/Util.cpp -- October 2013, partial (see Util.h).

#include "Util.h"

namespace RBX {
namespace Network {

// MSVC evaluates the arguments right to left: GetPort is called first (iOS calls GetBinaryAddress first).
// FUNCTION: PLAYER 0x005c6c50
// FUNCTION: STUDIO 0x0064c350
const RBX::SystemAddress RakNetToRbxAddress(const RakNet::SystemAddress& raknetAddress)
{
	return RBX::SystemAddress(raknetAddress.GetBinaryAddress(), raknetAddress.GetPort());
}

// FUNCTION: PLAYER 0x005c6c80
// FUNCTION: STUDIO 0x0064c380
std::string RakNetAddressToString(const RakNet::SystemAddress& raknetAddress, bool writePort, char portDelineator)
{
	char buffer[32];
	raknetAddress.ToString(writePort, buffer, portDelineator);
	return buffer;
}

} // namespace Network
} // namespace RBX

/* Network/NetworkOwner.h -- October 2013 (network-ownership sentinel addresses).
   2013 evidence: NetworkOwner::Unassigned (PLAYER 0x00559100, out of line in a Network TU;
   STUDIO 0x005b7bc0) holds a default SystemAddress and asserts on lines 28 and 29 only:
   "s == SystemAddress()" and "s != NetworkOwner::Server()", with Server() inlined as the
   guarded static {1, 0}. Nothing else of the class is referenced by the 2013 code we have.
   __FILE__: ...\Client\Network\include\Network/NetworkOwner.h. */

#pragma once

#include "Util/SystemAddress.h"
#include "rbx/Debug.h"
namespace RBX {
namespace Network {

	class NetworkOwner
	{
	public:
		static const SystemAddress Server()
		{
			static SystemAddress s(1, 0);
			return s;
		}

		// FUNCTION: PLAYER 0x00559100
		// FUNCTION: STUDIO 0x005b7bc0
		static const SystemAddress Unassigned() {
			static SystemAddress s;
			RBXASSERT(s == SystemAddress());
			RBXASSERT(s != NetworkOwner::Server());
			return s;
		}
	};

} // namespace Network
} // namespace RBX

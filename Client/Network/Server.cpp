/* Network/Server.cpp — October 2013 (presence gate; shell otherwise).
   TODO: full decompile (replication server machinery). */

#include "stdafx.h"

#include "Network/Server.h"

namespace RBX {
namespace Network {
	const char* const sServer = "NetworkServer";
}
}

using namespace RBX;
using namespace RBX::Network;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Network::Server);
}

namespace RBX {
namespace Network {

	// mac: 0x87D908: provider lookup + assert, present iff find<Server> hits.
	// STUB: PLAYER 0x005b34f0
	// STUB: STUDIO 0x006134f0
	bool Server::serverIsPresent(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* provider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || provider != NULL);
		if (provider == NULL)
			return false;
		return ServiceProvider::find<Server>(provider) != NULL;
	}

} // namespace Network
} // namespace RBX

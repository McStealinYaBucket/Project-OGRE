/* Network/Client.cpp — October 2013 (presence gates; shell otherwise).
   TODO: full decompile (replication peer machinery). */

#include "stdafx.h"

#include "Network/Client.h"

namespace RBX {
namespace Network {
	const char* const sClient = "NetworkClient";
}
}

using namespace RBX;
using namespace RBX::Network;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Network::Client);
}

namespace RBX {
namespace Network {

	// mac: 0x844AEC: provider lookup + assert, then find<Client> (or NULL).
	// STUB: PLAYER 0x005adbd0
	// STUB: STUDIO 0x0060c610
	Client* Client::findClient(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* provider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || provider != NULL);
		if (provider != NULL)
			return ServiceProvider::find<Client>(provider);
		return NULL;
	}

	// mac: 0x844B7C: presence = lookup hit.
	bool Client::clientIsPresent(const Instance* context, bool testInDatamodel)
	{
		return findClient(context, testInDatamodel) != NULL;
	}

} // namespace Network
} // namespace RBX

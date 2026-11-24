/* Network/Client.cpp -- October 2013, partial (ctor, dtor, the presence gates).
   Client::findClient's assert is Client.cpp:82, Client::onServiceProvider's (0x005ae170) :426.
   VC9 emits ~Client (0x005ad760) before findClient (0x005adbd0) and Client() (0x005b0880). */

#include "stdafx.h"

#include "Network/Client.h"
#include "Network/Players.h"
#include "V8DataModel/NetworkSettings.h"
#include "security/SecurityContext.h"
#include "FastLog.h"

LOGGROUP(Network)

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
// Client is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}

namespace RBX {
namespace Network {

	// The 2013 ctor stores GlobalAdvancedSettingsItem<NetworkSettings, sNetworkSettings>::singleton()
	// (PLAYER 0x00521d60) in networkSettings, between userId and the three signals
	// (NetworkSettings derives GlobalAdvancedSettingsItem since wave 6).
	// FUNCTION: PLAYER 0x005b0880
	// FUNCTION: STUDIO 0x0060f190
	Client::Client()
		: userId(-1), networkSettings(&NetworkSettings::singleton())
	{
		RBX::Security::Context::current().requirePermission(RBX::Security::RobloxPlace, "create a NetworkClient");
		setName(sClient);

		FASTLOG(FLog::Network, "NetworkClient:Create");
	}

	// FUNCTION: PLAYER 0x005ad760
	// FUNCTION: STUDIO 0x0060c1a0
	Client::~Client()
	{
		FASTLOG(FLog::Network, "NetworkClient:Remove");
	}





















	// mac: 0x844AEC: provider lookup + assert, then find<Client> (or NULL).
	// FUNCTION: PLAYER 0x005adbd0
	// FUNCTION: STUDIO 0x0060c610
	Client* Client::findClient(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* serviceProvider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || serviceProvider!=NULL);
		if (serviceProvider != NULL)
			return ServiceProvider::find<Client>(serviceProvider);
		return NULL;
	}

	// mac: 0x844B7C: presence = lookup hit. Same body as Players::clientIsPresent, which the
	// linker kept.
	// FUNCTION: PLAYER 0x005adc70 FOLDED
	// FUNCTION: STUDIO 0x0060c6b0 FOLDED
	bool Client::clientIsPresent(const Instance* context, bool testInDatamodel)
	{
		return findClient(context, testInDatamodel) != NULL;
	}

	// 2013 keeps it here, between Client::findClient and Client::onServiceProvider.
	// FUNCTION: PLAYER 0x005adc70 FOLDED
	// FUNCTION: STUDIO 0x0060c6b0 FOLDED
	bool Players::clientIsPresent(const Instance* context, bool testInDatamodel)
	{
		return Client::findClient(context, testInDatamodel) != NULL;
	}

} // namespace Network
} // namespace RBX

/* Network/Server.cpp — October 2013 (presence gate; shell otherwise).
   TODO: full decompile (replication server machinery). */

#include "stdafx.h"

#include "Network/Server.h"
#include "V8DataModel/NetworkSettings.h"
#include "security/SecurityContext.h"
#include "FastLog.h"

LOGGROUP(Network)
DECOMP_SIZE_ASSERT(RBX::Network::Server, 0xb78);

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
// Server is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}

namespace RBX {
namespace Network {

	// STUB: PLAYER 0x005b32b0
	// STUB: STUDIO 0x006132b0
	Server::~Server()
	{
		FASTLOG(FLog::Network, "NetworkServer:Destroy");
	}


















































	// mac: 0x87D908: provider lookup + assert, present iff find<Server> hits.
	// FUNCTION: PLAYER 0x005b34f0
	// FUNCTION: STUDIO 0x006134f0
	bool Server::serverIsPresent(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* serviceProvider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || serviceProvider!=NULL);
		return ServiceProvider::find<Server>(serviceProvider) != NULL;
	}

	// The 2013 body also fills the multi_index member and registers ids 1..17 through 0x005b20c0.
	// STUB: PLAYER 0x005b4fd0
	// STUB: STUDIO 0x00615420
	Server::Server()
		: m_unk0xac4(0), m_unk0xadc(false), m_unk0xae8(1)
		, networkSettings(&NetworkSettings::singleton())
	{
		RBX::Security::Context::current().requirePermission(RBX::Security::LocalUser);
		setName(sServer);
		FASTLOG(FLog::Network, "NetworkServer:Create");
	}

} // namespace Network
} // namespace RBX

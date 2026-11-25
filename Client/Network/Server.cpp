/* Network/Server.cpp — October 2013 (presence gate; shell otherwise).
   TODO: full decompile (replication server machinery). */

#include "stdafx.h"

#include "Network/Server.h"
#include "ServerReplicator.h"
#include "ConcurrentRakPeer.h"
#include "RakPeerInterface.h"
#include "V8DataModel/NetworkSettings.h"
#include "security/SecurityContext.h"
#include "FastLog.h"

LOGGROUP(Network)

namespace RBX { namespace Network { extern std::string versionB; } }

// Stand-in: 2013 registers it from App's FastLogSettings.cpp block (PLAYER initializer 0x00a657f0).
namespace FFlag
{
	// GLOBAL: PLAYER 0x00de3f5f
	// GLOBAL: STUDIO 0x00f84e38
	bool DebugLocalRccServerConnection = false;
	int DebugLocalRccServerConnectionIniter = FLog::RegisterFlag("DebugLocalRccServerConnection", &DebugLocalRccServerConnection, NULL, FASTVARTYPE_STATIC);
}
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

	// FUNCTION: PLAYER 0x005b0db0
	// FUNCTION: STUDIO 0x0060fa20
	void Server::onCreateRakPeer()
	{
		Peer::onCreateRakPeer();
		rakPeer->rawPeer()->SetMaximumIncomingConnections(128);
		if (FFlag::DebugLocalRccServerConnection)
			versionB = "test";
		rakPeer->rawPeer()->SetIncomingPassword(versionB.c_str(), versionB.size());
	}

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

	// FUNCTION: PLAYER 0x005b3690
	// FUNCTION: STUDIO 0x00613690
	bool Server::askAddChild(const Instance* instance) const
	{
		return Instance::fastDynamicCast<ServerReplicator>(instance) != NULL;
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

	// Old provider: remove the NetworkOwnerJob (0xad0), stop(1000) 0x005b1dd0, drop 0xac8 and the
	// connection 0xad8; Peer::onServiceProvider; new provider: Players (0xac8), the connections, the
	// NetworkSettings switches (+0x79, +0x7a, +0xdc) and a new NetworkOwnerJob (operator new 0x230).
	// STUB: PLAYER 0x005b5310
	// STUB: STUDIO 0x00615760
	void Server::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		STUB(0x005b5310);
	}

} // namespace Network
} // namespace RBX

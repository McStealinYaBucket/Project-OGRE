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

// FFlag::DebugLocalRccServerConnection is defined in App's FastLogSettings.cpp (2013 initializer 0x00a65800).
FASTFLAG(DebugLocalRccServerConnection)






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

	// FUNCTION: PLAYER 0x005b32b0
	// FUNCTION: STUDIO 0x006132b0
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

	// FUNCTION: PLAYER 0x005b4fd0
	// FUNCTION: STUDIO 0x00615420
	Server::Server()
		: m_unk0xac4(0), m_unk0xadc(false), m_unk0xae8(1)
		, networkSettings(&NetworkSettings::singleton())
	{
		RBX::Security::Context::current().requirePermission(RBX::Security::LocalUser);
		setName(sServer);

		// 2016: registerLegalScript(""); the "" converts to the pair's const std::string& here.
		m_unk0xb18.insert(boost::bimap<std::string, long>::value_type("", m_unk0xae8++));

		int ids[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
		m_unk0xaec.assign(ids, ids + 17);

		FASTLOG(FLog::Network, "NetworkServer:Create");
	}

	// Read off (2016 shape minus the clientIsPresent throw, the players null check, workspaceLoaded and
	// analytics): old provider: TaskScheduler::remove(networkOwnerJob 0xad0), reset; players(0xac8)->
	// setConnection(NULL); stop(1000) 0x005b1dd0; players.reset(); itemAddedConnection(0xad8).disconnect().
	// Peer::onServiceProvider. New provider: players = shared_from(create<Players>()), setConnection
	// (rakPeer.get()); visitDescendants(bind(&Server::onItemAdded 0x005b4560, this, _1)) 0x005cfdd0;
	// itemAddedConnection = onDemandWrite()->descendantAddedSignal.connect(same bind);
	// NetworkSettings +0x79 / +0x7a gate create<PhysicsPacketCache> 0x005b2960 -> 0xae0 and
	// create<InstancePacketCache> 0x005b2ae0 -> 0xae4; FFlag::UseClusterCache (byte 0x00de3f88,
	// registration 0x00a661d0 in App's block) gates create<ClusterPacketCache> 0x005b2c60; NetworkSettings
	// +0xdc gates networkOwnerJob = shared_ptr<NetworkOwnerJob>(new NetworkOwnerJob(shared_from(
	// static_cast<DataModel*>(newProvider)))) and TaskScheduler::add. Blocked on the three cache classes,
	// NetworkOwnerJob (its ctor reads private NetworkSettings+0xf0) and the private NetworkSettings bools.
	// STUB: PLAYER 0x005b5310
	// STUB: STUDIO 0x00615760
	void Server::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		STUB(0x005b5310);
	}

} // namespace Network
} // namespace RBX

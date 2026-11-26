/* Network/Server.cpp — October 2013 (presence gate; shell otherwise).
   TODO: full decompile (replication server machinery). */

#include "stdafx.h"

#include "Network/Server.h"
#include "ServerReplicator.h"
#include "ConcurrentRakPeer.h"
#include "RakPeerInterface.h"
#include "V8DataModel/NetworkSettings.h"
#include "V8DataModel/DataModel.h"
#include "Network/Players.h"
#include "Network/NetworkPacketCache.h"
#include "Network/NetworkClusterPacketCache.h"
#include "NetworkOwnerJob.h"
#include <boost/bind.hpp>
#include "security/SecurityContext.h"
#include "FastLog.h"

LOGGROUP(Network)

namespace RBX { namespace Network { extern std::string versionB; } }

// FFlag::DebugLocalRccServerConnection is defined in App's FastLogSettings.cpp (2013 initializer 0x00a65800).
FASTFLAG(DebugLocalRccServerConnection)
DYNAMIC_FASTFLAG(UseClusterCache)

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

	// FUNCTION: PLAYER 0x005b1dd0
	// FUNCTION: STUDIO 0x00610f70
	void Server::stop(int blockDuration)
	{
		FASTLOG1(FLog::Network, "NetworkServer:Stop blockDuration(%d)", blockDuration);
		visitChildren(boost::bind(&Instance::unlockParent, _1));
		removeAllChildren();
		if (rakPeer->rawPeer()->IsActive())
			rakPeer->rawPeer()->Shutdown(blockDuration);
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

	// STUB: PLAYER 0x005b4560
	__declspec(noinline) void Server::onItemAdded(shared_ptr<Instance> item)
	{
		STUB(0x005b4560);
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

	// STUB: PLAYER 0x005b5310
	// STUB: STUDIO 0x00615760
	void Server::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		if (oldProvider)
		{
			TaskScheduler::singleton().remove(networkOwnerJob);
			networkOwnerJob.reset();
			players->setConnection(NULL);
			stop(1000);
			players.reset();
			itemAddedConnection.disconnect();
		}

		Peer::onServiceProvider(oldProvider, newProvider);

		if (newProvider)
		{
			DataModel* dataModel = boost::polymorphic_downcast<DataModel*>(newProvider);
			players = shared_from(ServiceProvider::create<Players>(newProvider));
			players->setConnection(rakPeer.get());

			newProvider->visitDescendants(boost::bind(&Server::onItemAdded, this, _1));
			itemAddedConnection = newProvider->onDemandWrite()->descendantAddedSignal.connect(boost::bind(&Server::onItemAdded, this, _1));

			if (networkSettings->usePhysicsPacketCache)
				physicsPacketCache = ServiceProvider::create<PhysicsPacketCache>(newProvider);
			if (networkSettings->useInstancePacketCache)
				instancePacketCache = ServiceProvider::create<InstancePacketCache>(newProvider);
			if (DFFlag::UseClusterCache)
				ServiceProvider::create<ClusterPacketCache>(newProvider);

			if (networkSettings->distributedPhysicsEnabled)
			{
				networkOwnerJob = shared_ptr<NetworkOwnerJob>(new NetworkOwnerJob(shared_from(dataModel)));
				TaskScheduler::singleton().add(networkOwnerJob);
			}
		}
	}

} // namespace Network
} // namespace RBX

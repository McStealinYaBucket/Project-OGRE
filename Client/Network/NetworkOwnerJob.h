#pragma once

// Network/NetworkOwnerJob.h -- October 2013, partial. RTTI NetworkOwnerJob : DataModelJob (vftable
// PLAYER 0x00b2791c), 0x230 bytes (operator new in Server::onServiceProvider 0x005b5310). Layout from
// the ctor (0x005f7370). The client map's node holds 0x1c bytes (isnil at +0x29); 2016's
// map<SystemAddress, ClientLocation> fits, ClientLocation's 2013 members are unproven.

#include "V8DataModel/DataModelJob.h"
#include "Util/SystemAddress.h"
#include "decomp.h"
#include <map>

namespace RBX {
	class DataModel;

namespace Network {

	// SIZE 0x230
	class NetworkOwnerJob : public DataModelJob
	{
		struct ClientLocation
		{
			undefined m_unk0x00[0x14];                  // 0x00
		};
		typedef std::map<RBX::SystemAddress, ClientLocation> ClientMap;

		boost::weak_ptr<DataModel> dataModel;           // 0x218
		float networkOwnerRate;                         // 0x220
		ClientMap clientMap;                            // 0x224
	public:
		NetworkOwnerJob(shared_ptr<DataModel> dataModel);

	private:
		/*override*/ Time::Interval sleepTime(const Stats& stats);                       // vtable+0x04
		/*override*/ Job::Error error(const Stats& stats);                               // vtable+0x08
		/*override*/ TaskScheduler::StepResult stepDataModelJob(const Stats& stats);     // vtable+0x18
	};

} // namespace Network
} // namespace RBX

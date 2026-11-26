/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

// Network/Peer.cpp -- October 2013, partial. One assert (line 238, Peer::onServiceProvider
// 0x005ba1d0, not written yet). VC9 emits ~Peer (0x005b8bf0) well before Peer() (0x005b9ac0).

#include "Peer.h"
#include "ConcurrentRakPeer.h"
#include "Replicator.h"
#include "RakPeer.h"
#include "RakPeerInterface.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/DataModelJob.h"
#include "V8DataModel/NetworkSettings.h"
#include "V8DataModel/PhysicsService.h"
#include "V8DataModel/StatsItem.h"
#include "V8DataModel/StatsService.h"
#include "rbx/rbxtime.h"
#include "FastLog.h"
#include "decomp.h"

namespace RBX {
namespace Network {

const char* const sPeer = "NetworkPeer";

unsigned char Peer::aesKey[16];

// GLOBAL: PLAYER 0x00d25280
// GLOBAL: STUDIO 0x00e9fa98
static double lerp = 0.05;

DECOMP_SIZE_ASSERT(Peer, 0xac0);

// The peer's own GUID seeds the random generator; occasional pings on.
// FUNCTION: PLAYER 0x005b8910
// FUNCTION: STUDIO 0x0064a880
void Peer::onCreateRakPeer()
{
	RakNet::RakNetGUID guid = rakPeer->rawPeer()->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	rnr.SeedMT((unsigned int)((guid.g >> 32) ^ guid.g));
	rakPeer->rawPeer()->SetOccasionalPing(true);
}

// STUB: PLAYER 0x005b8bf0
// STUB: STUDIO 0x0064ab60
Peer::~Peer()
{
}

// RakPeer plus the owning Peer (0x5e8), 0x5f0 bytes; vftable 0x00b1f0cc (STUDIO 0x00bd4dcc) has
// RakPeer's 83 slots and this class's own RunUpdateCycle at slot 83.
class ProfiledRakPeer : public RakNet::RakPeer
{
private:
	Peer& peer;	// 0x5e8
	typedef RakNet::RakPeer Super;

public:
	ProfiledRakPeer(Peer& peer):peer(peer) {}

	// FUNCTION: PLAYER 0x005b8e80
	// FUNCTION: STUDIO 0x0064adf0
	virtual bool RunUpdateCycle(RakNet::TimeUS timeNS, RakNet::Time timeMS)
	{
		Timer<Time::Benchmark> timer;
		RakNet::BitStream updateBitStream(MAXIMUM_MTU_SIZE);
		bool result = Super::RunUpdateCycle(timeNS, timeMS, updateBitStream);
		peer.rakDutyCycle.sample(timer.delta());
		return result;
	}
};

// DataModelJob (0x218), then dataModel 0x218, receiveRate 0x220, rakPeer 0x228; 0x230 bytes.
// sleepTime and error fold with SoundService::SoundJob's (0x006f0680, 0x005b9480).
class PacketReceiveJob : public DataModelJob
{
	weak_ptr<DataModel> dataModel;	// 0x218
	double receiveRate;				// 0x220
public:
	weak_ptr<ConcurrentRakPeer> rakPeer;	// 0x228

	// FUNCTION: PLAYER 0x005b92c0
	// STUB: STUDIO 0x0064b030
	PacketReceiveJob(shared_ptr<ConcurrentRakPeer> rakPeer, DataModel* dataModel)
		:DataModelJob("Net PacketReceive", DataModelJob::DataIn, false, shared_from(dataModel), Time::Interval(0))
		,dataModel(shared_from(dataModel))
		,receiveRate(NetworkSettings::singleton().getReceiveRate())
		,rakPeer(rakPeer)
	{
	}

private:
	Time::Interval sleepTime(const Stats& stats)
	{
		return computeStandardSleepTime(stats, receiveRate);
	}

	virtual Job::Error error(const Stats& stats)
	{
		return computeStandardError(stats, receiveRate);
	}

	// FUNCTION: PLAYER 0x005b9560
	// FUNCTION: STUDIO 0x0064b2d0
	virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats)
	{
		if (shared_ptr<DataModel> safeDataModel = dataModel.lock())
		{
			FASTLOG1(FLog::DataModelJobs, "Packet receive start, data model: %p", safeDataModel.get());
			DataModel::scoped_write_request request(safeDataModel.get());

			if (shared_ptr<ConcurrentRakPeer> safeRakPeer = rakPeer.lock())
				while (RakNet::Packet* packet = safeRakPeer->rawPeer()->Receive())
					safeRakPeer->DeallocatePacket(packet);

			FASTLOG1(FLog::DataModelJobs, "Packet receive finish, data model: %p", safeDataModel.get());
			return TaskScheduler::Stepped;
		}
		return TaskScheduler::Done;
	}
};

// FUNCTION: PLAYER 0x005b9960
// FUNCTION: STUDIO 0x0064b5b0
bool Peer::askAddChild(const Instance* instance) const
{
	return Instance::fastDynamicCast<Replicator>(instance) != NULL;
}

// FUNCTION: PLAYER 0x005b9ac0
// FUNCTION: STUDIO 0x0064b710
Peer::Peer()
	:rakDutyCycle(lerp)
{
	for (int i = 0; i < 16; ++i)
		aesKey[i] = 0xFE ^ 7*i;

	protocolVersion = 18;
}

// Stats::Item (0x88), then peer 0x88 and four child items 0x8c..0x98; 0xa0 bytes (new in
// Creatable<Instance>::create<PeerStatsItem> 0x005ba150).
class PeerStatsItem : public Stats::Item
{
	Peer* peer;
	Stats::Item* rakRate;
	Stats::Item* rakActivity;
	Stats::Item* physicsSenders;
	Stats::Item* bufferHealth;

public:
	// FUNCTION: PLAYER 0x005b9bb0
	// FUNCTION: STUDIO 0x0064b800
	PeerStatsItem(Peer* peer):peer(peer)
	{
		Stats::Item* item = createChildItem("Packets Thread");
		rakRate = item->createChildItem("Rate");
		rakActivity = item->createChildItem("Activity");
		physicsSenders = item->createChildItem("Physics Senders");
		bufferHealth = item->createChildItem("Send Buffer Health");
	}

	// FUNCTION: PLAYER 0x005ba060
	// FUNCTION: STUDIO 0x0064bcc0
	/*override*/ void update()
	{
		double dutyCycle = peer->rakDutyCycle.dutyCycle();
		rakActivity->formatValue(dutyCycle, "%.1f%%", 100.0 * dutyCycle);

		double rate = peer->rakDutyCycle.rate();
		rakRate->formatValue(rate, "%.2g/s", rate);

		{
			int senders;
			if (PhysicsService* physicsService = ServiceProvider::find<PhysicsService>(this))
				senders = physicsService->numSenders();
			else
				senders = 0;
			physicsSenders->formatValue(senders);
		}

		double bufferHealthValue = peer->rakPeer->GetBufferHealth();
		bufferHealth->formatValue(bufferHealthValue, "%.4g", bufferHealthValue);
	}
};







// FUNCTION: PLAYER 0x005ba1d0
// FUNCTION: STUDIO 0x0064be30
void Peer::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	Stats::StatsService* stats = ServiceProvider::find<Stats::StatsService>(oldProvider);
	if (stats)
	{
		shared_ptr<Stats::Item> network = shared_from_polymorphic_downcast<Stats::Item>(stats->findFirstChildByName("Network"));
		if (network)
			network->setParent(NULL);
	}

	if (receiveJob)
	{
		receiveJob->rakPeer.reset();
		TaskScheduler::singleton().remove(receiveJob);
		receiveJob.reset();
	}

	this->removeAllChildren();

	if (rakPeer)
	{
		rakPeer->rawPeer()->DetachPlugin(this);
		rakPeer.reset();
	}

	Super::onServiceProvider(oldProvider, newProvider);

	if (newProvider)
	{
		rakPeer.reset(new ConcurrentRakPeer(
			new ProfiledRakPeer(*this),
			boost::polymorphic_downcast<DataModel*>(newProvider)
		));
		rakPeer->rawPeer()->AttachPlugin(this);
		onCreateRakPeer();

		receiveJob = shared_ptr<PacketReceiveJob>(new PacketReceiveJob(rakPeer, boost::polymorphic_downcast<DataModel*>(newProvider)));
		TaskScheduler::singleton().add(receiveJob);
	}

	stats = ServiceProvider::find<Stats::StatsService>(newProvider);
	if (stats)
	{
		RBXASSERT(!shared_from_polymorphic_downcast<Stats::Item>(stats->findFirstChildByName("Network")));
		shared_ptr<Stats::Item> network = Creatable<Instance>::create<PeerStatsItem>(this);
		network->setName("Network");
		network->setParent(stats);
	}
}

} }

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
// Peer is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}

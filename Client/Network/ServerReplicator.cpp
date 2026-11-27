// Network/ServerReplicator.cpp -- October 2013, partial.

#include "ServerReplicator.h"
#include "V8DataModel/NetworkSettings.h"
#include "FastLog.h"
#include "BitStream.h"
#include "Util.h"
#include "Network/Players.h"
#include "Network/Player.h"
#include "V8DataModel/PartInstance.h"
#include "V8World/Mechanism.h"
#include "Replicator.StreamJob.h"
#include "Util/standardout.h"

// FLog::JoinSendExtraItemCount is App's (script/ScriptContext.cpp).
LOGGROUP(JoinSendExtraItemCount)
LOGGROUP(Network)

// DFLog::NetworkJoin is App's too (dynamic, kind 1; ScriptContext.obj initializer PLAYER 0x00a66270).
DYNAMIC_LOGGROUP(NetworkJoin)

namespace RBX {
namespace Network {

// GLOBAL: PLAYER 0x00b17468
// GLOBAL: STUDIO 0x00bd6a78
const char* const sServerReplicator = "ServerReplicator";

// FUNCTION: PLAYER 0x00571870
// FUNCTION: STUDIO 0x00654010
bool ServerReplicator::canUseProtocolVersion(int protocolVersion) const
{
	return remoteProtocolVersion == 0 || protocolVersion <= remoteProtocolVersion;
}

// FUNCTION: PLAYER 0x005718b0
// FUNCTION: STUDIO 0x00654050
void ServerReplicator::receiveCluster(RakNet::BitStream& inBitstream, Instance* instance)
{
	if (acceptsTerrainChanges)
		Super::receiveCluster(inBitstream, instance);
}

// While joining (no send-buffer limit) the server sends extra item packets per step until the first
// marker goes out.
namespace {
	// 2016 PacketIds.h ID_SPAWN_NAME; 0x8f in 2013 (OnReceive 0x005719ef).
	const unsigned char ID_SPAWN_NAME = 0x8f;
}

// Clients older than the protocol that added these Lighting properties do not get them.
// FUNCTION: PLAYER 0x005718e0
// FUNCTION: STUDIO 0x00654070
bool ServerReplicator::canReplicateProperty(const Reflection::ConstProperty& prop)
{
	if (prop.getInstance() == m_unk0x2b00)
	{
		if (&prop.getDescriptor() == m_unk0x2b04)
		{
			FASTLOG1(FLog::Network, "ServerReplicator::canReplicateProperty trying to rep GlobalShadows with protocol %i", remoteProtocolVersion);
			return remoteProtocolVersion >= 10;
		}
		else if (&prop.getDescriptor() == m_unk0x2b08)
		{
			FASTLOG1(FLog::Network, "ServerReplicator::canReplicateProperty trying to rep OutdoorAmbient with protocol %i", remoteProtocolVersion);
			return remoteProtocolVersion >= 11;
		}
		else if (&prop.getDescriptor() == m_unk0x2b0c)
		{
			FASTLOG1(FLog::Network, "ServerReplicator::canReplicateProperty trying to rep Outlines with protocol %i", remoteProtocolVersion);
			return remoteProtocolVersion >= 14;
		}
	}
	return true;
}

// FUNCTION: PLAYER 0x005719b0
// FUNCTION: STUDIO 0x00654140
RakNet::PluginReceiveResult ServerReplicator::OnReceive(RakNet::Packet* packet)
{
	if (packet->systemAddress != remotePlayerId)
		return RakNet::RR_CONTINUE_PROCESSING;

	if ((unsigned char)packet->data[0] != ID_SPAWN_NAME)
		return Super::OnReceive(packet);

	RakNet::BitStream inBitstream(packet->data, packet->length, false);
	inBitstream.IgnoreBits(8);
	RBX::operator>>(inBitstream, initialSpawnName);
	FASTLOGS(DFLog::NetworkJoin, "initialSpawnName: %s", initialSpawnName);
	return RakNet::RR_STOP_PROCESSING_AND_DEALLOCATE;
}

// FUNCTION: PLAYER 0x00571a90
// FUNCTION: STUDIO 0x00654220
bool ServerReplicator::sendItemsPacket()
{
	if (!Super::sendItemsPacket())
		return false;
	if (settings().m_unk0xec == -1 && waitingForMarker)
		for (int i = 0; i < FLog::JoinSendExtraItemCount; i++)
			if (!Super::sendItemsPacket())
				return false;
	return true;
}

// The FLog::ForEachVariable visitor serializeSFFlags passes (2016 name; a file static). Not written yet.
// STUB: PLAYER 0x00571af0
// STUB: STUDIO 0x00654280
static void serializeSFFlag(const std::string& name, const std::string& value, void* context)
{
	STUB(0x00571af0);
}

// File statics (2016 names; the 2013 assert at line 244 spells the const one), inlined.
static PartInstance* getMechanismRootMovingPart(PartInstance* part)
{
	Primitive* p = Mechanism::getRootMovingPrimitive(part->getPartPrimitive());
	return PartInstance::fromPrimitive(p);
}

static const PartInstance* getConstMechanismRootMovingPart(const PartInstance* part)
{
	const Primitive* p = Mechanism::getConstRootMovingPrimitive(part->getPartPrimitive());
	return PartInstance::fromConstPrimitive(p);
}

// Accept physics from the client only for a root part it owns. The three checkDistributed* bodies differ
// from 2013 only in PartInstance::primitive's offset (2013 +0xb8, ours +0xb0: App's PartInstance layout).
// STUB: PLAYER 0x00572050
// STUB: STUDIO 0x00654490
bool ServerReplicator::checkDistributedReceive(PartInstance* part)
{
	PartInstance* rootMoving = getMechanismRootMovingPart(part);
	bool isRootPart = (rootMoving == part);
	bool fromOwner = (rootMoving->getNetworkOwner() == RakNetToRbxAddress(remotePlayerId));

	return (isRootPart && fromOwner);
}

// Blank lines: the asserts below sit on their 2013 lines (231, 243, 244).




















































































// 2013 tests the owner itself (no call to checkDistributedSendFast) and allows a NULL root.
// STUB: PLAYER 0x005720c0
// STUB: STUDIO 0x00654500
bool ServerReplicator::checkDistributedSend(const PartInstance* part)
{
	RBXASSERT(part);

	const PartInstance* rootMoving = getConstMechanismRootMovingPart(part);
	bool isRootPart = (rootMoving == part);
	bool toOwner = rootMoving && (rootMoving->getNetworkOwner() == RakNetToRbxAddress(remotePlayerId));
	return (isRootPart && !toOwner);
}
// assumes part is root
// STUB: PLAYER 0x00572190
// STUB: STUDIO 0x006545d0
bool ServerReplicator::checkDistributedSendFast(const PartInstance* part)
{
	RBXASSERT(part);
	RBXASSERT(getConstMechanismRootMovingPart(part) == part);

	return part->getNetworkOwner() != RakNetToRbxAddress(remotePlayerId);
}

// The first marker went out: physics replication starts, and so does streaming.
// FUNCTION: PLAYER 0x00572270
// FUNCTION: STUDIO 0x006546b0
void ServerReplicator::onSentMarker(long id)
{
	createPhysicsSender(settings().getPhysicsSendMethod());
	if (streamJob)
	{
		streamJob->sendPackets(-1);
		streamJob->setReady(true);
	}
	waitingForMarker = false;
}

// FUNCTION: PLAYER 0x005722b0
// FUNCTION: STUDIO 0x006546f0
void ServerReplicator::installRemotePlayer(std::string preferedSpawnName)
{
	remotePlayer->m_unk0x100 = RakNetToRbxAddress(remotePlayerId);
	remotePlayer->setParent(players);
	FASTLOG(FLog::Network, "ServerReplicator:InstallRemotePlayer - LoadCharacter");
	if (players->characterAutoSpawn)
		remotePlayer->loadCharacter(true, preferedSpawnName);
}

// noinline: 2013's readItem calls it out of line.
// FUNCTION: PLAYER 0x00572390
// FUNCTION: STUDIO 0x006547d0
__declspec(noinline) void ServerReplicator::readClientQuotaUpdate(RakNet::BitStream& bitStream)
{
	int diff;
	short maxRegionRadius;
	RBX::operator>>(bitStream, diff);
	RBX::operator>>(bitStream, maxRegionRadius);

	if (streamJob)
		streamJob->updateClientQuota(diff, maxRegionRadius);
}

// FUNCTION: PLAYER 0x00572ec0
// FUNCTION: STUDIO 0x00654e50
void ServerReplicator::rebroadcastEvent(Reflection::EventInvocation& eventInvocation)
{
	eventInvocation.replicateEvent();
}

// STUB: PLAYER 0x00572ed0
__declspec(noinline) void ServerReplicator::readRequestCharacter(RakNet::BitStream& bitStream)
{
	STUB(0x00572ed0);
}

// FUNCTION: PLAYER 0x00573160
// FUNCTION: STUDIO 0x006550f0
void ServerReplicator::serializeSFFlags(RakNet::BitStream& outBitStream)
{
	outBitStream.Write(FLog::GetNumSynchronizedVariable());
	FLog::ForEachVariable(&serializeSFFlag, &outBitStream, FASTVARTYPE_SYNC);
}

// 97.6%, pairing only: ~NetworkFilter folds with another empty-body twin in our link (2013 0x00528720).
// STUB: PLAYER 0x00573350
// STUB: STUDIO 0x006551c0
void ServerReplicator::setBasicFilteringEnabled(bool value)
{
	basicFilter.reset(value ? new NetworkFilter(this) : NULL);
}

// 2016 inlines this template member in PropertySynchronization.h; 2013's copy sits in ServerReplicator.obj.
// STUB: PLAYER 0x00574f90
// STUB: STUDIO 0x006502d0
template<>
__declspec(noinline) void PropSync::detail::Base<PropSync::detail::MasterItem>::expireItems()
{
	STUB(0x00574f90);
	RBX::Time::now<RBX::Time::Fast>(); // stand-in cross-TU call: keeps dataOutStep's `this` in esi, as 2013's real body does
}

// Folded with ClientReplicator::dataOutStep in both targets.
// FUNCTION: PLAYER 0x00576990
// FUNCTION: STUDIO 0x00650b30
void ServerReplicator::dataOutStep()
{
	propSync.expireItems();
	Super::dataOutStep();
}

// STUB: PLAYER 0x00578a70
// STUB: STUDIO 0x00657d20
__declspec(noinline) void PropSync::Master::onPropertyChanged(Reflection::ConstProperty prop)
{
	STUB(0x00578a70);
	RBX::Time::now<RBX::Time::Fast>(); // stand-in cross-TU call (see expireItems)
}

// STUB: PLAYER 0x00578e70
// STUB: STUDIO 0x00658120
__declspec(noinline) FilterResult PropSync::Master::onReceivedPropertyChanged(Reflection::ConstProperty prop)
{
	STUB(0x00578e70);
	RBX::Time::now<RBX::Time::Fast>(); // stand-in cross-TU call (see expireItems)
	return Accept;
}

// STUB: PLAYER 0x00579410
// STUB: STUDIO 0x006586b0
FilterResult ServerReplicator::filterPhysics(PartInstance* instance)
{
	if (Super::filterPhysics(instance) == Reject)
		return Reject;
	return propSync.onReceivedPropertyChanged(Reflection::ConstProperty(PartInstance::prop_CFrame, instance));
}

// FUNCTION: PLAYER 0x00579460
// FUNCTION: STUDIO 0x00658700
void ServerReplicator::onPropertyChanged(Instance* instance, const Reflection::PropertyDescriptor* descriptor)
{
	Super::onPropertyChanged(instance, descriptor);
	propSync.onPropertyChanged(Reflection::ConstProperty(*descriptor, instance));
}

// STUB: PLAYER 0x005799f0
__declspec(noinline) void ServerReplicator::readPropAcknowledgement(RakNet::BitStream& bitStream)
{
	STUB(0x005799f0);
}

// FUNCTION: PLAYER 0x0057b210
// FUNCTION: STUDIO 0x00659ce0
void ServerReplicator::readItem(RakNet::BitStream& inBitstream, Item::ItemType itemType)
{
	switch (itemType)
	{
	default:
		Super::readItem(inBitstream, itemType);
		break;
	case Item::ItemTypeUpdateClientQuota:
		readClientQuotaUpdate(inBitstream);
		break;
	case Item::ItemTypeRegionRemoval:
		streamJob->readRegionRemoval(inBitstream);
		break;
	case Item::ItemTypeInstanceRemoval:
		streamJob->readInstanceRemoval(inBitstream);
		break;
	case Item::ItemTypeRequestCharacter:
		readRequestCharacter(inBitstream);
		break;
	case Item::ItemTypePropAcknowledgement:
		readPropAcknowledgement(inBitstream);
		break;
	case Item::ItemTypeRocky:
		StandardOut::singleton()->printf(MESSAGE_INFO, "Rocky item found");
		throw std::runtime_error("rocky");
	}
}

} // namespace Network
} // namespace RBX

#pragma once

// Network/ServerReplicator.h -- October 2013, partial (RTTI DescribedNonCreatable<ServerReplicator,
// Replicator, sServerReplicator>, vftable PLAYER 0x00b1848c / STUDIO 0x00bd7404, classDescriptor PLAYER
// 0x00525ea0). It overrides 34 slots of the Debugable-primary vftable; the ones declared below are
// written. Members start at sizeof(Replicator) (PLAYER 0x2aa8, STUDIO 0x2328) with the same layout in
// both targets (STUDIO offsets are PLAYER's - 0x780). Offsets from the ctor PLAYER 0x0057c3b0 (STUDIO
// 0x0065ad00); names are 2016's where a written function proves the use.

#include "Replicator.h"
#include "PropertySynchronization.h"

namespace RBX {
namespace Network {

extern const char* const sServerReplicator;

class ServerReplicator : public DescribedNonCreatable<ServerReplicator, Replicator, sServerReplicator, Reflection::ClassDescriptor::INTERNAL_LOCAL>
{
	typedef Replicator Super;

	PropSync::Master propSync;                // +0x00 (Base ctor 0x005becb0)
	undefined4 m_unk0x2af8;                   // +0x50 ctor 0
	bool acceptsTerrainChanges;               // +0x54 ctor true; receiveCluster 0x005718b0
	// +0x58..+0x64: the ctor stores the Lighting service found through the Server (helper 0x0057be00,
	// adjusted to its DescribedBase) and three of its property descriptors (trie lookups); the
	// canReplicateProperty log texts name them GlobalShadows, OutdoorAmbient and Outlines.
	const Reflection::DescribedBase* m_unk0x2b00;           // +0x58
	const Reflection::PropertyDescriptor* m_unk0x2b04;      // +0x5c GlobalShadows
	const Reflection::PropertyDescriptor* m_unk0x2b08;      // +0x60 OutdoorAmbient
	const Reflection::PropertyDescriptor* m_unk0x2b0c;      // +0x64 Outlines
	std::string initialSpawnName;             // +0x68 OnReceive 0x005719b0 (log text "initialSpawnName: %s")
	undefined4 m_unk0x2b2c;                   // +0x84 ctor 0
	undefined4 m_unk0x2b30;                   // +0x88 ctor 0x005733e0
	shared_ptr<Player> remotePlayer;          // +0x8c (2016 name) installRemotePlayer 0x005722b0; find/getRemotePlayer 0x00520c90
	int remoteProtocolVersion;                // +0x94 ctor 0x12; canUseProtocolVersion 0x00571870
	undefined4 m_unk0x2b40;                   // +0x98
	bool waitingForMarker;                    // +0x9c ctor true; sendItemsPacket 0x00571a90, onSentMarker clears it

protected:
	/*override*/ bool canUseProtocolVersion(int protocolVersion) const;                 // vtable+0x9c
	/*override*/ bool sendItemsPacket();                                                // vtable+0xf8
	/*override*/ void receiveCluster(RakNet::BitStream& inBitstream, Instance* instance); // vtable+0x118
	/*override*/ void serializeSFFlags(RakNet::BitStream& outBitStream);              // vtable+0x120
	/*override*/ bool canReplicateProperty(const Reflection::ConstProperty& prop);    // vtable+0xf4
	/*override*/ void rebroadcastEvent(Reflection::EventInvocation& eventInvocation);   // vtable+0xc0
	/*override*/ FilterResult filterPhysics(PartInstance* instance);                  // vtable+0xd8
	/*override*/ void onPropertyChanged(Instance* instance, const Reflection::PropertyDescriptor* descriptor); // vtable+0xe0
	/*override*/ void dataOutStep();                                                  // vtable+0x108

	/*override*/ void readItem(RakNet::BitStream& inBitstream, Item::ItemType itemType); // vtable+0x100

	// readItem's callees (PLAYER 0x00572390 2016 name; 0x00572ed0, 0x005799f0 iOS names), not written yet.
	void readClientQuotaUpdate(RakNet::BitStream& bitStream);
	void readRequestCharacter(RakNet::BitStream& bitStream);
	void readPropAcknowledgement(RakNet::BitStream& bitStream);

	// The one virtual ServerReplicator adds (vtable+0x12c); 2013 takes the name by value (ret 0x1c).
	virtual void installRemotePlayer(std::string preferedSpawnName);

	// RakNet::PluginInterface2 (this = ServerReplicator+0x1094)
	/*override*/ RakNet::PluginReceiveResult OnReceive(RakNet::Packet* packet);
};

} // namespace Network
} // namespace RBX

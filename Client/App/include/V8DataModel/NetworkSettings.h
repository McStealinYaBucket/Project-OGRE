/* V8DataModel/NetworkSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<NetworkSettings, GlobalAdvancedSettings::Item> 0x8703B2 (Item 0x86DF76, Creator 0x8396A0; Item template TODO);
     sNetworkSettings = "NetworkSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NetworkSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"
#include "decomp.h"

namespace RBX {

	extern const char* const sNetworkSettings;

	// 2013 base: GlobalAdvancedSettingsItem<NetworkSettings, sNetworkSettings> (ctor PLAYER 0x005b7790).
	// Members from the 2013 ctor 0x005b7cf0 (instance name "Network"); size 0x120 (Creator::create
	// 0x00521460). Types follow the stores (fstp qword = double, movss = float); names unknown.
	// SIZE 0x120
	class NetworkSettings
		: public GlobalAdvancedSettingsItem<NetworkSettings, sNetworkSettings>
	{
	public:
		enum PhysicsReceiveMethod
		{
			Direct = 0,
			Interpolation = 1,
		};

		enum PhysicsSendMethod
		{
			ErrorComputation = 0,
			ErrorComputation2 = 1,
			RoundRobin = 2,
			TopNErrors = 3,
		};

		NetworkSettings();

		// Replicator::OnInternalPacket 0x00581b50 reads 0x64 (2016 name).
		bool printSplitMessages;		// 0x64  false (one byte store each in the ctor)
	private:
		bool m_unk0x65;					// 0x65  false (one byte store each in the ctor)
		bool m_unk0x66;					// 0x66  false (one byte store each in the ctor)
		bool m_unk0x67;					// 0x67  false (one byte store each in the ctor)
		bool m_unk0x68;					// 0x68  false (one byte store each in the ctor)
		bool m_unk0x69;					// 0x69  false (one byte store each in the ctor)
		bool m_unk0x6a;					// 0x6a  false (one byte store each in the ctor)
		bool m_unk0x6b;					// 0x6b  false (one byte store each in the ctor)
		bool m_unk0x6c;					// 0x6c  false (one byte store each in the ctor)
		bool m_unk0x6d;					// 0x6d  false (one byte store each in the ctor)
		double m_unk0x70;				// 0x70  0
	public:
		bool isQueueErrorComputed;		// 0x78  true (src2016 name; PacketJob ctor 0x005c5610 copies it)
		// Server::onServiceProvider 0x005b5310 reads 0x79/0x7a/0xdc (names: 2016, same use).
		bool usePhysicsPacketCache;		// 0x79  false
		bool useInstancePacketCache;	// 0x7a  false
	private:
		bool m_unk0x7b;					// 0x7b  false
		bool m_unk0x7c;					// 0x7c  false
		bool m_unk0x7d;					// 0x7d  false
		std::string m_unk0x80;			// 0x80  "127.0.0.1"
		int m_unk0x9c;					// 0x9c  0x94eb (38123)
		std::string m_unk0xa0;			// 0xa0  ""
		double m_unk0xc0;				// 0xc0  0
		bool m_unk0xc8;					// 0xc8  true
		int m_unk0xcc;					// 0xcc  0
		int m_unk0xd0;					// 0xd0  1
		int m_unk0xd4;					// 0xd4  1
	public:
		// ItemSender ctor 0x005da3f0 reads 0xd8 (2016: PacketPriority dataSendPriority).
		int dataSendPriority;			// 0xd8  2
		bool distributedPhysicsEnabled;	// 0xdc  true
	private:
		int m_unk0xe0;					// 0xe0  0
		float m_unk0xe4;				// 0xe4  10.0f
		int m_unk0xe8;					// 0xe8  1
	public:
		int m_unk0xec;					// 0xec  -1 (ServerReplicator::sendItemsPacket reads it)
		float networkOwnerRate;			// 0xf0  10.0f (NetworkOwnerJob ctor 0x005f7370)
	private:
		bool m_unk0xf4;					// 0xf4  false
		bool m_unk0xf5;					// 0xf5  false
		undefined4 m_unk0xf8;			// 0xf8  not stored by the ctor
		int physicsMtuAdjust;			// 0xfc  -200 (assert Replicator.cpp:3424)
		int replicationMtuAdjust;		// 0x100 -200 (assert Replicator.cpp:3418)
		float m_unk0x104;				// 0x104 30.0f
		float m_unk0x108;				// 0x108 (0x00b1ec54)
		float m_unk0x10c;				// 0x10c (0x00b1ec54)
		double receiveRate;				// 0x110 (0x00b1ec60)
		int m_unk0x118;					// 0x118 0

	public:
		// PacketReceiveJob ctor 0x005b92c0: fld qword [eax+0x110].
		double getReceiveRate() const { return receiveRate; }
		// Replicator::getPhysicsMtuSize 0x00581ca0 / getAdjustedMtuSize 0x00581c10: cmp [eax+0xfc|0x100],0.
		int getPhysicsMtuAdjust() const { return physicsMtuAdjust; }
		int getReplicationMtuAdjust() const { return replicationMtuAdjust; }
		int getDataSendPriority() const { return dataSendPriority; }
	};

} // namespace RBX

// The FactoryProduct base's out-of-line members; their names spell the 2013 base
// GlobalAdvancedSettings::Item (RTTI), so they pair through these markers.
// clang-format off
// TEMPLATE: PLAYER 0x00520f80
// TEMPLATE: STUDIO 0x004770d0
// ?getClassName@Creator@?$FactoryProduct@VNetworkSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sNetworkSettings@2@3QBDBVInstance@2@@RBX@@QBEABVName@3@XZ

// TEMPLATE: PLAYER 0x005b60a0
// TEMPLATE: STUDIO 0x00608300
// ?static_getCreator@?$FactoryProduct@VNetworkSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sNetworkSettings@2@3QBDBVInstance@2@@RBX@@KAABVCreator@12@XZ

// TEMPLATE: PLAYER 0x005b64f0
// TEMPLATE: STUDIO 0x00608750
// ?getClassName@?$FactoryProduct@VNetworkSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sNetworkSettings@2@3QBDBVInstance@2@@RBX@@UBEABVName@2@XZ
// clang-format on

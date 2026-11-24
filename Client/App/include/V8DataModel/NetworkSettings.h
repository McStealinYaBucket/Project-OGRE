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

	private:
		bool m_unk0x64[0x6e - 0x64];	// 0x64  all false
		double m_unk0x70;				// 0x70  0
		bool m_unk0x78;					// 0x78  true
		bool m_unk0x79[0x7e - 0x79];	// 0x79  all false
		std::string m_unk0x80;			// 0x80  "127.0.0.1"
		int m_unk0x9c;					// 0x9c  0x94eb (38123)
		std::string m_unk0xa0;			// 0xa0  ""
		double m_unk0xc0;				// 0xc0  0
		bool m_unk0xc8;					// 0xc8  true
		int m_unk0xcc;					// 0xcc  0
		int m_unk0xd0;					// 0xd0  1
		int m_unk0xd4;					// 0xd4  1
		int m_unk0xd8;					// 0xd8  2
		bool m_unk0xdc;					// 0xdc  true
		int m_unk0xe0;					// 0xe0  0
		float m_unk0xe4;				// 0xe4  10.0f
		int m_unk0xe8;					// 0xe8  1
		int m_unk0xec;					// 0xec  -1
		float m_unk0xf0;				// 0xf0  10.0f
		bool m_unk0xf4;					// 0xf4  false
		bool m_unk0xf5;					// 0xf5  false
		undefined4 m_unk0xf8;			// 0xf8  not stored by the ctor
		int m_unk0xfc;					// 0xfc  -200
		int m_unk0x100;					// 0x100 -200
		float m_unk0x104;				// 0x104 30.0f
		float m_unk0x108;				// 0x108 (0x00b1ec54)
		float m_unk0x10c;				// 0x10c (0x00b1ec54)
		double m_unk0x110;				// 0x110 (0x00b1ec60)
		int m_unk0x118;					// 0x118 0
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

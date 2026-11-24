/* Network/Player.h — October 2013.
   Layout from the 2013 constructor PLAYER 0x00550d40 (operator new(0x1d8) in 0x00421940) and
   destructor 0x00553fa0, both not yet matched, so member types are the 2013 evidence named beside
   each member and the names come from the reflection descriptors that push the member's offset
   (dynamic initializers 0x00a5f720..0x00a60960) or stay placeholders:
     0xcc "SimulationRadius", 0xd0 "MaximumSimulationRadius" (BoundProp<float> ctor 0x005409d0),
     0xd4 "OsPlatform" (BoundProp<std::string> ctor 0x00540c50), 0x148 "ScriptSecurityError",
     0x150 "RemoteInsert", 0x160 "RemoteFriendServiceSignal", 0x170 "StatsAvailable",
     0x17c "Chatted", 0x180 "CharacterAdded", 0x184 "CharacterRemoving", 0x18c "Idled",
     0x190 "FriendStatusChanged". Signal signatures: the signal ctor/dtor names the ctor and dtor
     call (rtti/match rows), and iOS 10-29's EventDescImpl<…, Player, Sig> instantiations.
   Base: DescribedCreatable<Player, Instance, sPlayer> (RTTI says functionality 19, see below);
   Instance is 0x60 (Diagnostics::Countable<Player> sits at 0x60 as an empty base). */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/BrickColor.h"
#include "rbx/rbxtime.h"
#include "decomp.h"
#include <boost/function.hpp>
#include <list>

namespace RBX {
	class ModelInstance;

namespace Network {

	extern const char* const sPlayer;

	// SIZE 0x1d8
	class Player
		// 2013 RTTI says functionality 19 (RUNTIME: DescribedCreatable<...$0BD@...>). Switching to it
		// renames the ??_E/??_G template bodies the CSVs pair by name, and FactoryProduct<Player>'s
		// vftable and ??_G 0x00529050 leave 100% (measured 2026-11-13); kept at the default until
		// the CSVs are regenerated with it.
		: public DescribedCreatable<Player, Instance, sPlayer>
	{
	public:
		enum ChatMode
		{
			CHAT_MODE_MENU = 0,
			CHAT_MODE_TEXT_AND_MENU = 1,
		};

		enum MembershipType
		{
			MEMBERSHIP_NONE = 0,
			MEMBERSHIP_BUILDERS_CLUB = 1,
			MEMBERSHIP_TURBO_BUILDERS_CLUB = 2,
			MEMBERSHIP_OUTRAGEOUS_BUILDERS_CLUB = 3,
		};

		Player() {}

		shared_ptr<Instance> character;     // 0x60  dtor releases +0x64; PlayerChatLine and
			// Teams::updateTeamColor read it. 2013 type unproven (setCharacter takes ModelInstance*).
		BrickColor teamColor;               // 0x68  ctor 1; Teams::getTeamFromPlayer 0x007a4770
		bool neutral;                       // 0x6c  ctor true; getTeamFromPlayer 0x007a4774
		undefined4 m_unk0x70;               // 0x70  ctor 0
		int dataComplexityLimit;            // 0x74  ctor 45000; getDataComplexityLimit 0x00527390
		bool m_unk0x78;                     // 0x78  ctor false
		rbx::signals::scoped_connection m_unk0x7c; // 0x7c  dtor 0x00483de0
		rbx::signals::scoped_connection m_unk0x80; // 0x80  dtor 0x00483de0
		rbx::signals::scoped_connection m_unk0x84; // 0x84  dtor 0x00483de0
		boost::function<void()> teamStatusChangedCallback; // 0x88  src2016:, confirmed by setTeamColor 0x00535090
			// (`if (f) f()` through function0<void>::operator() 0x00434c20)
		std::string characterAppearance;    // 0xa8  getCharacterAppearance 0x005273e0
		bool canLoadCharacterAppearance;    // 0xc4  ctor true; get 0x005273a0, set 0x00527790
		bool superSafeChat;                 // 0xc5  ctor false; setSuperSafeChat 0x00527620
		bool under13;                       // 0xc6  ctor true; getUnder13 0x00527410, setUnder13 0x00527690
		int userId;                         // 0xc8  ctor 0; getUserID 0x00527420 ("userId" descriptor)
		float simulationRadius;             // 0xcc  ctor 10.0f; "SimulationRadius" pushes 0xcc
		float maxSimulationRadius;          // 0xd0  ctor 1000.0f; "MaximumSimulationRadius" pushes 0xd0
		std::string osPlatform;             // 0xd4  "OsPlatform" pushes 0xd4
		bool hasGroupBuildTools;            // 0xf0  ctor false; setHasGroupBuildTools 0x00527760
		int personalServerRank;             // 0xf4  ctor 0; getPersonalServerRank 0x005273b0
		undefined4 m_unk0xf8;               // 0xf8  ctor 0
		bool appearanceDidLoad;             // 0xfc  ctor false; getAppearanceDidLoad 0x00527460
		undefined4 m_unk0x100;              // 0x100 ctor -1
		undefined2 m_unk0x104;              // 0x104 ctor 0xffff
		undefined2 m_unk0x106;              // 0x106 not stored by the ctor
		bool forceEarlySpawnLocationCalculation; // 0x108 ctor false; src2016:, confirmed by 0x00527890/0x005278a0
		bool m_unk0x109;                    // 0x109 ctor false
		std::string m_unk0x10c;             // 0x10c
		undefined4 m_unk0x128;              // 0x128 ctor 0; the dtor operator-deletes it (scoped_ptr of a trivial type)
		shared_ptr<Instance> m_unk0x12c;    // 0x12c dtor releases +0x130; pointee type unproven
		std::list<boost::function<void()> > m_unk0x134; // 0x134 _Buynode 0x00694940, clear 0x005352c0
		Time m_unk0x140;                    // 0x140 ctor Time::now<0> 0x009bb680
		rbx::remote_signal<void(std::string, std::string, std::string)> scriptSecurityErrorSignal; // 0x148
		rbx::remote_signal<void(std::string, G3D::Vector3)> remoteInsertSignal;  // 0x150
		rbx::remote_signal<void()> m_unk0x158;                                    // 0x158
		rbx::remote_signal<void(bool, int)> remoteFriendServiceSignal;            // 0x160
		rbx::remote_signal<void()> m_unk0x168;                                    // 0x168
		rbx::remote_signal<void(std::string)> statsAvailableSignal;               // 0x170
		rbx::signals::scoped_connection m_unk0x178;                               // 0x178
		rbx::signals::signal<void(std::string, shared_ptr<Instance>)> chattedSignal;  // 0x17c ctor 0x0052e7a0
		rbx::signals::signal<void(shared_ptr<Instance>)> characterAddedSignal;        // 0x180 ctor 0x005257e0
		rbx::signals::signal<void(shared_ptr<Instance>)> characterRemovingSignal;     // 0x184 ctor 0x005257e0
		rbx::signals::signal<void(G3D::Vector3)> m_unk0x188;                          // 0x188 ctor 0x0052e800
		rbx::signals::signal<void(double)> idledSignal;                               // 0x18c ctor 0x0052e6c0
		// 0x190: iOS says signal<void(shared_ptr<Instance>, FriendService::FriendStatus)>. Spelled with int
		// here: including V8DataModel/FriendService.h from Network TUs makes them supply
		// FactoryProduct<FriendService>::Creator with Network's include\Util/Object.h spelling, and
		// PLAYER 0x004200e0/0x00422ee0 (include\util/object.h) drop to 92% (measured 2026-11-24).
		rbx::signals::signal<void(shared_ptr<Instance>, int)> friendStatusChangedSignal; // 0x190
		double m_unk0x198;                  // 0x198 ctor fldz/fstp qword (8-aligned)
		MembershipType membershipType;      // 0x1a0 ctor 0; get 0x00527440, set 0x00527800
		int accountAge;                     // 0x1a4 ctor 0; setAccountAge 0x00527830
		bool m_unk0x1a8;                    // 0x1a8 ctor false
		undefined4 m_unk0x1ac;              // 0x1ac ctor 0
		std::string m_unk0x1b0;             // 0x1b0 ctor ""
		shared_ptr<Instance> m_unk0x1cc;    // 0x1cc dtor releases +0x1d0 first; pointee type unproven
		undefined4 m_unk0x1d4;              // 0x1d4 ctor 0

		static Reflection::BoundProp<float> prop_SimulationRadius;    // PLAYER 0x00dd9b38 (getset 0x00dd9b60)
		static Reflection::BoundProp<float> prop_MaxSimulationRadius; // PLAYER 0x00dd8e18 (getset 0x00dd8e40)

		// FUNCTION: PLAYER 0x005273c0
		// FUNCTION: STUDIO 0x005b7a80
		BrickColor getTeamColor() const { return teamColor; }

		// FUNCTION: PLAYER 0x005273d0
		// FUNCTION: STUDIO 0x005b7a90
		bool getNeutral() const { return neutral; }

		int getDataComplexityLimit() const;
		bool getCanLoadCharacterAppearance() const;
		void setCanLoadCharacterAppearance(bool value);
		int getPersonalServerRank() const;
		std::string getCharacterAppearance() const;
		bool getUnder13();
		void setUnder13(bool value);
		int getUserID() const;
		bool isGuest() const;
		MembershipType getMembershipType() const;
		void setMembershipType(MembershipType value);
		int getAccountAge() const { return accountAge; }
		void setAccountAge(int value);
		bool getAppearanceDidLoad() const;
		bool getHasGroupBuildTools() const { return hasGroupBuildTools; }
		void setHasGroupBuildTools(bool value);

		/*override*/ virtual void setName(const std::string& value); // vtable+0x14

		void setForceEarlySpawnLocationCalculation();
		bool calculatesSpawnLocationEarly() const;
		void setTeamColor(BrickColor color);
		void setNeutral(bool neutral);

		void setCharacter(ModelInstance* value);
		void removeCharacter();

		void setSimulationRadius(float value);
		void setMaxSimulationRadius(float value);
	};

} // namespace Network
} // namespace RBX

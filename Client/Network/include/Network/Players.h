/* Network/Players.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<Network::Players> from DataModel::
     initializeContents 0x2D3336 (tail); "Players" Lua name string in both 2013 PEs
     (doCloseDataModel finds Players and removes its children).
   Service base inferred (parents to the DataModel via the ServiceProvider path).
   TODO: full decompile (player list/replication machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/Players.h
   (exact 2013 path TODO). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Network/Player.h"
#include "Util/Guid.h"
#include "Util/utilities.h"
#include "rbx/Thread.hpp"
#include "decomp.h"
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <list>
#include <map>
#include <set>
#include <string>

namespace RakNet {
	struct SystemAddress;
	class BitStream;
}

namespace RBX {
namespace Network {

	class ConcurrentRakPeer;

	// iOS 10-29 names the type (signal<void(AbuseReport)>, AbuseReport::addMessage); 2013 passes it by
	// value as 0x30 bytes (OnReceiveReportAbuse 0x005678ae), not 2016's 0x50. Members not needed yet.
	struct AbuseReport;

	// Network::ChatMessage -- VALUE-type chat packet
	// (std::list<ChatMessage> storage: _M_create_node 0x8AD55A /
	// _M_erase 0x8AD4CA).  Layout pinned by the 3 C2s (0x89BBD2 /
	// mac: 0x89BC8E / 0x89BD10):
	//   +0 Guid (ctor tail: Guid::generateStandardGUID) | +4 std::string
	//   text | +8 ChatType | +12 shared_ptr<Player> (px; rc@+16) | +20
	//   shared_ptr<Player> (0 in the 3-arg ctors).
	// GCC offsets; MSVC layout DELTA -- member ORDER is the invariant.
	class ChatMessage
	{
	public:
		// DELTA: enumerator NAMES unresolved in dbq24 (no enum-debug
		// surface); behaviors pinned by isVisibleToPlayer 0x89BDD6:
		// 0/3 always visible; 1 = team match with both +104 neutral bytes
		// clear; 2 = sender==player || receiver==player.
		enum ChatType
		{
			CHATMESSAGE_ALL_0 = 0,   // isVisibleToPlayer: always true
			CHATMESSAGE_TEAM = 1,    // team match, both non-neutral
			CHATMESSAGE_WHISPER = 2, // sender or receiver match
			CHATMESSAGE_ALL_3 = 3    // isVisibleToPlayer: always true
		};

		// +0 is a plain std::string carrying the "{uuid}" guid TEXT:
		// raw ctor asm writes _S_empty_rep_storage+12 (a COW string slot)
		// and the tails call Guid::generateStandardGUID(std::string&)
		// with this = the +0 slot (0x89BD85 for the 4-arg ctor; C2b
		// mac: 0x89BC8E opens with std::string::string(+0, other+0)).  The
		// landed Guid class ({Scope, index}) is a different carrier.
		std::string guid;             // +0
		std::string message;          // +4
		ChatType chatType;            // +8
		shared_ptr<Player> player;    // +12 (sender; px/rc)
		shared_ptr<Player> receiver;  // +20 (whisper receiver)

		ChatMessage(const char* message, ChatType type, const shared_ptr<Player>& player);			// 0x89BBD2
		ChatMessage(const ChatMessage& other, const std::string& message);							// 0x89BC8E
		ChatMessage(const char* message, ChatType type, const shared_ptr<Player>& player,
			const shared_ptr<Player>& receiver);													// 0x89BD10
		bool isVisibleToPlayer(const shared_ptr<Player>& viewer) const;								// 0x89BDD6
		std::string getReportAbuseMessage() const;													// 0x89BE50
	};

	// Implicit destructor PLAYER 0x0055b8b0 (requestProcessor at +8 first, then _data's count at +4).
	// iOS: AbuseReporter(std::string), AbuseReporter::data, processRequests, add.
	class AbuseReporter
	{
		struct data;
		shared_ptr<data> _data;                          // 0x0
		boost::scoped_ptr<worker_thread> requestProcessor; // 0x8
	public:
		AbuseReporter(std::string abuseUrl);
	};

	extern const char* const sPlayers;

	// SIZE 0x1f8
	class Players
		: public DescribedNonCreatable<Players, Instance, sPlayers> // 2013 RTTI: NonFactoryProduct, 27
		, public Service
	{
	public:
		enum PlayerChatType
		{
			PLAYER_CHAT_TYPE_ALL = 0,
			PLAYER_CHAT_TYPE_TEAM = 1,
			PLAYER_CHAT_TYPE_WHISPER = 2,
		};

		enum ChatOption
		{
			CLASSIC_CHAT = 0,
			BUBBLE_CHAT = 1,
			CLASSIC_AND_BUBBLE_CHAT = 2,
		};

		// PLAYER 0x00570980 / 0x005705c0 (operator new(0x1f8) in the creator 0x0058eb00).
		Players();
		~Players();

		// mac: 0x2E32CC caller: direct-child count (binary reads the children
		// cow at Players+180).
		int getNumPlayers() const;

		// Server-gate (0x89B206; 2016 static shape carried):
		// asserts a provider exists when testInDatamodel, returns true when
		// no Client is present (server context).
		static bool backendProcessing(const Instance* context, bool testInDatamodel = true);

		// mac: 0x89B10E: delegates to Client::clientIsPresent.
		static bool clientIsPresent(const Instance* context, bool testInDatamodel = true);

		// mac: 0x89B16A: provider assert + !Server::serverIsPresent (Players.cpp:182).
		static bool frontendProcessing(const Instance* context, bool testInDatamodel = true);

		// mac: 0x89B130: delegates to Server::serverIsPresent.
		static bool serverIsPresent(const Instance* context, bool testInDatamodel = true);

		// 2016 name; PLAYER 0x0056b8c0: fastDynamicCast<Client>(instance) != NULL.
		static bool isNetworkClient(const Instance* instance);

		// PLAYER 0x00559790 (iOS name); inline in ~Players.
		void setConnection(ConcurrentRakPeer* peer);

		// Chat era (0x898962): send a chat message.  Stubbed (network
		// unported) -- body is empty; the real body uses RakNet::BitStream,
		// getGuidRegistry, ConcurrentRakPeer::Send, ChatMessage.
		void chat(std::string message);

		// mac: 0x898962 helper: resolve a player from a character instance.
		// Stubbed (network unported) -- returns 0.
		static int playerFromCharacter(const shared_ptr<Instance>& character);

		// Sound era (0x244B9A): local-player detection stub.
		static const Instance* findConstLocalPlayer(const Instance* context);
		static Player* findLocalPlayer(const Instance* context);

		std::string getLoadDataUrl(int userId) const;
		std::string getSaveDataUrl(int userId) const;
		std::string getSaveLeaderboardDataUrl(int userId) const;

	protected:
		// Instance overrides, vftable 0x00b17034 (STUDIO 0x00bca804).
		/*override*/ bool askAddChild(const Instance* instance) const;                          // +0x3c
		/*override*/ void onDescendantRemoving(const shared_ptr<Instance>& instance);            // +0x54
		/*override*/ void onChildAdded(Instance* child);                                         // +0x58
		/*override*/ void onChildRemoving(Instance* child);                                      // +0x5c
		/*override*/ void onChildChanged(Instance* instance, const PropertyChanged& event);      // +0x64

	public:

		// 2013 layout from the constructor PLAYER 0x00570980 (operator new(0x1f8) in 0x0058eb00) and
		// destructor 0x005705c0, both not yet matched. Instance is 0x60; Service's bool sits at 0x60.
		// Event names from the descriptors that push the offset (initializers 0x00a60e6a..0x00a612cb).
		std::string saveDataUrl;                     // 0x64 getSaveDataUrl 0x00559450
		std::string loadDataUrl;                     // 0x80 getLoadDataUrl 0x005593b0
		std::string saveLeaderboardDataUrl;          // 0x9c getSaveLeaderboardDataUrl 0x005594e0
		// 0xb8: the inline table ctor (prime-list _Lower_bound for 11 buckets, mlf 1.0f) and delete_nodes
		// 0x0055e890 in ~Players. Name from 2016.
		boost::unordered_set<std::string> leaderboardKeys; // 0xb8
		std::string m_unk0xd0;                       // 0xd0
		std::string m_unk0xec;                       // 0xec
		std::string m_unk0x108;                      // 0x108 ctor ""
		std::string m_unk0x124;                      // 0x124
		std::string m_unk0x140;                      // 0x140
		std::string m_unk0x15c;                      // 0x15c
		bool m_unk0x178;                             // 0x178 ctor false
		bool m_unk0x179;                             // 0x179 ctor true
		boost::scoped_ptr<AbuseReporter> abuseReporter; // 0x17c dtor: ~AbuseReporter 0x0055b8b0, operator delete
		shared_ptr<GuidItem<Instance>::Registry> m_unk0x180; // 0x180 dtor releases +0x184; Registry per the task note (IdSerializer 0x107c has the same type)
		std::list<ChatMessage> chatHistory;          // 0x188 node dtor ~ChatMessage 0x0055a560 (clear 0x0055f320)
		copy_on_write_ptr<Instances> m_unk0x194;     // 0x194 ctor 0x00567c30
		shared_ptr<Player> localPlayer;              // 0x19c ChatOutput reads it (mac +188)
		ConcurrentRakPeer* rakPeer;                  // 0x1a4 ctor 0; setConnection 0x00559790; ~Players clears it
		shared_ptr<Instance> m_unk0x1a8;             // 0x1a8 dtor releases +0x1ac; pointee unproven
		undefined4 m_unk0x1b0;                       // 0x1b0 ctor 12
		undefined4 m_unk0x1b4;                       // 0x1b4 ctor 0
		undefined4 m_unk0x1b8;                       // 0x1b8 ctor 0
		undefined4 m_unk0x1bc;                       // 0x1bc ctor 0
		// 0x1c0 "FriendRequestEvent": iOS says (..., FriendService::FriendEventType); int here for the
		// include reason given at Player::friendStatusChangedSignal.
		rbx::signal<void(shared_ptr<Instance>, shared_ptr<Instance>, int)> friendRequestEventSignal; // 0x1c0
		rbx::signal<void(shared_ptr<Instance>)> playerAddedEarlySignal;    // 0x1c4 "PlayerAddedEarly"
		rbx::signal<void(shared_ptr<Instance>)> playerAddedSignal;         // 0x1c8 "PlayerAdded"
		rbx::signal<void(shared_ptr<Instance>)> playerRemovingSignal;      // 0x1cc "PlayerRemoving"
		rbx::signal<void(shared_ptr<Instance>)> playerRemovingLateSignal;  // 0x1d0 "PlayerRemovingLate"
		rbx::signal<void(bool)> m_unk0x1d4;                                // 0x1d4 ctor 0x0055ca50
		// ChatOutput::onServiceProvider PLAYER 0x00973f9e connects here (mac +0xf8). The mac build
		// mangles it signal<void(ChatMessage const&)>; the name is ours.
		rbx::signal<void(const ChatMessage&)> messageReceivedSignal;      // 0x1d8
		// OnReceiveReportAbuse 0x00567710 fires this one with a by-value AbuseReport (iOS signal type).
		rbx::signal<void(AbuseReport)> abuseReportedReceived;             // 0x1dc
		// Fired by 0x00566710 under a DataModel::LegacyLock (iOS signal type).
		rbx::signal<void(const RakNet::SystemAddress&, const shared_ptr<RakNet::BitStream>&, const std::string&, const std::string&)> sendFilteredChatMessageSignal; // 0x1e0
		rbx::signal<void(PlayerChatType, shared_ptr<Instance>, std::string, shared_ptr<Instance>)> playerChattedSignal; // 0x1e4 "PlayerChatted"
		rbx::signal<void(std::string)> gameAnnounceSignal;                 // 0x1e8 "GameAnnounce"
		// 0x1ec: ~Players' inline map dtor (erase 0x0056f800, _Erase 0x0056d7f0) destroys a std::set<std::string>
		// at node+0x10 after a 4-byte key; key type and name from 2016.
		std::map<int, std::set<std::string> > cheatingPlayers; // 0x1ec
	};

} // namespace Network
} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x0056a5c0
// TEMPLATE: STUDIO 0x0080f810
// ??1?$storage4@U?$arg@$00@boost@@V?$value@PAVInstance@RBX@@@_bi@2@V?$value@W4RequestResult@AsyncHttpQueue@RBX@@@42@V?$value@V?$shared_ptr@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@boost@@@42@@_bi@boost@@QAE@XZ


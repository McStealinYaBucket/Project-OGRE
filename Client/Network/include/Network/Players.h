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
#include "decomp.h"
#include <list>
#include <map>
#include <string>

namespace RBX {
namespace Network {

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
		// 0xb8: boost::unordered_set<std::string> (delete_nodes 0x0055e890; ctor bucket count from the
		// prime list, mlf 1.0f). Placeholder bytes for now: declaring it makes the first Network TU
		// (API.obj) emit prime_list_template<unsigned>::value with a 1.0f constant right after it, and
		// every unordered ctor that pushes value+end then reads as "push 1.0" (PLAYER DataModel ctor
		// 0x0064ec50, StarterGuiService 0x0070c150, ContactManager 0x007b63a0, ... dropped, measured
		// 2026-11-24). Restore the type with Players' ctor, when the link order supplies it as 2013 did.
		undefined m_unk0xb8[0xd0 - 0xb8];            // 0xb8
		std::string m_unk0xd0;                       // 0xd0
		std::string m_unk0xec;                       // 0xec
		std::string m_unk0x108;                      // 0x108 ctor ""
		std::string m_unk0x124;                      // 0x124
		std::string m_unk0x140;                      // 0x140
		std::string m_unk0x15c;                      // 0x15c
		bool m_unk0x178;                             // 0x178 ctor false
		bool m_unk0x179;                             // 0x179 ctor true
		undefined4 m_unk0x17c;                       // 0x17c dtor: ~T 0x0055b8b0 then operator delete (scoped_ptr)
		shared_ptr<Instance> m_unk0x180;             // 0x180 dtor releases +0x184; pointee unproven
		std::list<shared_ptr<Instance> > m_unk0x188; // 0x188 element type unproven (_Buynode 0x0079f910 is folded)
		copy_on_write_ptr<Instances> m_unk0x194;     // 0x194 ctor 0x00567c30
		shared_ptr<Player> localPlayer;              // 0x19c ChatOutput reads it (mac +188)
		undefined4 m_unk0x1a4;                       // 0x1a4 ctor 0; the dtor clears it first
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
		rbx::signal<void(const ChatMessage&)> m_unk0x1dc;                  // 0x1dc signature unproven
		rbx::signal<void(const ChatMessage&)> m_unk0x1e0;                  // 0x1e0 signature unproven
		rbx::signal<void(PlayerChatType, shared_ptr<Instance>, std::string, shared_ptr<Instance>)> playerChattedSignal; // 0x1e4 "PlayerChatted"
		rbx::signal<void(std::string)> gameAnnounceSignal;                 // 0x1e8 "GameAnnounce"
		std::map<const void*, int> m_unk0x1ec;      // 0x1ec a _Tree with 16-byte values; key/value unproven
	};

} // namespace Network
} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x0056a5c0
// TEMPLATE: STUDIO 0x0080f810
// ??1?$storage4@U?$arg@$00@boost@@V?$value@PAVInstance@RBX@@@_bi@2@V?$value@W4RequestResult@AsyncHttpQueue@RBX@@@42@V?$value@V?$shared_ptr@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@boost@@@42@@_bi@boost@@QAE@XZ


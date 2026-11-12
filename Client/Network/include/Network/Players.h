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

class Players
		: public DescribedCreatable<Players, Instance, sPlayers>
		, public Service
	{
	public:
		Players() {}

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

		// Chat era (0x898962): send a chat message.  Stubbed (network
		// unported) -- body is empty; the real body uses RakNet::BitStream,
		// getGuidRegistry, ConcurrentRakPeer::Send, ChatMessage.
		void chat(std::string message);

		// mac: 0x898962 helper: resolve a player from a character instance.
		// Stubbed (network unported) -- returns 0.
		static int playerFromCharacter(const shared_ptr<Instance>& character);

		// Sound era (0x244B9A): local-player detection stub.
		static const Instance* findConstLocalPlayer(const Instance* context);

		// Players+188 = the local player (px; rc@+192): the ChatOutput
		// own-line chains read *(players+188) -- onPlayerChatMessage
		// mac: 0x5C2B69 compares message.player.px == *(players+188);
		// onGameChatMessage 0x5C2DF1-2 compares origin.px against
		// *( *(players+188) + 92 ) = localPlayer->character.px (the
		// landed Player::character seat).  Landed name DELTA:
		// behavior-pinned (name unresolved in dbq24).
		shared_ptr<Player> localPlayer;        // +188

		// onServiceProvider 0x5C2866-0x5C2875 (ChatOutput): the mf1 bind
		// of ChatOutput::onPlayerChatMessage lands on this signal; its
		// storage sits at +248 (0xF8) in the mac build, after the +188
		// localPlayer.  DELTA: name ours -- the storage mangles as
		// signal<void ()(RBX::Network::ChatMessage const&)> (rarc28 raw
		// mac: 0x5C2875); no nameable accessor in dbq24.
		rbx::signal<void(const ChatMessage&)> messageReceivedSignal;
	};

} // namespace Network
} // namespace RBX

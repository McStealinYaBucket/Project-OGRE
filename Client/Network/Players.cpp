/* Network/Players.cpp -- October 2013, partial. Asserts sit on their 2013 lines (182, 189). */

#include "stdafx.h"
#include "Network/Players.h"
#include "Network/Client.h"
#include "Network/Server.h"
#include "V8DataModel/FriendService.h"
#include <sstream>
DECOMP_SIZE_ASSERT(RBX::Network::Players, 0x1f8);
namespace RBX {
namespace Network {
	const char* const sPlayers = "Players";
}
}
using namespace RBX;
using namespace RBX::Network;
REFLECTION_BEGIN();
REFLECTION_END();
#include "V8DataModel/Teams.h"
#include "V8DataModel/OverlayDataModel.h"
#include "FastLog.h" // Players is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
FASTFLAGVARIABLE(PreventRedundantRebuildLocalGui, true) // registration 0x00a60b00, byte 0x00d17b04 = 1
namespace RBX {
namespace Network {
	void Players::chat(std::string message)
	{
		(void)message;
		// DELTA: network unported (RakNet::BitStream, getGuidRegistry,
		// ConcurrentRakPeer::Send, ChatMessage).  Stub no-ops.
	}
	int Players::getNumPlayers() const
	{
		// OGRE vc9: was a C++11 lambda counting visitChildren calls. Mac 0x008a3c66 (37 bytes) calls
		// the copy_on_write_ptr at this+0xb4 and returns its vector's size (no null test), the 2016
		// form (int)players->size(). numChildren() is the closest existing helper; that +0xb4 is
		// Instance::children (Players.h says so) is not yet proven by a Windows ctor.
		return (int)numChildren();
	}
	// Players::clientIsPresent is in Client.cpp (2013: PLAYER 0x005adc70, after Client::findClient).
	bool Players::serverIsPresent(const Instance* context, bool testInDatamodel)
	{
		return Server::serverIsPresent(context, testInDatamodel);
	}
	int Players::playerFromCharacter(const shared_ptr<Instance>& character)
	{
		(void)character;
		return 0;
	}
	// Network::ChatMessage value-ctor family (mac: 0x89BBD2 / 0x89BC8E / 0x89BD10). Member order:
	// guid string (+0), text, ChatType, player, receiver; the tail Guid::generateStandardGUID writes
	// the "{uuid}" text into +0. The 3-arg ctor leaves receiver empty; the copy form copies every slot
	// from `other` and overwrites the text. mac: 0x89BD10 builds the text with
	// std::string(const char*, const allocator&) (the 3-arg ctor has the same shape).
	ChatMessage::ChatMessage(const char* message, ChatType type, const shared_ptr<Player>& player)
	{
		this->message = message;
		chatType = type;
		this->player = player;
		Guid::generateStandardGUID(guid);
	}
	ChatMessage::ChatMessage(const ChatMessage& other, const std::string& message)
	{
		guid = other.guid;
		this->message = message;
		chatType = other.chatType;
		player = other.player;
		receiver = other.receiver;
	}
	ChatMessage::ChatMessage(const char* message, ChatType type, const shared_ptr<Player>& player,
		const shared_ptr<Player>& receiver)
	{
		this->message = message;
		chatType = type;
		this->player = player;
		this->receiver = receiver;
		Guid::generateStandardGUID(guid);
	}
	// mac: 0x89BDD6 (104B; mangle
	// __ZNK3RBX7Network11ChatMessage17isVisibleToPlayerEN5boost10shared_ptrINS0_6PlayerEEE).
	// Switch on chatType (full 0..3 jmp table at 0x89BE79): 0/3 ->
	// true; 1 -> viewer px non-null && sender px non-null && both
	// neutral bytes (+104) clear && teamColor (+100) equal; 2 -> viewer
	// px non-null && (sender px == viewer || receiver px == viewer);
	// anything else -> false.
	bool ChatMessage::isVisibleToPlayer(const shared_ptr<Player>& viewer) const
	{
		switch (chatType)
		{
			case CHATMESSAGE_ALL_0:
			case CHATMESSAGE_ALL_3:
				return true;
			case CHATMESSAGE_TEAM:
			{
				Player* look = viewer.get();
				if (look != NULL && player.get() != NULL
					&& this->player->neutral == 0 && look->neutral == 0
					&& this->player->teamColor == look->teamColor)
					return true;
				return false;
			}
			case CHATMESSAGE_WHISPER:
			{
				Player* look = viewer.get();
				if (look == NULL)
					return false;
				if (this->player.get() == look)
					return true;
				return this->receiver.get() == look;
			}
			default:
				return false;
		}
	}
	// FUNCTION: PLAYER 0x005593b0
	// FUNCTION: STUDIO 0x005b7e70
	std::string Players::getLoadDataUrl(int userId) const
	{
		if (loadDataUrl.empty())
			throw std::runtime_error("No LoadData url set");
		return format(loadDataUrl.c_str(), userId);
	}
	// FUNCTION: PLAYER 0x00559450
	// FUNCTION: STUDIO 0x005b7f10
	std::string Players::getSaveDataUrl(int userId) const
	{
		if (saveDataUrl.empty())
			throw std::runtime_error("No SaveData url set");
		return format(saveDataUrl.c_str(), userId);
	}
	// FUNCTION: PLAYER 0x005594e0
	// FUNCTION: STUDIO 0x005b7fa0
	std::string Players::getSaveLeaderboardDataUrl(int userId) const
	{
		if (saveLeaderboardDataUrl.empty())
			throw std::runtime_error("No SaveLeaderboardData url set");
		return format(saveLeaderboardDataUrl.c_str(), userId);
	}

	// iOS: Players::setConnection(ConcurrentRakPeer*). 2013 keeps it out of line here and inlines it in
	// ~Players (`mov [esi+0x1a4], 0` before the member dtors). Server and Client call it from their
	// onServiceProvider overrides. The 2016 body is the same one store.
	// FUNCTION: PLAYER 0x00559790
	// FUNCTION: STUDIO 0x005b8250
	void Players::setConnection(ConcurrentRakPeer* peer)
	{
		rakPeer = peer;
	}
	// mac: 0x89BE50 (340B): switch on chatType (TEAM "[[team]]", WHISPER ostringstream "[[to " +
	// receiver name or "???" + "]]", GAME "[[game]]", default copy) + message.
	// The receiver name walks fw(receiver.px)+24, as Instance::getName() does.
	// STUB: PLAYER 0x0055b350
	// STUB: STUDIO 0x005ba800
	std::string ChatMessage::getReportAbuseMessage() const
	{
		switch (chatType)
		{
			case CHATMESSAGE_TEAM:
				return std::string("[[team]]") + message;
			case CHATMESSAGE_WHISPER:
			{
				std::ostringstream stream;
				stream << "[[to ";
				if (receiver != NULL)
					stream << receiver->getName();
				else
					stream << "???";
				stream << "]]";
				stream << message;
				return stream.str();
			}
			case CHATMESSAGE_ALL_3:
				return std::string("[[game]]") + message;
			default:
				return message;
		}
	}
	// FUNCTION: PLAYER 0x00568900
	// FUNCTION: STUDIO 0x005ccb00
	bool Players::frontendProcessing(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* serviceProvider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || serviceProvider!=NULL);
		return serviceProvider && !Server::serverIsPresent(context, testInDatamodel); }
	// FUNCTION: PLAYER 0x005689c0
	// FUNCTION: STUDIO 0x005ccbc0
	bool Players::backendProcessing(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* serviceProvider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || serviceProvider!=NULL);
		return serviceProvider && !Players::clientIsPresent(context, testInDatamodel);
	}

	// The parent lock is FWInstance's first byte (fw+0x18) through FWValue<bool>::set 0x0052c000, as in
	// DataModel's ctor; 2016 calls it lockParent().
	// FUNCTION: PLAYER 0x00569260
	// FUNCTION: STUDIO 0x005cd460
	void Players::onDescendantRemoving(const shared_ptr<Instance>& instance)
	{
		if (Players::backendProcessing(this, false))
			if (Player* player = Instance::fastDynamicCast<Player>(instance.get()))
				player->fw()->b21.set(true, player->fwRef());
		Instance::onDescendantRemoving(instance);
	}

	// FUNCTION: PLAYER 0x0056b8c0
	// FUNCTION: STUDIO 0x005d0d70
	bool Players::isNetworkClient(const Instance* instance)
	{
		return Instance::fastDynamicCast<Client>(instance) != NULL;
	}

	// Bound by onChildAdded to Player::remoteFriendServiceSignal. Calls, stack slots and branch order match
	// (reject falls through); 2013 keeps `this` in ebp and pushes ebp/esi/edi up front, ours shrink-wraps
	// ebp. Probed 7 shapes (early returns, a ternary either way, raw-pointer locals, a `self` copy): none moves it.
	// STUB: PLAYER 0x0056b960
	// STUB: STUDIO 0x005d0e10
	void Players::friendServiceRequest(bool makeFriends, weak_ptr<Player> sourcePlayer, int otherUserId)
	{
		shared_ptr<Player> source = sourcePlayer.lock();
		shared_ptr<Player> other = getPlayerByID(otherUserId);
		if (source && other)
			if (FriendService* friendService = ServiceProvider::find<FriendService>(this))
			{
				if (!makeFriends)
					friendService->rejectFriendRequestOrBreakFriendship(source->userId, other->userId);
				else
					friendService->issueFriendRequestOrMakeFriendship(source->userId, other->userId);
			}
	}

	// FUNCTION: PLAYER 0x0056baa0
	// FUNCTION: STUDIO 0x005d0f50
	Player* Players::findLocalPlayer(const Instance* context)
	{
		if (Players* players = ServiceProvider::find<Players>(context))
			return players->localPlayer.get();
		return NULL;
	}

	// No FirePlayerAddedAndPlayerRemovingOnClient flag and no Http::playerCount (2016).
	// STUB: PLAYER 0x0056bd20
	// STUB: STUDIO 0x005d11d0
	void Players::onChildRemoving(Instance* child)
	{
		if (Player* player = Instance::fastDynamicCast<Player>(child))
		{
			if (FriendService* friendService = ServiceProvider::find<FriendService>(this))
				friendService->playerRemoving(player->userId);
			{
				shared_ptr<Instances>& c = players.write();
				c->erase(std::find(c->begin(), c->end(), shared_from(child)));
			}
			if (Players::backendProcessing(this, false))
			{
				playerRemovingSignal(shared_from(player));
				playerRemovingLateSignal(shared_from(player));
			}
		}
	}

} // namespace Network
} // namespace RBX
// Included here, below the 2013 assert lines (182/189).
#include "V8DataModel/ScriptInformationProvider.h"
#include "V8DataModel/Humanoid.h"
namespace RBX {
namespace Network {
	// Bound by onChildAdded to Player::scriptSecurityErrorSignal. 2013 keeps only the service lookup (the
	// out-of-line create<ScriptInformationProvider>(const Instance*) 0x0056b280; iOS 10-29 the same).
	// FUNCTION: PLAYER 0x0056bf20
	// FUNCTION: STUDIO 0x005d13d0
	void Players::reportScriptSecurityError(int userId, std::string hash, std::string error, std::string stack)
	{
		ServiceProvider::create<ScriptInformationProvider>(this);
	}

	// Bound by onChildAdded to Player::remoteInsertSignal; not written yet.

	// STUB: PLAYER 0x0056bf90
	// STUB: STUDIO 0x005d1440
	void Players::remoteInsert(int userId, std::string url, G3D::Vector3 position)
	{
		STUB(0x0056bf90);
	}

	// FUNCTION: PLAYER 0x0056efb0
	const Instance* Players::findConstLocalCharacter(const Instance* context)
	{
		if (Players* players = ServiceProvider::find<Players>(context))
			if (Player* player = players->localPlayer.get())
				return player->character.get();
		return NULL;
	}

	// Bound by onChildAdded to Player::killPlayerSignal. No null tests past the player (2013 has none).
	// FUNCTION: PLAYER 0x0056efe0
	// FUNCTION: STUDIO 0x005d50e0
	void Players::killPlayer(int userId)
	{
		if (Player* player = Instance::fastDynamicCast<Player>(getPlayerByID(userId).get()))
			player->character->findFirstChildOfType<Humanoid>()->setHealth(0.0f);
	}

	// STUB: PLAYER 0x0056f8f0
	// STUB: STUDIO 0x005d59f0
	void Players::onRemoteSysStats(int userId, const std::string& resolution, const std::string& stats, bool valid)
	{
		STUB(0x0056f8f0);
	}

	// 2013 has no createPlayerGui call, no isCloudEdit test and no FirePlayerAddedAndPlayerRemovingOnClient
	// flag (2016); the rebuildGui tail under PreventRedundantRebuildLocalGui is 2013 only (iOS 10-29:
	// fastDynamicCast<OverlayDataModel>, called out of line at the shell copy 0x0044fdf0). Byte-identical to
	// 2013 in both targets (probe); STUB until App's Impersonator ctor/dtor markers (shell copies 0x0041cf40 /
	// 0x0041cf80) pair, which the call sites need.
	// STUB: PLAYER 0x0056ff40
	// STUB: STUDIO 0x005d6040
	void Players::onChildAdded(Instance* child)
	{
		if (Player* player = Instance::fastDynamicCast<Player>(child))
		{
			int userId = player->userId;
			if (userId > 0)
			{
				shared_ptr<Instance> otherPlayer = getPlayerByID(userId);
				if (otherPlayer)
					otherPlayer->setParent(NULL);
			}

			{
				shared_ptr<Instances>& c = players.write();
				c->push_back(shared_from(child));
			}

			if (FriendService* friendService = ServiceProvider::find<FriendService>(this))
				friendService->playerAdded(userId);

			if (Players::backendProcessing(this))
			{
				if (userId == 0 && player->getName() == "Player")
				{
					Security::Impersonator impersonate(Security::Replicator_);
					player->setName(format("Player%d", ++testPlayerNameId));
					player->setUserId(--testPlayerUserId);
				}

				player->scriptSecurityErrorSignal.connect(boost::bind(&Players::reportScriptSecurityError, this, userId, _1, _2, _3));
				player->remoteInsertSignal.connect(boost::bind(&Players::remoteInsert, this, userId, _1, _2));
				player->killPlayerSignal.connect(boost::bind(&Players::killPlayer, this, userId));
				player->statsAvailableSignal.connect(boost::bind(&Players::onRemoteSysStats, this, userId, "1920x1200", _1, true));
				player->remoteFriendServiceSignal.connect(boost::bind(&Players::friendServiceRequest, this, _1, weak_from(player), _2));

				if (!player->calculatesSpawnLocationEarly())
					if (Teams* teams = ServiceProvider::find<Teams>(this))
						teams->assignNewPlayerToTeam(player);

				playerAddedEarlySignal(shared_from(player));
				playerAddedSignal(shared_from(player));

				if (!FFlag::PreventRedundantRebuildLocalGui || Instance::fastDynamicCast<OverlayDataModel>(DataModel::get(this)))
					player->rebuildGui();
			}
		}
	}

	// FUNCTION: PLAYER 0x005705c0
	// FUNCTION: STUDIO 0x005d66c0
	Players::~Players()
	{
		setConnection(NULL);
	}

	// FUNCTION: PLAYER 0x00570980
	// FUNCTION: STUDIO 0x005d6a80
	Players::Players()
		: m_unk0x108("")
		, m_unk0x178(false)
		, m_unk0x179(true)
		, players(Instances())
		, rakPeer(NULL)
		, m_unk0x1b0(12)
		, testPlayerNameId(0)
		, testPlayerUserId(0)
		, m_unk0x1bc(0)
	{
		setName("Players");
	}

	// Folded by the linker with PlayerGui::askSetParent (identical body).
	// FUNCTION: PLAYER 0x0070a240 FOLDED
	// FUNCTION: STUDIO 0x00796960
	bool Players::askAddChild(const Instance* instance) const
	{
		return Instance::fastDynamicCast<Player>(instance) != NULL;
	}

	// getPlayerByID, friendStatusChanged and friendEventFired sit below the 2013 asserts (182/189) like
	// onChildChanged: there is no room above them.
	// FUNCTION: PLAYER 0x0055dc60
	// FUNCTION: STUDIO 0x005bdce0
	shared_ptr<Player> Players::getPlayerByID(int userId)
	{
		Instances::const_iterator iter = players->begin();
		Instances::const_iterator end = players->end();
		for (; iter != end; ++iter)
		{
			shared_ptr<Player> player = shared_polymorphic_downcast<Player>(*iter);
			if (player->userId == userId)
				return player;
		}
		return shared_ptr<Player>();
	}

	// FUNCTION: PLAYER 0x0055f070
	// FUNCTION: STUDIO 0x005bf850
	void Players::friendStatusChanged(int userId, int otherUserId, FriendService::FriendStatus friendStatus)
	{
		shared_ptr<Player> playerA = getPlayerByID(userId);
		shared_ptr<Player> playerB = getPlayerByID(otherUserId);
		if (playerA && playerB)
			playerA->onFriendStatusChanged(playerB, friendStatus);
	}

	// Fires m_unk0x1d4 when the local player's SuperSafeChatReplicate changes. Below the 2013 asserts on
	// purpose.
	// FUNCTION: PLAYER 0x005661f0
	// FUNCTION: STUDIO 0x005c9a10
	void Players::onChildChanged(Instance* instance, const PropertyChanged& event)
	{
		if (instance == localPlayer.get() && &event.getDescriptor() == &Player::prop_SuperSafeChat)
		{
			m_unk0x1d4(localPlayer ? Player::prop_SuperSafeChat.getValue(localPlayer.get()) : false);
		}
	}

	// FUNCTION: PLAYER 0x00566790
	// FUNCTION: STUDIO 0x005c9fb0
	void Players::friendEventFired(int userId, int otherUserId, FriendService::FriendEventType friendEvent)
	{
		shared_ptr<Player> playerA = getPlayerByID(userId);
		shared_ptr<Player> playerB = getPlayerByID(otherUserId);
		if (playerA && playerB)
			friendRequestEventSignal(playerA, playerB, friendEvent);
	}

} // namespace Network
} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x005668c0
// FUNCTION: STUDIO 0x005ca0e0
template<>
EnumDesc<Network::Players::ChatOption>::EnumDesc()
	: EnumDescriptor("ChatStyle", typeid(Network::Players::ChatOption))
{
	addPair(Network::Players::CLASSIC_CHAT, "Classic");
	addPair(Network::Players::BUBBLE_CHAT, "Bubble");
	addPair(Network::Players::CLASSIC_AND_BUBBLE_CHAT, "ClassicAndBubble");
}

template<>
const Type& Type::getSingleton<Network::Players::ChatOption>()
{
	return EnumDesc<Network::Players::ChatOption>::singleton();
}

template<> EnumRegistrar<Network::Players::ChatOption> EnumRegistrar<Network::Players::ChatOption>::registrar(0);
template<> TypeRegistrar<Network::Players::ChatOption> TypeRegistrar<Network::Players::ChatOption>::registrar(0);

// FUNCTION: PLAYER 0x005669e0
// FUNCTION: STUDIO 0x005ca200
template<>
EnumDesc<Network::Players::PlayerChatType>::EnumDesc()
	: EnumDescriptor("PlayerChatType", typeid(Network::Players::PlayerChatType))
{
	addPair(Network::Players::PLAYER_CHAT_TYPE_ALL, "All");
	addPair(Network::Players::PLAYER_CHAT_TYPE_TEAM, "Team");
	addPair(Network::Players::PLAYER_CHAT_TYPE_WHISPER, "Whisper");
}

template<>
const Type& Type::getSingleton<Network::Players::PlayerChatType>()
{
	return EnumDesc<Network::Players::PlayerChatType>::singleton();
}

template<> EnumRegistrar<Network::Players::PlayerChatType> EnumRegistrar<Network::Players::PlayerChatType>::registrar(0);
template<> TypeRegistrar<Network::Players::PlayerChatType> TypeRegistrar<Network::Players::PlayerChatType>::registrar(0);

} // namespace Reflection
} // namespace RBX

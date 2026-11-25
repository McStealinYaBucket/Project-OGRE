/* Network/Players.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"
#include "Network/Players.h"
#include "Network/Client.h"
#include "Network/Server.h"
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
// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
// Players is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}
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
	// Network::ChatMessage value-ctor family (0x89BBD2 / 0x89BC8E /
	// mac: 0x89BD10).  Binary member order: guid string default (empty COW
	// slot at +0) -> text -> ChatType (+8) -> player px/rc (+12/+16) ->
	// receiver px/rc (+20/+24) -> tail Guid::generateStandardGUID 0x204380
	// writing the "{uuid}" text into +0.  The 3-arg ctor leaves receiver
	// as an empty shared_ptr; the 0x89BC8E ctor copies every slot from
	// `other` (opens with std::string::string(+0, other+0)) and overwrites
	// the text with its second argument.
	//
	// mac: 0x89BD10 raw-asm correction: the text construction at 0x89BD30-41
	// pushes (&text, message, &allocator) into
	// std::string::string(char const*, std::allocator<char> const&) --
	// the earlier decompile rendered that call as a 1-arg default
	// construction and dropped the message; the raw call sites settle it
	// (the 3-arg ctor carries the same shape).
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

	// FUNCTION: PLAYER 0x0056baa0
	// FUNCTION: STUDIO 0x005d0f50
	Player* Players::findLocalPlayer(const Instance* context)
	{
		if (Players* players = ServiceProvider::find<Players>(context))
			return players->localPlayer.get();
		return NULL;
	}

	// Callees: ServiceProvider::find<FriendService> 0x00548490 (cdecl), FriendService::playerRemoving
	// 0x006afb40 (not declared by App's FriendService.h), the erase from the players list 0x0055fbb0,
	// then playerRemovingSignal (0x1cc) and playerRemovingLateSignal (0x1d0) when backendProcessing.
	// STUB: PLAYER 0x0056bd20
	// STUB: STUDIO 0x005d11d0
	void Players::onChildRemoving(Instance* child)
	{
		STUB(0x0056bd20);
	}

	// Player id lookup (getPlayerByID 0x0055dc60), the list append 0x00564bc0, FriendService::playerAdded
	// 0x006b36a0, guest naming ("Player" + format, 0x00b17000), five connections into the Player's
	// signals, Teams::assignNewPlayerToTeam, playerAddedEarly/playerAdded, Player::rebuildGui.
	// STUB: PLAYER 0x0056ff40
	// STUB: STUDIO 0x005d6040
	void Players::onChildAdded(Instance* child)
	{
		STUB(0x0056ff40);
	}

	// STUB: PLAYER 0x005705c0
	Players::~Players()
	{
	}

	// STUB: PLAYER 0x00570980
	Players::Players()
		: m_unk0x178(false), m_unk0x179(true), m_unk0x1a4(0), m_unk0x1b0(12)
		, m_unk0x1b4(0), m_unk0x1b8(0), m_unk0x1bc(0)
	{
	}

	// Folded by the linker with PlayerGui::askSetParent (identical body).
	// FUNCTION: PLAYER 0x0070a240 FOLDED
	// FUNCTION: STUDIO 0x00796960
	bool Players::askAddChild(const Instance* instance) const
	{
		return Instance::fastDynamicCast<Player>(instance) != NULL;
	}

	// The 2013 body (0x005661f0): when the local player's SuperSafeChatReplicate (PropDescriptor at
	// 0x00dd8e44, getter 0x00527600, setter 0x00527620) changes, fire m_unk0x1d4 with its value (false
	// without a local player). That descriptor is not declared yet. Below the 2013 asserts on purpose.
	// STUB: PLAYER 0x005661f0
	// STUB: STUDIO 0x005c9a10
	void Players::onChildChanged(Instance* instance, const PropertyChanged& event)
	{
		STUB(0x005661f0);
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

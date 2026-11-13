/* Network/Players.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "Network/Players.h"
#include "Network/Client.h"
#include "Network/Server.h"
#include <sstream>

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
RBX_REGISTER_CLASS(RBX::Network::Players);
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

	// 2013 has Players::clientIsPresent in Client.cpp (0x005adc70, between Client::findClient
	// and Client::onServiceProvider).
	// STUB: PLAYER 0x005adc70
	// STUB: STUDIO 0x0060c6b0
	bool Players::clientIsPresent(const Instance* context, bool testInDatamodel)
	{
		return Client::clientIsPresent(context, testInDatamodel);
	}

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
	// STUB: PLAYER 0x005689c0
	// STUB: STUDIO 0x005ccbc0
	bool Players::backendProcessing(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* serviceProvider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || serviceProvider!=NULL);
		return serviceProvider && !Players::clientIsPresent(context, testInDatamodel);
	}

	// FUNCTION: PLAYER 0x0056b8c0
	// FUNCTION: STUDIO 0x005d0d70
	bool Players::isNetworkClient(const Instance* instance)
	{
		return Instance::fastDynamicCast<Client>(instance) != NULL;
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

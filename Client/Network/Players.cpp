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

	// STUB: PLAYER 0x005689c0
	// STUB: STUDIO 0x005ccbc0
	bool Players::backendProcessing(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* provider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || provider != NULL);
		if (provider != NULL)
			return !Client::clientIsPresent(context, testInDatamodel);
		return false;
	}

	// STUB: PLAYER 0x005adc70
	// STUB: STUDIO 0x0060c6b0
	bool Players::clientIsPresent(const Instance* context, bool testInDatamodel)
	{
		return Client::clientIsPresent(context, testInDatamodel);
	}

	// STUB: PLAYER 0x00568900
	// STUB: STUDIO 0x005ccb00
	bool Players::frontendProcessing(const Instance* context, bool testInDatamodel)
	{
		const ServiceProvider* provider = ServiceProvider::findServiceProvider(context);
		RBXASSERT(!testInDatamodel || provider != NULL);
		if (provider != NULL)
			return !Server::serverIsPresent(context, testInDatamodel);
		return false;
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
	// neutral bytes (+104) clear && teamNum (+100) equal; 2 -> viewer
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
					&& this->player->teamNum == look->teamNum)
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

	// mac: 0x89BE50 (340B; mangle __ZNK3RBX7Network11ChatMessage21getReportAbuseMessageEv).
	// The sret slot is the first stack arg (`this` in the raw disasm)
	// and the const method object is the second.  Jump table at
	// mac: 0x89BE79 (0..3): TEAM -> "[[team]]" (0xB9A0B6) + message via
	// operator+(char const*, std::string const&); WHISPER ->
	// ostringstream "[[to " + receiver name or "???" + "]]" + message,
	// .str(); GAME -> "[[game]]" (0xB9A0AD) + message; case 0/default
	// -> plain string copy ctor from message.  The receiver name walks
	// fw(receiver.px)+24 (0x544AF8 + 0x18) -- the landed Instance::
	// getName() reads the same FWInstance+24 string (see Instance.h).
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

} // namespace Network
} // namespace RBX

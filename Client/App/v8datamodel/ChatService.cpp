/* App/v8datamodel/ChatService.cpp */

#include "stdafx.h"

#include "V8DataModel/ChatService.h"

namespace RBX {
	const char* const sChatService = "Chat";
}

using namespace RBX;

namespace RBX {
namespace Reflection {

	// RemoteEventDesc for ChatService::Chatted (dword_F10060).
	class ChatServiceChattedDesc : public RemoteEventDescImpl<
		3,
		RBX::ChatService,
		void (boost::shared_ptr<RBX::Instance>, std::string, RBX::ChatService::ChatColor),
		rbx::remote_signal<void (boost::shared_ptr<RBX::Instance>, std::string, RBX::ChatService::ChatColor)>,
		rbx::remote_signal<void (boost::shared_ptr<RBX::Instance>, std::string, RBX::ChatService::ChatColor)> RBX::ChatService::*>
	{
	public:
		ChatServiceChattedDesc()
			: RemoteEventDescImpl<
				3,
				RBX::ChatService,
				void (boost::shared_ptr<RBX::Instance>, std::string, RBX::ChatService::ChatColor),
				rbx::remote_signal<void (boost::shared_ptr<RBX::Instance>, std::string, RBX::ChatService::ChatColor)>,
				rbx::remote_signal<void (boost::shared_ptr<RBX::Instance>, std::string, RBX::ChatService::ChatColor)> RBX::ChatService::*>
			  (&RBX::ChatService::chattedSignal, "Chatted", "part", "message", "color",
			   Security::None, Descriptor::Attributes())
		{}
	};

	ChatServiceChattedDesc chattedDesc;

	// Fire arm helper.
	void fireChattedEvent(ChatService* instance, const shared_ptr<Instance>& partOrCharacter, const std::string& message, ChatService::ChatColor color)
	{
		chattedDesc.fireAndReplicateEvent(instance, partOrCharacter, message, color);
	}

	template<>
	EnumDesc<RBX::ChatService::ChatColor>::EnumDesc()
		: EnumDescriptor("ChatColor", typeid(RBX::ChatService::ChatColor))
	{
		addPair(RBX::ChatService::Blue, "Blue");
		addPair(RBX::ChatService::Green, "Green");
		addPair(RBX::ChatService::Red, "Red");
	}

	template<>
	const Type& Type::getSingleton<RBX::ChatService::ChatColor>()
	{
		return EnumDesc<RBX::ChatService::ChatColor>::singleton();
	}

	template<> EnumRegistrar<RBX::ChatService::ChatColor> EnumRegistrar<RBX::ChatService::ChatColor>::registrar(0);
	template<> TypeRegistrar<RBX::ChatService::ChatColor> TypeRegistrar<RBX::ChatService::ChatColor>::registrar(0);

} // namespace Reflection
} // namespace RBX

using namespace RBX;

// STUB: PLAYER 0x008024b0
// STUB: STUDIO 0x008f1660
void ChatService::chat(const shared_ptr<Instance>& partOrCharacter, const std::string& message, ChatColor color)
{
	if (partOrCharacter.get() == NULL)
		throw std::runtime_error("partOrCharacter must be non-nil");

	if (Workspace::getWorkspaceIfInWorkspace(partOrCharacter.get()) == NULL)
		throw std::runtime_error("partOrCharacter must be in the Workspace");

	if (partOrCharacter->getDescriptor().isA(ModelInstance::classDescriptor()))
	{
		int player = Network::Players::playerFromCharacter(partOrCharacter);
		if (player == 0)
			throw std::runtime_error("partOrCharacter is not a legal character");
	}
	else
	{
		if (!partOrCharacter->getDescriptor().isA(PartInstance::classDescriptor()))
			throw std::runtime_error("partOrCharacter must be a Part or a Character");
	}

	if (message.empty())
		throw std::runtime_error("message must be non-empty");

	// Fire arm.
	RBX::Reflection::fireChattedEvent(this, partOrCharacter, message, color);
}

REFLECTION_BEGIN();
REFLECTION_END();

namespace RBX {
RBX_REGISTER_CLASS(RBX::ChatService);
}
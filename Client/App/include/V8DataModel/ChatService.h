/* V8DataModel/ChatService.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<ChatService> from DataModel::initializeContents
     0x2D3336; ChatService ctor 0x2AE4A0 + chat() 0x2ADD76 + ChatColor EnumDesc 0x2AE20E
     mangles in studio_function_list.txt. (The "ChatService" Lua-name string was not
     found in the PE string tables — the registered Lua name is TODO.)
   Bases (DescribedCreatable + Service) carried from March 2016 header.
   TODO: full decompile (chat/filter + ChatColor enum + chatted remote event).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ChatService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "V8DataModel/ModelInstance.h"
#include "V8DataModel/PartInstance.h"
#include "V8DataModel/Workspace.h"

namespace RBX {

	extern const char* const sChatService;

	class ChatService
		: public DescribedCreatable<ChatService, Instance, sChatService, Reflection::ClassDescriptor::INTERNAL>
		, public Service
		, public Reflection::EventSource
	{
	public:
		// Chat color enum (EnumDesc 0x2AE20E): Blue=0, Green=1, Red=2.
		enum ChatColor
		{
			Blue = 0,
			Green,
			Red
		};

		// +0x5C: ctor-true byte.
		bool flagDelta0x5C;

		// +0x60: remote signal.
		rbx::remote_signal<void (boost::shared_ptr<Instance>, std::string, ChatColor)> chattedSignal;

		ChatService()
			: flagDelta0x5C(true)
		{
			Instance::setName("Chat");
		}

		// EventSource virtuals (delegated to Instance).
		virtual void processRemoteEvent(const Reflection::EventDescriptor& descriptor, const Reflection::EventArguments& args, const SystemAddress& source) {}
		virtual void raiseEventInvocation(const Reflection::EventDescriptor& descriptor, const Reflection::EventArguments& args, const SystemAddress* target = NULL) { Instance::raiseEventInvocation(descriptor, args, target); }

		// mac: 0x2ADD76: chat(partOrCharacter, message, color).  Validation per
		// binary: non-nil, in workspace, ModelInstance-or-PartInstance, non-empty
		// message.  Fire arm calls RemoteEventDesc::fireAndReplicateEvent.
		void chat(const shared_ptr<Instance>& partOrCharacter, const std::string& message, ChatColor color = Blue);
	};

} // namespace RBX

/* V8DataModel/ChatOption.h -- October 2013 (safe-chat tree node).
   Evidence: C2(string) 0x46F958 (116B: empty vector + assign label);
     D2 0x46F6A6 (ownership TBD -- next dump); SafeChat::loadChildren 0x46EF64
     fills the vector via push_back (children are `new`ed 0x10 nodes);
     GuiBuilder 4-arg buildChatMenu 0x3A51A8 reads children[i] + label at +12.
   Layout: +0 vector<ChatOption*> children (12B), +12 string label.
   NOTE the name trap: RBX::ChatOption (this node) vs
   Network::Players::ChatOption (an enum) -- different types, both real.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SafeChat.cpp (class decl in header) */

#pragma once

#include <string>
#include <vector>

namespace RBX {

	class ChatOption
	{
	public:
		// mac: 0x46F958: empty child vector + label assign.  Order is load-bearing
		// (+0 vector, +12 label).
		explicit ChatOption(const std::string& label);
		// Default for the loadChatTree ROOT node (binary news a bare
		// ChatOption then assigns label "ROOT").
		ChatOption() {}
		// mac: 0x46F6A6: OWNS the children -- recursively destroys + deletes each
		// entry (vector buffer + label die with their own dtors).
		~ChatOption()
		{
			for (size_t i = 0; i < children.size(); ++i)
				delete children[i];
		}
		std::vector<ChatOption*> children;  // +0.
		std::string label;  // +12.
	};

} // namespace RBX

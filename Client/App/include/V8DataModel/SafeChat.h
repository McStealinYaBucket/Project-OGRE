/* V8DataModel/SafeChat.h -- October 2013 (safe-chat tree service).
   Evidence: singleton 0x46EEC6 / loadChildren 0x46EF64 / loadChatTree
   0x46F1DE / getMessage 0x46F558 (batch285); ChatOption layout
   (+0 children vector, +12 label string) proven by the 0x3A51A8 worker
   reads. 2016 shapes carried for the class skeleton (singleton via
   boost::call_once, scoped_ptr root); bodies transcribed from the
   2013 disasm (loadChildren XmlNameValuePair walk + trim + new;
   getMessage sc-descent with bounds-break).
   Original file: Client/App/include/V8DataModel/SafeChat.h (path
   from 2016). */

#pragma once

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>

class XmlElement;

namespace RBX {

	class ChatOption;

	class SafeChat
	{
	public:
		SafeChat();
		~SafeChat();

		static SafeChat& singleton();

		ChatOption* getChatRoot() { return chatRoot.get(); }
		std::string getMessage(std::vector<std::string> code);

	private:
		boost::scoped_ptr<ChatOption> chatRoot;

		void loadChatTree();
		void loadChildren(ChatOption* node, const XmlElement* DOMsubTree);
	};

} // namespace RBX

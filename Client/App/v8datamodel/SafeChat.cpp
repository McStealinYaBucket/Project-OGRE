/* App/v8datamodel/SafeChat.cpp -- October 2013 (safe-chat tree).
   Bodies: singleton 0x46EEC6 / loadChildren 0x46EF64 /
   loadChatTree 0x46F1DE / getMessage 0x46F558 (all batch285).
   loadChatTree reads fonts/safechat.xml through ContentProvider::
   getAssetFile (exe-dir content/ layout); test environments stage it
   (see parsetest section 178 note). */

#include "stdafx.h"

#include "V8DataModel/SafeChat.h"
#include "V8DataModel/ChatOption.h"
#include "V8DataModel/ContentProvider.h"
#include "v8xml/TextXmlParser.h"
#include "v8xml/xmlelement.h"
#include "rbx/Debug.h"
#include "Util/Name.h"
#include <boost/algorithm/string.hpp>
#include <boost/thread/once.hpp>
#include <fstream>

namespace RBX {

	static boost::scoped_ptr<SafeChat> singleChat;
	static boost::once_flag once_SafeChat_singleton = BOOST_ONCE_INIT;
	static void initSafeChatSingleton()
	{
		singleChat.reset(new SafeChat());
	}

	// mac: 0x46EEC6 (batch285: call_once singleton).
	SafeChat& SafeChat::singleton()
	{
		boost::call_once(&initSafeChatSingleton, once_SafeChat_singleton);
		return *singleChat;
	}

	SafeChat::SafeChat()
	{
		loadChatTree();
	}

	SafeChat::~SafeChat()
	{
	}

	// mac: 0x46F1DE (batch285: asset file -> XmlParser/TextXmlParser -> ROOT
	// node labeled ROOT -> recursive load).
	// STUB: PLAYER 0x007e4960
	// STUB: STUDIO 0x008a7de0
	void SafeChat::loadChatTree()
	{
		std::string filename = ContentProvider::getAssetFile("fonts/safechat.xml");
		std::ifstream stream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
		TextXmlParser machine(stream.rdbuf());
		std::auto_ptr<XmlElement> root(machine.parse());
		chatRoot.reset(new ChatOption());
		chatRoot->label = "ROOT";
		loadChildren(chatRoot.get(), root.get());
	}

	// mac: 0x46EF64 (batch285: utterance-tag walk; value trimmed; new child
	// per node; recurse).
	// STUB: PLAYER 0x007e47b0
	// STUB: STUDIO 0x008a7c30
	void SafeChat::loadChildren(ChatOption* node, const XmlElement* DOMsubTree)
	{
		static const Name* utterance = Name::declare("utterance");
		for (const XmlElement* x = DOMsubTree->findFirstChildByTag(*utterance);
			x != NULL;
			x = DOMsubTree->findNextChildWithSameTag(x))
		{
			std::string text;
			bool succeeded = x->getValue(text);
			RBXASSERT(succeeded);  // :71 "succeeded"
			boost::trim(text);
			ChatOption* child = new ChatOption(text);
			node->children.push_back(child);
			loadChildren(child, x);
		}
	}

	// mac: 0x46F558 (batch285: :101 chatRoot + :102 sc-gate asserts; <2 codes
	// -> root label; atoi descent with bounds-break to ""; node label).
	// STUB: PLAYER 0x007e4590
	// STUB: STUDIO 0x008a7a10
	std::string SafeChat::getMessage(std::vector<std::string> code)
	{
		RBXASSERT(chatRoot.get() != NULL);  // :101 "chatRoot"
		RBXASSERT(code[0] == "sc");  // :102 'code[0] == "sc"'
		ChatOption* cur = chatRoot.get();
		if (code.size() < 2)
			return cur->label;
		for (size_t i = 1; i < code.size(); ++i)
		{
			unsigned int n = (unsigned int)atoi(code[i].c_str());
			if (n >= cur->children.size())
				break;
			cur = cur->children[n];
			if (i + 1 >= code.size())
				return cur->label;
		}
		return "";
	}

} // namespace RBX

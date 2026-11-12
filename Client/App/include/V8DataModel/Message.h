/* V8DataModel/Message.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Message, Instance> 0x4335B8;
     sMessage = "Message" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Message.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sMessage;

	class Message
		: public DescribedCreatable<Message, Instance, sMessage>
	{
	public:
		Message() {}
	};

} // namespace RBX

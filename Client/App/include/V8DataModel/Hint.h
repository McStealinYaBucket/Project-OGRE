/* V8DataModel/Hint.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Hint, Message>;
     sHint = "Hint" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Hint.h */

#pragma once

#include "V8DataModel/Message.h"

namespace RBX {

	extern const char* const sHint;

	class Hint
		: public DescribedCreatable<Hint, Message, sHint>
	{
	public:
		Hint() {}
	};

} // namespace RBX

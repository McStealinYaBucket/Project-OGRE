/* V8DataModel/BackpackItem.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<BackpackItem, Widget> 0x3EEEE0;
     sBackpackItem = "BackpackItem" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BackpackItem.h */

#pragma once

#include "V8DataModel/Widget.h"

namespace RBX {

	extern const char* const sBackpackItem;

	class BackpackItem
		: public DescribedNonCreatable<BackpackItem, Widget, sBackpackItem>
	{
	public:
		BackpackItem() {}
	};

} // namespace RBX

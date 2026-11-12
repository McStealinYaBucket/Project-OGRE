/* V8DataModel/Widget.h -- October 2013 STUB (minimal shell).
   Evidence: BackpackItem parent (NonFactoryProduct<BackpackItem, Widget> 0x3EEEE0); Widget C2 0x5D2E3C;
     sWidget = "Widget" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Widget.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sWidget;

	class Widget
		: public DescribedNonCreatable<Widget, Instance, sWidget>
	{
	public:
		Widget() {}
	};

} // namespace RBX

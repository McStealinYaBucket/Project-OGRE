/* V8DataModel/Mouse.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Mouse, Instance> 0x43B128; abstract (no creator by design);
     sMouse = "Mouse".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Mouse.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sMouse;

	class Mouse
		: public DescribedNonCreatable<Mouse, Instance, sMouse>
	{
	public:
		Mouse() {}
	};

} // namespace RBX

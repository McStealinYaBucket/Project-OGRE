/* V8DataModel/Frame.h -- October 2013 STUB (minimal shell).
   Evidence: Described<Frame, sFrame, FactoryProduct<GuiObject>> EAs (see session notes);
     sFrame = "Frame" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Frame.h */

#pragma once

#include "V8DataModel/GuiObject.h"

namespace RBX {

	extern const char* const sFrame;

	class Frame
		: public DescribedCreatable<Frame, GuiObject, sFrame>
	{
	public:
		Frame() {}
	};

} // namespace RBX

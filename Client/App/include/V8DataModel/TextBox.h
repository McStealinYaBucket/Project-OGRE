/* V8DataModel/TextBox.h -- October 2013 STUB (minimal shell).
   Evidence: Described<TextBox, sTextBox, FactoryProduct<GuiObject>> EAs (see session notes);
     sTextBox = "TextBox" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TextBox.h */

#pragma once

#include "V8DataModel/GuiObject.h"

namespace RBX {

	extern const char* const sTextBox;

	class TextBox
		: public DescribedCreatable<TextBox, GuiObject, sTextBox>
	{
	public:
		TextBox() {}
	};

} // namespace RBX

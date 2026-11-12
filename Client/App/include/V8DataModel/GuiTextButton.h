/* V8DataModel/GuiTextButton.h -- October 2013 STUB (minimal shell).
   Evidence: Described<GuiTextButton, sGuiTextButton, FactoryProduct<GuiButton>> EAs (see session notes);
     sGuiTextButton = "TextButton" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiTextButton.h */

#pragma once

#include "V8DataModel/GuiButton.h"

namespace RBX {

	extern const char* const sGuiTextButton;

	class GuiTextButton
		: public DescribedCreatable<GuiTextButton, GuiButton, sGuiTextButton>
	{
	public:
		GuiTextButton() {}
	};

} // namespace RBX

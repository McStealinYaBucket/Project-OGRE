/* V8DataModel/GuiButton.h -- October 2013 STUB (minimal shell).
   Evidence: Described<GuiButton, sGuiButton, NonFactoryProduct<GuiObject>> EAs (see session notes);
     sGuiButton = "GuiButton" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiButton.h */

#pragma once

#include "V8DataModel/GuiObject.h"

namespace RBX {

	extern const char* const sGuiButton;

	class GuiButton
		: public DescribedNonCreatable<GuiButton, GuiObject, sGuiButton>
	{
	public:
		GuiButton() {}
		// GuiImageButton C2(Verb*) 0x3F4CDE calls GuiButton::GuiButton(name)
		// mac: 0x3ADB58 with "ImageButton".  The direct base here is the
		// DescribedNonCreatable chain (not GuiObject itself), so the name
		// goes through setName in the body -- same observable as the base
		// C2 (every named widget C2 sets the name; ChatWidget 0x5CB07C
		// proven for the direct-call shape, and Instance::setName in a
		// body is the ChatWidget precedent).
		explicit GuiButton(const char* name)
		{
			Instance::setName(name == NULL ? "" : name);
		}
	};

} // namespace RBX

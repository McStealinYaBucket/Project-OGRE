/* V8DataModel/GuiLabel.h — October 2013 (label shell).
   Evidence: GuiLabel C2 0x3AEE42 (GuiObject(name, false);
     Described<GuiLabel, sGuiLabel, GuiObject, 27>).
     sGuiLabel 16 hits in the 0.125 PE string table.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiLabel.h */

#pragma once

#include "V8DataModel/GuiObject.h"

namespace RBX {

	extern const char* const sGuiLabel;

	class GuiLabel
		: public DescribedNonCreatable<GuiLabel, GuiObject, sGuiLabel, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		GuiLabel(const char* name = NULL);
	};

} // namespace RBX

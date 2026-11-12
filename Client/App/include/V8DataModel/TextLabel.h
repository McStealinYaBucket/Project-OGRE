/* V8DataModel/TextLabel.h -- October 2013 STUB (minimal shell).
   Evidence: Described<TextLabel, sTextLabel, FactoryProduct<GuiLabel>> EAs (see session notes);
     sTextLabel = "TextLabel" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TextLabel.h */

#pragma once

#include "V8DataModel/GuiLabel.h"

namespace RBX {

	extern const char* const sTextLabel;

	class TextLabel
		: public DescribedCreatable<TextLabel, GuiLabel, sTextLabel>
	{
	public:
		TextLabel() {}
	};

} // namespace RBX

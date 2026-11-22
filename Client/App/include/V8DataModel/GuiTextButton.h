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
		typedef DescribedCreatable<GuiTextButton, GuiButton, sGuiTextButton> Super;
	public:
		GuiTextButton() : Super("TextButton") {}
	};

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x007a2180
// ??$?0PBD@?$Described@VGuiTextButton@RBX@@$E?sGuiTextButton@2@3QBDBV?$FactoryProduct@VGuiTextButton@RBX@@VGuiButton@2@$E?sGuiTextButton@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
// TEMPLATE: PLAYER 0x007a2210
// ??1?$Described@VGuiTextButton@RBX@@$E?sGuiTextButton@2@3QBDBV?$FactoryProduct@VGuiTextButton@RBX@@VGuiButton@2@$E?sGuiTextButton@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@UAE@XZ
// clang-format on

// SYNTHETIC: PLAYER 0x007a22e0
// RBX::GuiTextButton::`scalar deleting destructor'

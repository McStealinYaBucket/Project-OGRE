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

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x006c7120
// ??1?$FactoryProduct@VTextBox@RBX@@VGuiObject@2@$E?sTextBox@2@3QBDBVInstance@2@@RBX@@MAE@XZ
// TEMPLATE: PLAYER 0x006c7150
// ??_G?$FactoryProduct@VTextBox@RBX@@VGuiObject@2@$E?sTextBox@2@3QBDBVInstance@2@@RBX@@MAEPAXI@Z
// TEMPLATE: PLAYER 0x006c71a0
// ?classDescriptor@?$Described@VTextBox@RBX@@$E?sTextBox@2@3QBDBV?$FactoryProduct@VTextBox@RBX@@VGuiObject@2@$E?sTextBox@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@SAAAVClassDescriptor@23@XZ
// clang-format on

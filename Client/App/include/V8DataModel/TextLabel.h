/* V8DataModel/TextLabel.h -- October 2013.
   Layout from the PLAYER ctor 0x006c1c10 (STUDIO 0x00854b20): the Described ctor ("TextLabel"), then the
   GuiTextMixin base at +0x21c ("Label", BrickColor(26).color3()), then the TextLabel vftables and
   guiQueue = GUIQUEUE_TEXT. Creatable<Instance>::create<TextLabel> 0x00421af0 allocates 0x270. */

#pragma once

#include "V8DataModel/GuiLabel.h"
#include "V8DataModel/GuiText.h"

namespace RBX {

	extern const char* const sTextLabel;

	// VTABLE: PLAYER 0x00b3b4bc RBX::Debugable
	// VTABLE: STUDIO 0x00c1d1c4 RBX::Debugable
	// VTABLE: PLAYER 0x00b3b490 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c1d198 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b3b45c RBX::IAdornable
	// VTABLE: STUDIO 0x00c1d164 RBX::IAdornable
	// SIZE 0x270
	class TextLabel
		: public DescribedCreatable<TextLabel, GuiLabel, sTextLabel>
		, public GuiTextMixin  // 0x21c
	{
		typedef DescribedCreatable<TextLabel, GuiLabel, sTextLabel> Super;
	public:
		TextLabel();
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x006c1b60
	// TEMPLATE: STUDIO 0x00854aa0
	// ??$?0PBD@?$Described@VTextLabel@RBX@@$E?sTextLabel@2@3QBDBV?$FactoryProduct@VTextLabel@RBX@@VGuiLabel@2@$E?sTextLabel@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on

} // namespace RBX

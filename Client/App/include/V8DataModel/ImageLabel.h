/* V8DataModel/ImageLabel.h -- October 2013.
   Layout from the PLAYER ctor 0x006bdb40 (STUDIO 0x00851f80): the Described ctor ("ImageLabel"), the
   GuiImageMixin base at +0x21c (ctor 0x006bc910), then the ImageLabel vftables, no members of its own.
   The mixin's three setters are ImageLabel's (vftable 0x00b3a96c: 0x006bc280, 0x006bbe20, 0x006bbe70);
   each raises its property, which these bodies do not yet. */

#pragma once

#include "V8DataModel/GuiLabel.h"
#include "V8DataModel/GuiMixin.h"

namespace RBX {

	extern const char* const sImageLabel;

	// VTABLE: PLAYER 0x00b3a8c4 RBX::Debugable
	// VTABLE: STUDIO 0x00c1c73c RBX::Debugable
	// VTABLE: PLAYER 0x00b3a898 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c1c710 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b3a864 RBX::IAdornable
	// VTABLE: STUDIO 0x00c1c6dc RBX::IAdornable
	// VTABLE: PLAYER 0x00b3a96c RBX::GuiImageMixin
	// VTABLE: STUDIO 0x00c1c7e4 RBX::GuiImageMixin
	// SIZE 0x2d4
	class ImageLabel
		: public DescribedCreatable<ImageLabel, GuiLabel, sImageLabel>
		, public GuiImageMixin  // 0x21c
	{
		typedef DescribedCreatable<ImageLabel, GuiLabel, sImageLabel> Super;
	public:
		ImageLabel();

		/*override*/ void setImage(TextureId value);
		/*override*/ void setImageRectOffset(G3D::Vector2 value);
		/*override*/ void setImageRectSize(G3D::Vector2 value);
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x006bdab0
	// TEMPLATE: STUDIO 0x00851e90
	// ??$?0PBD@?$Described@VImageLabel@RBX@@$E?sImageLabel@2@3QBDBV?$FactoryProduct@VImageLabel@RBX@@VGuiLabel@2@$E?sImageLabel@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on

} // namespace RBX

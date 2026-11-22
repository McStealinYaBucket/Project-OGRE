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

		static Reflection::PropDescriptor<ImageLabel, TextureId> prop_Image;
		static const Reflection::PropertyDescriptor* prop_ImageRectOffset;
		static const Reflection::PropertyDescriptor* prop_ImageRectSize;
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x006bdab0
	// TEMPLATE: STUDIO 0x00851e90
	// ??$?0PBD@?$Described@VImageLabel@RBX@@$E?sImageLabel@2@3QBDBV?$FactoryProduct@VImageLabel@RBX@@VGuiLabel@2@$E?sImageLabel@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on


	// Out-of-line template and implicit-member copies the original emitted for this class, paired by
	// identical masked bytes (tools/unpaired_twins.py on a 2026-11-13 build).
	// clang-format off
	// TEMPLATE: PLAYER 0x006bc1f0
	// TEMPLATE: STUDIO 0x00851ab0
	// ?getValue@?$GetSetImpl@P8GuiImageMixin@RBX@@BE?AVContentId@2@XZP8ImageLabel@2@AEXV32@@Z@?$PropDescriptor@VImageLabel@RBX@@VContentId@2@@Reflection@RBX@@UBE?AVContentId@4@PBVDescribedBase@34@@Z
	// SYNTHETIC: PLAYER 0x006bc990
	// SYNTHETIC: STUDIO 0x00851c90
	// RBX::GuiImageMixin::~GuiImageMixin
	// TEMPLATE: PLAYER 0x006bda30
	// ??1?$Described@VImageLabel@RBX@@$E?sImageLabel@2@3QBDBV?$FactoryProduct@VImageLabel@RBX@@VGuiLabel@2@$E?sImageLabel@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@UAE@XZ
	// TEMPLATE: PLAYER 0x006bda60
	// ??_G?$FactoryProduct@VImageLabel@RBX@@VGuiLabel@2@$E?sImageLabel@2@3QBDBVInstance@2@@RBX@@MAEPAXI@Z
	// SYNTHETIC: PLAYER 0x006bdc10
	// SYNTHETIC: STUDIO 0x00852050
	// RBX::ImageLabel::~ImageLabel
	// TEMPLATE: PLAYER 0x006bdff0
	// TEMPLATE: STUDIO 0x00852430
	// ?setValue@?$GetSetImpl@P8GuiImageMixin@RBX@@BE?AVContentId@2@XZP8ImageLabel@2@AEXV32@@Z@?$PropDescriptor@VImageLabel@RBX@@VContentId@2@@Reflection@RBX@@UBEXPAVDescribedBase@34@ABVContentId@4@@Z
	// TEMPLATE: PLAYER 0x006be5e0
	// ??_G?$DescribedCreatable@VGuiImageButton@RBX@@VGuiButton@2@$E?sGuiImageButton@2@3QBDB$0BL@$0A@@RBX@@UAEPAXI@Z
	// TEMPLATE: PLAYER 0x006be6c0
	// ??1?$Described@VGuiImageButton@RBX@@$E?sGuiImageButton@2@3QBDBV?$FactoryProduct@VGuiImageButton@RBX@@VGuiButton@2@$E?sGuiImageButton@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@UAE@XZ
	// clang-format on

} // namespace RBX
// SYNTHETIC: PLAYER 0x006bdc90
// SYNTHETIC: STUDIO 0x008520d0
// RBX::ImageLabel::`scalar deleting destructor'

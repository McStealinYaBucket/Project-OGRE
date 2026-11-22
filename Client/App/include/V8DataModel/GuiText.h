/* V8DataModel/GuiText.h -- October 2013.
   No 2013 string names this header; the 2016 path (App/include/v8datamodel/GuiText.h).
   GuiTextMixin is the non-polymorphic second base of TextLabel (and of TextBox and GuiTextButton) at
   +0x21c. Layout from its ctor, PLAYER 0x006be930 (STUDIO 0x00836e40), which TextLabel's ctor 0x006c1c10
   calls with ("Label", BrickColor(26).color3()): the same members, order and initial values as 2016. */

#pragma once

#include <string>

#include "G3D/Color3.h"
#include "Util/BrickColor.h"
#include "V8DataModel/TextService.h"
#include "decomp.h"

#define category_Text "Text"

namespace RBX {

	// SIZE 0x54
	class GuiTextMixin
	{
	public:
		// FUNCTION: PLAYER 0x006be930
		// FUNCTION: STUDIO 0x00836e40
		GuiTextMixin(const std::string& text, const G3D::Color3& textColor)
			: filterState(0)
			, text(text)
			, fontSize(TextService::SIZE_8)
			, textColor(textColor)
			, textTransparency(0)
			, textStrokeColor(0, 0, 0)
			, textStrokeTransparency(1.0f)
			, textWrap(false)
			, textScale(false)
			, xAlignment(TextService::XALIGNMENT_CENTER)
			, yAlignment(TextService::YALIGNMENT_CENTER)
			, font(TextService::FONT_0)
		{
		}

		// The getters the TextLabel prop descriptors take, in their 2013 out-of-line order.
		// FUNCTION: PLAYER 0x006be9a0
		// FUNCTION: STUDIO 0x00836eb0
		std::string getText() const { return text; }
		// FUNCTION: PLAYER 0x006be9d0
		// FUNCTION: STUDIO 0x00836ee0
		TextService::FontSize getFontSize() const { return fontSize; }
		// The out-of-line copy at PLAYER 0x006be9e0 is ICF-folded with Instance::onDemandRead.
		TextService::Font getFont() const { return font; }
		// FUNCTION: PLAYER 0x006be9f0
		// FUNCTION: STUDIO 0x00836f00
		BrickColor getTextColor() const { return BrickColor::closest(textColor); }
		// FUNCTION: PLAYER 0x006bea20
		// FUNCTION: STUDIO 0x00836f30
		G3D::Color3 getTextColor3() const { return textColor; }
		// FUNCTION: PLAYER 0x006bea40
		// FUNCTION: STUDIO 0x00836f50
		float getTextTransparency() const { return textTransparency; }
		// FUNCTION: PLAYER 0x006bea50
		// FUNCTION: STUDIO 0x00836f60
		float getTextStrokeTransparency() const { return textStrokeTransparency; }
		// FUNCTION: PLAYER 0x006bea60
		// FUNCTION: STUDIO 0x00836f70
		G3D::Color3 getTextStrokeColor3() const { return textStrokeColor; }
		// FUNCTION: PLAYER 0x006bea80
		// FUNCTION: STUDIO 0x00836f90
		bool getTextWrap() const { return textWrap; }
		// FUNCTION: PLAYER 0x006bea90
		// FUNCTION: STUDIO 0x00836fa0
		bool getTextScale() const { return textScale; }
		// The out-of-line copies at PLAYER 0x006beaa0/0x006beab0 are ICF-folded with Instance::fw and
		// BuoyancyContact::numConnectors.
		TextService::XAlignment getXAlignment() const { return xAlignment; }
		TextService::YAlignment getYAlignment() const { return yAlignment; }

	protected:
		undefined4 filterState;                // 0x00  2016: ContentFilter::FilterResult (Waiting = 0)
		std::string text;                      // 0x04
		TextService::FontSize fontSize;        // 0x20
		G3D::Color3 textColor;                 // 0x24
		float textTransparency;                // 0x30
		G3D::Color3 textStrokeColor;           // 0x34
		float textStrokeTransparency;          // 0x40
		bool textWrap;                         // 0x44
		bool textScale;                        // 0x45
		TextService::XAlignment xAlignment;    // 0x48
		TextService::YAlignment yAlignment;    // 0x4c
		TextService::Font font;                // 0x50
	};

} // namespace RBX

/* V8DataModel/GuiText.h -- October 2013.
   No 2013 string names this header; the 2016 path (App/include/v8datamodel/GuiText.h).
   GuiTextMixin is the non-polymorphic second base of TextLabel (and of TextBox and GuiTextButton) at
   +0x21c. Layout from its ctor, PLAYER 0x006be930 (STUDIO 0x00836e40), which TextLabel's ctor 0x006c1c10
   calls with ("Label", BrickColor(26).color3()): the same members, order and initial values as 2016. */

#pragma once

#include <string>

#include "G3D/Color3.h"
#include "V8DataModel/TextService.h"
#include "decomp.h"

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

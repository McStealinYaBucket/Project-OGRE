/* App/v8datamodel/TextLabel.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/TextLabel.h"
#include "Util/BrickColor.h"
#include "V8Tree/Property.h"
#include "Gui/ProfanityFilter.h"
#include "V8DataModel/ContentFilter.h"

namespace RBX {
	const char* const sTextLabel = "TextLabel";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::TextLabel);
}

DECOMP_SIZE_ASSERT(RBX::GuiTextMixin, 0x54);
DECOMP_SIZE_ASSERT(RBX::TextLabel, 0x270);

namespace RBX {

	// The 2013 dynamic initializers (PLAYER 0x00a6bb40..0x00a6c030) build these in this order.
	// GLOBAL: PLAYER 0x00dec85c
	// GLOBAL: STUDIO 0x00f8e7b0
	// TextLabel::prop_Text
	Reflection::PropDescriptor<TextLabel, std::string> TextLabel::prop_Text("Text", category_Text, &TextLabel::getText, &TextLabel::setText);
	// GLOBAL: PLAYER 0x00dec910
	// GLOBAL: STUDIO 0x00f8e86c
	// TextLabel::prop_FontSize
	Reflection::EnumPropDescriptor<TextLabel, TextService::FontSize> TextLabel::prop_FontSize("FontSize", category_Text, &TextLabel::getFontSize, &TextLabel::setFontSize);
	// GLOBAL: PLAYER 0x00dec70c
	// GLOBAL: STUDIO 0x00f8e668
	// TextLabel::prop_Font
	Reflection::EnumPropDescriptor<TextLabel, TextService::Font> TextLabel::prop_Font("Font", category_Text, &TextLabel::getFont, &TextLabel::setFont);
	// GLOBAL: PLAYER 0x00dec76c
	// GLOBAL: STUDIO 0x00f8e6c8
	// TextLabel::prop_TextColor
	Reflection::PropDescriptor<TextLabel, BrickColor> TextLabel::prop_TextColor("TextColor", category_Text, &TextLabel::getTextColor, &TextLabel::setTextColor, Reflection::PropertyDescriptor::LEGACY_SCRIPTING);
	// GLOBAL: PLAYER 0x00dec7d4
	// GLOBAL: STUDIO 0x00f8e728
	// TextLabel::prop_TextColor3
	Reflection::PropDescriptor<TextLabel, G3D::Color3> TextLabel::prop_TextColor3("TextColor3", category_Text, &TextLabel::getTextColor3, &TextLabel::setTextColor3);
	// GLOBAL: PLAYER 0x00dec7a0
	// GLOBAL: STUDIO 0x00f8e6fc
	// TextLabel::prop_TextTransparency
	Reflection::PropDescriptor<TextLabel, float> TextLabel::prop_TextTransparency("TextTransparency", category_Text, &TextLabel::getTextTransparency, &TextLabel::setTextTransparency);
	// GLOBAL: PLAYER 0x00dec8e0
	// GLOBAL: STUDIO 0x00f8e83c
	// TextLabel::prop_TextWrap
	Reflection::PropDescriptor<TextLabel, bool> TextLabel::prop_TextWrap("TextWrapped", category_Text, &TextLabel::getTextWrap, &TextLabel::setTextWrap);
	// GLOBAL: PLAYER 0x00dec740
	// GLOBAL: STUDIO 0x00f8e69c
	// TextLabel::prop_depTextWrap
	Reflection::PropDescriptor<TextLabel, bool> TextLabel::prop_depTextWrap("TextWrap", category_Text, &TextLabel::getTextWrap, &TextLabel::setTextWrap, Reflection::PropertyDescriptor::Attributes::deprecated(prop_TextWrap, Reflection::PropertyDescriptor::UI));
	// GLOBAL: PLAYER 0x00dec6e0
	// GLOBAL: STUDIO 0x00f8e63c
	// TextLabel::prop_TextScale
	Reflection::PropDescriptor<TextLabel, bool> TextLabel::prop_TextScale("TextScaled", category_Text, &TextLabel::getTextScale, &TextLabel::setTextScale);
	// GLOBAL: PLAYER 0x00dec978
	// GLOBAL: STUDIO 0x00f8e8d4
	// TextLabel::prop_TextXAlignment
	Reflection::EnumPropDescriptor<TextLabel, TextService::XAlignment> TextLabel::prop_TextXAlignment("TextXAlignment", category_Text, &TextLabel::getXAlignment, &TextLabel::setXAlignment);
	// GLOBAL: PLAYER 0x00dec944
	// GLOBAL: STUDIO 0x00f8e8a0
	// TextLabel::prop_TextYAlignment
	Reflection::EnumPropDescriptor<TextLabel, TextService::YAlignment> TextLabel::prop_TextYAlignment("TextYAlignment", category_Text, &TextLabel::getYAlignment, &TextLabel::setYAlignment);
	// GLOBAL: PLAYER 0x00dec8b4
	// GLOBAL: STUDIO 0x00f8e808
	// TextLabel::prop_TextBounds
	Reflection::PropDescriptor<TextLabel, G3D::Vector2> TextLabel::prop_TextBounds("TextBounds", category_Text, &TextLabel::getTextBounds, NULL, Reflection::PropertyDescriptor::UI);
	// GLOBAL: PLAYER 0x00dec800
	// GLOBAL: STUDIO 0x00f8e754
	// TextLabel::prop_TextFits
	Reflection::PropDescriptor<TextLabel, bool> TextLabel::prop_TextFits("TextFits", category_Text, &TextLabel::getTextFits, NULL, Reflection::PropertyDescriptor::UI);
	// GLOBAL: PLAYER 0x00dec830
	// GLOBAL: STUDIO 0x00f8e784
	// TextLabel::prop_TextStrokeColor3
	Reflection::PropDescriptor<TextLabel, G3D::Color3> TextLabel::prop_TextStrokeColor3("TextStrokeColor3", category_Text, &TextLabel::getTextStrokeColor3, &TextLabel::setTextStrokeColor3);
	// GLOBAL: PLAYER 0x00dec888
	// GLOBAL: STUDIO 0x00f8e7dc
	// TextLabel::prop_TextStrokeTransparency
	Reflection::PropDescriptor<TextLabel, float> TextLabel::prop_TextStrokeTransparency("TextStrokeTransparency", category_Text, &TextLabel::getTextStrokeTransparency, &TextLabel::setTextStrokeTransparency);

	// FUNCTION: PLAYER 0x006beb80
	// FUNCTION: STUDIO 0x00852d60
	void TextLabel::checkForResize()
	{
		Super::checkForResize();
		raisePropertyChanged(prop_TextBounds);
		raisePropertyChanged(prop_TextFits);
	}

	// FUNCTION: PLAYER 0x006bebb0
	// FUNCTION: STUDIO 0x00852d90
	void TextLabel::setFontSize(TextService::FontSize value)
	{
		if (fontSize != value)
		{
			fontSize = value;
			raisePropertyChanged(prop_FontSize);
			raisePropertyChanged(prop_TextBounds);
		}
	}

	// FUNCTION: PLAYER 0x006bebe0
	// FUNCTION: STUDIO 0x00852dc0
	void TextLabel::setFont(TextService::Font value)
	{
		if (font != value)
		{
			font = value;
			raisePropertyChanged(prop_Font);
			raisePropertyChanged(prop_TextBounds);
		}
	}

	// FUNCTION: PLAYER 0x006bec10
	// FUNCTION: STUDIO 0x00852df0
	void TextLabel::setTextColor3(G3D::Color3 value)
	{
		if (textColor != value)
		{
			textColor = value;
			raisePropertyChanged(prop_TextColor);
			raisePropertyChanged(prop_TextColor3);
		}
	}

	// FUNCTION: PLAYER 0x006bec90
	// FUNCTION: STUDIO 0x00852e70
	void TextLabel::setTextTransparency(float value)
	{
		if (textTransparency != value)
		{
			textTransparency = value;
			raisePropertyChanged(prop_TextTransparency);
		}
	}

	// FUNCTION: PLAYER 0x006becc0
	// FUNCTION: STUDIO 0x00852ea0
	void TextLabel::setTextStrokeTransparency(float value)
	{
		if (textStrokeTransparency != value)
		{
			textStrokeTransparency = value;
			raisePropertyChanged(prop_TextStrokeTransparency);
		}
	}

	// FUNCTION: PLAYER 0x006becf0
	// FUNCTION: STUDIO 0x00852ed0
	void TextLabel::setTextStrokeColor3(G3D::Color3 value)
	{
		if (textStrokeColor != value)
		{
			textStrokeColor = value;
			raisePropertyChanged(prop_TextStrokeColor3);
		}
	}

	// FUNCTION: PLAYER 0x006bed60
	// FUNCTION: STUDIO 0x00852f40
	void TextLabel::setTextWrap(bool value)
	{
		if (textWrap != value)
		{
			textWrap = value;
			raisePropertyChanged(prop_TextWrap);
			raisePropertyChanged(prop_TextBounds);
			raisePropertyChanged(prop_TextFits);
		}
	}

	// FUNCTION: PLAYER 0x006beda0
	// FUNCTION: STUDIO 0x00852f80
	void TextLabel::setTextScale(bool value)
	{
		if (textScale != value)
		{
			textScale = value;
			raisePropertyChanged(prop_TextScale);
			if (value)
			{
				setTextWrap(value);
			}
			else
			{
				raisePropertyChanged(prop_TextBounds);
				raisePropertyChanged(prop_TextFits);
			}
		}
	}

	// FUNCTION: PLAYER 0x006bedf0
	// FUNCTION: STUDIO 0x00852fd0
	void TextLabel::setXAlignment(TextService::XAlignment value)
	{
		if (xAlignment != value)
		{
			xAlignment = value;
			raisePropertyChanged(prop_TextXAlignment);
			raisePropertyChanged(prop_TextBounds);
			raisePropertyChanged(prop_TextFits);
		}
	}

	// FUNCTION: PLAYER 0x006bee30
	// FUNCTION: STUDIO 0x00853010
	void TextLabel::setYAlignment(TextService::YAlignment value)
	{
		if (yAlignment != value)
		{
			yAlignment = value;
			raisePropertyChanged(prop_TextYAlignment);
			raisePropertyChanged(prop_TextBounds);
			raisePropertyChanged(prop_TextFits);
		}
	}

	// FUNCTION: PLAYER 0x006bee70
	// FUNCTION: STUDIO 0x00853050
	void TextLabel::setTransparencyLegacy(float value)
	{
		setTextTransparency(value);
		Super::setTransparencyLegacy(value);
	}

	// The 2013 body reads ContentFilter::MAX_CONTENT_FILTER_SIZE (0x400) from memory at PLAYER
	// 0x00b77cc0, a static member ContentFilter.cpp defines.
	// FUNCTION: PLAYER 0x006beeb0
	// FUNCTION: STUDIO 0x00853090
	void TextLabel::setText(std::string value)
	{
		if (value.size() > ContentFilter::MAX_CONTENT_FILTER_SIZE)
			value = value.substr(0, ContentFilter::MAX_CONTENT_FILTER_SIZE);
		if (!ProfanityFilter::ContainsProfanity(value) || getRobloxLocked())
		{
			if (text != value)
			{
				text = value;
				filterState = 0;
				raisePropertyChanged(prop_Text);
				raisePropertyChanged(prop_TextBounds);
				raisePropertyChanged(prop_TextFits);
			}
		}
	}

	// FUNCTION: PLAYER 0x006befb0
	// FUNCTION: STUDIO 0x00853190
	void TextLabel::setTextColor(BrickColor value)
	{
		setTextColor3(value.color3());
	}

	// TODO: body (measures text through TextService::getTypesetter, PLAYER 0x0073a7e0).
	// STUB: PLAYER 0x006c0880
	// STUB: STUDIO 0x00853800
	G3D::Vector2 TextLabel::getTextBounds() const
	{
		STUB(0x006c0880);
		return G3D::Vector2::zero();
	}

	// TODO: body (as getTextBounds, then compares the measured width with getRect2D's).
	// STUB: PLAYER 0x006c09c0
	// STUB: STUDIO 0x00853940
	bool TextLabel::getTextFits() const
	{
		STUB(0x006c09c0);
		return false;
	}

	// FUNCTION: PLAYER 0x006c1c10
	// FUNCTION: STUDIO 0x00854b20
	TextLabel::TextLabel()
		: Super("TextLabel")
		, GuiTextMixin("Label", BrickColor(BrickColor::brick_26).color3())
	{
		setGuiQueue(GUIQUEUE_TEXT);
	}

} // namespace RBX

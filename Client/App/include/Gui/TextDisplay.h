/* gui/TextDisplay.h -- October 2013 C++-only widget (NO reflection).
   Evidence: N (no Described; GuiItem base; vtable 0xEEFDF0); C2(title,label) 0x5CF04A (label at +112); NO C2(void) in binary; init (180x30/font18/purple/clear) TODO.
   TODO: full decompile (member map + init/render/process bodies).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/TextDisplay.h */

#pragma once

#include "V8DataModel/Item.h"
#include <string>

namespace RBX {

	class TextDisplay
		: public GuiItem
	{
	public:
		// C2(title,label) 0x5CF04A: title -> virtual setName, label at +112.
		// textColor inlined as (0.7,0,1.0) == Color3::purple() per Color3.cpp
		// (calling purple() would link Color3.cpp whose static init pulls
		// Crypto::crc32 -> bare zlib.h, absent from Contribs — link-debt trap).
		// +104/+108 reuse the INHERITED GuiItem size slots (init overwrites 0,0).
		TextDisplay(const std::string& title, const std::string& label)
			: labelText(label)
			, fontSize(18), textColor(G3D::Color3(0.7f, 0.0f, 1.0f)), textAlpha(1.0f)
			, backdrop(G3D::Color4::clear()), xAlign(1), visible(true)
		{
			setName(title);
			sizeX = 180.0f;
			sizeY = 30.0f;
		}
		const std::string& getLabel() const { return labelText; }  // 0x5CC03C (semantic-certain).
		virtual int getFontSize() const { return fontSize; }
		virtual bool isVisible() const { return visible; }
		virtual void render2d(Adorn* adorn);  // 0x5CF0F2 (80B, batch787; body in gui/GUI.cpp; the slot-41 override)
		// Bring-up views for the updateSummaryStats tests (binary reads the
		// members directly).
		G3D::Color3 getTextColorForTest() const { return textColor; }
		float getTextAlphaForTest() const { return textAlpha; }
	friend class GuiBuilder;  // restyle loop (buildCustomStats:1172) writes members.
	protected:
		std::string labelText;  // +112.
		int fontSize;  // +116 (18).
		G3D::Color3 textColor;  // +120 (purple).
		float textAlpha;  // +132 (1.0, forms Color4 fg).
		G3D::Color4 backdrop;  // +136..151 (clear; restyle target, was unmapped).
		int xAlign;  // +152 (1).
		bool visible;  // +156.
	};

} // namespace RBX


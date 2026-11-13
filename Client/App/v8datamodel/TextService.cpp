/* App/v8datamodel/TextService.cpp -- October 2013. The typesetter registry. The 2013 asserts put

   FromTextFont's assert "0" at line 65, registerTypesetter's at 128 and getTypesetter's at 134

   (three more assert "0" converters sit at 79/95/109 and are not reconstructed yet). */
#include "stdafx.h"
#include "V8DataModel/TextService.h"
#include "rbx/Debug.h"
namespace RBX {
	const char* const sTextService = "TextService";














































	// mac: 0x5F9896 (115B): the Text::Font fence convert; the unsigned `cmp esi,5 / jb` is the
	// unsigned-cast compare, and the overflow arm returns FONT_0 after asserting "0".
	// STUB: PLAYER 0x0073a620
	// STUB: STUDIO 0x00884d30
	TextService::Font TextService::FromTextFont(Text::Font font)
	{
		if (static_cast<unsigned int>(font) >= 5)
		{
			RBXASSERT(0);
			return FONT_0;
		}
		return static_cast<Font>(font);
	}





















































	// mac: 0x5F9D02: assert, then shared_ptr<Typesetter>::operator=(const&) into typesetters[font].
	// FUNCTION: PLAYER 0x0073a860
	// FUNCTION: STUDIO 0x00884fa0
	void TextService::registerTypesetter(Font font, boost::shared_ptr<Typesetter> typesetter)
	{
		RBXASSERT(font < FONT_LAST);
		typesetters[font] = typesetter;
	}
	// FUNCTION: PLAYER 0x0073a7e0
	// FUNCTION: STUDIO 0x00884f20
	boost::shared_ptr<Typesetter> TextService::getTypesetter(Font font) {
		RBXASSERT(font < FONT_LAST);
		return typesetters[font];
	}

	// mac: 0x5F9C4C: operator new[](0x2C) = the 4-byte count header + 5 zero {px, ctrl} pairs; the
	// +0x60 pointer swaps first, then the old base is delete[]d.
	void TextService::clearTypesetters()
	{
		boost::shared_ptr<Typesetter>* fresh =
			new boost::shared_ptr<Typesetter>[FONT_LAST];
		boost::shared_ptr<Typesetter>* old = typesetters;
		typesetters = fresh;
		delete[] old;
	}

} // namespace RBX

// Class registrar (see Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::TextService);
}

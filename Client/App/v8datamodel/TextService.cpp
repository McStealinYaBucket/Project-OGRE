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

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0073aa20
// FUNCTION: STUDIO 0x00885160
template<>
EnumDesc<TextService::FontSize>::EnumDesc()
	: EnumDescriptor("FontSize", typeid(TextService::FontSize))
{
	addPair(TextService::SIZE_8, "Size8");
	addPair(TextService::SIZE_9, "Size9");
	addPair(TextService::SIZE_10, "Size10");
	addPair(TextService::SIZE_11, "Size11");
	addPair(TextService::SIZE_12, "Size12");
	addPair(TextService::SIZE_14, "Size14");
	addPair(TextService::SIZE_18, "Size18");
	addPair(TextService::SIZE_24, "Size24");
	addPair(TextService::SIZE_36, "Size36");
	addPair(TextService::SIZE_48, "Size48");
}

template<>
const Type& Type::getSingleton<TextService::FontSize>()
{
	return EnumDesc<TextService::FontSize>::singleton();
}

template<> EnumRegistrar<TextService::FontSize> EnumRegistrar<TextService::FontSize>::registrar(0);
template<> TypeRegistrar<TextService::FontSize> TypeRegistrar<TextService::FontSize>::registrar(0);

// FUNCTION: PLAYER 0x0073aba0
// FUNCTION: STUDIO 0x008852e0
template<>
EnumDesc<TextService::Font>::EnumDesc()
	: EnumDescriptor("Font", typeid(TextService::Font))
{
	addPair(TextService::FONT_0, "Legacy");
	addPair(TextService::FONT_1, "Arial");
	addPair(TextService::FONT_2, "ArialBold");
	addPair(TextService::FONT_3, "SourceSans");
	addPair(TextService::FONT_4, "SourceSansBold");
}

template<>
const Type& Type::getSingleton<TextService::Font>()
{
	return EnumDesc<TextService::Font>::singleton();
}

template<> EnumRegistrar<TextService::Font> EnumRegistrar<TextService::Font>::registrar(0);
template<> TypeRegistrar<TextService::Font> TypeRegistrar<TextService::Font>::registrar(0);

// FUNCTION: PLAYER 0x0073ace0
// FUNCTION: STUDIO 0x00885420
template<>
EnumDesc<TextService::XAlignment>::EnumDesc()
	: EnumDescriptor("TextXAlignment", typeid(TextService::XAlignment))
{
	addPair(TextService::XALIGNMENT_LEFT, "Left");
	addPair(TextService::XALIGNMENT_CENTER, "Center");
	addPair(TextService::XALIGNMENT_RIGHT, "Right");
}

template<>
const Type& Type::getSingleton<TextService::XAlignment>()
{
	return EnumDesc<TextService::XAlignment>::singleton();
}

template<> EnumRegistrar<TextService::XAlignment> EnumRegistrar<TextService::XAlignment>::registrar(0);
template<> TypeRegistrar<TextService::XAlignment> TypeRegistrar<TextService::XAlignment>::registrar(0);

// FUNCTION: PLAYER 0x0073ae00
// FUNCTION: STUDIO 0x00885540
template<>
EnumDesc<TextService::YAlignment>::EnumDesc()
	: EnumDescriptor("TextYAlignment", typeid(TextService::YAlignment))
{
	addPair(TextService::YALIGNMENT_TOP, "Top");
	addPair(TextService::YALIGNMENT_CENTER, "Center");
	addPair(TextService::YALIGNMENT_BOTTOM, "Bottom");
}

template<>
const Type& Type::getSingleton<TextService::YAlignment>()
{
	return EnumDesc<TextService::YAlignment>::singleton();
}

template<> EnumRegistrar<TextService::YAlignment> EnumRegistrar<TextService::YAlignment>::registrar(0);
template<> TypeRegistrar<TextService::YAlignment> TypeRegistrar<TextService::YAlignment>::registrar(0);

} // namespace Reflection
} // namespace RBX

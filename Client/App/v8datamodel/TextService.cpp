/* App/v8datamodel/TextService.cpp -- October 2013 (NEW reconstruction
   file).  The typesetter registry: getTypesetter 0x5F9D8A (147B) /
   registerTypesetter 0x5F9D02 / clearTypesetters 0x5F9C4C (rarc24 +
   rarc27 raws).  The binary gates the table walks with ReleaseAssert
   ("font < FONT_LAST") at file lines 128/134 -- FLog::Asserts-gated
   with _debugHook; the landed RBXASSERT carries the same expression
   (DELTA in the expansion shape). */

#include "stdafx.h"

#include "V8DataModel/TextService.h"
#include "rbx/Debug.h"

namespace RBX {

	const char* const sTextService = "TextService";

	// mac: 0x5F9D8A (147B): assert, then deref *(this+0x60) and copy the
	// {px, ctrl} pair at [base + 8*font]; when ctrl != 0 the +4 use
	// count is atomically incremented (the GCC-inlined shared_ptr copy
	// ctor; the landed by-value return expands to the same inc when
	// the caller copies the slot).
	// STUB: PLAYER 0x0073a7e0
	// STUB: STUDIO 0x00884f20
	boost::shared_ptr<Typesetter> TextService::getTypesetter(Font font)
	{
		RBXASSERT(font < FONT_LAST);
		return typesetters[font];
	}

	// mac: 0x5F9D02: assert, then shared_ptr<Typesetter>::operator=(const&)
	// mac: 0x5FABBE into typesetters[font].
	// STUB: PLAYER 0x0073a860
	// STUB: STUDIO 0x00884fa0
	void TextService::registerTypesetter(Font font,
		boost::shared_ptr<Typesetter> typesetter)
	{
		RBXASSERT(font < FONT_LAST);
		typesetters[font] = typesetter;
	}

	// mac: 0x5F9C4C: operator new[](0x2C) = the 4-byte count header + 5
	// zero {px, ctrl} pairs; the +0x60 pointer swaps first, then the
	// old base releases each pair's control block in order (the
	// count-guarded GCC array destroy) and delete[]s back through the
	// header.
	void TextService::clearTypesetters()
	{
		boost::shared_ptr<Typesetter>* fresh =
			new boost::shared_ptr<Typesetter>[FONT_LAST];
		boost::shared_ptr<Typesetter>* old = typesetters;
		typesetters = fresh;
		delete[] old;
	}

	// mac: 0x5F9896 (115B): the Text::Font fence convert.  The unsigned
	// `cmp esi,5 / jb` (rarc31 raw 0x5F98A8-AB) = the C++ unsigned-cast
	// compare; the overflow arm zero-folds the return and fires the
	// shipped RBXASSERT(0) -- its expansion string reads "0 file:
	// .../Client/App/v8datamodel/TextService.cpp line: 65" (the shipped
	// source expression was "0").  DELTA: the landed RBXASSERT expands
	// through the corpus Debug.h (the _debugHook dance rides the macro,
	// not this line).
	// STUB: PLAYER 0x0073a620
	// STUB: STUDIO 0x00884d30
	TextService::Font TextService::FromTextFont(Text::Font font)
	{
		if (static_cast<unsigned int>(font) >= 5)
		{
			RBXASSERT(0);
			return FONT_0;
		}
		// The enum-to-enum convert is explicit in C++ (the decomp's raw
		// unsigned identity ride = the same bit pattern end to end).
		return static_cast<Font>(font);
	}

} // namespace RBX

// Class registrar (see Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::TextService);
}

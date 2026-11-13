/* V8DataModel/TextService.h -- October 2013 (NEW reconstruction file;
   the text-rendering service: a per-font Typesetter registry the chat
   bubbles measure their lines through).
   Evidence (rarc23/24/25/27):
     __ZN3RBX11TextService13getTypesetterENS0_4FontE @0x5F9D8A (147B):
       ReleaseAssert "font < FONT_LAST" (file
       /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/
       v8datamodel/TextService.cpp line 134), then the manual count-
       incrementing copy of typesetters[font] (the GCC-inlined
       shared_ptr copy ctor: lock inc [ctrl+4]).
     registerTypesetter @0x5F9D02 (assert at file line 128), then
       shared_ptr<Typesetter>::operator=(const&) 0x5FABBE into the
       table slot.
     clearTypesetters @0x5F9C4C: operator new[](0x2C) = the 4-byte
       count header + 5 zero {px, ctrl} pairs, the +0x60 pointer
       swaps, then the old base releases each pair's control block
       (count-guarded, the GCC array destroy) and delete[]s back
       through the header.
     this+0x60 = a POINTER to the 5x8B array (mov edx,[ecx+60h], not
       lea -- rarc27); GCC shared_ptr = {T* px; sp_counted_base* pn}.
     Reflection: Described<TextService, sTextService, NonFactoryProduct
       <Instance, sTextService>, Functionality 1 = INTERNAL_LOCAL,
       Permissions 0> (rarc25) -- exercised by the
       ServiceProvider::create<TextService> instantiation inside
       ChatOutput::renderBubbles 0x5C4EC0 (the size pass).
   Font enum: FONT_LAST = 5 pinned by the assert; the EnumDesc<Font>
     converters are table-driven -- the 0..4 enumerator names are
     unobservable (DELTA: names invented).
   C2 ctor 0x5F9A86 (rarc30, probed 2026-10-22): Instance base (NULL
     FWInstance) + the Described vptr fragments (off_F82E28 family),
     ++ClassRegistrar (rides RBX_REGISTER_CLASS), byte +92/0x5C = 1,
     +0x60 = NULL, then setName("TextService") (the std::string temp
     + its _Rep release) and the tail CALL clearTypesetters(this)
     at 0x5F9B4C (rarc30.txt decomp line 35).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/
   Client/App/include/V8DataModel/TextService.h */

#pragma once

#include "Reflection/Reflection.h"
#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "GfxBase/Typesetter.h"

#include "Gui/Adorn.h"

namespace RBX {

	extern const char* const sTextService;

	class TextService
		: public DescribedNonCreatable<TextService, Instance, sTextService,
			Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		// Values from GuiTextMixin's ctor (PLAYER 0x006be930): xAlignment = 2 (center), yAlignment = 1
		// (center), fontSize = 0. Names from the 2016 TextService.h; the other FontSize values are unproven.
		enum XAlignment
		{
			XALIGNMENT_LEFT = 0,
			XALIGNMENT_RIGHT = 1,
			XALIGNMENT_CENTER = 2
		};

		enum YAlignment
		{
			YALIGNMENT_TOP = 0,
			YALIGNMENT_CENTER = 1,
			YALIGNMENT_BOTTOM = 2
		};

		enum FontSize
		{
			SIZE_8 = 0, SIZE_9, SIZE_10, SIZE_11, SIZE_12, SIZE_14, SIZE_18, SIZE_24, SIZE_36, SIZE_48  // EnumDesc<FontSize> 0x0073aa20
		};

		// FONT_LAST = 5 pinned by the "font < FONT_LAST" asserts.
		enum Font
		{
			FONT_0 = 0,   // the register/get arc font (DELTA name)
			FONT_1,
			FONT_2,
			FONT_3,
			FONT_4,
			FONT_LAST
		};

		// mac: 0x5F9A86 C2 (rarc30, probed 2026-10-22): the base machinery
		// (Instance(NULL FWInstance), the Described vptr fragments, the
		// registrar increment via RBX_REGISTER_CLASS) rides the corpus
		// class plumbing.  Observable stores: byte +92/0x5C = 1 (the
		// ChatService.h:42 flagDelta0x5C precedent), +0x60 = NULL, then
		// setName("TextService") (the string temp + its _Rep release --
		// rarc30 raw 0x5F9B24-76) and the tail CALL clearTypesetters(this)
		// at 0x5F9B4C (decomp line 35): new[FONT_LAST] / swap the +0x60
		// pointer / delete[] the NULL -- ONE live allocation, the landed
		// init-list NULL mirrors the binary's +0x60 zero (decomp line 26).
		// STUB: PLAYER 0x0073b010
		// STUB: STUDIO 0x008856d0
		TextService()
			: flagDelta0x5C(true)
			, typesetters(NULL)
		{
			Instance::setName("TextService");
			clearTypesetters();
		}

		// mac: 0x5F9D8A (147B): assert, then copy typesetters[font] out
		// (count-incrementing; GCC inlines the copy ctor).
		boost::shared_ptr<Typesetter> getTypesetter(Font font);

		// mac: 0x5F9D02: assert (file line 128), then the op= into the
		// table slot (the value param addressed as const& inside).
		void registerTypesetter(Font font,
			boost::shared_ptr<Typesetter> typesetter);

		// mac: 0x5F9C4C: new[FONT_LAST], swap the +0x60 pointer,
		// delete[] old (the guarded release walk).
		void clearTypesetters();

		// mac: 0x5F9896 (115B): the Text::Font identity convert the view
		// font bootstraps feed (FromTextFont(i) -> registerTypesetter,
		// rarc30 bootstrap 1).  The mangle
		// __ZN3RBX11TextService12FromTextFontENS_4Text4FontE pins the
		// Text::Font param (Adorn.h); the return rides the service-own
		// Font.  The unsigned `cmp 5 / jb` fence (rarc31 raw
		// mac: 0x5F98A8-AB) folds out-of-range to FONT_0 and fires the
		// shipped RBXASSERT(0) -- its expansion string is literally
		// "0" at TextService.cpp line 65.
		static Font FromTextFont(Text::Font font);

	private:
		// +92/0x5C: ctor-true byte (rarc30 C2 decomp line 21; DELTA name
		// ours -- ChatService.h pins the same byte public, access is
		// unobservable in the binary).
		bool flagDelta0x5C;

		// this+0x60: a POINTER to the 5x8B {px, ctrl} array (not a
		// vector -- rarc27's new[](0x2C) = 4b count + 5 pairs).  C2
		// zeroes it (decomp line 26) then the tail clearTypesetters
		// call swaps the fresh array in.
		boost::shared_ptr<Typesetter>* typesetters;
	};

} // namespace RBX

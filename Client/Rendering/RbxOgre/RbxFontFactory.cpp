/* Rendering/RbxOgre/RbxFontFactory.cpp -- October 2013 render board
   (NEW reconstruction TU).  The per-font TypesetterBitmap factory
   getter the view bootstraps scatter registerTypesetter through.

   Binary evidence (rarc31):
     sub_7C19BA @0x7C19BA (115B): the assert-string PINS the source
       shapes -- RBXASSERT "font < Text::FONT_LAST" files itself at
       .../Client/Rendering/RbxOgre/RbxFontFactory.cpp line 25, so the
       shipped TU name, the param name ('font') and the enum member
       (Text::FONT_LAST) are ground truth.  The body = the raw row
       step: `return base + 8 * font` -- the caller derefs it as a
       boost shared_ptr pair row ({px, ctrl}, the GCC 8B stride) and
       copy-increments the slot straight from the row: BOTH view
       bootstraps land every Text::Font 0..4 the same way (rarc30
       bootstrap 1 raw 0x7B8C43-0x7B8CB2 / bootstrap 2's mirror).

   DELTAs: the factory class itself (whose +156 dword aims at the row
     base the getter receives) stays unprobed; the symbol name is
     unresolved (the sub_7C19BA anon); this spell floats the getter as
     a file-static since nothing outside the TU references it until
     the view bootstraps land.
   FLog::Asserts-gated expansion = the RBXASSERT/_debugHook shape
     (DELTA in the vinyl; the landed macro carries it). */

#include "stdafx.h"

#include "GfxBase/TypesetterBitmap.h"

#include "Gui/Adorn.h"

#include "rbx/Debug.h"

#include <boost/shared_ptr.hpp>

namespace RBX {

	/* mac: 0x7C19BA (115B): the slot getter.  rows = the shared_ptr pair
	   row base (the factory owner derefs its +156 dword); font = the
	   unsigned fence prior (the raw `cmp esi,5; jl` pre-assert, the
	   SIGNED jump = the shipped int compare). */
	// STUB: PLAYER 0x004c10b0
	static boost::shared_ptr<Typesetter>& fontFactorySlot(
		boost::shared_ptr<Typesetter>* rows, int font)
	{
		RBXASSERT(font < Text::FONT_LAST);
		return rows[font];
	}

} // namespace RBX

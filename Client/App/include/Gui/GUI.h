/* include/gui/GUI.h -- October 2013 (NEW landing; the GuiItem gui-member
   inline annex + Canvas).  Assert anchors from the binary:
     - GuiItem::getPosition asserts getGuiParent() at GUI.h:64 (0x4D8490
       calls getGuiParent, then dispatches the parent's vt+140)
     - GuiItem::getChildPosition asserts 0 at GUI.h:69 (0x37AD10 calls
       G3D::Vector2::zero and returns its two floats).
   Fidelity ~99%; the out-of-line bodies live in App/gui/GUI.cpp (family
   assert file).  DELTA: Canvas field names ours (8B floats shape fixed,
   names unobservable).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/Gui/GUI.h */

#pragma once

#include <math.h>

#include "Util/G3DCore.h"
#include "Util/Rect.h"
#include "V8DataModel/Item.h"

namespace RBX {

	// 8-byte by-value argument (6CanvasE mangling); the getMyRect body
	// copies it and passes stack copies into the vt+136/vt+156 virtuals.
	// DELTA: x/y names ours (unobservable in the binary).
	class Canvas
	{
	public:
		float x;
		float y;
	};

	// Non-assert members (bodies reconstructed from the out-of-line
	// symbols 0x5CD2C4/0x3EC564; binary Rect order = x0y0/x1y1, which maps
	// onto our Util/Rect.h low/high layout).
	// STUB: PLAYER 0x00768200
	// STUB: STUDIO 0x007b3ca0
	inline Rect GuiItem::getMyRect(Canvas canvas) const
	{
		Vector2 position = getPosition(canvas);		// vt+136 (slot 34)
		Vector2 size = getSize(canvas);				// vt+156 (slot 39; ignores canvas)
		return Rect(position.x, position.y, position.x + size.x, position.y + size.y);
	}
	// getMyRect2D (0x3EC564): min/max canonicalization (the binary calls
	// fminf/fmaxf; VS2012 lacks the C99 pair, so the ternaries here are
	// the identical min/max selects).
	inline Rect GuiItem::getMyRect2D(Canvas canvas) const
	{
		Rect r = getMyRect(canvas);
		return Rect(r.low.x < r.high.x ? r.low.x : r.high.x,
					r.low.y < r.high.y ? r.low.y : r.high.y,
					r.low.x < r.high.x ? r.high.x : r.low.x,
					r.low.y < r.high.y ? r.high.y : r.low.y);
	}
	// The binary expanded getPosition/getChildPosition from this header:
	// the debugHook strings say "include/Gui/GUI.h" at :64/:69, and the
	// verbatim expressions are #expr-generated.  The bodies are out-of-
	// line in App/gui/GUI.cpp (MSVC needs one addressable definition per
	// vtable slot); these hooks keep the RBXASSERT expansion site exactly
	// here, so #expr stays "getGuiParent()" / "0" and __FILE__/__LINE__
	// stay include/Gui/GUI.h:64/:69.
	// DELTA (documented): two hook members absent from the binary symbol
	// table (mechanism, not semantics); the GUI.cpp bodies open with them.
	inline void GuiItem::assertGetGuiParentHook() const {
		RBXASSERT(getGuiParent());					// binary: include/Gui/GUI.h:64
	}


	inline void GuiItem::assertChildPosZeroHook() const {
		RBXASSERT(0);								// binary: include/Gui/GUI.h:69
	}

}  // namespace RBX

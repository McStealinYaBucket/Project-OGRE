/* App/gui/GUI.cpp -- October 2013 (thirteenth-era render family on the
   twelfth-era landing: the GuiItem focus
   machinery + the UnifiedWidget process family).  Evidence: batch781
   (InTitle :503 / processShown / OutOfTitle :619/:635 / Hover :532 /
   Nothing :561), batch783 (base vt dump), batch784 (getGuiParent / getTitle /
   getClassName / firstChildPosition / childOffset / getChildPosition :463 /
   GuiItem::process :118 / UnifiedWidget::process), batch785 (processNonFocus
   / loseFocus / getGuiItem / askAddChild / onDescendantRemoving / focus +
   render family).  Assert anchors (binary line numbers, blank padding):
   118 focus == NULL (process 0x5CD47E), 463 "0" (getChildPosition
   0x5CE6B8), 503 menuState == SHOWN_APPEARING (InTitle 0x5CE7FA), 532
   menuState == HOVER (Hover 0x5CEBF6), 561 menuState == NOTHING (Nothing
   0x5CED6A), 619 getFocus() == NULL / 635 "0" (OutOfTitle 0x5CE982).
   Fidelity ~99%.  DELTAS: OutOfTitle +112 menuState via
   getMenuStateForTest/setMenuState (single-inh downcast); canLoseFocus
   non-const per the mangles; getPosition/getChildPosition hooks in GUI.h
   (HOOK DELTA) + the render era landed (Adorn vt F0D9C0, pure names DELTA).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/GUI.cpp */

#include "stdafx.h"
#include "Gui/GuiEvent.h"
#include "Gui/GUI.h"
#include "Gui/UnifiedWidget.h"
#include "Gui/TextDisplay.h"

namespace RBX {

	// -- GuiItem support members --

	// mac: 0x5CD184 (84B, batch784; const per mangle __ZNK3RBX7GuiItem12getGuiParentEv):
	// returns the Instance-tree parent when it isA<GuiItem>.
	GuiItem* GuiItem::getGuiParent() const
	{
		Instance* parentInstance = const_cast<Instance*>(getParent()); // the raw +0x34 parent read (5CD199)
		if (parentInstance != NULL && Reflection::DescribedBase::isA<GuiItem>(parentInstance))
		{
			return (GuiItem*)parentInstance;
		}
		return NULL;
	}

	// mac: 0x5CD1D8 / 0x5CD23C (batch785): children[index] + isA<GuiItem>.
	// The binary reads the vector raw (no bound check); getChild(vector
	// operator[]) is the same unchecked read.
	GuiItem* GuiItem::getGuiItem(int index)
	{
		Instance* child = getChild((size_t)index);
		if (child != NULL && Reflection::DescribedBase::isA<GuiItem>(child))
		{
			return (GuiItem*)child;
		}
		return NULL;
	}

	const GuiItem* GuiItem::getGuiItem(int index) const
	{
		const Instance* child = getChild((size_t)index);
		if (child != NULL && Reflection::DescribedBase::isA<GuiItem>(child))
		{
			return (const GuiItem*)child;
		}
		return NULL;
	}

	// mac: 0x5CF196 (61B, batch785): notify slot 32 when the +96 px is live,
	// then write the px to 0 and release the +100 control block -- the
	// same write-then-release order a shared_ptr reset performs.
	void GuiItem::loseFocus()
	{
		if (focus.get() != NULL)
		{
			focus->onLoseFocus();               // vt+0x80 (slot 32)
		}
		focus.reset();
	}
	// mac: 0x3EC670 (42B, batch784): copies the FWInstance name string at
	// fw()+24 (no NULL gate on fw(), like the binary).
	// STUB: PLAYER 0x006c8c10
	// STUB: STUDIO 0x007aa5c0
	std::string GuiItem::getTitle()
	{
		FWInstance* fwInstance = (FWInstance*)fw();
		return fwInstance->name.value;
	}
	// GuiItem::process 0x5CD47E (332B, batch784) -- the base click router:
	// type 7 short-circuits zero; the rest walks the focus chain.
	// STUB: PLAYER 0x00769440
	// STUB: STUDIO 0x007b4ee0
	GuiResponse GuiItem::process(const GuiEvent& e)
	{
		if ((unsigned int)(e.eventType - 1) <= 8 && e.eventType == 7)
		{
			return GuiResponse();
		}
		// With a live focus: the focused child gets the first crack when
		// it can lose focus (its own slot-33 gate), then the focused
		// widget itself (vt+0xA0), then the focus is dropped and the
		// event retries through the children walk below.
		if (focus.get() != NULL)
		{
			if (focus->canLoseFocus())          // vt+0x84 (slot 33)
			{
				GuiResponse r = processNonFocus(e);
				if (r.wasSunk())
				{
					return r;
				}
			}
			// The focus item's own process (vt+0xA0, slot 40).  Sunk is
			// final; otherwise the focus is dropped at 5CD544.
			GuiResponse r = focus->process(e);
			if (r.wasSunk())
			{
				return r;
			}
			loseFocus();
		}
		RBXASSERT(focus == NULL);               // binary: App/gui/GUI.cpp:118
		return processNonFocus(e);
	}

	// ---- GuiItem::processNonFocus (0x5CD370, 270B, batch785) ----
	// First non-focus child that sinks the event takes the focus:
	// loseFocus(this), the shared_from hand-off (op= releases the old
	// block), then the NEW focus's own teardown (5CD440).
	GuiResponse GuiItem::processNonFocus(const GuiEvent& e)
	{
		int n = numChildren();
		if (n != 0)
		{
			for (int i = 0; i < n; ++i)
			{
				GuiItem* child = getGuiItem(i);
				if (child != NULL && child != focus.get())
				{
					GuiResponse r = child->process(e);    // vt+0xA0 (slot 40)
					if (r.wasSunk())
					{
						loseFocus();                      // this (5CD403)
						focus = shared_from(child);       // the hand-off op=
						child->loseFocus();               // the new focus (5CD440)
						return r;
					}
				}
			}
		}
		return GuiResponse();
	}

	// ---- UnifiedWidget::getChildPosition (0x5CE6B8, 274B, batch784) ----
	// Base = firstChildPosition (vt+0xAC); one childOffset step (vt+0xB0)
	// per skipped child; misses the found child only when no GuiItem
	// child matches, which is the asserted misuse ("0" :463).
	// STUB: PLAYER 0x00769f00
	// STUB: STUDIO 0x007b59a0
	Vector2 UnifiedWidget::getChildPosition(const GuiItem* child, Canvas canvas) const
	{
		Vector2 result = firstChildPosition(canvas);
		int n = numChildren();
		if (n != 0)
		{
			for (int i = 0; i < n; ++i)
			{
				const GuiItem* gi = getGuiItem(i);
				if (gi != NULL)
				{
					if (gi == child)
					{
						return result;
					}
					Vector2 offset = childOffset();
					result.x += offset.x;
					result.y += offset.y;
				}
			}
			// The binary loops while (i < numChildren()), re-reading the
			// count per pass; children cannot change inside this walk, so
			// the hoisted n is behaviorally identical.
		}



























































































































































































































































































		RBXASSERT(0);                           // binary: App/gui/GUI.cpp:463
		return result;
	}
	// ---- UnifiedWidget::processShown family (batch781) ----
	// mac: 0x5CE8D2 (175B): canvas from the e+8/+10 shorts, getMyRect, then the
	// in-title interval test on the e+4 short pair (LO=x HI=y, the same
	// ucomiss jb directions as the binary).
	// STUB: PLAYER 0x0076a360
	// STUB: STUDIO 0x007b5d80
	GuiResponse UnifiedWidget::processShown(const GuiEvent& e)
	{
		Canvas canvas;
		canvas.x = (float)e.canvasX;
		canvas.y = (float)e.canvasY;
		Rect rect = getMyRect(canvas);
		float mx = (float)e.mouseX;
		float my = (float)e.mouseY;
		if (mx >= rect.low.x && rect.high.x >= mx && my >= rect.low.y && rect.high.y >= my)
		{
			return processShown_InTitle(e);
		}
		return processShown_OutOfTitle(e);
	}
	// mac: 0x5CE7FA (215B, assert "menuState == SHOWN_APPEARING" :503): the
	// press LOUD: SHOWN demotes to HOVER; SHOWN_APPEARING promotes to SHOWN.
	// STUB: PLAYER 0x00768d40
	// STUB: STUDIO 0x007b47e0
	GuiResponse UnifiedWidget::processShown_InTitle(const GuiEvent& e)
	{
		loseFocus();
		if (menuState == SHOWN)
		{
			if (e.eventType == 4)
			{
				menuState = HOVER;
				onMenuStateChanged();
			}
		}
		else
		{
			RBXASSERT(menuState == SHOWN_APPEARING); // binary: App/gui/GUI.cpp:503
			if (e.eventType == 4 && menuState != SHOWN)
			{
				menuState = SHOWN;
				onMenuStateChanged();
			}
		}
		return GuiResponse::sunk();
	}

	// mac: 0x5CEBF6 (371B, assert "menuState == HOVER" :532): in-rect press
	// walks SHOWN, release walks SHOWN_APPEARING, every other type just
	// sinks; out-of-rect demotes to NOTHING.
	// STUB: PLAYER 0x00768e10
	// STUB: STUDIO 0x007b48b0
	GuiResponse UnifiedWidget::processHover(const GuiEvent& e)
	{












		RBXASSERT(menuState == HOVER);          // binary: App/gui/GUI.cpp:532
		loseFocus();
		Canvas canvas;
		canvas.x = (float)e.canvasX;
		canvas.y = (float)e.canvasY;
		Rect rect = getMyRect(canvas);
		float mx = (float)e.mouseX; float my = (float)e.mouseY;
		if (mx >= rect.low.x && rect.high.x >= mx && my >= rect.low.y && rect.high.y >= my)
		{
			if (e.eventType == 4 && menuState != SHOWN)
			{
				menuState = SHOWN;
				onMenuStateChanged();
			}
			else if (e.eventType == 3 && menuState != SHOWN_APPEARING)
			{
				menuState = SHOWN_APPEARING;
				onMenuStateChanged();
			}
			return GuiResponse::sunk();
		}
		if (menuState != NOTHING) { menuState = NOTHING; onMenuStateChanged(); }
		return GuiResponse();
	}
	// mac: 0x5CED6A (374B, assert "menuState == NOTHING" :561): in-rect press/release state walks; out-of-rect demotes.
	// STUB: PLAYER 0x00768fb0
	// STUB: STUDIO 0x007b4a50
	GuiResponse UnifiedWidget::processNothing(const GuiEvent& e)
	{
		RBXASSERT(menuState == NOTHING);        // binary: App/gui/GUI.cpp:561
		loseFocus();
		Canvas canvas;
		canvas.x = (float)e.canvasX;
		canvas.y = (float)e.canvasY;
		Rect rect = getMyRect(canvas);
		float mx = (float)e.mouseX;
		float my = (float)e.mouseY;
		bool inTitle = (mx >= rect.low.x && rect.high.x >= mx && my >= rect.low.y && rect.high.y >= my);
		if (!inTitle)
		{
			if (menuState != NOTHING)
			{
				menuState = NOTHING;
				onMenuStateChanged();
			}
			return GuiResponse();
		}
		if ((unsigned int)(e.eventType - 3) < 2 && menuState != SHOWN_APPEARING)
		{
			menuState = SHOWN_APPEARING;
			onMenuStateChanged();
		}
		else if (e.eventType == 5 && menuState != HOVER)
		{
			menuState = HOVER;
			onMenuStateChanged();
		}
		else if ((unsigned int)(e.eventType - 3) >= 2 && e.eventType != 5)
		{
			return GuiResponse();
		}
		return GuiResponse::sunk();
	}
	// mac: 0x5CE982 (628B, asserts :619/:635, batch781): the out-of-title machine.
	// STUB: PLAYER 0x0076a090
	// STUB: STUDIO 0x007b5b30
	GuiResponse UnifiedWidget::processShown_OutOfTitle(const GuiEvent& e)
	{
		GuiItem* focusMenu = NULL;
		GuiItem* f = getFocus();
		if (f != NULL && Reflection::DescribedBase::isA<GuiItem>(f))
		{
			focusMenu = f;
		}
		int type = e.eventType;
		if ((unsigned int)(type - 3) < 2)
		{
			GuiResponse r = GuiItem::process(e);
			if (r.wasSunk())
			{
				if (r.wasSunkAndFinished())
				{
					loseFocus();
					if (menuState != NOTHING) { menuState = NOTHING; onMenuStateChanged(); }
				}
				return r;
			}
			RBXASSERT(getFocus() == NULL);      // binary: App/gui/GUI.cpp:619
			if (menuState != NOTHING) { menuState = NOTHING; onMenuStateChanged(); }
			return r;
		}
		if (type != 5)
		{










			RBXASSERT(0);                       // binary: App/gui/GUI.cpp:635
			return GuiResponse();
		}
		// The binary reads/writes the focus object's +112 dword after only
		// the GuiItem gate; our menuState sits on UnifiedWidget, so the
		// same unchecked touch goes through the single-inheritance
		// downcast, with setMenuState performing the iff-different write
		// plus the vt+0xA8 onMenuStateChanged notify.
		if (focusMenu != NULL)
		{
			int preState = ((UnifiedWidget*)focusMenu)->getMenuStateForTest();
			GuiItem::process(e);
			GuiItem* newFocus = getFocus();     // the +96 px re-read
			if (newFocus != NULL && Reflection::DescribedBase::isA<GuiItem>(newFocus)
				&& newFocus != focusMenu
				&& ((UnifiedWidget*)newFocus)->getMenuStateForTest() != preState)
			{
				((UnifiedWidget*)newFocus)->setMenuState((MenuState)preState);
			}
		}
		else
		{
			GuiItem::process(e);
		}
		return GuiResponse::sunk();
	}

	// ---- UnifiedWidget::process dispatcher (0x5CEEE0, 153B, batch784) ----

	// STUB: PLAYER 0x0076a450
	// STUB: STUDIO 0x007b5e70
	GuiResponse UnifiedWidget::process(const GuiEvent& e)
	{
		if (isVisible() && (unsigned int)(e.eventType - 1) <= 8)
		{
			int state = menuState;
			if ((unsigned int)(state - 2) < 2)
			{
				return processShown(e);
			}
			if (state == HOVER)
			{
				return processHover(e);
			}
			if (state == NOTHING)
			{
				return processNothing(e);
			}
		}
		return GuiResponse();
	}

	// mac: 0x5CE6A0 (23B, batch784): {0, 28.0f}.
	// FUNCTION: PLAYER 0x007683e0
	// FUNCTION: STUDIO 0x007b3e80
	Vector2 UnifiedWidget::childOffset() const
	{
		return Vector2(0.0f, 28.0f);
	}

	// mac: 0x5CE5AE (241B, batch784): x anchors at position + size + 4; y wraps
	// the visible rows (row pitch = (int)size.y + 4, 20pt rails, the row
	// list clamped around numChildren()/2 by the 120px canvas band).
	// STUB: PLAYER 0x00768bc0
	// STUB: STUDIO 0x007b4660
	Vector2 UnifiedWidget::firstChildPosition(Canvas canvas) const
	{
		Vector2 position = getPosition(canvas);     // vt+0x88 (slot 34)
		Vector2 size = getSize(canvas);             // vt+0x9C (slot 39)
		Vector2 result;
		result.x = (size.x + 4.0f) + position.x;
		int rowPitch = (int)size.y + 4;
		int row = ((int)position.y - 20) / rowPitch;
		int half = numChildren() / 2;
		if (half >= row)
		{
			row = half;
		}
		int visible = ((int)canvas.y - 120) / rowPitch - half;
		if (row <= visible)
		{
			visible = row;
		}
		result.y = (float)(rowPitch * (visible - half) + 20);
		return result;
	}

	// mac: 0x5CE7CA (15B): +112 = 0.
	// FUNCTION: PLAYER 0x006c8a10
	// FUNCTION: STUDIO 0x007aa320
	void UnifiedWidget::onLoseFocus()
	{
		menuState = NOTHING;
	}

	// mac: 0x5CF370 (10B): NON-const per mangle __ZN3RBX13UnifiedWidget12canLoseFocusEv.
	// FUNCTION: PLAYER 0x006be550 FOLDED
	// FUNCTION: STUDIO 0x0078ad10 FOLDED
	bool UnifiedWidget::canLoseFocus()
	{
		return true;
	}

	// The three GuiItem bodies below are placed in the tail (source order
	// is free; keeping the pre-process band inside the :118 anchor budget).

	// mac: 0x4EACE0 (83B, batch785; const per mangle
	// __ZNK3RBX7GuiItem11askAddChildEPKNS_8InstanceE): true iff the
	// candidate child isA<GuiItem>.
	// STUB: PLAYER 0x006c9fe0
	// STUB: STUDIO 0x007ad740
	bool GuiItem::askAddChild(const Instance* instance) const
	{
		return instance != NULL && Reflection::DescribedBase::isA<GuiItem>(instance);
	}

	// mac: 0x5CD2A0 (36B, batch785): drop the focus when the removed child IS
	// the focus, then the base teardown (the binary tail-jumps it).
	// FUNCTION: PLAYER 0x00768d00
	// FUNCTION: STUDIO 0x007b47a0
	void GuiItem::onDescendantRemoving(const shared_ptr<Instance>& instance)
	{
		if (instance.get() == focus.get())
		{
			loseFocus();
		}
		Instance::onDescendantRemoving(instance);
	}

	// mac: 0x4DB1F0 (32B, batch783): {sizeX, sizeY} (+104/+108); the canvas is
	// ignored (the binary's sret copy is the same 2-float shuffle).
	// FUNCTION: PLAYER 0x006c8c40
	// FUNCTION: STUDIO 0x007aa5f0
	Vector2 GuiItem::getSize(Canvas canvas) const
	{
		(void)canvas;
		return Vector2(sizeX, sizeY);
	}

	// mac: 0x5CE572 (59B, batch786): { if (isVisible()) { render2dMe(adorn); }
	// } followed by the 5CE5A2 tail-jmp to render2dChildren (a direct
	// __ZN3RBX13UnifiedWidget16render2dChildrenE target -- a non-virtual
	// call, hence the plain body here).
	// STUB: PLAYER 0x00769ed0
	// STUB: STUDIO 0x007b5970
	void UnifiedWidget::render2d(Adorn* adorn)
	{
		if (isVisible())
		{
			render2dMe(adorn);
		}
		render2dChildren(adorn);
	}

	// mac: 0x5CE51A (87B, batch787): the menuState >= 2 gate (cmp [esi+70h], 2;
	// jl -- SHOWN_APPEARING/SHOWN), then the do-while keyed to
	// numChildren() re-read per pass (5CE560-5CE567); each child's
	// non-NULL GuiItem gets the slot-41 (vt+164 = 0xA4) render2d dispatch.
	// STUB: PLAYER 0x00769dd0
	// STUB: STUDIO 0x007b5870
	void UnifiedWidget::render2dChildren(Adorn* adorn)
	{
		if (menuState >= 2 && numChildren() != 0)
		{
			int i = 0;
			do
			{
				GuiItem* item = getGuiItem(i);
				if (item != NULL)
					item->render2d(adorn);
				++i;
			} while (i < numChildren());
		}
	}

	// mac: 0x5CE32E (491B, batch786): extents +0x20 -> Canvas {f[2]-f[0],
	// f[3]-f[1]} -> getMyRect2D; fill ([a2+0x40]) with menuSelect() when
	// menuState != 0 else Color4(white,1.0f); re-extents and repeat;
	// outline ([a2+0x30]) rect + 1.0f + Color4(black,1.0f); then label2d
	// with Color4(black,1.0f) fore, Color4::clear() back and XAlign 2 on
	// the fw()+24 name string.
	// STUB: PLAYER 0x00768a00
	// STUB: STUDIO 0x007b44a0
	void UnifiedWidget::render2dMe(Adorn* adorn)
	{
		Rect extents = adorn->get2DExtents();           // 5CE34E-58
		Canvas canvas;
		canvas.x = extents.high.x - extents.low.x;
		canvas.y = extents.high.y - extents.low.y;
		Rect rect = getMyRect2D(canvas);                // 5CE37F-90 (inline GUI.h)
		G3D::Color4 fillColor;
		if (menuState != 0)                             // 5CE398-9C (+112 gate)
		{
			fillColor = menuSelect();                   // 5CE39E-3D7 (the FB8810-1C statics)
		}
		else
		{
			fillColor = G3D::Color4(G3D::Color3::white(), 1.0f);  // 5CE3D9-40A
		}
		adorn->fillRectangle(rect, fillColor);          // 5CE402-B8 [a2+0x40]

		extents = adorn->get2DExtents();                // 5CE41B-27 (re-read per pass)
		canvas.x = extents.high.x - extents.low.x;
		canvas.y = extents.high.y - extents.low.y;
		Rect rectBorder = getMyRect2D(canvas);
		const G3D::Color4 outlineColor(G3D::Color3::black(), 1.0f);  // 5CE42A-B5
		adorn->drawRectangle(rectBorder, 1.0f, outlineColor);        // 5CE486-AA, width 3F800000
		const G3D::Color4 labelColor(G3D::Color3::black(), 1.0f);    // 5CE4B0-C2
		FWInstance* fwInstance = (FWInstance*)fw();     // 5CE4C8-CC
		label2d(adorn, fwInstance->name.value, labelColor, G3D::Color4::clear(), (Text::XAlign)2);  // 5CE4CA-516
	}

	// mac: 0x5CCE1E (103B, batch787): the guarded singleton.  The binary keeps
	// the Color4 in .bss (guard __cxa_guard_acquire; the init writes
	// 3F333333 x3 + 3F800000 = 0.7/0.7/0.7/1.0); the local static plays
	// the same role (NON-const per mangle __ZN3RBX7GuiItem10menuSelectEv).
	const G3D::Color4& GuiItem::menuSelect()
	{
		static G3D::Color4 menuSelectColor(0.7f, 0.7f, 0.7f, 1.0f);
		return menuSelectColor;
	}

	// mac: 0x37AD10 (slot 35; batch785): the "0" assert is the GUI.h:69 hook;
	// the binary then returns G3D::Vector2::zero()'s two floats.
	// FUNCTION: PLAYER 0x006c89b0
	// FUNCTION: STUDIO 0x007aa2c0
	Vector2 GuiItem::getChildPosition(const GuiItem* child, Canvas canvas) const
	{
		assertChildPosZeroHook();
		(void)child; (void)canvas;
		return G3D::Vector2::zero();
	}

	// mac: 0x4D8490 (slot 34; batch783): dispatches the parent's walk.
	// STUB: PLAYER 0x006c8b90
	// STUB: STUDIO 0x007aa540
	Vector2 GuiItem::getPosition(Canvas canvas) const
	{
		assertGetGuiParentHook();
		return getGuiParent()->getChildPosition(this, canvas);
	}


	// ---- Slot-34/35 bodies out-of-line (the binary had the expansions in
	// include/Gui/GUI.h; MSVC needs one addressable definition per vtable
	// slot, so the bodies live here and open with the GUI.h assert hooks,
	// which keeps the debugHook strings at the header site). ----

	// mac: 0x5CD5F4 (387B, batch787; CONST per mangle __ZNK...): the length
	// guard (rep -12), then extents (+0x20), Canvas = {f[2]-f[0],
	// f[3]-f[1]}, getMyRect (+0x5CE65C), the center start ((high+low)*0.5f
	// both axes, C1E4BC), the XAlign arms (0 -> high.x + w*-0.1f C86268;
	// 1 -> low.x + w*0.1f CA8984; 2 -> keep the center), then the
	// [a2+0x4C] drawFont2D call (5CD717-5CD766): (string& message, start,
	// (float)getFontSize via vt+0x90, fore, back, Font 0, align, YAlign 1,
	// Vector2::zero(), the four raw -1.0f clip locals); the returned 8B
	// Vector2 is discarded (the caller pattern has no consumer).
	// STUB: PLAYER 0x007688b0
	// STUB: STUDIO 0x007b4350
	void GuiItem::label2d(Adorn* adorn, const std::string& message, const G3D::Color4& fore, const G3D::Color4& back, Text::XAlign align) const
	{
		if (message.size() != 0)
		{
			Rect extents = adorn->get2DExtents();       // 5CD627
			Canvas canvas;
			canvas.x = extents.high.x - extents.low.x;  // 5CD63C-46
			canvas.y = extents.high.y - extents.low.y;
			Rect rect = getMyRect(canvas);              // 5CD65C (inline GUI.h)
			G3D::Vector2 start((rect.high.x + rect.low.x) * 0.5f,
							   (rect.high.y + rect.low.y) * 0.5f);   // 5CD671-696
			const float width = rect.high.x - rect.low.x;             // 5CD69C-F
			if (align != (Text::XAlign)0)                             // 5CD6A3-6BD arm structure
			{
				if (align != (Text::XAlign)2)
					start.x = width * 0.1f + rect.low.x;              // CA8984 = 0.1f
			}
			else
			{
				start.x = rect.high.x + width * -0.1f;                // C86268 = -0.1f
			}
			G3D::Rect2D clip = G3D::Rect2D::xyxy(-1.0f, -1.0f, -1.0f, -1.0f);  // 5CD6F4-710 (the four raw -1.0f locals)
			(void)adorn->drawFont2D(message, start, (float)getFontSize(), fore, back,
				(Text::Font)0, align, (Text::YAlign)1,
				G3D::Vector2::zero(), clip);                          // 5CD766 [a2+0x4C]
		}
	}

	// mac: 0x5CF0F2 (80B, batch787): the isVisible gate (vt+0x94), then
	// label2d(adorn, labelText (+112), the Color4 fg view of
	// textColor(+120..131)+textAlpha(+132..135), backdrop (+136), xAlign
	// (+152)).  DELTA: the binary reads the 16-byte fg Color4 straight out
	// of the textColor/textAlpha pair; the reinterpret keeps the same
	// memory in our layout.
	// FUNCTION: PLAYER 0x00768cc0
	// FUNCTION: STUDIO 0x007b4760
	void TextDisplay::render2d(Adorn* adorn)
	{
		if (isVisible())
		{
			const G3D::Color4& fgColor = *(const G3D::Color4*)(const void*)&textColor;  // the +120..135 fg view
			label2d(adorn, labelText, fgColor, backdrop, (Text::XAlign)xAlign);
		}
	}

	// mac: 0x5CCDB6 (103B): the guarded static singleton.  The body reads the
	// __cxa guard at FB8808 and fills FB87F8..FB8804 with four float
	// 0x3F19999A writes (0.6f); returns &FB87F8.  The implicit static
	// Color4 init under MSVC has the same shape; the receiver is never
	// read.  Consumed by the classic chat painter (0x5C4382/0x5C43F5 in
	// render2d_classicStyle) and ten GuiItem-family render sites.
	const G3D::Color4& GuiItem::translucentBackdrop()
	{
		static G3D::Color4 backdrop(0.6f, 0.6f, 0.6f, 0.6f);  // 0x3F19999A x4
		return backdrop;
	}

	// GuiTarget's default handler: an unsunk, unfinished response (PLAYER 0x007bc510, folded with
	// GuiObject::processKeyEvent).
	GuiResponse GuiTarget::process(const GuiEvent& event)
	{
		return GuiResponse();
	}

}  // namespace RBX


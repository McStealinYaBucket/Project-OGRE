/* gui/UnifiedWidget.h -- October 2013 C++-only widget base (NO reflection).
   Evidence: N; GuiItem base (C2 0x5CB38C); init 0x5CE310 sets +104=140.0,
     +108=24.0, +112 MenuState dword=0; getFontSize const 8;
     setMenuState 0x5CE7DA (32B: +112 write on change + virtual slot +168).
   Twelfth-era landing (batch781/784/785): MenuState names from the family
     assert strings (NOTHING=0 :561 / HOVER=1 :532 / SHOWN_APPEARING=2 :503;
     the value-3 name is never asserted -> SHOWN by semantics, DELTA-flagged);
     the process family + dispatcher decoded verbatim: dispatcher 0x5CEEEE0 --
     process 0x5CEEE0 (isVisible gate vt+148, type in [1,9], menuState
     2-3->processShown / 1->processHover / 0->processNothing, else zero);
     firstChildPosition 0x5CE5AE / childOffset 0x5CE6A0 {0,28.0f} /
     getChildPosition 0x5CE6B8 (the child walk w/ assert "0" :463);
     onLoseFocus 0x5CE7CA (menuState=0) / canLoseFocus 0x5CF370 (true).
     UnifiedWidget::process reinforces slot40; its own new virtuals land at
     binary slots 42-45 (onMenuStateChanged / firstChildPosition /
     childOffset / render2dMe).
   Deferred: render2d/render2dMe bodies + render2dChildren (banked in
     batch785_out = the render era); ChatWidget::process 0x5CB13E (chat era).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/UnifiedWidget.h */

#pragma once

#include "V8DataModel/Item.h"

namespace RBX {

	class UnifiedWidget
		: public GuiItem
	{
	public:
		// Value names: NOTHING/HOVER/SHOWN_APPEARING appear verbatim in the
		// binary assert strings (GUI.cpp :561/:532/:503).  Value 3's name is
		// never in an assert -> SHOWN is our semantic name (DELTA-flagged).
		enum MenuState
		{
			NOTHING = 0,         MENU_STATE_0 = NOTHING,        // probe-era gate-test alias
			HOVER = 1,           MENU_STATE_1 = HOVER,          // probe-era gate-test alias
			SHOWN_APPEARING = 2, MENU_STATE_2 = SHOWN_APPEARING,// probe-era gate-test alias
			SHOWN = 3,           MENU_STATE_3 = SHOWN,          // probe-era gate-test alias; value 3's real name is never asserted (DELTA-flagged)
		};
		// C2 0x5CB38C (+104/+108 overwrite inherited size slots, like TextDisplay).
		UnifiedWidget()
			: menuState(NOTHING)
		{
			sizeX = 140.0f;
			sizeY = 24.0f;
		}
		// FUNCTION: PLAYER 0x00992a80 FOLDED
		// FUNCTION: STUDIO 0x007a06a0 FOLDED
		virtual int getFontSize() const { return 8; }  // 0x5CF37A (slot 36 override)

		// process family -- bodies in App/gui/GUI.cpp (the binary's assert
		// file; members are non-virtual except process, which is the slot-40
		// dispatcher override):
		virtual GuiResponse process(const GuiEvent& e);          // 0x5CEEE0
		GuiResponse processShown(const GuiEvent& e);             // 0x5CE8D2
		GuiResponse processShown_InTitle(const GuiEvent& e);     // 0x5CE7FA (assert :503)
		GuiResponse processShown_OutOfTitle(const GuiEvent& e);  // 0x5CE982 (asserts :619/:635)
		GuiResponse processHover(const GuiEvent& e);             // 0x5CEBF6 (assert :532)
		GuiResponse processNothing(const GuiEvent& e);           // 0x5CED6A (assert :561)

		// UnifiedWidget overrides of the GuiItem focus vtable entries:
		virtual Vector2 getChildPosition(const GuiItem* child, Canvas canvas) const; // 35: 0x5CE6B8 (GUI.cpp; the child walk, assert "0" :463)
		virtual void onLoseFocus();                              // 0x5CE7CA (slot 32): menuState = NOTHING
		virtual bool canLoseFocus();                             // 0x5CF370 (slot 33; NON-const, mangle __ZN3RBX13UnifiedWidget12canLoseFocusEv; GUI.cpp)

		// UnifiedWidget extension virtuals, binary slots 42-45:
		virtual void onMenuStateChanged() {}                     // 42 (base empty; ChatWidget overrides)
		virtual Vector2 firstChildPosition(Canvas canvas) const; // 43: 0x5CE5AE (GUI.cpp)
		virtual Vector2 childOffset() const;                     // 44: 0x5CE6A0 {0, 28.0f}
		virtual void render2d(Adorn* adorn);                     // 41: 0x5CE572 (GUI.cpp; visible gate + tail-call)
		virtual void render2dMe(Adorn* adorn);                   // 45: 0x5CE32E (GUI.cpp real body, thirteenth era)
		void render2dChildren(Adorn* adorn);                     // 0x5CE51A (87B, NON-virtual: render2d tail-jumps the symbol directly; batch787)

		// render2d (0x5CE572, 59B): { if (isVisible()) { render2dMe(adorn);
		// } } then jmp render2dChildren (5CE5A2 -- the direct
		// __ZN3RBX13UnifiedWidget16render2dChildrenE target, so the body is
		// a plain call in the port).  render2dMe (0x5CE32E, 491B) = the
		// white/0.7-accent frame fill, the 1.0f black outline, and the
		// fw-name label2d at XAlign 2 (thirteenth era, batch786/787).

		// mac: 0x5CE7DA (32B): write +112 only on change, then the virtual slot
		// +168 (onMenuStateChanged) via tail-jump -- dispatched the same way
		// here (virtual call, so overrides run like the binary).
		// STUB: PLAYER 0x00768400
		// STUB: STUDIO 0x007b3ea0
		void setMenuState(MenuState state)
		{
			if (menuState != (int)state)
			{
				menuState = (int)state;
				onMenuStateChanged();
			}
		}
		// Bring-up view of the +112 slot (binary reads it directly).
		int getMenuStateForTest() const { return menuState; }
	protected:
		int menuState;  // +112 (MenuState slot; names from the assert strings)
	};

} // namespace RBX

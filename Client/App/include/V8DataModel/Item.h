/* V8DataModel/Item.h -- October 2013 (class RENAMED 2026-10-05: C++ name
   is GuiItem per 0x5CCE86 __ZN3RBX7GuiItemC2Ev; Lua/XML name stays "Item"
   via sGuiItem. File name kept to avoid vcxproj churn.)
   Evidence: NonFactoryProduct<GuiItem, Instance> 0x54E400 (C++ GuiItem; Lua
   Item); sGuiItem="Item".  GuiItem C2 0x5CCE86 (112B): +96/+100 = the
   shared_ptr<GuiItem> focus (identity proved by batch785 decode of
   processNonFocus 0x5CD370 / loseFocus 0x5CF196); +104/+108 sizeX/sizeY.
   Binary virtual surface in ZTV 0xEEF9E0 slot order (batch783 vt dump);
   bodies land in App/gui/GUI.cpp + the GUI.h inline annex
   (GuiEvent included for the 16B by-value GuiResponse).
   Fidelity ~99%; DELTAS: Canvas/GuiEvent field names ours (shapes
   binary-fixed); getClassName stays with the landed Described machinery
   (binary GuiItem::getClassName 0x431430 tail-jumps Name::getNullName).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Item.h */

#pragma once

#include "Util/G3DCore.h"
#include "Util/Rect.h"
#include "Gui/Adorn.h"
#include "V8Tree/Instance.h"

// Adorn = RBX::Adorn via the Gui/Adorn.h include above (binary
// mangle truth label2d/menuSelect: PNS_5AdornE, batch791); no
// global fwd-decl -- the ::Adorn form was a wrong-namespace carry.

namespace RBX {

	extern const char* const sGuiItem;

	class Canvas;      // 8B by-value arg (GUI.h; batch783 decode)
	class GuiEvent;    // 12B const& arg (GuiEvent.h; batch781)
	class GuiResponse; // 16B by-value return (GuiEvent.h)

	class GuiItem
		: public DescribedNonCreatable<GuiItem, Instance, sGuiItem>
	{
	public:
		// STUB: PLAYER 0x0076a4f0
		// STUB: STUDIO 0x007b5f10
		GuiItem()
			: focus(), sizeX(0.0f), sizeY(0.0f)
		{}
		virtual ~GuiItem() {}  // D1/D0 0x5CD05A/0x5CD02A (POD teardown)

		// -- Binary virtual surface, slot order (ZTV 0xEEF9E0) --
		virtual bool askAddChild(const Instance* instance) const;                // 18: 0x4EACE0
		virtual void onDescendantRemoving(const shared_ptr<Instance>& instance); // 24: 0x5CD2A0
		// STUB: PLAYER 0x00528720
		virtual void onLoseFocus() {}                                            // 32: 0x3ECC70 (empty)
		// STUB: PLAYER 0x00927cc0
		// STUB: STUDIO 0x008db930
		virtual bool canLoseFocus() { return false; }                            // 33: 0x3EC660 (NON-const, mangle __ZN3RBX7GuiItem12canLoseFocusEv)
		virtual Vector2 getPosition(Canvas canvas) const;                        // 34: 0x4D8490 (inline GUI.h, assert :64)
		virtual Vector2 getChildPosition(const GuiItem* child, Canvas canvas) const; // 35: 0x37AD10 (inline GUI.h, assert :69)
		// FUNCTION: PLAYER 0x006c8740 FOLDED
		// FUNCTION: STUDIO 0x007a6470 FOLDED
		virtual int getFontSize() const { return 12; }                           // 36: 0x3A6ED0
		// FUNCTION: PLAYER 0x006be550 FOLDED
		// FUNCTION: STUDIO 0x0078ad10 FOLDED
		virtual bool isVisible() const { return true; }                          // 37: 0x4E7FC0
		virtual std::string getTitle();                                          // 38: 0x3EC670 (NON-const per mangle)
		virtual Vector2 getSize(Canvas canvas) const;                            // 39: 0x4DB1F0 (GUI.cpp)
		virtual GuiResponse process(const GuiEvent& e);                          // 40: 0x5CD47E (GUI.cpp)
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void render2d(Adorn* adorn) {}                                   // 41: 0x5CF338 (empty)

		// Non-virtual helpers (bodies in gui/GUI.cpp unless noted):
		GuiItem* getGuiParent() const;               // 0x5CD184 (isA<GuiItem> parent gate)
		GuiItem* getGuiItem(int index);              // 0x5CD1D8
		const GuiItem* getGuiItem(int index) const;  // 0x5CD23C
		// Assert-cited inline accessor (binary: "getFocus() == NULL" :619/:118).
		GuiItem* getFocus() const { return focus.get(); }  // +0x60 px
		Rect getMyRect(Canvas canvas) const;         // 0x5CD2C4 (inline GUI.h)
		Rect getMyRect2D(Canvas canvas) const;       // 0x3EC564 (inline GUI.h)
		void loseFocus();                            // 0x5CF196
		GuiResponse processNonFocus(const GuiEvent& e); // 0x5CD370 (270B, non-virtual; batch785)
		// Assert expand-site hooks: the RBXASSERT text lives in gui/GUI.h so
		// the debugHook strings expand there (binary include/Gui/GUI.h:64/:69);
		// the GUI.cpp bodies open with the hook call:
		void assertGetGuiParentHook() const;   // getGuiParent() @ GUI.h:64
		void assertChildPosZeroHook() const;   // "0" @ GUI.h:69
		// Render family (thirteenth era, batch787):
		const G3D::Color4& menuSelect();         // 0x5CCE1E (NON-const per mangle; the guarded Color4(0.7,0.7,0.7,1.0) singleton)
		void label2d(Adorn* adorn, const std::string& message, const G3D::Color4& fore, const G3D::Color4& back,
			Text::XAlign align) const;           // 0x5CD5F4 (CONST per mangle __ZNK...; GUI.cpp)

		// mac: 0x5CCDB6 (103B): the guarded static Color4(0.6,0.6,0.6,0.6)
		// singleton by reference (rarc29 decompile; receiver fully unread
		// in the body -- the classicStyle painter feeds it straight into
		// Adorn::fillRectangle's Color4&).  Body in gui/GUI.cpp.
		const G3D::Color4& translucentBackdrop();  // __ZN3RBX7GuiItem19translucentBackdropEv

	protected:
		shared_ptr<GuiItem> focus;  // +96/+100 (the old reserved96/100; loseFocus releases the +0x64 block)
		float sizeX;  // +104 (getSize reads; derived inits overwrite)
		float sizeY;  // +108
	};

} // namespace RBX

/* V8DataModel/GuiObject.h — October 2013 (layout surface ported; full C2 TODO).
   Evidence: GuiObject C2 0x3A8E82 (1656B: GuiBase2d(name) + mouse-event
     remote_signals at +160..+196/+360..+372 with *ConnectionCount props,
     UDim2 signal +368, two Color3s at +488/+500, +512 = 1, +513 = bool arg,
     +120 = 0, +121 = 1; Described<GuiObject, sGuiObject, GuiBase2d, 27>).
     sGuiObject 122 hits in the 0.125 PE string table.
     setZIndex evidenced by Workspace::start/stop 0x515636/0x515896.
   Layout members (batch173): size UDim2 @+0x1C0, position @+0x1D0,
     backgroundTransparency float @+0x204, visible bool @+0x200, visible
     cache int @+0x1BC (zeroed when hidden).
   DELTAS: the C2 signal/prop/replicator block, the raisePropertyChanged
     calls in the setters (descriptor globals unk_FB2BB4/FB2BE0/FB2D48/FB2DCC
     -- EAs unknown, nothing in-tree listens), and the tail-jump virtuals
     after setSize/setPosition (absolute-layout recalc -- nothing in-tree
     reads absolute layout) are unported.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiObject.h */

#pragma once

#include "V8DataModel/GuiBase2d.h"
#include "Util/UDim.h"
#include "rbx/signal.h"

namespace RBX {

	extern const char* const sGuiObject;

	class GuiObject
		: public DescribedNonCreatable<GuiObject, GuiBase2d, sGuiObject, Reflection::ClassDescriptor::PERSISTENT>
	{
		int zIndex;
		bool unknown513;  // +513: the C2 bool arg (TODO identity)
	public:
		GuiObject(const char* name = NULL, bool unknownFlag = false);
		void setZIndex(int value);  // 0x515636/0x515896 callee
		int getZIndex() const { return zIndex; }

		// GuiObject::SizeConstraint (mangle
		// __ZN3RBX9GuiObject17setSizeConstraintENS0_14SizeConstraintE
		// @0x3A8358 -- an enum BY VALUE).  Enumerator order unpinned in
		// the binary; the 2013 order = RelativeXY/RelativeXX/RelativeYY
		// and the bubble factories (fac3 0x5C14AF) pass 1 = RelativeXX
		// (DELTA if the order is ever contradicted).
		enum SizeConstraint
		{
			RelativeXY,
			RelativeXX,
			RelativeYY
		};

		// Layout surface (batch173; UDim2 layout matches the binary exactly:
		// float@0, int16@4, float@8, int16@12 -- see Util/UDim.h).
		// mac: 0x3A811A (185B): NaN-aware change-detect (NaN always stores),
		// then store; raise + tail virtual are DELTAs (see header).
		void setSize(const UDim2& value)
		{
			if (size != value)
				size = value;
		}
		// mac: 0x3AEF2A (38B; mangled getSize though requested as getPosition):
		// 16-byte copy of +0x1C0.
		UDim2 getSize() const { return size; }
		// mac: 0x3A81D4 (185B): same shape at +0x1D0.
		void setPosition(const UDim2& value)
		{
			if (position != value)
				position = value;
		}
		// mac: 0x3AEF7E (38B; mangled getPosition): 16-byte copy of +0x1D0.
		UDim2 getPosition() const { return position; }
		// mac: 0x3A854E (66B): change-detect + store at +0x204.
		// STUB: PLAYER 0x007bd610
		// STUB: STUDIO 0x008d9fd0
		void setBackgroundTransparency(float value)
		{
			if (backgroundTransparency != value)
				backgroundTransparency = value;
		}
		float getBackgroundTransparency() const { return backgroundTransparency; }  // 0x3AF160 (14B).
		// mac: 0x3A828E (setBorderSizePixel, int by value): change-detect +
		// store + raisePropertyChanged tail (unported DELTA, same idiom
		// as the setters above).
		void setBorderSizePixel(int value)
		{
			if (borderSizePixel != value)
				borderSizePixel = value;
		}
		int getBorderSizePixel() const { return borderSizePixel; }
		// mac: 0x3A8358 (setSizeConstraint, SizeConstraint by value): same shape.
		void setSizeConstraint(SizeConstraint value)
		{
			if (sizeConstraint != value)
				sizeConstraint = value;
		}
		// mac: 0x3A8614 (84B): store at +0x200; when hidden, zero the +0x1BC
		// visible cache.
		void setVisible(bool value)
		{
			if (visible != value)
			{
				visible = value;
				if (!visible)
					visibleCache = 0;
			}
		}
		bool getVisible() const { return visible; }  // 0x3AF1EA (15B).
		// mac: 0x3A85D2 (65B): change-detect at +0x1B9, store; raise
		// unk_FB2DA0 is a DELTA (descriptor EA unknown, nothing listens).
		void setClipping(bool value)
		{
			if (clipping != value)
				clipping = value;
		}
		bool getClipping() const { return clipping; }  // 0x3AF1DA (15B).
		// mac: 0x3A8590 (65B): change-detect at +0x95, store; raise
		// unk_FB2D74 is a DELTA (same reason).
		void setDraggable(bool value)
		{
			if (draggable != value)
				draggable = value;
		}
		bool getDraggable() const { return draggable; }  // 0x3AF19C (15B).
		// mac: 0x3AC01A (87B): nearest ancestor (parent chain from +0x34) that
		// isA GuiObject with +0x1B9 (clipping) set; NULL when none.  isA
		// plays as fastDynamicCast (established equivalent), clipping as
		// getClipping (just landed).  Non-const per the mangling.
		GuiObject* firstAncestorClipping();
		// mac: 0x3AC072 (147B): own getRect2D intersected with every clipping
		// ancestor's getRect2D up the chain (vendor intersect is header-only
		// inline -- no link debt).  Non-const per the mangling (sret form).
		G3D::Rect2D getClippedRect();
		// mac: 0x3AD14A (282B): point-in-rect over getRect2D, or over the
		// clipped intersection when the flag is on and a clipping ancestor
		// exists.  The int16 truncation is spelled (int16)(int) directly:
		// the vendor Vector2int16(Vector2) ctor lives in a G3D .cpp outside
		// the link (same link-debt rule as Color3 statics); identical for
		// all on-screen coordinates.  Non-const per the mangling.
		bool mouseIsOver(const G3D::Vector2& point);
		// mac: 0x3AC1EA (98B): NULL -> false; non-GuiBase2d -> false; else the
		// NEGATION of a vtbl+144 virtual taking (Instance*)->bool.
		// Identification: of Instance's bool(Instance*) virtuals, only
		// askForbidChild fits semantically ("may I parent under you" =
		// "don't you forbid me").  VTABLE-PROVEN (rarc11
		// __ZTVN3RBX9GuiObjectE dump): our landed 0x3AC1EA sits at the
		// primary object vptr slot 21 (+0x54), the slot the body's
		// [vptr+144] vcall reaches from the same table.
		virtual bool askSetParent(const Instance* parent) const;

		// mac: 0x3ABA9A (327B; mangle
		// __ZN3RBX9GuiObject13getWindowRectEPNS_9GuiBase2dE = one
		// explicit GuiBase2d* start; the binary's retn 4 = sret and
		// the receiver slot is never read -- the single call site
		// passes this twice): climb to the first ScreenGui copying
		// its windowRect verbatim; climb steps gated on the parent
		// isA(GuiBase2d); stop node isA(BillboardGui) copies ITS
		// windowRect; else the zero rect.  Body + walk details in
		// GuiObject.cpp (rarc15 raws).
		G3D::Rect2D getWindowRect(GuiBase2d* start);
		// mac: 0x3ABBE2 (113B, the slot-38 override): getWindowRect(this)
		// copied verbatim into windowRectCache, then the DIRECT
		// (non-virtual, mangle-proven) base call
		// GuiBase2d::handleResize(rect, flag).  Body in GuiObject.cpp.
		virtual void handleResize(const G3D::Rect2D& rect, bool flag);

		// mac: 0x3AC2AC (55B): NON-VIRTUAL parentless render entry called by
		// ChatOutput::renderBubbleImposters / renderBubbles (rarc13
		// census).  Binary ORDER (rarc10): the [this+0x34] parent == 0
		// guard (3AC2B2..3AC2B9 -- the render-when-parentless reading
		// stays CAVEATED, the raw shape is exact), then the slot-38
		// VCALL [vptr+0x98] staged (this, rect, 1) at 3AC2C0..3AC2D4 =
		// handleResize(rect, true) through the dynamic dispatch, then
		// the TAIL-JMP recursiveRender2d(this, adorn) (3AC2DE -- the
		// epilogue first; the jmp carries (this, adorn) in the arg
		// slots = the clean compile of `return recursiveRender2d(adorn);`).
		// Mangle never observed: name = the decomp's, signature = the
		// call shapes (rect = const G3D::Rect2D& per the 4-byte arg
		// staging into the handleResize call).  Body in GuiObject.cpp.
		// Spell RBX::Adorn explicitly -- the binary mangle = PNS_5AdornE
		// (the rarc18 5C4E3A call), the same class tree Gui/Adorn.h
		// defines and AdornBillboarder derives; include-order immune.
		void legacyRender2d(RBX::Adorn* adorn, const G3D::Rect2D& rect);

		// Mouse/drag input signals (0x3A8E82: +160..+196, +360, +368;
		// 2016 member names carried). The *ConnectionCount props bound to
		// these in 2013 (prop_ symbols in the C2) need their PropDescriptor
		// EAs for Lua names/getters — TODO.
		rbx::remote_signal<void(int, int)> mouseEnterSignal;
		rbx::remote_signal<void(int, int)> mouseLeaveSignal;
		rbx::remote_signal<void(int, int)> mouseMovedSignal;
		rbx::remote_signal<void(int, int)> mouseWheelForwardSignal;
		rbx::remote_signal<void(int, int)> mouseWheelBackwardSignal;
		rbx::remote_signal<void(int, int)> dragStoppedSignal;
		rbx::remote_signal<void(UDim2)> dragBeginSignal;

	private:
		// Binary layout slots (model-relative class; offsets documented).
		UDim2 size;  // +0x1C0.
		UDim2 position;  // +0x1D0.
		int visibleCache;  // +0x1BC (zeroed when hidden).
		bool visible;  // +0x200.
		float backgroundTransparency;  // +0x204.
		// +0x208..+0x214 (C2 zero-inits 3A9325..3A934D byte-exact):
		// handleResize 0x3ABBE2 copies the getWindowRect result in
		// verbatim before forwarding to GuiBase2d::handleResize.  No
		// reader landed yet (identity beyond the resize snapshot
		// pending) -- name behavioral.
		G3D::Rect2D windowRectCache;  // +0x208..+0x214.
		bool clipping;  // +0x1B9 (getClipping 0x3AF1DA).
		bool draggable;  // +0x95 (getDraggable 0x3AF19C).
		// Prop stores landed for the factory surface (0x3A828E/0x3A8358);
		// binary prop offsets unverified -- MSVC layout DELTA.
		int borderSizePixel;
		SizeConstraint sizeConstraint;
	};

} // namespace RBX

/* App/v8datamodel/GuiObject.cpp — October 2013 (ctor + ZIndex + the resize
   arc: GuiObject::handleResize 0x3ABBE2 + GuiObject::getWindowRect
   0x3ABA9A + the NON-virtual parentless render entry legacyRender2d
   0x3AC2AC the bubble chat calls).
   Evidence: GuiObject C2 0x3A8E82 (GuiBase2d(name); signal/prop block TODO —
     see header; +513 = bool arg); setZIndex by Workspace::start/stop.
     getWindowRect/handleResize raws banked rarc12/rarc15, legacyRender2d's
     raw banked rarc10 (comments carry the raw lines).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GuiObject.h"
#include "V8DataModel/ScreenGui.h"
#include "V8DataModel/BillboardGui.h"
#include "Util/FastFlags.h"

namespace RBX {
	const char* const sGuiObject = "GuiObject";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiObject);
}

namespace RBX {

	typedef DescribedNonCreatable<GuiObject, GuiBase2d, sGuiObject, Reflection::ClassDescriptor::PERSISTENT> GuiObjectSuper;

	// mac: 0x3A8E82: GuiBase2d(name); +513 = bool arg (signal/prop wiring TODO).
	// Layout inits from the C2: size/position zeroed (+0x1C0-+0x1DC),
	// visibleCache 0 (+0x1BC), visible true (+0x200 = 1),
	// backgroundTransparency 0.0f (+0x204 = 0), clipping false (+0x1B9 = 0,
	// C2 `mov [edi+1B9h],0`), draggable false (+0x95, zero-region),
	// windowRectCache zero rect (+0x208..+0x214, rarc15 C2 slice
	// 3A9325/3A932F/3A9339/3A9343/3A934D all store 0).
	GuiObject::GuiObject(const char* name, bool unknownFlag)
		: GuiObjectSuper(name)
		, zIndex(1)
		, unknown513(unknownFlag)
		, visibleCache(0)
		, visible(true)
		, backgroundTransparency(0.0f)
		, windowRectCache(G3D::Rect2D::xyxy(0.0f, 0.0f, 0.0f, 0.0f))
		, clipping(false)
		, draggable(false)
	{
	}

	// mac: 0x515636/0x515896 callee: store (render-dirty propagation TODO).
	void GuiObject::setZIndex(int value)
	{
		zIndex = value;
	}

	// mac: 0x3AC1EA (98B).
	// STUB: PLAYER 0x007c3a60
	// STUB: STUDIO 0x008e05e0
	bool GuiObject::askSetParent(const Instance* parent) const
	{
		if (parent == NULL)
			return false;
		if (Instance::fastDynamicCast<GuiBase2d>(parent) == NULL)
			return false;
		return !parent->askForbidChild(this);
	}

	// mac: 0x3AC01A (87B).
	GuiObject* GuiObject::firstAncestorClipping()
	{
		for (Instance* anc = getParent(); anc != NULL; anc = anc->getParent())
		{
			const GuiObject* go = Instance::fastDynamicCast<GuiObject>(anc);
			if (go != NULL && go->getClipping())
				return const_cast<GuiObject*>(go);
		}
		return NULL;
	}

	// mac: 0x3AC072 (147B).
	G3D::Rect2D GuiObject::getClippedRect()
	{
		G3D::Rect2D out = getRect2D();
		for (GuiObject* anc = firstAncestorClipping(); anc != NULL;
			anc = anc->firstAncestorClipping())
		{
			out = out.intersect(anc->getRect2D());
		}
		return out;
	}

	// mac: 0x3AD14A (282B).
	bool GuiObject::mouseIsOver(const G3D::Vector2& point)
	{
		G3D::Rect2D rect = getRect2D();
		if (FFlag::CheckMouseOverWithClipping && firstAncestorClipping() != NULL)
			rect = getClippedRect();
		float ix = (float)(G3D::int16)(int)point.x;
		float iy = (float)(G3D::int16)(int)point.y;
		return ix >= rect.x0y0().x && rect.x1y1().x >= ix
			&& iy >= rect.x0y0().y && rect.x1y1().y >= iy;
	}

	// mac: 0x3ABA9A (327B; mangle __ZN3RBX9GuiObject13getWindowRectEPNS_9GuiBase2dE
	// = ONE explicit GuiBase2d* start; the binary's __stdcall retn 4 =
	// the sret struct return, and the RECEIVER slot [ebp+arg_4] is
	// never read -- the single call site (handleResize 3ABBF4) passes
	// this twice).  The walk (rarc15 raw): EVERY node isA(ScreenGui) ->
	// verbatim [node+0xB0..+0xBC] copy (raw 3ABBA9..3ABBD2 = the PIC
	// chunk loads landing on our typed ScreenGui members -- the
	// chunk-crawl edi = ebx+24h and the PIC base = compiler artifacts);
	// every climb STEP is gated on the PARENT isA(GuiBase2d) (raw
	// 3ABAFC calls the sGuiBase2d-described classDescriptor), so the
	// walk stops when the chain leaves the gui tree; after the break
	// the stop node isA(BillboardGui) (raw 3ABB2C, sAdornmentGui
	// described) -> verbatim [node+0xB4..+0xC0] (raw 3ABB4F..3ABB7D);
	// else all four sret dwords = 0 (3ABB84..3ABB98).  A null start
	// faults in the binary ([0+34h] read; the call site passes self,
	// so the shape is unobservable) -- our walk is null-safe anyway.
	// STUB: PLAYER 0x007c87e0
	// STUB: STUDIO 0x008e52c0
	G3D::Rect2D GuiObject::getWindowRect(GuiBase2d* start)
	{
		Instance* node = start;
		while (node != NULL)
		{
			if (node->getDescriptor().isA(ScreenGui::classDescriptor()))
				return static_cast<ScreenGui*>(node)->windowRect;
			Instance* next = node->getParent();
			if (next == NULL || !next->getDescriptor().isA(GuiBase2d::classDescriptor()))
				break;
			node = next;
		}
		if (node != NULL && node->getDescriptor().isA(BillboardGui::classDescriptor()))
			return static_cast<BillboardGui*>(node)->windowRect;
		return G3D::Rect2D::xyxy(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// mac: 0x3AC2AC (55B, rarc10 raw 3AC2B2..3AC2DE): see GuiObject.h for
	// the ordering note.  Our spelling: the guard = getParent() == NULL
	// (the +0x34 read = the landed getParent), the resize vcall = our
	// unqualified virtual call (same dynamic dispatch as [vptr+0x98]),
	// the tail-jmp = a plain statement (GCC's tail form carries the
	// same (this, adorn) argument slots).
	void GuiObject::legacyRender2d(RBX::Adorn* adorn, const G3D::Rect2D& rect)
	{
		if (getParent() == NULL)
		{
			handleResize(rect, true);
			recursiveRender2d(adorn);
		}
	}

	// mac: 0x3ABBE2 (113B; GuiObject's slot-38 override of GuiBase2d::
	// handleResize, rarc11 ZTV).  Binary ORDER: getWindowRect(this) --
	// raw 3ABBE9..3ABBFA passes the receiver twice ([ebp+arg_0] =
	// [ebp+arg_4] = the self-pass, sret = var_18) -- then the VERBATIM
	// four-dword copy into the +0x208..+0x214 cache (3ABC02..3ABC2E:
	// sret dwords -> movss [esi+208h]/+20Ch/+210h/+214h), then the
	// DIRECT (non-virtual; 3ABC48 mangle __ZN3RBX9GuiBase2d12
	// handleResizeERKN3G3D6Rect2DEb) base call with the INCOMING rect.
	// In our spelling the verbatim copy = the plain Rect2D assign (a
	// four-float copy, same observable).
	// STUB: PLAYER 0x007c88f0
	// STUB: STUDIO 0x008e53d0
	void GuiObject::handleResize(const G3D::Rect2D& rect, bool flag)
	{
		windowRectCache = getWindowRect(this);
		GuiBase2d::handleResize(rect, flag);
	}

} // namespace RBX

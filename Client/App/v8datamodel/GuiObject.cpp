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
#include "decomp.h"

#include "V8DataModel/GuiObject.h"
#include "V8Tree/Property.h"
#include "V8DataModel/ScreenGui.h"
#include "V8DataModel/BillboardGui.h"
#include "Util/FastFlags.h"
#include "Util/BrickColor.h"

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

DECOMP_SIZE_ASSERT(RBX::GuiObject, 0x21c);

// Registered at PLAYER 0x00a7bd20.
// GLOBAL: PLAYER 0x00d6f3ad
// GLOBAL: STUDIO 0x00efb97d
// FFlag::BorderSizePixelUpTo100
FASTFLAGVARIABLE(BorderSizePixelUpTo100, true)

namespace RBX {

	// GLOBAL: PLAYER 0x00e0ee0c
	// GLOBAL: STUDIO 0x00f940d0
	// GuiObject::prop_ClipsDescendants
	Reflection::PropDescriptor<GuiObject, bool> GuiObject::prop_ClipsDescendants("ClipsDescendants", category_Behavior, &GuiObject::getClipping, &GuiObject::setClipping);
	// GLOBAL: PLAYER 0x00e0ed84
	// GLOBAL: STUDIO 0x00f9404c
	// GuiObject::prop_BorderSizePixel
	Reflection::PropDescriptor<GuiObject, int> GuiObject::prop_BorderSizePixel("BorderSizePixel", category_Appearance, &GuiObject::getBorderSizePixel, &GuiObject::setBorderSizePixel);
	// GLOBAL: PLAYER 0x00e0ef78
	// GLOBAL: STUDIO 0x00f9423c
	// GuiObject::prop_Visible
	Reflection::PropDescriptor<GuiObject, bool> GuiObject::prop_Visible("Visible", category_Data, &GuiObject::getVisible, &GuiObject::setVisible);
	// GLOBAL: PLAYER 0x00e0ebac
	// GLOBAL: STUDIO 0x00f93e74
	// GuiObject::prop_BackgroundTransparency
	Reflection::PropDescriptor<GuiObject, float> GuiObject::prop_BackgroundTransparency("BackgroundTransparency", category_Appearance, &GuiObject::getBackgroundTransparency, &GuiObject::setBackgroundTransparency);

	// FUNCTION: PLAYER 0x007bd3a0
	// FUNCTION: STUDIO 0x008d9d60
	void GuiObject::setClipping(bool value)
	{
		if (clipping != value)
		{
			clipping = value;
			raisePropertyChanged(prop_ClipsDescendants);
		}
	}

	// FUNCTION: PLAYER 0x007bd400
	// FUNCTION: STUDIO 0x008d9dc0
	void GuiObject::setBorderSizePixel(int value)
	{
		if (value < 0)
			value = 0;
		int maxValue = 1;
		if (FFlag::BorderSizePixelUpTo100)
			maxValue = 100;
		if (value > maxValue)
			value = maxValue;
		if (borderSizePixel != value)
		{
			borderSizePixel = value;
			raisePropertyChanged(prop_BorderSizePixel);
		}
	}

	// FUNCTION: PLAYER 0x007bd540
	// FUNCTION: STUDIO 0x008d9f00
	void GuiObject::setVisible(bool value)
	{
		if (visible != value)
		{
			visible = value;
			raisePropertyChanged(prop_Visible);
			if (!visible)
				m_unk0x1c0 = 0;
		}
	}

	// FUNCTION: PLAYER 0x007bd610
	// FUNCTION: STUDIO 0x008d9fd0
	void GuiObject::setBackgroundTransparency(float value)
	{
		if (value != backgroundTransparency)
		{
			backgroundTransparency = value;
			raisePropertyChanged(prop_BackgroundTransparency);
		}
	}

	// FUNCTION: PLAYER 0x007bf190
	// FUNCTION: STUDIO 0x008dbba0
	G3D::Rect2D GuiObject::Scale9Rect2D(const G3D::Rect2D& rect, float border, float minSize)
	{
		float scale = 1.0f;
		scale = std::min(scale, rect.width() / minSize);
		scale = std::min(scale, rect.height() / minSize);

		float xInset = border * scale;
		float yInset = border * scale;

		return G3D::Rect2D::xywh(rect.x0() + xInset, rect.y0() + yInset, rect.width() - xInset * 2, rect.height() - yInset * 2);
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
	// STUB: PLAYER 0x007c77c0
	// STUB: STUDIO 0x008e4260
	GuiObject::GuiObject(const char* name, bool active)
		: Super(name)
		, m_unk0x94(0)
		, m_unk0x98(false)
		, m_unk0x99(false)
		, m_unk0x9a(false)
		, m_unk0x9c(0.0f, 0.0f)
		, varMouseEnterConnectionCount(0)
		, varMouseLeaveConnectionCount(0)
		, varMouseMovedConnectionCount(0)
		, varMouseWheelForwardConnectionCount(0)
		, varMouseWheelBackwardConnectionCount(0)
		, varDragStoppedConnectionCount(0)
		, varDragBeginConnectionCount(0)
		, serverGuiObject(false)
		, clipping(false)
		, m_unk0x1c0(0)
		, size()
		, position()
		, sizeConstraint(RelativeXY)
		, borderSizePixel(1)
		, backgroundColor(BrickColor(194).color3())
		, borderColor(BrickColor(26).color3())
		, visible(true)
		, active(active)
		, backgroundTransparency(0.0f)
		, windowRectCache()
	{
	}

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
	// FUNCTION: PLAYER 0x007c88f0
	// FUNCTION: STUDIO 0x008e53d0
	void GuiObject::handleResize(const G3D::Rect2D& rect, bool flag)
	{
		windowRectCache = getWindowRect(this);
		GuiBase2d::handleResize(rect, flag);
	}

} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"
#include "V8DataModel/GuiButton.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x007c29c0
// FUNCTION: STUDIO 0x008df540
template<>
EnumDesc<GuiObject::TweenEasingDirection>::EnumDesc()
	: EnumDescriptor("EasingDirection", typeid(GuiObject::TweenEasingDirection))
{
	addPair(GuiObject::EASING_DIRECTION_IN, "In");
	addPair(GuiObject::EASING_DIRECTION_OUT, "Out");
	addPair(GuiObject::EASING_DIRECTION_IN_OUT, "InOut");
}

template<>
const Type& Type::getSingleton<GuiObject::TweenEasingDirection>()
{
	return EnumDesc<GuiObject::TweenEasingDirection>::singleton();
}

template<> EnumRegistrar<GuiObject::TweenEasingDirection> EnumRegistrar<GuiObject::TweenEasingDirection>::registrar(0);
template<> TypeRegistrar<GuiObject::TweenEasingDirection> TypeRegistrar<GuiObject::TweenEasingDirection>::registrar(0);

// FUNCTION: PLAYER 0x007c2ae0
// FUNCTION: STUDIO 0x008df660
template<>
EnumDesc<GuiObject::TweenEasingStyle>::EnumDesc()
	: EnumDescriptor("EasingStyle", typeid(GuiObject::TweenEasingStyle))
{
	addPair(GuiObject::EASING_STYLE_LINEAR, "Linear");
	addPair(GuiObject::EASING_STYLE_SINE, "Sine");
	addPair(GuiObject::EASING_STYLE_BACK, "Back");
	addPair(GuiObject::EASING_STYLE_QUAD, "Quad");
	addPair(GuiObject::EASING_STYLE_QUART, "Quart");
	addPair(GuiObject::EASING_STYLE_QUINT, "Quint");
	addPair(GuiObject::EASING_STYLE_BOUNCE, "Bounce");
	addPair(GuiObject::EASING_STYLE_ELASTIC, "Elastic");
}

template<>
const Type& Type::getSingleton<GuiObject::TweenEasingStyle>()
{
	return EnumDesc<GuiObject::TweenEasingStyle>::singleton();
}

template<> EnumRegistrar<GuiObject::TweenEasingStyle> EnumRegistrar<GuiObject::TweenEasingStyle>::registrar(0);
template<> TypeRegistrar<GuiObject::TweenEasingStyle> TypeRegistrar<GuiObject::TweenEasingStyle>::registrar(0);

// FUNCTION: PLAYER 0x007c2c50
// FUNCTION: STUDIO 0x008df7d0
template<>
EnumDesc<GuiObject::TweenStatus>::EnumDesc()
	: EnumDescriptor("TweenStatus", typeid(GuiObject::TweenStatus))
{
	addPair(GuiObject::TWEEN_CANCELED, "Canceled");
	addPair(GuiObject::TWEEN_COMPLETED, "Completed");
}

template<>
const Type& Type::getSingleton<GuiObject::TweenStatus>()
{
	return EnumDesc<GuiObject::TweenStatus>::singleton();
}

template<> EnumRegistrar<GuiObject::TweenStatus> EnumRegistrar<GuiObject::TweenStatus>::registrar(0);
template<> TypeRegistrar<GuiObject::TweenStatus> TypeRegistrar<GuiObject::TweenStatus>::registrar(0);

// FUNCTION: PLAYER 0x007c2d60
// FUNCTION: STUDIO 0x008df8e0
template<>
EnumDesc<GuiButton::Style>::EnumDesc()
	: EnumDescriptor("ButtonStyle", typeid(GuiButton::Style))
{
	addPair(GuiButton::CUSTOM_STYLE, "Custom");
	addPair(GuiButton::ROBLOX_RED_STYLE, "RobloxButtonDefault");
	addPair(GuiButton::ROBLOX_GREY_STYLE, "RobloxButton");
}

template<>
const Type& Type::getSingleton<GuiButton::Style>()
{
	return EnumDesc<GuiButton::Style>::singleton();
}

template<> EnumRegistrar<GuiButton::Style> EnumRegistrar<GuiButton::Style>::registrar(0);
template<> TypeRegistrar<GuiButton::Style> TypeRegistrar<GuiButton::Style>::registrar(0);

} // namespace Reflection
} // namespace RBX

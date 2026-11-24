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
#include "security/SecurityContext.h"

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

	// The 2013 dynamic initializers (PLAYER 0x00a7bf70..0x00a7c490) build these in this order.
	// GLOBAL: PLAYER 0x00e0ec38
	// GLOBAL: STUDIO 0x00f93f00
	// GuiObject::prop_Size
	Reflection::PropDescriptor<GuiObject, UDim2> GuiObject::prop_Size("Size", category_Data, &GuiObject::getSize, &GuiObject::setSize);
	// GLOBAL: PLAYER 0x00e0f36c
	// GLOBAL: STUDIO 0x00f94634
	// GuiObject::prop_Position
	Reflection::PropDescriptor<GuiObject, UDim2> GuiObject::prop_Position("Position", category_Data, &GuiObject::getPosition, &GuiObject::setPosition);
	// GLOBAL: PLAYER 0x00e0ed84
	// GLOBAL: STUDIO 0x00f9404c
	// GuiObject::prop_BorderSizePixel
	Reflection::PropDescriptor<GuiObject, int> GuiObject::prop_BorderSizePixel("BorderSizePixel", category_Data, &GuiObject::getBorderSizePixel, &GuiObject::setBorderSizePixel);
	// GLOBAL: PLAYER 0x00e0eb54
	// GLOBAL: STUDIO 0x00f93e1c
	// GuiObject::prop_ZIndex
	Reflection::PropDescriptor<GuiObject, int> GuiObject::prop_ZIndex("ZIndex", category_Data, &GuiObject::getZIndex, &GuiObject::setZIndex);
	// GLOBAL: PLAYER 0x00e0f27c
	// GLOBAL: STUDIO 0x00f94544
	// GuiObject::prop_SizeConstraint
	Reflection::EnumPropDescriptor<GuiObject, GuiObject::SizeConstraint> GuiObject::prop_SizeConstraint("SizeConstraint", category_Data, &GuiObject::getSizeConstraint, &GuiObject::setSizeConstraint);
	// GLOBAL: PLAYER 0x00e0efa4
	// GLOBAL: STUDIO 0x00f94268
	// GuiObject::prop_BorderColor
	Reflection::PropDescriptor<GuiObject, BrickColor> GuiObject::prop_BorderColor("BorderColor", category_Data, &GuiObject::getBorderColor, &GuiObject::setBorderColor);
	// GLOBAL: PLAYER 0x00e0f39c
	// GLOBAL: STUDIO 0x00f94664
	// GuiObject::prop_BorderColor3
	Reflection::PropDescriptor<GuiObject, G3D::Color3> GuiObject::prop_BorderColor3("BorderColor3", category_Data, &GuiObject::getBorderColor3, &GuiObject::setBorderColor3);
	// GLOBAL: PLAYER 0x00e0f3c8
	// GLOBAL: STUDIO 0x00f94690
	// GuiObject::prop_BackgroundColor
	Reflection::PropDescriptor<GuiObject, BrickColor> GuiObject::prop_BackgroundColor("BackgroundColor", category_Data, &GuiObject::getBackgroundColor, &GuiObject::setBackgroundColor);
	// GLOBAL: PLAYER 0x00e0ea68
	// GLOBAL: STUDIO 0x00f93d30
	// GuiObject::prop_BackgroundColor3
	Reflection::PropDescriptor<GuiObject, G3D::Color3> GuiObject::prop_BackgroundColor3("BackgroundColor3", category_Data, &GuiObject::getBackgroundColor3, &GuiObject::setBackgroundColor3);
	// GLOBAL: PLAYER 0x00e0ebac
	// GLOBAL: STUDIO 0x00f93e74
	// GuiObject::prop_BackgroundTransparency
	Reflection::PropDescriptor<GuiObject, float> GuiObject::prop_BackgroundTransparency("BackgroundTransparency", category_Data, &GuiObject::getBackgroundTransparency, &GuiObject::setBackgroundTransparency);
	// GLOBAL: PLAYER 0x00e0f15c
	// GLOBAL: STUDIO 0x00f94424
	// GuiObject::prop_Draggable
	Reflection::PropDescriptor<GuiObject, bool> GuiObject::prop_Draggable("Draggable", category_Behavior, &GuiObject::getDraggable, &GuiObject::setDraggable);
	// GLOBAL: PLAYER 0x00e0ee0c
	// GLOBAL: STUDIO 0x00f940d0
	// GuiObject::prop_ClipsDescendants
	Reflection::PropDescriptor<GuiObject, bool> GuiObject::prop_ClipsDescendants("ClipsDescendants", category_Behavior, &GuiObject::getClipping, &GuiObject::setClipping);
	// GLOBAL: PLAYER 0x00e0ef78
	// GLOBAL: STUDIO 0x00f9423c
	// GuiObject::prop_Visible
	Reflection::PropDescriptor<GuiObject, bool> GuiObject::prop_Visible("Visible", category_Data, &GuiObject::getVisible, &GuiObject::setVisible);
	// GLOBAL: PLAYER 0x00e0ed58
	// GLOBAL: STUDIO 0x00f94020
	// GuiObject::prop_Active
	Reflection::PropDescriptor<GuiObject, bool> GuiObject::prop_Active("Active", category_Data, &GuiObject::getActive, &GuiObject::setActive);
	// GLOBAL: PLAYER 0x00e0ede0
	// GLOBAL: STUDIO 0x00f940a4
	// GuiObject::prop_Transparency
	Reflection::PropDescriptor<GuiObject, float> GuiObject::prop_Transparency("Transparency", category_Data, &GuiObject::getBackgroundTransparency, &GuiObject::setTransparencyLegacy, Reflection::PropertyDescriptor::LEGACY_SCRIPTING);

	// The remote events and their connection counts (dynamic initializers PLAYER 0x00a7c4e0..0x00a7c900).
	// GLOBAL: PLAYER 0x00e0f3f4
	// GLOBAL: STUDIO 0x00f946bc
	// event_MouseEnter
	static Reflection::RemoteEventDesc<GuiObject, void(int, int)> event_MouseEnter(&GuiObject::mouseEnterSignal, "MouseEnter", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0f0e0
	// GLOBAL: STUDIO 0x00f943a8
	// event_MouseLeave
	static Reflection::RemoteEventDesc<GuiObject, void(int, int)> event_MouseLeave(&GuiObject::mouseLeaveSignal, "MouseLeave", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0ee6c
	// GLOBAL: STUDIO 0x00f94130
	// event_MouseMoved
	static Reflection::RemoteEventDesc<GuiObject, void(int, int)> event_MouseMoved(&GuiObject::mouseMovedSignal, "MouseMoved", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0ecf0
	// GLOBAL: STUDIO 0x00f93fb8
	// event_MouseWheelForward
	static Reflection::RemoteEventDesc<GuiObject, void(int, int)> event_MouseWheelForward(&GuiObject::mouseWheelForwardSignal, "MouseWheelForward", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0f0a4
	// GLOBAL: STUDIO 0x00f9436c
	// event_MouseWheelBackward
	static Reflection::RemoteEventDesc<GuiObject, void(int, int)> event_MouseWheelBackward(&GuiObject::mouseWheelBackwardSignal, "MouseWheelBackward", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0f11c
	// GLOBAL: STUDIO 0x00f943e4
	// event_DragStopped
	static Reflection::RemoteEventDesc<GuiObject, void(int, int)> event_DragStopped(&GuiObject::dragStoppedSignal, "DragStopped", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0efd0
	// GLOBAL: STUDIO 0x00f94294
	// event_DragBegin
	static Reflection::RemoteEventDesc<GuiObject, void(UDim2)> event_DragBegin(&GuiObject::dragBeginSignal, "DragBegin", "initialPosition", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0eaec
	// GLOBAL: STUDIO 0x00f93db4
	// GuiObject::prop_MouseEnterConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_MouseEnter, "MouseEnter", MouseEnter)
	// GLOBAL: PLAYER 0x00e0ea3c
	// GLOBAL: STUDIO 0x00f93d04
	// GuiObject::prop_MouseLeaveConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_MouseLeave, "MouseLeave", MouseLeave)
	// GLOBAL: PLAYER 0x00e0f24c
	// GLOBAL: STUDIO 0x00f94514
	// GuiObject::prop_MouseMovedConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_MouseMoved, "MouseMoved", MouseMoved)
	// GLOBAL: PLAYER 0x00e0ea94
	// GLOBAL: STUDIO 0x00f93d5c
	// GuiObject::prop_MouseWheelForwardConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_MouseWheelForward, "MouseWheelForward", MouseWheelForward)
	// GLOBAL: PLAYER 0x00e0f078
	// GLOBAL: STUDIO 0x00f94338
	// GuiObject::prop_MouseWheelBackwardConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_MouseWheelBackward, "MouseWheelBackward", MouseWheelBackward)
	// GLOBAL: PLAYER 0x00e0eac0
	// GLOBAL: STUDIO 0x00f93d88
	// GuiObject::prop_DragStoppedConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_DragStopped, "DragStopped", DragStopped)
	// GLOBAL: PLAYER 0x00e0f220
	// GLOBAL: STUDIO 0x00f944e8
	// GuiObject::prop_DragBeginConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiObject, event_DragBegin, "DragBegin", DragBegin)

	// FUNCTION: PLAYER 0x007bd250
	// FUNCTION: STUDIO 0x008d9c10
	void GuiObject::setSize(UDim2 value)
	{
		if (size != value)
		{
			size = value;
			raisePropertyChanged(prop_Size);
			checkForResize();
		}
	}

	// FUNCTION: PLAYER 0x007bd2e0
	// FUNCTION: STUDIO 0x008d9ca0
	void GuiObject::setSizeConstraint(SizeConstraint value)
	{
		if (sizeConstraint != value)
		{
			sizeConstraint = value;
			raisePropertyChanged(prop_SizeConstraint);
			checkForResize();
		}
	}

	// FUNCTION: PLAYER 0x007bd310
	// FUNCTION: STUDIO 0x008d9cd0
	void GuiObject::setPosition(UDim2 value)
	{
		if (position != value)
		{
			position = value;
			raisePropertyChanged(prop_Position);
			checkForResize();
		}
	}

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

	// FUNCTION: PLAYER 0x007bd3d0
	// FUNCTION: STUDIO 0x008d9d90
	void GuiObject::setDraggable(bool value)
	{
		if (draggable != value)
		{
			draggable = value;
			raisePropertyChanged(prop_Draggable);
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

	// FUNCTION: PLAYER 0x007bd440
	// FUNCTION: STUDIO 0x008d9e00
	void GuiObject::setBackgroundColor3(G3D::Color3 value)
	{
		if (backgroundColor != value)
		{
			backgroundColor = value;
			raisePropertyChanged(prop_BackgroundColor);
			raisePropertyChanged(prop_BackgroundColor3);
		}
	}

	// FUNCTION: PLAYER 0x007bd4c0
	// FUNCTION: STUDIO 0x008d9e80
	void GuiObject::setBorderColor3(G3D::Color3 value)
	{
		if (borderColor != value)
		{
			borderColor = value;
			raisePropertyChanged(prop_BorderColor);
			raisePropertyChanged(prop_BorderColor3);
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

	// FUNCTION: PLAYER 0x007bd580
	// FUNCTION: STUDIO 0x008d9f40
	void GuiObject::setActive(bool value)
	{
		if (active != value)
		{
			active = value;
			raisePropertyChanged(prop_Active);
			if (!active)
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

	// mac: 0x3AC2AC (55B, rarc10 raw 3AC2B2..3AC2DE): see GuiObject.h for
	// the ordering note.  Our spelling: the guard = getParent() == NULL
	// (the +0x34 read = the landed getParent), the resize vcall = our
	// unqualified virtual call (same dynamic dispatch as [vptr+0x98]),
	// the tail-jmp = a plain statement (GCC's tail form carries the
	// same (this, adorn) argument slots).
	// FUNCTION: PLAYER 0x007bd6a0
	// FUNCTION: STUDIO 0x008da060
	void GuiObject::legacyRender2d(RBX::Adorn* adorn, const G3D::Rect2D& rect)
	{
		if (getParent() == NULL)
		{
			handleResize(rect, true);
			recursiveRender2d(adorn);
		}
	}

	// GuiTarget::process: event types 1..9 are mouse events, 10 and 11 key events.
	// FUNCTION: PLAYER 0x007bd6d0
	// FUNCTION: STUDIO 0x008da090
	GuiResponse GuiObject::process(const GuiEvent& event)
	{
		if (event.isMouseEvent())
			return processMouseEvent(event);
		else if (event.isKeyEvent())
			return processKeyEvent(event);
		return GuiResponse::notSunk();
	}

	// FUNCTION: PLAYER 0x007be770
	// FUNCTION: STUDIO 0x008db0c0
	void GuiObject::setBackgroundColor(BrickColor value)
	{
		setBackgroundColor3(value.color3());
	}

	// FUNCTION: PLAYER 0x007be800
	// FUNCTION: STUDIO 0x008db150
	void GuiObject::setBorderColor(BrickColor value)
	{
		setBorderColor3(value.color3());
	}

	// TODO: body (selection outline around getRect2D, inset by the 0x00be6a44 constant).
	// STUB: PLAYER 0x007bf080
	// STUB: STUDIO 0x008dba90
	void GuiObject::renderSelectionBox(Adorn* adorn)
	{
		(void)adorn;
		STUB(0x007bf080);
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

	// FUNCTION: PLAYER 0x007bfe00
	// FUNCTION: STUDIO 0x008dc970
	void GuiObject::onPropertyChanged(const Reflection::PropertyDescriptor& descriptor)
	{
		eventReplicatorMouseEnter.onPropertyChanged(descriptor);
		eventReplicatorMouseLeave.onPropertyChanged(descriptor);
		eventReplicatorMouseMoved.onPropertyChanged(descriptor);
		eventReplicatorMouseWheelForward.onPropertyChanged(descriptor);
		eventReplicatorMouseWheelBackward.onPropertyChanged(descriptor);
		eventReplicatorDragStopped.onPropertyChanged(descriptor);
		eventReplicatorDragBegin.onPropertyChanged(descriptor);
	}

	// The 2016 security split: WritePlayer callers may use 0, everyone else 1 (minZIndex2d).
	// FUNCTION: PLAYER 0x007c1da0
	// FUNCTION: STUDIO 0x008de920
	void GuiObject::setZIndex(int value)
	{
		if (Security::Context::current().hasPermission(Security::RobloxScript))
		{
			if (value < minZIndex())
				value = minZIndex();
			else if (value > maxZIndex())
				value = maxZIndex();
		}
		else
		{
			if (value < minZIndex2d())
				value = minZIndex2d();
			else if (value > maxZIndex2d())
				value = maxZIndex2d();
		}
		if (zIndex != value)
		{
			zIndex = value;
			raisePropertyChanged(prop_ZIndex);
		}
	}

	// FUNCTION: PLAYER 0x007c34e0
	// FUNCTION: STUDIO 0x008e0060
	void GuiObject::setServerGuiObject()
	{
		serverGuiObject = true;
		eventReplicatorMouseEnter.setListenerMode(!mouseEnterSignal.empty());
		eventReplicatorMouseLeave.setListenerMode(!mouseLeaveSignal.empty());
		eventReplicatorMouseMoved.setListenerMode(!mouseMovedSignal.empty());
		eventReplicatorMouseWheelForward.setListenerMode(!mouseWheelForwardSignal.empty());
		eventReplicatorMouseWheelBackward.setListenerMode(!mouseWheelBackwardSignal.empty());
		eventReplicatorDragBegin.setListenerMode(!dragBeginSignal.empty());
		eventReplicatorDragStopped.setListenerMode(!dragStoppedSignal.empty());
	}

	// FUNCTION: PLAYER 0x007c39a0
	// FUNCTION: STUDIO 0x008e0520
	void GuiObject::forceResize()
	{
		if (GuiBase2d* parent = Instance::fastDynamicCast<GuiBase2d>(getParent()))
			handleResize(parent->getChildRect2D(), true);
	}

	// FUNCTION: PLAYER 0x007c3a00
	// FUNCTION: STUDIO 0x008e0580
	void GuiObject::checkForResize()
	{
		if (GuiBase2d* parent = Instance::fastDynamicCast<GuiBase2d>(getParent()))
			handleResize(parent->getChildRect2D(), false);
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
	// FUNCTION: PLAYER 0x007c4b00
	// FUNCTION: STUDIO 0x008e16a0
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

	// 2016 name and parameters (GuiObject.h); Scale9Frame::render2d 0x00956f80 calls it.
	// STUB: PLAYER 0x007c4d20
	// STUB: STUDIO 0x008e18c0
	void GuiObject::render2dScale9Impl(Adorn*, const ContentId&, const G3D::Vector2int16&,
		const G3D::Vector2&, GuiDrawImage&, G3D::Rect2D&, GuiObject*)
	{
		STUB(0x007c4d20);
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
	// TODO: body (mouse enter/leave/move/wheel and drag handling).
	// STUB: PLAYER 0x007c5540
	// STUB: STUDIO 0x008e20e0
	GuiResponse GuiObject::processMouseEvent(const GuiEvent& event)
	{
		(void)event;
		STUB(0x007c5540);
		return GuiResponse();
	}

	// FUNCTION: PLAYER 0x007c77c0
	// FUNCTION: STUDIO 0x008e4260
	GuiObject::GuiObject(const char* name, bool active)
		: Super(name)
		, selectionBox(false)
		, draggable(false)
		, dragging(false)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, mouseEnterSignal, event_MouseEnter, MouseEnter)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, mouseLeaveSignal, event_MouseLeave, MouseLeave)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, mouseMovedSignal, event_MouseMoved, MouseMoved)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, mouseWheelForwardSignal, event_MouseWheelForward, MouseWheelForward)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, mouseWheelBackwardSignal, event_MouseWheelBackward, MouseWheelBackward)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, dragStoppedSignal, event_DragStopped, DragStopped)
		, CONSTRUCT_EVENT_REPLICATOR(GuiObject, dragBeginSignal, event_DragBegin, DragBegin)
		, serverGuiObject(false)
		, clipping(false)
		, m_unk0x1c0(0)
		, size()
		, position()
		, sizeConstraint(RelativeXY)
		, borderSizePixel(1)
		, backgroundColor(BrickColor(BrickColor::brick_194).color3())
		, borderColor(BrickColor(BrickColor::brick_26).color3())
		, visible(true)
		, active(active)
		, backgroundTransparency(0.0f)
		, windowRectCache()
	{
		CONNECT_EVENT_REPLICATOR(MouseEnter);
		CONNECT_EVENT_REPLICATOR(MouseLeave);
		CONNECT_EVENT_REPLICATOR(MouseMoved);
		CONNECT_EVENT_REPLICATOR(DragStopped);
		CONNECT_EVENT_REPLICATOR(DragBegin);
		CONNECT_EVENT_REPLICATOR(MouseWheelForward);
		CONNECT_EVENT_REPLICATOR(MouseWheelBackward);
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

// 2013 compiled GuiButton inside GuiObject.cpp; prop_Style lives here so EnumDesc<GuiButton::Style>'s
// convertToValue is emitted with singleton() inlined (PLAYER 0x0078dfd0).
// GLOBAL: PLAYER 0x00e0ec04
// GLOBAL: STUDIO 0x00f93ecc
// GuiButton::prop_Style
Reflection::EnumPropDescriptor<GuiButton, GuiButton::Style> GuiButton::prop_Style("Style", category_Data, &GuiButton::getStyle, &GuiButton::setStyle);
} // namespace RBX

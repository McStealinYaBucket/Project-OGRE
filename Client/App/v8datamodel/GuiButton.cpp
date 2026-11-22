/* App/v8datamodel/GuiButton.cpp -- October 2013.
   2013 compiled GuiButton inside GuiObject.cpp (its asserts and its functions sit in GuiObject.obj,
   PLAYER 0x007bd060..0x007cb2e0); this file keeps the class apart and is ordered by those addresses. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/GuiButton.h"
#include "V8Tree/Property.h"

namespace RBX {
	const char* const sGuiButton = "GuiButton";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiButton);
}

DECOMP_SIZE_ASSERT(RBX::GuiButton, 0x33c);

namespace RBX {

	// The remote events and their connection counts (dynamic initializers PLAYER 0x00a7c980..0x00a7cd00).
	// GLOBAL: PLAYER 0x00e0eea8
	// GLOBAL: STUDIO 0x00f9416c
	// event_MouseButton1Click
	static Reflection::RemoteEventDesc<GuiButton, void()> event_MouseButton1Click(&GuiButton::mouseButton1ClickSignal, "MouseButton1Click", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0ec64
	// GLOBAL: STUDIO 0x00f93f2c
	// event_MouseButton2Click
	static Reflection::RemoteEventDesc<GuiButton, void()> event_MouseButton2Click(&GuiButton::mouseButton2ClickSignal, "MouseButton2Click", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0eb18
	// GLOBAL: STUDIO 0x00f93de0
	// event_MouseButton1Down
	static Reflection::RemoteEventDesc<GuiButton, void(int, int)> event_MouseButton1Down(&GuiButton::mouseButton1DownSignal, "MouseButton1Down", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0ef10
	// GLOBAL: STUDIO 0x00f941d4
	// event_MouseButton1Up
	static Reflection::RemoteEventDesc<GuiButton, void(int, int)> event_MouseButton1Up(&GuiButton::mouseButton1UpSignal, "MouseButton1Up", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0f03c
	// GLOBAL: STUDIO 0x00f942fc
	// event_MouseButton2Down
	static Reflection::RemoteEventDesc<GuiButton, void(int, int)> event_MouseButton2Down(&GuiButton::mouseButton2DownSignal, "MouseButton2Down", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0f2b4
	// GLOBAL: STUDIO 0x00f9457c
	// event_MouseButton2Up
	static Reflection::RemoteEventDesc<GuiButton, void(int, int)> event_MouseButton2Up(&GuiButton::mouseButton2UpSignal, "MouseButton2Up", "x", "y", Security::None, Reflection::RemoteEventCommon::SCRIPTING, Reflection::RemoteEventCommon::CLIENT_SERVER);
	// GLOBAL: PLAYER 0x00e0f340
	// GLOBAL: STUDIO 0x00f94608
	// GuiButton::prop_MouseButton1ClickConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiButton, event_MouseButton1Click, "MouseButton1Click", MouseButton1Click)
	// GLOBAL: PLAYER 0x00e0ef4c
	// GLOBAL: STUDIO 0x00f94210
	// GuiButton::prop_MouseButton2ClickConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiButton, event_MouseButton2Click, "MouseButton2Click", MouseButton2Click)
	// GLOBAL: PLAYER 0x00e0eb80
	// GLOBAL: STUDIO 0x00f93e48
	// GuiButton::prop_MouseButton1DownConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiButton, event_MouseButton1Down, "MouseButton1Down", MouseButton1Down)
	// GLOBAL: PLAYER 0x00e0edb0
	// GLOBAL: STUDIO 0x00f94078
	// GuiButton::prop_MouseButton1UpConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiButton, event_MouseButton1Up, "MouseButton1Up", MouseButton1Up)
	// GLOBAL: PLAYER 0x00e0eee4
	// GLOBAL: STUDIO 0x00f941a8
	// GuiButton::prop_MouseButton2DownConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiButton, event_MouseButton2Down, "MouseButton2Down", MouseButton2Down)
	// GLOBAL: PLAYER 0x00e0ebd8
	// GLOBAL: STUDIO 0x00f93ea0
	// GuiButton::prop_MouseButton2UpConnectionCount
	IMPLEMENT_EVENT_REPLICATOR(GuiButton, event_MouseButton2Up, "MouseButton2Up", MouseButton2Up)

	// GLOBAL: PLAYER 0x00e0ed2c
	// GLOBAL: STUDIO 0x00f93ff4
	// GuiButton::prop_AutoButtonColor
	Reflection::PropDescriptor<GuiButton, bool> GuiButton::prop_AutoButtonColor("AutoButtonColor", category_Data, &GuiButton::getAutoButtonColor, &GuiButton::setAutoButtonColor);
	// GLOBAL: PLAYER 0x00e0ee38
	// GLOBAL: STUDIO 0x00f940fc
	// GuiButton::prop_Selected
	Reflection::PropDescriptor<GuiButton, bool> GuiButton::prop_Selected("Selected", category_Data, &GuiButton::getSelected, &GuiButton::setSelected);
	// GLOBAL: PLAYER 0x00e0f010
	// GLOBAL: STUDIO 0x00f942d0
	// GuiButton::prop_Modal
	Reflection::PropDescriptor<GuiButton, bool> GuiButton::prop_Modal("Modal", category_Data, &GuiButton::getModal, &GuiButton::setModal);
	// prop_Style is defined in GuiObject.cpp, after the ButtonStyle EnumDesc: its EnumPropDescriptor
	// instantiates EnumDesc<Style>::convertToValue, which has to come from a TU that sees enumconverter.h.

	// FUNCTION: PLAYER 0x007bd740
	// FUNCTION: STUDIO 0x008da100
	void GuiButton::setModal(bool value)
	{
		if (modal != value)
		{
			modal = value;
			raisePropertyChanged(prop_Modal);
		}
	}

	// FUNCTION: PLAYER 0x007bd770
	// FUNCTION: STUDIO 0x008da130
	void GuiButton::setAutoButtonColor(bool value)
	{
		if (autoButtonColor != value)
		{
			autoButtonColor = value;
			raisePropertyChanged(prop_AutoButtonColor);
		}
	}

	// FUNCTION: PLAYER 0x007bd7a0
	// FUNCTION: STUDIO 0x008da160
	void GuiButton::setSelected(bool value)
	{
		if (selected != value)
		{
			selected = value;
			raisePropertyChanged(prop_Selected);
		}
	}

	// FUNCTION: PLAYER 0x007bfe70
	// FUNCTION: STUDIO 0x008dc9e0
	void GuiButton::onPropertyChanged(const Reflection::PropertyDescriptor& descriptor)
	{
		Super::onPropertyChanged(descriptor);
		eventReplicatorMouseButton1Click.onPropertyChanged(descriptor);
		eventReplicatorMouseButton2Click.onPropertyChanged(descriptor);
		eventReplicatorMouseButton1Down.onPropertyChanged(descriptor);
		eventReplicatorMouseButton1Up.onPropertyChanged(descriptor);
		eventReplicatorMouseButton2Down.onPropertyChanged(descriptor);
		eventReplicatorMouseButton2Up.onPropertyChanged(descriptor);
	}

	// FUNCTION: PLAYER 0x007c35a0
	// FUNCTION: STUDIO 0x008e0120
	void GuiButton::setServerGuiObject()
	{
		Super::setServerGuiObject();
		eventReplicatorMouseButton1Click.setListenerMode(!mouseButton1ClickSignal.empty());
		eventReplicatorMouseButton2Click.setListenerMode(!mouseButton2ClickSignal.empty());
		eventReplicatorMouseButton1Down.setListenerMode(!mouseButton1DownSignal.empty());
		eventReplicatorMouseButton1Up.setListenerMode(!mouseButton1UpSignal.empty());
		eventReplicatorMouseButton2Down.setListenerMode(!mouseButton2DownSignal.empty());
		eventReplicatorMouseButton2Up.setListenerMode(!mouseButton2UpSignal.empty());
	}

	// The red and grey styles draw from four 9-slice images.
	// FUNCTION: PLAYER 0x007c3ab0
	// FUNCTION: STUDIO 0x008e0630
	void GuiButton::setStyle(Style value)
	{
		if (style != value)
		{
			style = value;
			raisePropertyChanged(prop_Style);
			switch (style)
			{
			case CUSTOM_STYLE:
				images.reset();
				break;
			case ROBLOX_RED_STYLE:
			case ROBLOX_GREY_STYLE:
				if (!images)
					images.reset(new GuiDrawImage[4]);
				break;
			default:
				return;
			}
			forceResize();
		}
	}

	// FUNCTION: PLAYER 0x007cb2e0
	// FUNCTION: STUDIO 0x008e7d90
	GuiButton::GuiButton(const char* name)
		: Super(name, true)
		, clicked(false)
		, CONSTRUCT_EVENT_REPLICATOR(GuiButton, mouseButton1ClickSignal, event_MouseButton1Click, MouseButton1Click)
		, CONSTRUCT_EVENT_REPLICATOR(GuiButton, mouseButton2ClickSignal, event_MouseButton2Click, MouseButton2Click)
		, CONSTRUCT_EVENT_REPLICATOR(GuiButton, mouseButton1DownSignal, event_MouseButton1Down, MouseButton1Down)
		, CONSTRUCT_EVENT_REPLICATOR(GuiButton, mouseButton1UpSignal, event_MouseButton1Up, MouseButton1Up)
		, CONSTRUCT_EVENT_REPLICATOR(GuiButton, mouseButton2DownSignal, event_MouseButton2Down, MouseButton2Down)
		, CONSTRUCT_EVENT_REPLICATOR(GuiButton, mouseButton2UpSignal, event_MouseButton2Up, MouseButton2Up)
		, autoButtonColor(true)
		, selected(false)
		, modal(false)
		, style(CUSTOM_STYLE)
		, verb(NULL)
		, verbToSet("")
	{
		CONNECT_EVENT_REPLICATOR(MouseButton1Click);
		CONNECT_EVENT_REPLICATOR(MouseButton2Click);
		CONNECT_EVENT_REPLICATOR(MouseButton1Down);
		CONNECT_EVENT_REPLICATOR(MouseButton1Up);
		CONNECT_EVENT_REPLICATOR(MouseButton2Down);
		CONNECT_EVENT_REPLICATOR(MouseButton2Up);
	}

} // namespace RBX

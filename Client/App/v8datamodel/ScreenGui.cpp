/* App/v8datamodel/ScreenGui.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"

#include "V8DataModel/ScreenGui.h"
#include "V8DataModel/BasePlayerGui.h"
#include "V8Tree/Property.h"
#include "decomp.h"

namespace RBX {
	const char* const sScreenGui = "ScreenGui";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ScreenGui);
}

DECOMP_SIZE_ASSERT(RBX::ScreenGui, 0xec);

namespace RBX {

	// GLOBAL: PLAYER 0x00e13b08
	// GLOBAL: STUDIO 0x00f99778
	// ScreenGui::prop_ReplicateAbsoluteSize
	Reflection::PropDescriptor<ScreenGui, G3D::Vector2int16> ScreenGui::prop_ReplicateAbsoluteSize("ReplicatingAbsoluteSize", category_Data, &ScreenGui::getAbsoluteSize, &ScreenGui::setReplicatingAbsoluteSize, Reflection::PropertyDescriptor::REPLICATE_ONLY);
	// GLOBAL: PLAYER 0x00e13b34
	// GLOBAL: STUDIO 0x00f997a8
	// ScreenGui::prop_ReplicateAbsolutePosition
	Reflection::PropDescriptor<ScreenGui, G3D::Vector2int16> ScreenGui::prop_ReplicateAbsolutePosition("ReplicatingAbsolutePosition", category_Data, &ScreenGui::getAbsolutePosition, &ScreenGui::setReplicatingAbsolutePosition, Reflection::PropertyDescriptor::REPLICATE_ONLY);

	// The value came in over replication: only the resize matters.
	// FUNCTION: PLAYER 0x008b1d90 FOLDED
	// FUNCTION: STUDIO 0x00960160 FOLDED
	void ScreenGui::setReplicatingAbsoluteSize(G3D::Vector2int16 value)
	{
		handleResize(getRect2D(), false);
	}

	// FUNCTION: PLAYER 0x008b1d90 FOLDED
	// FUNCTION: STUDIO 0x00960160 FOLDED
	void ScreenGui::setReplicatingAbsolutePosition(G3D::Vector2int16 value)
	{
		handleResize(getRect2D(), false);
	}

	// FUNCTION: PLAYER 0x008b1dc0
	// FUNCTION: STUDIO 0x00960190
	void ScreenGui::onPropertyChanged(const Reflection::PropertyDescriptor& descriptor)
	{
		if (&descriptor == &GuiBase2d::prop_AbsoluteSize)
		{
			handleResize(windowRect, false);
			raisePropertyChanged(prop_ReplicateAbsoluteSize);
		}
		else if (&descriptor == &GuiBase2d::prop_AbsolutePosition)
		{
			handleResize(windowRect, false);
			raisePropertyChanged(prop_ReplicateAbsolutePosition);
		}
	}

	// FUNCTION: PLAYER 0x008b1e20
	// FUNCTION: STUDIO 0x009601f0
	void ScreenGui::onHeartbeat(const Heartbeat& event)
	{
		if (m_unk0xcc)
		{
			handleResize(windowRect, false);
			m_unk0xcc = false;
		}
	}

	// FUNCTION: PLAYER 0x008b2750
	// FUNCTION: STUDIO 0x00960a50
	bool ScreenGui::isAncestorRenderableGui() const
	{
		for (const Instance* parent = getParent(); parent != NULL; parent = parent->getParent())
		{
			if (parent->fastDynamicCast<BasePlayerGui>())
				return true;
		}
		return false;
	}

	// Blocked like GuiBase3d::setVisible: IAdornable::shouldRenderSetDirty is out of line in 2013
	// (0x009ca750) and inline-empty in GfxBase's IAdornable.h.
	// STUB: PLAYER 0x008b2790
	// STUB: STUDIO 0x00960a90
	void ScreenGui::onAncestorChanged(const AncestorChanged& event)
	{
		Super::onAncestorChanged(event);
		bool value = isAncestorRenderableGui();
		if (value != renderable)
		{
			renderable = value;
			shouldRenderSetDirty();
		}
		handleResize(windowRect, false);
	}

	// FUNCTION: PLAYER 0x008b2980
	// STUB: STUDIO 0x00960c80
	ScreenGui::ScreenGui()
		: Super("ScreenGui")
		, windowRect(G3D::Rect2D::xywh(0.0f, 0.0f, 800.0f, 600.0f))
		, m_unk0xcc(false)
		, renderable(false)
	{
	}

	// FUNCTION: PLAYER 0x008b2ae0
	// STUB: STUDIO 0x00960de0
	ScreenGui::ScreenGui(const char* name)
		: Super(name)
		, m_unk0xcc(false)
		, renderable(false)
	{
	}

} // namespace RBX

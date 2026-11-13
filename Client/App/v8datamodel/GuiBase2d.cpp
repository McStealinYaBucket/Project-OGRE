/* App/v8datamodel/GuiBase2d.cpp -- October 2013.
   No 2013 assert names this file; the 2016 path. Written from the PLAYER bodies 0x007bc530-0x007bcee0
   (the .obj also holds Rect2D::xywh 0x007bc730, the GetImpl ctor 0x007bc800, Name::doDeclare
   0x007bc850, visitChildren<ResizeChildren binder> 0x007bc9d0, the Described ctor 0x007bcab0 and the
   PropDescriptor ctor 0x007bcb30). The props' initializers are PLAYER 0x00a7bc50 / 0x00a7bc90. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/GuiBase2d.h"
#include "V8Tree/Instance.h"
#include "V8Tree/Property.h"
#include "Util/FastFlags.h"

namespace RBX {
	const char* const sGuiBase2d = "GuiBase2d";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiBase2d);
}

DECOMP_SIZE_ASSERT(RBX::GuiBase2d, 0x94);

namespace RBX {

	// GLOBAL: PLAYER 0x00e0e8a4
	// GLOBAL: STUDIO 0x00f93b7c
	// GuiBase2d::prop_AbsoluteSize
	Reflection::PropDescriptor<GuiBase2d, G3D::Vector2> GuiBase2d::prop_AbsoluteSize("AbsoluteSize", category_Data, &GuiBase2d::getAbsoluteSize, NULL, Reflection::PropertyDescriptor::UI);
	// GLOBAL: PLAYER 0x00e0e8d8
	// GLOBAL: STUDIO 0x00f93ba8
	// GuiBase2d::prop_AbsolutePosition
	Reflection::PropDescriptor<GuiBase2d, G3D::Vector2> GuiBase2d::prop_AbsolutePosition("AbsolutePosition", category_Data, &GuiBase2d::getAbsolutePosition, NULL, Reflection::PropertyDescriptor::UI);

	// FUNCTION: PLAYER 0x007bc530
	// FUNCTION: STUDIO 0x008d8f60
	bool GuiBase2d::setAbsolutePosition(const G3D::Vector2& value)
	{
		if (absolutePosition != value)
		{
			absolutePosition = value;
			raisePropertyChanged(prop_AbsolutePosition);
			return true;
		}
		return false;
	}

	// FUNCTION: PLAYER 0x007bc580
	// FUNCTION: STUDIO 0x008d8fb0
	bool GuiBase2d::setAbsoluteSize(const G3D::Vector2& value)
	{
		if (absoluteSize != value)
		{
			absoluteSize = value;
			raisePropertyChanged(prop_AbsoluteSize);
			return true;
		}
		return false;
	}

	// STUB: PLAYER 0x007bc5d0
	// STUB: STUDIO 0x008d9000
	bool GuiBase2d::recalculateAbsoluteSizeAndPosition(const G3D::Rect2D& viewport)
	{
		G3D::Vector2 position = viewport.x0y0();
		if (FFlag::RoundGuiAbsoluteSizeAndPosition)
			position = G3D::Vector2((float)G3D::iRound(position.x), (float)G3D::iRound(position.y));
		bool result = setAbsolutePosition(position);

		G3D::Vector2 size = viewport.x1y1() - viewport.x0y0();
		if (FFlag::RoundGuiAbsoluteSizeAndPosition)
			size = G3D::Vector2((float)G3D::iRound(size.x), (float)G3D::iRound(size.y));
		return setAbsoluteSize(size) || result;
	}

	// FUNCTION: PLAYER 0x007bc7e0
	// FUNCTION: STUDIO 0x008d9210
	G3D::Rect2D GuiBase2d::getRect2D() const
	{
		return G3D::Rect2D::xywh(absolutePosition, absoluteSize);
	}

	// FUNCTION: PLAYER 0x007bcc40
	// FUNCTION: STUDIO 0x008d9600
	GuiBase2d::GuiBase2d(const char* name)
		: Super(name)
		, absolutePosition(0.0f, 0.0f)
		, absoluteSize(0.0f, 0.0f)
		, zIndex(GuiBase::minZIndex2d())
		, guiQueue(GUIQUEUE_GENERAL)
	{
	}

	// FUNCTION: PLAYER 0x007bccc0
	// FUNCTION: STUDIO 0x008d9670
	void GuiBase2d::RecursiveRenderChildren(shared_ptr<Instance> instance, RBX::Adorn* adorn)
	{
		if (GuiBase2d* guiBase = Instance::fastDynamicCast<GuiBase2d>(instance.get()))
			guiBase->recursiveRender2d(adorn);
	}

	// FUNCTION: PLAYER 0x007bcd60
	// FUNCTION: STUDIO 0x008d9710
	void GuiBase2d::recursiveRender2d(RBX::Adorn* adorn)
	{
		render2d(adorn);
		visitChildren(boost::bind(&GuiBase2d::RecursiveRenderChildren, _1, adorn));
	}

	// FUNCTION: PLAYER 0x007bcda0
	// FUNCTION: STUDIO 0x008d9750
	static void ResizeChildren(shared_ptr<Instance> instance, const G3D::Rect2D& viewport, bool force)
	{
		if (GuiBase2d* guiBase = Instance::fastDynamicCast<GuiBase2d>(instance.get()))
			guiBase->handleResize(viewport, force);
	}

	// FUNCTION: PLAYER 0x007bce50
	// FUNCTION: STUDIO 0x008d9800
	void GuiBase2d::handleResize(const G3D::Rect2D& viewport, bool force)
	{
		if (recalculateAbsoluteSizeAndPosition(viewport) || force)
			visitChildren(boost::bind(&ResizeChildren, _1, getChildRect2D(), force));
	}

	// FUNCTION: PLAYER 0x007bcee0
	// FUNCTION: STUDIO 0x008d9890
	bool GuiBase2d::askAddChild(const Instance* instance) const
	{
		return Instance::fastDynamicCast<GuiBase2d>(instance) != NULL;
	}

} // namespace RBX

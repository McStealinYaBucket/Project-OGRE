/* App/v8datamodel/GuiBase3d.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"

#include "decomp.h"

#include "V8DataModel/GuiBase3d.h"
#include "V8Tree/Property.h"

namespace RBX {
	const char* const sGuiBase3d = "GuiBase3d";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiBase3d);
}

DECOMP_SIZE_ASSERT(RBX::GuiBase3d, 0x88);

namespace RBX {

	// The 2013 dynamic initializers (PLAYER 0x00a89230/0x00a89290/0x00a892f0) build Color,
	// Transparency and Visible in this order; 2013 has no Color3 property (2016's "Color3").
	// GLOBAL: PLAYER 0x00e18854
	// GLOBAL: STUDIO 0x00f9a878
	// GuiBase3d::prop_Color
	Reflection::PropDescriptor<GuiBase3d, BrickColor> GuiBase3d::prop_Color("Color", category_Appearance, &GuiBase3d::getColor, &GuiBase3d::setColor);
	// GLOBAL: PLAYER 0x00e187ec
	// GLOBAL: STUDIO 0x00f9a814
	// GuiBase3d::prop_Transparency
	Reflection::PropDescriptor<GuiBase3d, float> GuiBase3d::prop_Transparency("Transparency", category_Appearance, &GuiBase3d::getTransparency, &GuiBase3d::setTransparency);
	// GLOBAL: PLAYER 0x00e18824
	// GLOBAL: STUDIO 0x00f9a848
	// GuiBase3d::prop_Visible
	Reflection::PropDescriptor<GuiBase3d, bool> GuiBase3d::prop_Visible("Visible", category_Data, &GuiBase3d::getVisible, &GuiBase3d::setVisible);

	// FUNCTION: PLAYER 0x00952c10
	// FUNCTION: STUDIO 0x009869f0
	void GuiBase3d::setColor(BrickColor value)
	{
		if (color != value)
		{
			color = value;
			raisePropertyChanged(prop_Color);
		}
	}

	// FUNCTION: PLAYER 0x00952c30
	// FUNCTION: STUDIO 0x00986a10
	void GuiBase3d::setTransparency(float value)
	{
		if (transparency != value)
		{
			transparency = value;
			raisePropertyChanged(prop_Transparency);
		}
	}

	// Blocked: the 2013 body calls IAdornable::shouldRenderSetDirty out of line (0x009ca750);
	// GfxBase's IAdornable.h (Rendering) still defines it inline with an empty body.
	// STUB: PLAYER 0x00952c60
	// STUB: STUDIO 0x00986a40
	void GuiBase3d::setVisible(bool value)
	{
		if (visible != value)
		{
			visible = value;
			raisePropertyChanged(prop_Visible);
			shouldRenderSetDirty();
		}
	}

	// FUNCTION: PLAYER 0x00953230
	// FUNCTION: STUDIO 0x00987010
	GuiBase3d::GuiBase3d(const char* name)
		: Super(name)
		, color(BrickColor::brick_23)
		, transparency(0.0f)
		, visible(true)
	{
	}

} // namespace RBX

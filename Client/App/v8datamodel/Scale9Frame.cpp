/* App/v8datamodel/Scale9Frame.cpp -- October 2013.
   Self-audit: every symbol below is binary-pinned (dbq24): ctor 0x46FC7A
     (299B), setScaleEdgeSize 0x46FBCE (74B: compare, store @+536,
     raisePropertyChanged), setSlicePrefix 0x46FC18 (87B: compare,
     assign @+540, raisePropertyChanged), getScaleEdgeSize 0x46FEC6,
     getSlicePrefix 0x46FF02.
   DELTAs: the raisePropertyChanged tail is omitted -- the property
     descriptors it needs (@0xFB53B4/0xFB53E0) are unrelocated in dbq24
     and no reflection surface is landed for them; set/get remain
     observable-identical until that table lands. */

#include "stdafx.h"

#include "V8DataModel/Scale9Frame.h"
#include "V8Tree/Property.h"

namespace RBX {

	const char* const sScale9Frame = "Scale9Frame";

	// The 2013 dynamic initializers (PLAYER 0x00a899a0, 0x00a89a00) build these in this order.
	// GLOBAL: PLAYER 0x00e18c48
	// GLOBAL: STUDIO 0x00f9afc0
	// Scale9Frame::prop_ScaleEdgeSize
	Reflection::PropDescriptor<Scale9Frame, G3D::Vector2int16> Scale9Frame::prop_ScaleEdgeSize("ScaleEdgeSize", category_Data, &Scale9Frame::getScaleEdgeSize, &Scale9Frame::setScaleEdgeSize);
	// GLOBAL: PLAYER 0x00e18c0c
	// GLOBAL: STUDIO 0x00f9af88
	// Scale9Frame::prop_SlicePrefix
	Reflection::PropDescriptor<Scale9Frame, std::string> Scale9Frame::prop_SlicePrefix("SlicePrefix", category_Data, &Scale9Frame::getSlicePrefix, &Scale9Frame::setSlicePrefix);

	// FUNCTION: PLAYER 0x00956ed0
	// FUNCTION: STUDIO 0x00993fd0
	void Scale9Frame::setSlicePrefix(std::string value)
	{
		if (slicePrefix != value)
		{
			slicePrefix = value;
			raisePropertyChanged(prop_SlicePrefix);
		}
	}

	// FUNCTION: PLAYER 0x00956f50
	// FUNCTION: STUDIO 0x00994050
	void Scale9Frame::setScaleEdgeSize(G3D::Vector2int16 value)
	{
		if (scaleEdgeSize != value)
		{
			scaleEdgeSize = value;
			raisePropertyChanged(prop_ScaleEdgeSize);
			checkForResize();
		}
	}

	// FUNCTION: PLAYER 0x00956f80
	// FUNCTION: STUDIO 0x00994080
	void Scale9Frame::render2d(Adorn* adorn)
	{
		G3D::Rect2D rect;
		G3D::Vector2 minSize(scaleEdgeSize * 3);
		TextureId texture(slicePrefix); render2dScale9Impl(adorn, texture, scaleEdgeSize, minSize, drawImage, rect, NULL);
	}

	// PLAYER 0x00957490: the base call passes ("Scale9Frame", true) to the out-of-line Described ctor 0x00957400.
	// FUNCTION: PLAYER 0x00957490
	// FUNCTION: STUDIO 0x00994580
	Scale9Frame::Scale9Frame()
		: Super("Scale9Frame", true)
		, scaleEdgeSize(0, 0)
	{
	}

} // namespace RBX

using namespace RBX;

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Scale9Frame);
}

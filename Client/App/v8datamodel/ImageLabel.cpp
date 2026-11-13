/* App/v8datamodel/ImageLabel.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/ImageLabel.h"

namespace RBX {
	const char* const sImageLabel = "ImageLabel";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ImageLabel);
}

DECOMP_SIZE_ASSERT(RBX::GuiImageMixin, 0xb8);
DECOMP_SIZE_ASSERT(RBX::ImageLabel, 0x2d4);

namespace RBX {

	// The original raises prop_ImageRectOffset (through the pointer at PLAYER 0x00dec0e4) after the store.
	// STUB: PLAYER 0x006bbe20
	// STUB: STUDIO 0x00851820
	void ImageLabel::setImageRectOffset(G3D::Vector2 value)
	{
		if (imageRectOffset != value)
			imageRectOffset = value;
	}

	// The original raises prop_ImageRectSize (through the pointer at PLAYER 0x00dec0e8) after the store.
	// STUB: PLAYER 0x006bbe70
	// STUB: STUDIO 0x00851870
	void ImageLabel::setImageRectSize(G3D::Vector2 value)
	{
		if (imageRectSize != value)
			imageRectSize = value;
	}

	// The original raises prop_Image (PLAYER 0x00dec104) after the store.
	// STUB: PLAYER 0x006bc280
	// STUB: STUDIO 0x00851b40
	void ImageLabel::setImage(TextureId value)
	{
		if (image != value)
			image = value;
	}

	// FUNCTION: PLAYER 0x006bdb40
	// FUNCTION: STUDIO 0x00851f80
	ImageLabel::ImageLabel()
		: Super("ImageLabel")
	{
	}

} // namespace RBX

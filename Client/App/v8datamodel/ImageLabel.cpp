/* App/v8datamodel/ImageLabel.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/ImageLabel.h"
#include "V8Tree/Property.h"

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

	// The ImageRect properties are registered late, by registerNewImageAPI (DataModel.cpp,
	// PLAYER 0x0064ea00), which points these at its function-local descriptors.
	// GLOBAL: PLAYER 0x00dec0e4
	// GLOBAL: STUDIO 0x00f8e5a4
	// ImageLabel::prop_ImageRectOffset
	const Reflection::PropertyDescriptor* ImageLabel::prop_ImageRectOffset = NULL;
	// GLOBAL: PLAYER 0x00dec0e8
	// GLOBAL: STUDIO 0x00f8e5a8
	// ImageLabel::prop_ImageRectSize
	const Reflection::PropertyDescriptor* ImageLabel::prop_ImageRectSize = NULL;
	// GLOBAL: PLAYER 0x00dec104
	// GLOBAL: STUDIO 0x00f8e5c4
	// ImageLabel::prop_Image
	Reflection::PropDescriptor<ImageLabel, TextureId> ImageLabel::prop_Image("Image", category_Data, &GuiImageMixin::getImage, &ImageLabel::setImage);

	// FUNCTION: PLAYER 0x006bbe20
	// FUNCTION: STUDIO 0x00851820
	void ImageLabel::setImageRectOffset(G3D::Vector2 value)
	{
		if (imageRectOffset != value)
		{
			imageRectOffset = value;
			if (prop_ImageRectOffset)
				raisePropertyChanged(*prop_ImageRectOffset);
		}
	}

	// FUNCTION: PLAYER 0x006bbe70
	// FUNCTION: STUDIO 0x00851870
	void ImageLabel::setImageRectSize(G3D::Vector2 value)
	{
		if (imageRectSize != value)
		{
			imageRectSize = value;
			if (prop_ImageRectSize)
				raisePropertyChanged(*prop_ImageRectSize);
		}
	}

	// FUNCTION: PLAYER 0x006bc280
	// FUNCTION: STUDIO 0x00851b40
	void ImageLabel::setImage(TextureId value)
	{
		if (image != value)
		{
			image = value;
			raisePropertyChanged(prop_Image);
		}
	}

	// FUNCTION: PLAYER 0x006bdb40
	// FUNCTION: STUDIO 0x00851f80
	ImageLabel::ImageLabel()
		: Super("ImageLabel")
	{
	}

} // namespace RBX

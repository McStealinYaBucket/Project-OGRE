/* V8DataModel/GuiMixin.h -- October 2013.
   No 2013 string names this header; the 2016 path (App/include/v8datamodel/GuiMixin.h).
   GuiImageMixin is the polymorphic second base of ImageLabel at +0x21c. Layout from its ctor, PLAYER
   0x006bc910 (STUDIO 0x00851c10): its vftable, the TextureId image +0x04, imageRectOffset +0x24 and
   imageRectSize +0x2c (0.0f each), the GuiDrawImage +0x34 (ctor 0x006bc570); ImageLabel's size
   0x2d4 - 0x21c = 0xb8.
   2016 differs: no vftable (2013 has three pure virtual setters, the slots ImageLabel's vftable
   0x00b3a96c fills and ChatOutput's createChatBubbleTail 0x0096e5b0 calls through [vtbl+0]), and it
   adds imageTransparency, imageColor, sliceCenter and imageScale. */

#pragma once

#include "Util/ContentId.h"
#include "Gui/GuiDrawImage.h"
#include "G3D/Vector2.h"

namespace RBX {

	// VTABLE: PLAYER 0x00b3a604
	// VTABLE: STUDIO 0x00c1c47c
	// SIZE 0xb8
	class GuiImageMixin
	{
	public:
		// FUNCTION: PLAYER 0x006bc910
		// FUNCTION: STUDIO 0x00851c10
		GuiImageMixin() {}

		virtual void setImage(TextureId value) = 0;                 // vtable+0x00
		virtual void setImageRectOffset(G3D::Vector2 value) = 0;    // vtable+0x04
		virtual void setImageRectSize(G3D::Vector2 value) = 0;      // vtable+0x08

		TextureId getImage() const { return image; }
		G3D::Vector2 getImageRectOffset() const { return imageRectOffset; }
		G3D::Vector2 getImageRectSize() const { return imageRectSize; }

	protected:
		TextureId image;               // 0x04
		G3D::Vector2 imageRectOffset;  // 0x24
		G3D::Vector2 imageRectSize;    // 0x2c
		GuiDrawImage guiImageDraw;     // 0x34
	};

} // namespace RBX

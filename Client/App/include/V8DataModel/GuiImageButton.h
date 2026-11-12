/* V8DataModel/GuiImageButton.h -- October 2013 (Verb-image button).
   Evidence: FactoryProduct<GuiImageButton, GuiButton, sGuiImageButton>
     (script-creatable "ImageButton"); C2(void) 0x3F4A2C; C2(Verb*) 0x3F4CDE
     (689B: GuiButton("ImageButton") base + zeroed +0x320..+0x38C mixin
     region + ContentId @+804/+0x324 + GuiDrawImage(void) @+828/+0x33C +
     state 127 @+0x390/+228dw + verb @+0x314/+197dw); setImage(TextureId)
     0x3F4F90 (96B: ContentId change-detect at +804, assign, raise
     unk_FB3458); rect offset/size setters + render2d (Adorn -- TODO).
   DELTAS: GuiImageMixin flattened (secondary vtable at +0x320 unmodeled --
     same MI-flattening precedent as IAdornable); the setImage raise
     (descriptor EA unknown, nothing listens); texture binding (render).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiImageButton.h */

#pragma once

#include "V8DataModel/GuiButton.h"
#include "Gui/GuiDrawImage.h"
#include "Util/ContentId.h"

namespace RBX {

	extern const char* const sGuiImageButton;

	class Verb;

	class GuiImageButton
		: public DescribedCreatable<GuiImageButton, GuiButton, sGuiImageButton>
	{
	public:
		// Default ctor keeps FactoryProduct::createInstance() (which builds
		// arg-less) working; matches C2(void) 0x3F4A2C's shape (named, verb
		// NULL).  Verb-taking form below matches C2(Verb*) 0x3F4CDE.
		GuiImageButton()
			: imageVerb(NULL)
			, imageState(127)
		{
			Instance::setName("ImageButton");
		}
		// mac: 0x3F4CDE.  Image rect state (+0x320..+0x38C) is zeroed like the
		// binary; the draw member default-constructs (contents TODO).  The
		// "ImageButton" name rides the GuiButton(name) base like the binary
		// (verified end to end by the LuaGui name checks); the direct base
		// here is the DescribedCreatable chain, so it goes through setName
		// in the body -- same observable.
		explicit GuiImageButton(Verb* verb)
			: imageVerb(verb)
			, imageState(127)
		{
			Instance::setName("ImageButton");
		}

		// mac: 0x3F4F90 (96B): ContentId change-detect + assign (+808 name copy
		// rides along); raise is a DELTA (see header).
		void setImage(const TextureId& value)
		{
			if (image != value)
				image = value;
		}
		const TextureId& getImage() const { return image; }

		// +0x390/+228dw image state (C2 127; buildLuaGui writes 7).
		// Bring-up accessors: the binary touches it directly.
		int getImageState() const { return imageState; }
		void setImageState(int value) { imageState = value; }

		Verb* getImageVerb() const { return imageVerb; }

	private:
		// +0x314/+197dw: the activating verb (may be NULL in the harness;
		// binary always passes a real one).
		Verb* imageVerb;
		// +804/+0x324: image identity (bound to textures by the render
		// pipeline -- unported).
		TextureId image;
		// +828/+0x33C: draw member (void-C2 contents TODO).
		GuiDrawImage drawImage;
		// +0x390/+228dw.
		int imageState;
	};

} // namespace RBX

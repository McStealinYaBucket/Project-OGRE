/* V8DataModel/ImageLabel.h -- October 2013 STUB (image surface landing).
   Evidence: sImageLabel descriptor referenced by Workspace::start/stop
     0x515636/0x515896 isA-checks; vtable read of __ZTVN3RBX10ImageLabelE
     @0xEDE030: slot 49 = setImage(TextureId) 0x3F6A4A (the ChatOutput
     bubble factories call vtable+0xC4 = this slot), slot 50 =
     setImageRectOffset(G3D::Vector2) 0x3F6ACC, slot 51 = setImageRectSize
     (G3D::Vector2) 0x3F6B58, slot 52 = renderBackground2d(Adorn*)
     0x3F6E02 (slotted 49-52 TODO).  A __ZThn536 setImage thunk exists =
     a secondary base at +536 also exposes setImage(TextureId) (that base
     class is undecoded -- TODO).
   TextureId typing: the binary mangles a distinct RBX::TextureId in
     ImageLabel::setImage (no out-of-line TextureId ctors; the factories'
     ContentIdC2ERKSs runs 0x5C1192/0x5C1707/0x5C1C01/0x5C1DDB build the
     ContentId subobject at OFFSET 0 of the passed temp).  Landing uses
     ContentId.h's typedef ContentId TextureId -- at every factory call
     site the compiled form is byte-identical (temp ctor + straight
     pass); the type-NAME distinction shows only in callee mangles
     (DELTAs).
   TODO: full decompile (image rendering, rect fields, secondary base,
     the rest of the vtable).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ImageLabel.h */

#pragma once

#include "V8DataModel/GuiLabel.h"
#include "Util/ContentId.h"

namespace RBX {

	extern const char* const sImageLabel;

	class ImageLabel
		: public DescribedCreatable<ImageLabel, GuiLabel, sImageLabel>
		// FactoryProduct (was mis-stubbed NonCreatable; binary EAs show
		// FactoryProduct<ImageLabel, GuiLabel>).
	{
	public:
		ImageLabel() {}

		// vtable+0xC4 (slot 49 of the primary vtable) @0x3F6A4A: Image
		// property setter, parameter BY VALUE (mangle 8setImageENS_9Tex-
		// tureIdE).  The binary body also raises the Image property change
		// -- raisePropertyChanged is unported (DELTAs); store-only here.
		virtual void setImage(TextureId value) { image = value; }

	private:
		TextureId image;  // the Image prop store (stub; prop wiring TODO).
	};

} // namespace RBX

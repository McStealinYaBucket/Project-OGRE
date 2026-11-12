/* V8DataModel/Scale9Frame.h -- October 2013 (nine-slice backing frame).
   Evidence (dbq24): sScale9Frame Name object @0xEE32F0; class vtable
     __ZTVN3RBX11Scale9FrameE @0xEE3300; descriptor =
     Described<Scale9Frame, sScale9Frame, NonFactoryProduct<GuiObject,
     sScale9Frame>, ClassDescriptor::Functionality 27 (= PERSISTENT per
     the landed GuiLabel mapping), Permissions 0> (mangle 0x366A96;
     ClassRegistrar @0xFA7514; the __ZTS alias mangles exactly the
     DescribedNonCreatable shape).
   C2(void) 0x46FC7A (299B): GuiObject base + scaleEdgeSize
     G3D::Vector2int16 @+536 zero-init + slicePrefix std::string @+540
     (empty) + GuiDrawImage @+544 (default ctor).
   Surface (all EAs direct from the symbol table): setScaleEdgeSize
     0x46FBCE (74B: compare + store + raisePropertyChanged -- property
     descriptors @0xFB53B4/0xFB53E0 are unrelocated in dbq24, unported
     DELTA), getScaleEdgeSize 0x46FEC6 (14B), setSlicePrefix 0x46FC18
     (87B: compare + assign + raisePropertyChanged DELTA), getSlice-
     Prefix 0x46FF02, render2d 0x46FD9C (+96 thunk 0x46FEA6, unported),
     D1 0x46FF58 / D0 0x46FFEE.
   Creatable<Instance>::create<Scale9Frame> 0x5CA998 = the ChatOutput
     bubble factories' entry (0x5C0D8D..): "NonCreatable" describes
     REFLECTION creatability, not the Creatable<> template.
   DELTAs: member names behavior-pinned (property names resolve only
     through the unrelocated descriptor regions); sizeof GuiDrawImage/
     std::string differs from the binary (MSVC 32B strings vs 4B
     Mach-O COW) -- member ORDER is the invariant.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Scale9Frame.h */

#pragma once

#include "V8DataModel/GuiObject.h"
#include "Gui/GuiDrawImage.h"
#include "G3D/Vector2int16.h"
#include <string>

namespace RBX {

	extern const char* const sScale9Frame;

	class Scale9Frame
		: public DescribedNonCreatable<Scale9Frame, GuiObject, sScale9Frame, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		// mac: 0x46FC7A (C2, 299B).
		Scale9Frame();

		// mac: 0x46FBCE (74B).
		void setScaleEdgeSize(const G3D::Vector2int16& value);
		// mac: 0x46FEC6 (14B).
		G3D::Vector2int16 getScaleEdgeSize() const { return scaleEdgeSize; }
		// mac: 0x46FC18 (87B).
		void setSlicePrefix(const std::string& value);
		// mac: 0x46FF02.
		std::string getSlicePrefix() const { return slicePrefix; }

	private:
		// Binary layout slots (GCC offsets; MSVC sizeof DELTA):
		G3D::Vector2int16 scaleEdgeSize;  // +536 (C2 word-pair zero-init).
		std::string slicePrefix;          // +540 (empty).
		GuiDrawImage drawImage;           // +544 (member name DELTA).
	};

} // namespace RBX

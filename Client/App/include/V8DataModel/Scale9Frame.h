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
		typedef DescribedNonCreatable<Scale9Frame, GuiObject, sScale9Frame, Reflection::ClassDescriptor::PERSISTENT> Super;
	public:
		// mac: 0x46FC7A (C2, 299B).
		Scale9Frame();

		// mac: 0x46FBCE (74B).
		void setScaleEdgeSize(G3D::Vector2int16 value);
		// FUNCTION: PLAYER 0x00956ec0
		// FUNCTION: STUDIO 0x00993fc0
		G3D::Vector2int16 getScaleEdgeSize() const { return scaleEdgeSize; }
		// mac: 0x46FC18 (87B).
		void setSlicePrefix(std::string value);
		// FUNCTION: PLAYER 0x00956e90
		// FUNCTION: STUDIO 0x00993f90
		std::string getSlicePrefix() const { return slicePrefix; }

		/*override*/ void render2d(Adorn* adorn);

		static Reflection::PropDescriptor<Scale9Frame, G3D::Vector2int16> prop_ScaleEdgeSize;
		static Reflection::PropDescriptor<Scale9Frame, std::string> prop_SlicePrefix;

	private:
		// Binary layout slots (GCC offsets; MSVC sizeof DELTA):
		G3D::Vector2int16 scaleEdgeSize;  // +536 (C2 word-pair zero-init).
		std::string slicePrefix;          // +540 (empty).
		GuiDrawImage drawImage;           // +544 (member name DELTA).
	};


	// Out-of-line template and implicit-member copies the original emitted for this class, paired by
	// identical masked bytes (tools/unpaired_twins.py on a 2026-11-13 build).
	// clang-format off
	// The Described ctor Scale9Frame() 0x00957490 calls with ("Scale9Frame", true).
	// TEMPLATE: PLAYER 0x00957400
	// TEMPLATE: STUDIO 0x009944f0
	// ??$?0PBD_N@?$Described@VScale9Frame@RBX@@$E?sScale9Frame@2@3QBDBV?$NonFactoryProduct@VGuiObject@RBX@@$E?sScale9Frame@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD_N@Z

	// SYNTHETIC: PLAYER 0x00957550
	// SYNTHETIC: STUDIO 0x00994640
	// RBX::Scale9Frame::~Scale9Frame
	// clang-format on

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x00957050
// TEMPLATE: STUDIO 0x00994150
// ??$getset@P8Scale9Frame@RBX@@BE?AVVector2int16@G3D@@XZP812@AEXV34@@Z@?$PropDescriptor@VScale9Frame@RBX@@VVector2int16@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@@std@@P8Scale9Frame@2@BE?AVVector2int16@G3D@@XZP852@AEXV67@@Z@Z
// TEMPLATE: PLAYER 0x009570b0
// TEMPLATE: STUDIO 0x009941b0
// ??$getset@P8Scale9Frame@RBX@@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZP812@AEXV34@@Z@?$PropDescriptor@VScale9Frame@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@@std@@P8Scale9Frame@2@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@4@XZP852@AEXV64@@Z@Z
// clang-format on

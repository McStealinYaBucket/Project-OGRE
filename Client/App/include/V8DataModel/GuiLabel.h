/* V8DataModel/GuiLabel.h — October 2013 (label shell).
   Evidence: GuiLabel C2 0x3AEE42 (GuiObject(name, false);
     Described<GuiLabel, sGuiLabel, GuiObject, 27>).
     sGuiLabel 16 hits in the 0.125 PE string table.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiLabel.h */

#pragma once

#include "V8DataModel/GuiObject.h"

namespace RBX {

	extern const char* const sGuiLabel;

	// VTABLE: PLAYER 0x00b678f4 RBX::Debugable
	// VTABLE: STUDIO 0x00c353ac RBX::Debugable
	// VTABLE: PLAYER 0x00b678c8 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c35380 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b67894 RBX::IAdornable
	// VTABLE: STUDIO 0x00c3534c RBX::IAdornable
	// SIZE 0x21c
	class GuiLabel
		: public DescribedNonCreatable<GuiLabel, GuiObject, sGuiLabel, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		GuiLabel(const char* name = NULL);
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x007cafe0
	// TEMPLATE: STUDIO 0x008e7a90
	// ??$?0PBD_N@?$Described@VGuiLabel@RBX@@$E?sGuiLabel@2@3QBDBV?$NonFactoryProduct@VGuiObject@RBX@@$E?sGuiLabel@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD_N@Z
	// clang-format on

} // namespace RBX

/* V8DataModel/Frame.h -- October 2013.
   Layout from the PLAYER ctor 0x006c2b70 (STUDIO 0x00855690): the Described ctor ("Frame", false),
   style +0x21c = CUSTOM_STYLE, GuiDrawImage +0x220 (its ctor 0x006bc570); Creatable<Instance>::
   create<Frame> allocates 0x2a4. getChildRect2D 0x006c1dd0 switches on +0x21c over 0..5.
   2016 adds a seventh style (ROBLOX_DROPSHADOW_STYLE, value 6): 2013's jump table has six. */

#pragma once

#include "V8DataModel/GuiObject.h"
#include "Gui/GuiDrawImage.h"

namespace RBX {

	extern const char* const sFrame;

	// VTABLE: PLAYER 0x00b3ba04 RBX::Debugable
	// VTABLE: STUDIO 0x00c1d61c RBX::Debugable
	// VTABLE: PLAYER 0x00b3b9d4 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c1d5ec RBX::GuiTarget
	// VTABLE: PLAYER 0x00b3b9a0 RBX::IAdornable
	// VTABLE: STUDIO 0x00c1d5b8 RBX::IAdornable
	// SIZE 0x2a4
	class Frame
		: public DescribedCreatable<Frame, GuiObject, sFrame>
	{
		typedef DescribedCreatable<Frame, GuiObject, sFrame> Super;
	public:
		enum Style
		{
			CUSTOM_STYLE = 0,
			BLUE_CHAT_STYLE = 1,
			ROBLOX_SQUARE_STYLE = 2,
			ROBLOX_ROUND_STYLE = 3,
			GREEN_CHAT_STYLE = 4,
			RED_CHAT_STYLE = 5,
		};

		Frame();

		/*override*/ G3D::Rect2D getChildRect2D() const;

	private:
		Style style;         // 0x21c
		GuiDrawImage image;  // 0x220
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x006c2ab0
	// TEMPLATE: STUDIO 0x00855600
	// ??$?0PBD_N@?$Described@VFrame@RBX@@$E?sFrame@2@3QBDBV?$FactoryProduct@VFrame@RBX@@VGuiObject@2@$E?sFrame@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD_N@Z
	// clang-format on

} // namespace RBX

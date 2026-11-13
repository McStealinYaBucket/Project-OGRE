/* App/v8datamodel/Frame.cpp -- October 2013. The asserts record this path (Frame.cpp:43 in getChildRect2D
   0x006c1dd0; the file is laid out so that RBXASSERT lands on line 43). */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/Frame.h"

namespace RBX {
	const char* const sFrame = "Frame";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Frame);
}

DECOMP_SIZE_ASSERT(RBX::Frame, 0x2a4);

namespace RBX {

	// FUNCTION: PLAYER 0x006c1dd0
	// FUNCTION: STUDIO 0x00854ce0
	G3D::Rect2D Frame::getChildRect2D() const
	{
		switch (style)
		{
		case CUSTOM_STYLE:
			return getRect2D();
		case BLUE_CHAT_STYLE:
		case GREEN_CHAT_STYLE:
		case RED_CHAT_STYLE:
			return Scale9Rect2D(getRect2D(), 17, 60);
		case ROBLOX_SQUARE_STYLE:
		case ROBLOX_ROUND_STYLE:
			return Scale9Rect2D(getRect2D(), 8, 21);
		default:
			RBXASSERT(0);
			return getRect2D();
		}
	}

	// FUNCTION: PLAYER 0x006c2b70
	// FUNCTION: STUDIO 0x00855690
	Frame::Frame()
		: Super("Frame", false)
		, style(CUSTOM_STYLE)
	{
	}

} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006c2630
// FUNCTION: STUDIO 0x00855240
template<>
EnumDesc<Frame::Style>::EnumDesc()
	: EnumDescriptor("FrameStyle", typeid(Frame::Style))
{
	addPair(Frame::CUSTOM_STYLE, "Custom");
	addPair(Frame::BLUE_CHAT_STYLE, "ChatBlue");
	addPair(Frame::ROBLOX_SQUARE_STYLE, "RobloxSquare");
	addPair(Frame::ROBLOX_ROUND_STYLE, "RobloxRound");
	addPair(Frame::GREEN_CHAT_STYLE, "ChatGreen");
	addPair(Frame::RED_CHAT_STYLE, "ChatRed");
}

template<>
const Type& Type::getSingleton<Frame::Style>()
{
	return EnumDesc<Frame::Style>::singleton();
}

template<> EnumRegistrar<Frame::Style> EnumRegistrar<Frame::Style>::registrar(0);
template<> TypeRegistrar<Frame::Style> TypeRegistrar<Frame::Style>::registrar(0);

} // namespace Reflection
} // namespace RBX

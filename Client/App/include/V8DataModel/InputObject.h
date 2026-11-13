/* V8DataModel/InputObject.h -- October 2013 (first import, section 228).
   Evidence: Bridge<InputObject>::on_index disasm (batch370) reads the
     object as a PLAIN 20B struct -- UserInputType @+0, UserInputState @+4,
     ScreenPosition (2 floats, pushed as G3D::Vector2) @+8, KeyCode @+16.
     NO vtable slot is ever touched (arg_0 IS the payload start), so 2013's
     InputObject is NOT an Instance subclass -- the 2016 DescribedCreatable
     shape (position/delta/weakWorkspace/reflection props) is a later
     redesign and is NOT carried. Enum values: the 2013 EnumDesc ctors
     (PLAYER 0x008b6130, 0x008b62a0), not the 2016 header.
   Original file: Client/App/v8datamodel/InputObject.h (2013 path TBD). */

#pragma once

#include "G3D/Vector2.h"
#include "Util/KeyCode.h"

namespace RBX {

	class InputObject
	{
	public:
		// 2013 values: EnumDesc ctors PLAYER 0x008b6130 / 0x008b62a0 register exactly these
		// (the 2016 MouseIdle/MouseDelta/Focus/Gamepad/TextInput/Cancel values came later).
		enum UserInputType
		{
			TYPE_MOUSEBUTTON1 = 0,
			TYPE_MOUSEBUTTON2 = 1,
			TYPE_MOUSEBUTTON3 = 2,
			TYPE_MOUSEWHEEL = 3,
			TYPE_MOUSEMOVEMENT = 4,
			TYPE_TOUCH = 5,
			TYPE_KEYBOARD = 6,
			TYPE_NONE = 7
		};

		enum UserInputState
		{
			INPUT_STATE_BEGIN = 0,
			INPUT_STATE_CHANGE = 1,
			INPUT_STATE_END = 2,
			INPUT_STATE_NONE = 3
		};


		UserInputType inputType;
		UserInputState inputState;
		G3D::Vector2 screenPosition;
		KeyCode keyCode;

		InputObject()
			: inputType(TYPE_NONE)
			, inputState(INPUT_STATE_NONE)
			, screenPosition(0, 0)
			, keyCode(SDLK_UNKNOWN)
		{}

		InputObject(UserInputType type, UserInputState state,
			const G3D::Vector2& pos, KeyCode code)
			: inputType(type)
			, inputState(state)
			, screenPosition(pos)
			, keyCode(code)
		{}

		// Called by Bridge<InputObject>::on_eq (0x16FCAA); member-wise is
		// the only sane shape (no EA banked for the operator itself).
		bool operator==(const InputObject& other) const
		{
			return inputType == other.inputType
				&& inputState == other.inputState
				&& screenPosition == other.screenPosition
				&& keyCode == other.keyCode;
		}
		bool operator!=(const InputObject& other) const
		{
			return !(*this == other);
		}
	};

} // namespace RBX

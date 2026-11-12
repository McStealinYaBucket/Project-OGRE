/* V8DataModel/InputObject.h -- October 2013 (first import, section 228).
   Evidence: Bridge<InputObject>::on_index disasm (batch370) reads the
     object as a PLAIN 20B struct -- UserInputType @+0, UserInputState @+4,
     ScreenPosition (2 floats, pushed as G3D::Vector2) @+8, KeyCode @+16.
     NO vtable slot is ever touched (arg_0 IS the payload start), so 2013's
     InputObject is NOT an Instance subclass -- the 2016 DescribedCreatable
     shape (position/delta/weakWorkspace/reflection props) is a later
     redesign and is NOT carried. Enum values carried from March 2016
     v8datamodel/InputObject.h (loud DELTA: no 2013 EA verification yet;
     values surface only through the enum-descriptor era).
   Original file: Client/App/v8datamodel/InputObject.h (2013 path TBD). */

#pragma once

#include "G3D/Vector2.h"
#include "Util/KeyCode.h"

namespace RBX {

	class InputObject
	{
	public:
		enum UserInputType
		{
			TYPE_MOUSEBUTTON1 = 0,
			TYPE_MOUSEBUTTON2 = 1,
			TYPE_MOUSEBUTTON3 = 2,
			TYPE_MOUSEWHEEL = 3,
			TYPE_MOUSEMOVEMENT = 4,
			TYPE_MOUSEIDLE = 5,
			TYPE_MOUSEDELTA = 6,
			TYPE_TOUCH = 7,
			TYPE_KEYBOARD = 8,
			TYPE_FOCUS = 9,
			TYPE_ACCELEROMETER = 10,
			TYPE_GYRO = 11,
			TYPE_GAMEPAD1 = 12,
			TYPE_GAMEPAD2 = 13,
			TYPE_GAMEPAD3 = 14,
			TYPE_GAMEPAD4 = 15,
			TYPE_GAMEPAD5 = 16,
			TYPE_GAMEPAD6 = 17,
			TYPE_GAMEPAD7 = 18,
			TYPE_GAMEPAD8 = 19,
			TYPE_TEXTINPUT = 20,
			TYPE_NONE = 21
		};

		enum UserInputState
		{
			INPUT_STATE_BEGIN = 0,
			INPUT_STATE_CHANGE = 1,
			INPUT_STATE_END = 2,
			INPUT_STATE_CANCEL = 3,
			INPUT_STATE_NONE = 4
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

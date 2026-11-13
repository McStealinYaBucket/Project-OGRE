/* v8datamodel/InputObject.cpp -- October 2013 (first import, section 228).
   Evidence: StringConverter<InputObject>::convertToString 0x6F05F8
     (batch371: EnumDesc<UserInputType> + ", " + EnumDesc<UserInputState> +
     ", " + StringConverter<Vector2>(screen@+8) + ", " +
     StringConverter<int>(keycode@+16)). The EnumDesc pair lists are
     carried from March 2016 v8datamodel/InputObject.cpp (loud DELTA: no
     2013 EA verification for the name/value pairs yet; the tostring test
     verifies the composition shape, not the table text). convertToValue
     returns false like 2016. EnumRegistrar lines follow the PartInstance
     Material precedent (required by our EnumDesc dtor).
   Original file: Client/App/v8datamodel/InputObject.cpp */

#include "stdafx.h"

#include "V8DataModel/InputObject.h"
#include "Reflection/EnumConverter.h"
#include "Util/Utilities.h"

namespace RBX { namespace Reflection {

	// FUNCTION: PLAYER 0x008b6130
	// FUNCTION: STUDIO 0x00938100
	template<>
	EnumDesc<InputObject::UserInputType>::EnumDesc()
		: EnumDescriptor("UserInputType", typeid(InputObject::UserInputType))
	{
		addPair(InputObject::TYPE_MOUSEBUTTON1, "MouseButton1");
		addPair(InputObject::TYPE_MOUSEBUTTON2, "MouseButton2");
		addPair(InputObject::TYPE_MOUSEBUTTON3, "MouseButton3");
		addPair(InputObject::TYPE_MOUSEWHEEL, "MouseWheel");
		addPair(InputObject::TYPE_MOUSEMOVEMENT, "MouseMovement");
		addPair(InputObject::TYPE_TOUCH, "Touch");
		addPair(InputObject::TYPE_KEYBOARD, "Keyboard");
		addPair(InputObject::TYPE_NONE, "None");
	}

	template<> EnumRegistrar<InputObject::UserInputType>
		EnumRegistrar<InputObject::UserInputType>::registrar(0);

	template<> TypeRegistrar<InputObject::UserInputType>
		TypeRegistrar<InputObject::UserInputType>::registrar(0);

	// getSingleton pair (PartInstance Material precedent; required by the
	// EnumDesc machinery at link).
	template<>
	const Type& Type::getSingleton<InputObject::UserInputType>()
	{
		return EnumDesc<InputObject::UserInputType>::singleton();
	}

	template<>
	const Type& Type::getSingleton<InputObject::UserInputState>()
	{
		return EnumDesc<InputObject::UserInputState>::singleton();
	}

	// FUNCTION: PLAYER 0x008b62a0
	// FUNCTION: STUDIO 0x00938270
	template<>
	EnumDesc<InputObject::UserInputState>::EnumDesc()
		: EnumDescriptor("UserInputState", typeid(InputObject::UserInputState))
	{
		addPair(InputObject::INPUT_STATE_BEGIN, "Begin");
		addPair(InputObject::INPUT_STATE_CHANGE, "Change");
		addPair(InputObject::INPUT_STATE_END, "End");
		addPair(InputObject::INPUT_STATE_NONE, "None");
	}

	template<> EnumRegistrar<InputObject::UserInputState>
		EnumRegistrar<InputObject::UserInputState>::registrar(0);

	template<> TypeRegistrar<InputObject::UserInputState>
		TypeRegistrar<InputObject::UserInputState>::registrar(0);

	// getSingleton spec (Region3.cpp precedent): required by
	// Bridge<InputObject>::getValue<Variant> (0x163776).
	template<>
	const Type& Type::getSingleton<InputObject>()
	{
		static TType<InputObject> type("InputObject");
		return type;
	}

	// Pre-main touch (section 246 POSTMORTEM): Variant(InputObject) is
	// first constructed by the LuaArguments tests, i.e. post-lockdown
	// (first DescribedBase sets lockedDown), which would DebugBreak in
	// checkLockedDown. Same prescription as the UserInputType/State
	// registrars above.
	template<> TypeRegistrar<InputObject>
		TypeRegistrar<InputObject>::registrar(0);

} } // namespace RBX::Reflection

namespace RBX {

	template<>
	std::string StringConverter<InputObject>::convertToString(const InputObject& value)
	{
		using Reflection::EnumDesc;
		std::string result = EnumDesc<InputObject::UserInputType>::singleton().convertToString(value.inputType);
		result += ", ";
		result += EnumDesc<InputObject::UserInputState>::singleton().convertToString(value.inputState);
		result += ", ";
		result += StringConverter<G3D::Vector2>::convertToString(value.screenPosition);
		result += ", ";
		result += StringConverter<int>::convertToString((int)value.keyCode);
		return result;
	}

	template<>
	bool StringConverter<InputObject>::convertToValue(const std::string& text, InputObject& value)
	{
		return false;
	}

} // namespace RBX

/* App/v8datamodel/StringValue.cpp — October 2013.
   Evidence: StringValue C2 0x3497D0.
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/StringValue.h"

namespace RBX {
	const char* const sStringValue = "StringValue";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

namespace RBX {

	const std::string StringValue::defaultValue("");

	Reflection::BoundProp<std::string> StringValue::desc_Value("Value", category_Data, &StringValue::value, &StringValue::onValueChanged);
	Reflection::EventDesc<StringValue, void(std::string)> StringValue::desc_ValueChanged(&StringValue::valueChangedSignal, "Changed", "value");

	StringValue::StringValue()
		: value(defaultValue)
	{
		setName("Value");
	}

} // namespace RBX

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::StringValue);
}

/* V8DataModel/StringValue.h — October 2013 (string bridge, custom class).
   Evidence: StringValue C2 0x3497D0 (Instance(nullptr) + string from
     defaultValue + setName("Value"); FactoryProduct<StringValue, Instance,
     sStringValue>); sStringValue = "StringValue" (0.125 metadata).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StringValue.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sStringValue;

	class StringValue
		: public DescribedCreatable<StringValue, Instance, sStringValue>
	{
		typedef DescribedCreatable<StringValue, Instance, sStringValue> Super;
		static const std::string defaultValue;
		std::string value;
	public:
		StringValue();

		static Reflection::BoundProp<std::string> desc_Value;
		rbx::signal<void(std::string)> valueChangedSignal;
		static Reflection::EventDesc<StringValue, void(std::string)> desc_ValueChanged;

		const std::string& getValue() const { return this->value; }
		void setValue(const std::string& _value) { desc_Value.setValue(this, _value); }

	private:
		void onValueChanged(const Reflection::PropertyDescriptor&) {
			valueChangedSignal(value);
		}
	};

} // namespace RBX

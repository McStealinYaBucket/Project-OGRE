/* V8DataModel/ObjectValue.h — October 2013 (instance bridge, custom class).
   Evidence: ObjectValue C2 0x5E0550 (Instance(nullptr) + setName("Value")
     through the virtual dispatch; FactoryProduct<ObjectValue, Instance,
     sObjectValue>); sObjectValue = "ObjectValue" (0.125 metadata).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ObjectValue.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sObjectValue;

	class ObjectValue
		: public DescribedCreatable<ObjectValue, Instance, sObjectValue>
	{
		typedef DescribedCreatable<ObjectValue, Instance, sObjectValue> Super;
		shared_ptr<Instance> value;
	public:
		ObjectValue();

		static Reflection::BoundProp<shared_ptr<Instance> > desc_Value;
		rbx::signal<void(shared_ptr<Instance>)> valueChangedSignal;
		static Reflection::EventDesc<ObjectValue, void(shared_ptr<Instance>)> desc_ValueChanged;

		const shared_ptr<Instance>& getValue() const { return this->value; }
		void setValue(const shared_ptr<Instance>& _value) { desc_Value.setValue(this, _value); }

	private:
		void onValueChanged(const Reflection::PropertyDescriptor&) {
			valueChangedSignal(value);
		}
	};

} // namespace RBX

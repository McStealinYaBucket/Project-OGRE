/* App/v8datamodel/ObjectValue.cpp — October 2013.
   Evidence: ObjectValue C2 0x5E0550.
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ObjectValue.h"

namespace RBX {
	const char* const sObjectValue = "ObjectValue";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

namespace RBX {

	Reflection::BoundProp<shared_ptr<Instance> > ObjectValue::desc_Value("Value", category_Data, &ObjectValue::value, &ObjectValue::onValueChanged);
	Reflection::EventDesc<ObjectValue, void(shared_ptr<Instance>)> ObjectValue::desc_ValueChanged(&ObjectValue::valueChangedSignal, "Changed", "value");

	ObjectValue::ObjectValue()
	{
		setName("Value");
	}

} // namespace RBX

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ObjectValue);
}

/* V8DataModel/Value.h — October 2013 (scripting value bridges).
   Evidence: FactoryProduct<Value<int>, sIntValue> 0x4F6A5A,
     Value<CFrame> 0x4F3572 (sCFrameValue); custom StringValue C2 0x3497D0
     + ObjectValue C2 0x5E0550 (dedicated classes, NOT template instances —
     same split as March 2016). 2016 Value.h carried for the template shape
     MINUS LuaVM/secure-double machinery (RBX_SECURE_DOUBLE absent in 2013 —
     plain Value<int>/Value<double>; no NumericValue in the 2013 symbol list).
   s-names double as Lua class names (0.125 metadata: all present).
   TODO: full decompile (StringValue/ObjectValue custom bodies per their C2s;
     RayValue/CFrameValue need RbxRay/CoordinateFrame modules).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Value.h */

#pragma once

#include "V8Tree/Instance.h"
#include "G3D/Vector3.h"
#include "G3D/Color3.h"
#include "G3D/CoordinateFrame.h"
#include "RbxG3D/RbxRay.h"
#include "Util/BrickColor.h"

namespace RBX {

	extern const char* const sBoolValue;
	extern const char* const sIntValue;
	extern const char* const sDoubleValue;  // Lua name "NumberValue"
	extern const char* const sStringValue;
	extern const char* const sVector3Value;
	extern const char* const sColor3Value;
	extern const char* const sBrickColorValue;
	extern const char* const sCFrameValue;
	extern const char* const sRayValue;

	class IValue
	{
	};

	// Lightweight single-value holders (scripting bridges).
	template<class ValueType, const char* const& sClassName>
	class Value
		: public DescribedCreatable<Value<ValueType, sClassName>, Instance, sClassName>
		, public IValue
	{
		typedef DescribedCreatable<Value<ValueType, sClassName>, Instance, sClassName> Super;
		static const ValueType defaultValue;
		ValueType value;
	public:
		Value():value(defaultValue) {
			Super::setName("Value");
		}

		static Reflection::BoundProp<ValueType> desc_Value;
		rbx::signal<void(ValueType)> valueChangedSignal;
		static Reflection::EventDesc< Value<ValueType, sClassName>, void(ValueType)> desc_ValueChanged;

		const ValueType& getValue() const { return this->value; }
		void setValue(const ValueType& _value) { desc_Value.setValue(this, _value); }

	protected:
		virtual bool askSetParent(const Instance* instance) const {
			return true;
		}
	private:
		void onValueChanged(const Reflection::PropertyDescriptor&) {
			valueChangedSignal(value);
		}
	};

	typedef Value<bool, sBoolValue> BoolValue;
	typedef Value<int, sIntValue> IntValue;
	typedef Value<double, sDoubleValue> DoubleValue;
	typedef Value<G3D::Vector3, sVector3Value> Vector3Value;
	typedef Value<G3D::Color3, sColor3Value> Color3Value;
	typedef Value<BrickColor, sBrickColorValue> BrickColorValue;
	// 2013 factory evidence: FactoryProduct<Value<int>> 0x4F6A5A,
	// Value<CFrame> 0x4F3572. Value<CFrame>/Value<Ray> need their math
	// modules (CoordinateFrame/RbxRay — present) + type backing (done).
	typedef Value<G3D::CoordinateFrame, sCFrameValue> CFrameValue;
	typedef Value<RbxRay, sRayValue> RayValue;

} // namespace RBX

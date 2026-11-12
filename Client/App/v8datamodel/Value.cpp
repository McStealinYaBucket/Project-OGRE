/* App/v8datamodel/Value.cpp — October 2013 (value bridges).
   Evidence: FactoryProduct<Value<int>, sIntValue> 0x4F6A5A,
     Value<CFrame> 0x4F3572; 2016 Value.cpp carried for s-names/defaults/
     descriptor shapes (minus NumericValue/secure-double machinery, absent
     from the 2013 symbol list). s-names double as Lua classes (0.125
     metadata: all present).
   TODO: full decompile (per-type EAs; RayValue/CFrameValue need their modules).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Value.cpp */

#include "stdafx.h"

#include "V8DataModel/Value.h"

namespace RBX {
	const char* const sBoolValue = "BoolValue";
	const char* const sIntValue = "IntValue";
	const char* const sDoubleValue = "NumberValue";
	const char* const sVector3Value = "Vector3Value";
	const char* const sColor3Value = "Color3Value";
	const char* const sBrickColorValue = "BrickColorValue";
	const char* const sCFrameValue = "CFrameValue";
	const char* const sRayValue = "RayValue";
}

using namespace RBX;
using namespace RBX::Reflection;

REFLECTION_BEGIN();
REFLECTION_END();

namespace RBX {

	template<> const bool BoolValue::defaultValue(false);
	template<> Reflection::BoundProp<bool> BoolValue::desc_Value("Value", category_Data, &BoolValue::value, &BoolValue::onValueChanged);
	template<> Reflection::EventDesc<BoolValue, void(bool)> BoolValue::desc_ValueChanged(&BoolValue::valueChangedSignal, "Changed", "value");

	template<> const int IntValue::defaultValue(0);
	template<> Reflection::BoundProp<int> IntValue::desc_Value("Value", category_Data, &IntValue::value, &IntValue::onValueChanged);
	template<> Reflection::EventDesc<IntValue, void(int)> IntValue::desc_ValueChanged(&IntValue::valueChangedSignal, "Changed", "value");

	template<> const double DoubleValue::defaultValue(0.0);
	template<> Reflection::BoundProp<double> DoubleValue::desc_Value("Value", category_Data, &DoubleValue::value, &DoubleValue::onValueChanged);
	template<> Reflection::EventDesc<DoubleValue, void(double)> DoubleValue::desc_ValueChanged(&DoubleValue::valueChangedSignal, "Changed", "value");

	template<> const G3D::Vector3 Vector3Value::defaultValue(0, 0, 0);
	template<> Reflection::BoundProp<G3D::Vector3> Vector3Value::desc_Value("Value", category_Data, &Vector3Value::value, &Vector3Value::onValueChanged);
	template<> Reflection::EventDesc<Vector3Value, void(G3D::Vector3)> Vector3Value::desc_ValueChanged(&Vector3Value::valueChangedSignal, "Changed", "value");

	template<> const G3D::Color3 Color3Value::defaultValue(0, 0, 0);
	template<> Reflection::BoundProp<G3D::Color3> Color3Value::desc_Value("Value", category_Data, &Color3Value::value, &Color3Value::onValueChanged);
	template<> Reflection::EventDesc<Color3Value, void(G3D::Color3)> Color3Value::desc_ValueChanged(&Color3Value::valueChangedSignal, "Changed", "value");

	template<> const BrickColor BrickColorValue::defaultValue(BrickColor::brick_194);
	template<> Reflection::BoundProp<BrickColor> BrickColorValue::desc_Value("Value", category_Data, &BrickColorValue::value, &BrickColorValue::onValueChanged);
	template<> Reflection::EventDesc<BrickColorValue, void(BrickColor)> BrickColorValue::desc_ValueChanged(&BrickColorValue::valueChangedSignal, "Changed", "value");

	template<> const G3D::CoordinateFrame CFrameValue::defaultValue(G3D::Vector3(0, 0, 0));
	template<> Reflection::BoundProp<G3D::CoordinateFrame> CFrameValue::desc_Value("Value", category_Data, &CFrameValue::value, &CFrameValue::onValueChanged);
	template<> Reflection::EventDesc<CFrameValue, void(G3D::CoordinateFrame)> CFrameValue::desc_ValueChanged(&CFrameValue::valueChangedSignal, "Changed", "value");

	template<> const RbxRay RayValue::defaultValue(G3D::Vector3(0, 0, 0), G3D::Vector3(0, 0, 0));
	template<> Reflection::BoundProp<RbxRay> RayValue::desc_Value("Value", category_Data, &RayValue::value, &RayValue::onValueChanged);
	template<> Reflection::EventDesc<RayValue, void(RbxRay)> RayValue::desc_ValueChanged(&RayValue::valueChangedSignal, "Changed", "value");

} // namespace RBX

// Class registrars (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::BoolValue);
RBX_REGISTER_CLASS(RBX::IntValue);
RBX_REGISTER_CLASS(RBX::DoubleValue);
RBX_REGISTER_CLASS(RBX::Vector3Value);
RBX_REGISTER_CLASS(RBX::Color3Value);
RBX_REGISTER_CLASS(RBX::BrickColorValue);
RBX_REGISTER_CLASS(RBX::CFrameValue);
RBX_REGISTER_CLASS(RBX::RayValue);
}

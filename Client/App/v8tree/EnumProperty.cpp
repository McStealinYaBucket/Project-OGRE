/* App/v8tree/EnumProperty.cpp — October 2013 primitive-type reflection.
   Homes for Type::getSingleton + Variant::convert + TypedPropertyDescriptor
   specializations of primitive/shared_ptr types (2016 keeps these in
   App/v8tree/EnumProperty.cpp too).
   Evidence:
     - Type names from direct decompile: shared_ptr<DescribedBase> "Object"
       0x538268, shared_ptr<Instance> "Instance" 0x5382E0,
       shared_ptr<vector<shared_ptr<Instance>> const> "Objects" 0x538358,
       shared_ptr<Tuple const> "Tuple" 0x538D90. Scalar names (bool/int/long/
       float/double/string) carried from 2016 ("bool" etc.).
     - TypedPropertyDescriptor readValues: bool 0x537D58, string 0x537B5C,
       int 0x537E72, float 0x537CCA, double 0x537DE6 (all: isXsiNil + getValue
       + setValue — the generic pattern, verified identical to 2016).
     - Variant::convert bodies follow 2016 (cross-type int/float/double/bool/
       string coercions); string convert omits the Lua-WeakFunctionRef branch
       (Lua module pending — TODO).
   TODO: float/double/long getDataSize/string-value bodies (pattern-carried);
   ContentId singleton name ("Content" carried from 2016 — TODO verify). */

#include "stdafx.h"

#include "V8Tree/Instance.h"
#include "Reflection/Type.h"
#include "script/ThreadRef.h"
#include "Util/CellID.h"
#include "rbx/make_shared.h"
#include "Util/ContentId.h"
#include "Util/BrickColor.h"
#include "Util/NormalId.h"
#include "Util/Faces.h"
#include "Util/Axes.h"
#include "G3D/Vector2.h"
#include "Util/UDim.h"
#include "Util/Utilities.h"
#include "G3D/Color3.h"
#include "G3D/Color3uint8.h"
#include "G3D/Vector3.h"
#include "G3D/CoordinateFrame.h"
#include "RbxG3D/RbxRay.h"
#include "Util/Region3.h"
#include "Util/Region3int16.h"
#include "G3D/Vector3int16.h"
#include "G3D/Vector2int16.h"

#include <algorithm>

using namespace RBX;
using namespace RBX::Reflection;

namespace RBX {
namespace Reflection {

template<>
const Type& Type::getSingleton<bool>()
{
	static TType<bool> type("bool");
	return type;
}

template<>
bool& Variant::convert<bool>(void)
{
	// Convert int to bool
	if (_type->isType<int>())
	{
		value = (bool) (cast<int>()!=0);
		_type = &Type::singleton<bool>();
	}

	// Convert float to bool
	else if (_type->isType<float>())
	{
		value = (bool) (cast<float>()!=0.0f);
		_type = &Type::singleton<bool>();
	}

	// Convert double to bool
	else if (_type->isType<double>())
	{
		value = (bool) (cast<double>()!=0.0);
		_type = &Type::singleton<bool>();
	}

	return genericConvert<bool>();
}

template<>
const Type& Type::getSingleton<int>()
{
	static TType<int> type("int");
	return type;
}

template<>
const Type& Type::getSingleton<long>()
{
	static TType<long> type("long");
	return type;
}

template<>
int& Variant::convert<int>(void)
{
	// Convert double to int
	if (_type->isType<double>())
	{
		value = G3D::iRound(cast<double>());
		_type = &Type::singleton<int>();
	}

	// Convert float to int
	if (_type->isType<float>())
	{
		value = G3D::iRound(cast<float>());
		_type = &Type::singleton<int>();
	}

	// Convert bool to int
	if (_type->isType<bool>())
	{
		value = (cast<bool>() ? 1 : 0);
		_type = &Type::singleton<int>();
	}

	return genericConvert<int>();
}

template<>
const Type& Type::getSingleton<float>()
{
	static TType<float> type("float");
	return type;
}

template<>
float& Variant::convert<float>(void)
{
	// Convert double to float
	if (_type->isType<double>())
	{
		value = (float) cast<double>();
		_type = &Type::singleton<float>();
	}

	// Convert int to float
	else if (_type->isType<int>())
	{
		value = (float) cast<int>();
		_type = &Type::singleton<float>();
	}

	// Convert bool to float
	else if (_type->isType<bool>())
	{
		value = (float) (cast<bool>() ? 1 : 0);
		_type = &Type::singleton<float>();
	}

	return genericConvert<float>();
}

template<>
const Type& Type::getSingleton<double>()
{
	static TType<double> type("double");
	return type;
}

template<>
double& Variant::convert<double>(void)
{
	// Convert int to double
	if (_type->isType<int>())
	{
		value = (double) cast<int>();
		_type = &Type::singleton<double>();
	}

	// Convert bool to double
	else if (_type->isType<bool>())
	{
		value = (double) (cast<bool>() ? 1 : 0);
		_type = &Type::singleton<double>();
	}

	// Convert float to double
	else if (_type->isType<float>())
	{
		value = (double) cast<float>();
		_type = &Type::singleton<double>();
	}

	return genericConvert<double>();
}

template<>
const Type& Type::getSingleton<std::string>()
{
	static TType<std::string> type("string");
	return type;
}

template<>
RBX::ContentId& Variant::convert<RBX::ContentId>(void)
{
	// TODO: full conversion matrix (0x234EB6/0x2353A2); identity + string source for now.
	if (!isType<RBX::ContentId>())
	{
		if (isType<std::string>())
			value = RBX::ContentId(cast<std::string>());
		_type = &Type::singleton<RBX::ContentId>();
	}
	RBX::ContentId* id = tryCast<RBX::ContentId>();
	if (id == NULL)
		throw std::runtime_error("Unable to cast value to ContentId");
	return *id;
}

template<>
std::string& Variant::convert<std::string>(void)
{
	// TODO: This should be made compact somehow!!!
	if (isType<bool>())
	{
		value = StringConverter<bool>::convertToString(cast<bool>());
		_type = &Type::singleton<std::string>();
	}
	else if (isType<int>())
	{
		value = StringConverter<int>::convertToString(cast<int>());
		_type = &Type::singleton<std::string>();
	}
	else if (isType<long>())
	{
		value = StringConverter<long>::convertToString(cast<long>());
		_type = &Type::singleton<std::string>();
	}
	else if (isType<float>())
	{
		value = StringConverter<float>::convertToString(cast<float>());
		_type = &Type::singleton<std::string>();
	}
	else if (isType<double>())
	{
		value = StringConverter<double>::convertToString(cast<double>());
		_type = &Type::singleton<std::string>();
	}
	// TODO: Lua::WeakFunctionRef branch (Lua module pending).
	else if (isType<shared_ptr<Instance> >())
	{
		const shared_ptr<Instance>& v = cast<shared_ptr<Instance> >();
		if (v)
			value = std::string(Type::singleton< shared_ptr<Instance> >().name->c_str());
		else
			value = std::string("nil");
		_type = &Type::singleton<std::string>();
	}
	else if(isType<shared_ptr<const Reflection::ValueArray> >())
	{
		value = std::string("{}");
		_type = &Type::singleton<std::string>();
	}
	else if (const Reflection::EnumDescriptor* desc = Reflection::EnumDescriptor::lookupDescriptor(type()))
	{
		const Reflection::EnumDescriptor::Item* item = desc->lookup(*this);
		if (item!=NULL)
		{
			std::string stringValue;
			if(item->convertToString(stringValue)){
				value = stringValue;
				_type = &Type::singleton<std::string>();
			}
		}
	}

	std::string* id = tryCast<std::string>();
	if (id==NULL)
		throw std::runtime_error("Unable to cast value to std::string");
	return *id;
}

template<>
const Type& Type::getSingleton<shared_ptr<Reflection::DescribedBase> >()
{
	static TType<shared_ptr<Reflection::DescribedBase> > type("Object");  // 0x538268
	return type;
}

// FUNCTION: PLAYER 0x006ff9a0
// FUNCTION: STUDIO 0x00708c30
template<>
const Type& Type::getSingleton<shared_ptr<Instance> >()
{
	static TType<shared_ptr<Instance> > type("Instance");  // 0x5382E0
	return type;
}

// FUNCTION: PLAYER 0x006ffa10
// FUNCTION: STUDIO 0x00708ca0
template<>
const Type& Type::getSingleton<shared_ptr<const Instances> >()
{
	static TType<shared_ptr<const Instances> > type("Objects");  // 0x538358
	return type;
}

// Container singletons for the withVariantValue shared arms (section
// 246). "Tuple" is binary-pinned (0x538D90); ValueMap follows the same
// convention (name unobserved -- feeds type identity only). ValueArray
// ("Array") + ValueTable ("Dictionary") already live in
// reflection/type.cpp (canonical homes).
// FUNCTION: PLAYER 0x006ffee0
// FUNCTION: STUDIO 0x00709170
template<>
const Type& Type::getSingleton<shared_ptr<const Reflection::Tuple> >()
{
	static TType<shared_ptr<const Reflection::Tuple> > type("Tuple");
	return type;
}

template<>
const Type& Type::getSingleton<shared_ptr<const Reflection::ValueMap> >()
{
	static TType<shared_ptr<const Reflection::ValueMap> > type("ValueMap");
	return type;
}

// WeakFunctionRef singleton for the withVariantValue function arm
// (section 246; same convention, name unobserved).
template<>
const Type& Type::getSingleton<RBX::Lua::WeakFunctionRef>()
{
	static TType<RBX::Lua::WeakFunctionRef> type("WeakFunctionRef");
	return type;
}

// CellID singleton for Variant construction (section 246 test +
// withVariantValue arm identity). Name "CellId" is 2016-carried
// (same shape; the bridge className "CellId" corroborates the casing;
// no 2013 TType-name EA is banked).
template<>
const Type& Type::getSingleton<RBX::CellID>()
{
	static TType<RBX::CellID> type("CellId");
	return type;
}

// int16 value-type singletons (section 247): no spec existed anywhere
// (exe-link LNK2001 via get()'s userdata arm + the registrars below).
// Homes follow the Color3/Vector3/RbxRay precedent (this file); names
// follow the bridge classNames ("Vector3int16"/"Vector2int16"/
// "Region3int16", batches 347-444).
template<>
const Type& Type::getSingleton<G3D::Vector3int16>()
{
	static TType<G3D::Vector3int16> type("Vector3int16");
	return type;
}

template<>
const Type& Type::getSingleton<G3D::Vector2int16>()
{
	static TType<G3D::Vector2int16> type("Vector2int16");
	return type;
}

template<>
const Type& Type::getSingleton<RBX::Region3int16>()
{
	static TType<RBX::Region3int16> type("Region3int16");
	return type;
}

// Pre-main Type touches (section 246 POSTMORTEM). Descriptor::lockedDown
// goes true on the first DescribedBase construction, after which ANY
// Type construction hits checkLockedDown -> RBXCRASH -> DebugBreak()
// (0x80000003 -- the exact code this test died with at its first
// Variant(CellID)). These four Types are first-used by the LuaArguments
// era (nothing earlier in the tree constructs them; Faces/bool/int/
// string/etc. survive because property-descriptor statics / early uses
// built them pre-lockdown). Forcing pre-main construction here follows
// the Descriptor.h prescription ("put your class into
// FactoryRegistrator... or somewhere else") and the in-tree precedent
// (InputObject.cpp / DataModel.cpp / PartInstance.cpp TypeRegistrar
// statics). Observable effect: four extra entries in getAllTypes order;
// no gate observes that order.
template<> TypeRegistrar<RBX::CellID> TypeRegistrar<RBX::CellID>::registrar(0);
template<> TypeRegistrar<RBX::Lua::WeakFunctionRef> TypeRegistrar<RBX::Lua::WeakFunctionRef>::registrar(0);
// s700: post-lockdown DebugBreak (teardown-silent crash) on first
// singleton<long>/singleton<float> touch -- same prescription as s697
// (no property-descriptor static builds them pre-main; getAllTypes
// order unobserved).
template<> TypeRegistrar<long> TypeRegistrar<long>::registrar(0);
template<> TypeRegistrar<float> TypeRegistrar<float>::registrar(0);
template<> TypeRegistrar<shared_ptr<const Reflection::ValueMap> > TypeRegistrar<shared_ptr<const Reflection::ValueMap> >::registrar(0);
template<> TypeRegistrar<shared_ptr<const Reflection::Tuple> > TypeRegistrar<shared_ptr<const Reflection::Tuple> >::registrar(0);
// P2-140: bin2 first-touches singleton<ContentId>/singleton<ValueArray>
// post-lockdown (no descriptor static builds them pre-main in the small
// link; bin1 is saved by descriptor traffic + Lua table pushes). Same
// prescription as s697/s700. (void deliberately omitted: BoundFuncDesc
// statics pre-build it in every link.)
template<> TypeRegistrar<RBX::ContentId> TypeRegistrar<RBX::ContentId>::registrar(0);
template<> TypeRegistrar<shared_ptr<const Reflection::ValueArray> > TypeRegistrar<shared_ptr<const Reflection::ValueArray> >::registrar(0);

// Value-bridge payload Types first-touched by LuaArguments::get's
// userdata arm (section 247; same post-lockdown prescription -- get()
// assigns value = T, which constructs singleton<T> on first use).
// No-ops for the ones property-descriptor statics already built
// pre-main (getSingleton then just returns the existing object).
template<> TypeRegistrar<G3D::CoordinateFrame> TypeRegistrar<G3D::CoordinateFrame>::registrar(0);
template<> TypeRegistrar<RBX::Region3> TypeRegistrar<RBX::Region3>::registrar(0);
template<> TypeRegistrar<RBX::Region3int16> TypeRegistrar<RBX::Region3int16>::registrar(0);
template<> TypeRegistrar<G3D::Vector3int16> TypeRegistrar<G3D::Vector3int16>::registrar(0);
template<> TypeRegistrar<G3D::Vector2int16> TypeRegistrar<G3D::Vector2int16>::registrar(0);
template<> TypeRegistrar<G3D::Vector3> TypeRegistrar<G3D::Vector3>::registrar(0);
template<> TypeRegistrar<G3D::Vector2> TypeRegistrar<G3D::Vector2>::registrar(0);
template<> TypeRegistrar<RBX::RbxRay> TypeRegistrar<RBX::RbxRay>::registrar(0);
template<> TypeRegistrar<G3D::Color3> TypeRegistrar<G3D::Color3>::registrar(0);
template<> TypeRegistrar<RBX::BrickColor> TypeRegistrar<RBX::BrickColor>::registrar(0);
template<> TypeRegistrar<RBX::UDim> TypeRegistrar<RBX::UDim>::registrar(0);
template<> TypeRegistrar<RBX::UDim2> TypeRegistrar<RBX::UDim2>::registrar(0);
// P2-142: bin2 first-touches singleton<Faces> post-lockdown (bin1 is
// saved by main() warm-ups; same prescription as s697/s700).
template<> TypeRegistrar<RBX::Faces> TypeRegistrar<RBX::Faces>::registrar(0);
template<> TypeRegistrar<RBX::Axes> TypeRegistrar<RBX::Axes>::registrar(0);
template<> TypeRegistrar<shared_ptr<Instance> > TypeRegistrar<shared_ptr<Instance> >::registrar(0);
// s697: post-lockdown DebugBreak (teardown-silent crash) on first
// singleton<shared_ptr<DescribedBase>>/singleton<shared_ptr<const
// Instances>> touch -- same prescription as the lines above.
template<> TypeRegistrar<shared_ptr<Reflection::DescribedBase> > TypeRegistrar<shared_ptr<Reflection::DescribedBase> >::registrar(0);
template<> TypeRegistrar<shared_ptr<const Instances> > TypeRegistrar<shared_ptr<const Instances> >::registrar(0);

template<>
shared_ptr<Instance>& Variant::convert<shared_ptr<Instance> >(void)
{
	// Interpret "void" (Lua nil) as a null Instance
	if (isType<void>())
	{
		value = shared_ptr<Instance>();
		_type = &Type::singleton<shared_ptr<Instance> >();
	}
	else if (isType<shared_ptr<Reflection::DescribedBase> >())
	{
		const shared_ptr<Reflection::DescribedBase>& v = cast<shared_ptr<Reflection::DescribedBase> >();

		value = shared_dynamic_cast<Instance>(v);
		_type = &Type::singleton<shared_ptr<Instance> >();
	}

	shared_ptr<Instance>* v = tryCast<shared_ptr<Instance> >();
	if (v==NULL)
		throw std::runtime_error("Unable to cast value to Object");
	return *v;
}

template<>
shared_ptr<Reflection::DescribedBase>& Variant::convert<shared_ptr<Reflection::DescribedBase> >(void)
{
	// Interpret "void" (Lua nil) as a null Instance
	if (isType<void>())
	{
		value = shared_ptr<Reflection::DescribedBase>();
		_type = &Type::singleton<shared_ptr<Reflection::DescribedBase> >();
	}

	else if (isType<shared_ptr<Instance> >())
	{
		const shared_ptr<Instance>& v = cast<shared_ptr<Instance> >();
		value = shared_static_cast<Reflection::DescribedBase>(v);
		_type = &Type::singleton<shared_ptr<Reflection::DescribedBase> >();
	}

	shared_ptr<Reflection::DescribedBase>* v = tryCast<shared_ptr<Reflection::DescribedBase> >();
	if (v==NULL)
		throw std::runtime_error("Unable to cast value to Object");
	return *v;
}

} // namespace Reflection
} // namespace RBX

// TypedPropertyDescriptor primitive specializations. readValue bodies verified
// against 2013 (bool 0x537D58, string 0x537B5C, int 0x537E72, float 0x537CCA,
// double 0x537DE6 — all isXsiNil + getValue + setValue); the rest follows the
// same generic pattern.
template<>
int RBX::Reflection::TypedPropertyDescriptor<std::string>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(std::string) + getValue(instance).size();
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<std::string>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<std::string>::getStringValue(const DescribedBase* instance) const {
	return getValue(instance);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<std::string>::setStringValue(DescribedBase* instance, const std::string& text) const {
	setValue(instance, text);
	return true;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<std::string>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		std::string value;
		if (element->getValue(value))
			setValue(instance, value);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<std::string>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance));
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<bool>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(bool);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<bool>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<bool>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<bool>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<bool>::setStringValue(DescribedBase* instance, const std::string& text) const {
	bool value;
	if (RBX::StringConverter<bool>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	else
		return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<bool>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		bool value;
		if (element->getValue(value))
			setValue(instance, value);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<bool>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance));
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<int>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(int);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<int>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<int>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<int>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<int>::setStringValue(DescribedBase* instance, const std::string& text) const {
	int value;
	if (RBX::StringConverter<int>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	else
		return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<int>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		int value;
		if (element->getValue(value))
			setValue(instance, value);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<int>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance));
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<float>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(float);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<float>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<float>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<float>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<float>::setStringValue(DescribedBase* instance, const std::string& text) const {
	float value;
	if (RBX::StringConverter<float>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	else
		return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<float>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		float value;
		if (element->getValue(value))
			setValue(instance, value);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<float>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance));
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<double>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(double);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<double>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<double>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<double>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<double>::setStringValue(DescribedBase* instance, const std::string& text) const {
	double value;
	if (RBX::StringConverter<double>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	else
		return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<double>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		double value;
		if (element->getValue(value))
			setValue(instance, value);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<double>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance));
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::ContentId>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::ContentId) + getValue(instance).toString().size();
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::ContentId>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::ContentId>::getStringValue(const DescribedBase* instance) const {
	return getValue(instance).toString();
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::ContentId>::setStringValue(DescribedBase* instance, const std::string& text) const {
	setValue(instance, RBX::ContentId(text));
	return true;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::ContentId>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		RBX::ContentId value;
		if (element->getValue(value))
			setValue(instance, value);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::ContentId>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance));
}


//////////////////////////////////////////////////////////////////////////////////
// BrickColor + Color3 backing (required by PartInstance props 0x448F10+).
// Carried from March 2016 v8tree/EnumProperty.cpp (first-party shapes);
// readValue/writeValue bodies are 2016-pattern (TODO: verify vs 2013 EAs).
// StringConverter<BrickColor/Color3> live here (not in a Property.cpp —
// this tree has none; EnumProperty.cpp is the type-backing home).

template<>
std::string RBX::StringConverter<RBX::BrickColor>::convertToString(const RBX::BrickColor& value)
{
	return value.name();
}

template<>
bool RBX::StringConverter<RBX::BrickColor>::convertToValue(const std::string& text, RBX::BrickColor& value)
{
	value = RBX::BrickColor::parse(text.c_str());
	return true;
}

template<>
std::string RBX::StringConverter<G3D::Color3>::convertToString(const G3D::Color3& value)
{
	char szText[64];
	_snprintf(szText, 64, "%g, %g, %g", value[0], value[1], value[2]);
	return szText;
}

template<>
bool RBX::StringConverter<G3D::Color3>::convertToValue(const std::string& text, G3D::Color3& value)
{
	if (text[0]=='#')
	{
		if (text.size()!=7)
			return false;
		value[0] = static_cast<float>(strtol(text.substr(1,2).c_str(), NULL, 16))/255.0f;
		value[1] = static_cast<float>(strtol(text.substr(3,2).c_str(), NULL, 16))/255.0f;
		value[2] = static_cast<float>(strtol(text.substr(5,2).c_str(), NULL, 16))/255.0f;
		return true;
	}
	if (text.substr(0,2)=="0x")
	{
		if (text.size()!=8)
			return false;
		value[0] = static_cast<float>(strtol(text.substr(2,2).c_str(), NULL, 16))/255.0f;
		value[1] = static_cast<float>(strtol(text.substr(4,2).c_str(), NULL, 16))/255.0f;
		value[2] = static_cast<float>(strtol(text.substr(6,2).c_str(), NULL, 16))/255.0f;
		return true;
	}
	int pos = (int)text.find_first_of(",;", 0);
	if (pos<0)
		return false;
	value[0] = static_cast<float>(atof(text.substr(0, pos).c_str()));
	int last = pos+1;
	pos = (int)text.find_first_of(",;", last);
	if (pos<last)
		return false;
	value[1] = static_cast<float>(atof(text.substr(last, pos-last).c_str()));
	last = pos+1;
	pos = (int)text.length();
	if (pos<last)
		return false;
	value[2] = static_cast<float>(atof(text.substr(last, pos-last).c_str()));
	return true;
}

template<>
RBX::BrickColor& RBX::Reflection::Variant::convert<RBX::BrickColor>(void)
{
	return genericConvert<RBX::BrickColor>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<RBX::BrickColor>()
{
	static TType<RBX::BrickColor> type("BrickColor", "int");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::BrickColor>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::BrickColor);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::BrickColor>::hasStringValue() const {
	return false;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::BrickColor>::getStringValue(const DescribedBase* instance) const {
	return Super::getStringValue(instance);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::BrickColor>::setStringValue(DescribedBase* instance, const std::string& text) const {
	return Super::setStringValue(instance, text);
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::BrickColor>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		int number;
		if (element->getValue(number))
		{
			setValue(instance, RBX::BrickColor(number));
		}
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::BrickColor>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance).asInt());
}

template<>
G3D::Color3& RBX::Reflection::Variant::convert<G3D::Color3>(void)
{
	return genericConvert<G3D::Color3>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<G3D::Color3>()
{
	static TType<G3D::Color3> type("Color3");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<G3D::Color3>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(G3D::Color3);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::Color3>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<G3D::Color3>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<G3D::Color3>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::Color3>::setStringValue(DescribedBase* instance, const std::string& text) const {
	G3D::Color3 value;
	if (RBX::StringConverter<G3D::Color3>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::Color3>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		unsigned int argb;
		if (element->getValue(argb))
		{
			{ G3D::Color3uint8 u8 = G3D::Color3uint8::fromARGB(argb); setValue(instance,
			G3D::Color3((float)u8.r / 255.0f, (float)u8.g / 255.0f, (float)u8.b / 255.0f)); }
		}
		else
		{
			const XmlElement* xElement = element->findFirstChildByTag(*tag_R);
			const XmlElement* yElement = element->findFirstChildByTag(*tag_G);
			const XmlElement* zElement = element->findFirstChildByTag(*tag_B);
			if (xElement && yElement && zElement)
			{
				G3D::Color3 v;
				xElement->getValue(v.r);
				yElement->getValue(v.g);
				zElement->getValue(v.b);
				setValue(instance, v);
			}
		}
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::Color3>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	G3D::Color3uint8 v = G3D::Color3uint8(getValue(instance));
	element->setValue(v.asUInt32());
}


//////////////////////////////////////////////////////////////////////////////////
// Vector3 + shared_ptr<Instance> backing (required by Vector3Value /
// ObjectValue descs). Vector3 carried from March 2016 v8tree/EnumProperty.cpp
// (first-party shapes); read/write TODO-verify vs 2013 EAs. shared_ptr<Instance>
// getSingleton is TType "Instance" (2016 form); its XML read/write need the
// IDREF subsystem (binder/IIDREF/RefType singletons) — loud TODO stubs until
// then (silent data loss is worse than a hard error).

template<>
std::string RBX::StringConverter<G3D::Vector3>::convertToString(const G3D::Vector3& value)
{
	std::string result = RBX::StringConverter<float>::convertToString(value[0]);
	result += ", ";
	result += RBX::StringConverter<float>::convertToString(value[1]);
	result += ", ";
	result += RBX::StringConverter<float>::convertToString(value[2]);
	return result;
}

template<>
bool RBX::StringConverter<G3D::Vector3>::convertToValue(const std::string& text, G3D::Vector3& value)
{
	int pos = (int)text.find_first_of(",;", 0);
	if (pos<0)
		return false;
	value[0] = (float)atof(text.substr(0, pos).c_str());
	int last = pos+1;
	pos = (int)text.find_first_of(",;", last);
	if (pos<last)
		return false;
	value[1] = (float)atof(text.substr(last, pos-last).c_str());
	last = pos+1;
	pos = (int)text.length();
	if (pos<last)
		return false;
	value[2] = (float)atof(text.substr(last, pos-last).c_str());
	return true;
}

static bool ReadValue(G3D::Vector3& output, const XmlElement* element)
{
	if (!element->isXsiNil())
	{
		const XmlElement* xElement = element->findFirstChildByTag(*tag_X);
		const XmlElement* yElement = element->findFirstChildByTag(*tag_Y);
		const XmlElement* zElement = element->findFirstChildByTag(*tag_Z);
		if (xElement && yElement && zElement)
		{
			xElement->getValue(output.x);
			yElement->getValue(output.y);
			zElement->getValue(output.z);
			return true;
		}
	}
	return false;
}
static void WriteValue(XmlElement* element, const G3D::Vector3& v)
{
	XmlElement* xElement = element->addChild(*tag_X);
	XmlElement* yElement = element->addChild(*tag_Y);
	XmlElement* zElement = element->addChild(*tag_Z);
	xElement->setValue(v.x);
	yElement->setValue(v.y);
	zElement->setValue(v.z);
}

template<>
G3D::Vector3& RBX::Reflection::Variant::convert<G3D::Vector3>(void)
{
	return genericConvert<G3D::Vector3>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<G3D::Vector3>()
{
	static TType<G3D::Vector3> type("Vector3");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<G3D::Vector3>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(G3D::Vector3);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::Vector3>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<G3D::Vector3>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<G3D::Vector3>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::Vector3>::setStringValue(DescribedBase* instance, const std::string& text) const {
	G3D::Vector3 value;
	if (RBX::StringConverter<G3D::Vector3>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::Vector3>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	G3D::Vector3 v;
	if (ReadValue(v, element))
		setValue(instance, v);
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::Vector3>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	G3D::Vector3 v = getValue(instance);
	WriteValue(element, v);
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<shared_ptr<RBX::Instance> >::getDataSize(const DescribedBase* instance) const
{
	return sizeof(shared_ptr<RBX::Instance>);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<shared_ptr<RBX::Instance> >::hasStringValue() const {
	return false;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<shared_ptr<RBX::Instance> >::getStringValue(const DescribedBase* instance) const {
	return Super::getStringValue(instance);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<shared_ptr<RBX::Instance> >::setStringValue(DescribedBase* instance, const std::string& text) const {
	return Super::setStringValue(instance, text);
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<shared_ptr<RBX::Instance> >::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	throw std::runtime_error("shared_ptr<Instance> XML read needs the IDREF subsystem");
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<shared_ptr<RBX::Instance> >::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	throw std::runtime_error("shared_ptr<Instance> XML write needs the IDREF subsystem");
}


//////////////////////////////////////////////////////////////////////////////////
// CoordinateFrame + RbxRay backing (required by CFrameValue/RayValue).
// Carried from March 2016 v8tree/EnumProperty.cpp (RbxRay) + v8datamodel/
// PartInstance.cpp (CoordinateFrame read/write/getComponent); first-party
// shapes, read/write TODO-verify vs 2013 EAs. StringConverter<CoordinateFrame>
// lives in 2016 v8tree/Property.cpp (carried here; this tree has no Property.cpp).
// convertToValue returns false (2016 does the same — no CFrame string parse).

template<>
std::string RBX::StringConverter<G3D::CoordinateFrame>::convertToString(const G3D::CoordinateFrame& value)
{
	std::string result = RBX::StringConverter<G3D::Vector3>::convertToString(value.translation);
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
		{
			result += ", ";
			result += RBX::StringConverter<float>::convertToString(value.rotation[i][j]);
		}
	return result;
}

template<>
bool RBX::StringConverter<G3D::CoordinateFrame>::convertToValue(const std::string& text, G3D::CoordinateFrame& value)
{
	(void)text; (void)value;
	return false;
}

template<>
std::string RBX::StringConverter<RBX::RbxRay>::convertToString(const RBX::RbxRay& value)
{
	std::string result = "{" + RBX::StringConverter<G3D::Vector3>::convertToString(value.origin()) + "}";
	result += ", ";
	result += "{" + RBX::StringConverter<G3D::Vector3>::convertToString(value.direction()) + "}";
	return result;
}

template<>
bool RBX::StringConverter<RBX::RbxRay>::convertToValue(const std::string& text, RBX::RbxRay& value)
{
	int openBracket = (int)text.find_first_of("{", 0);
	if (openBracket < 0)
		return false;
	int closeBracket = (int)text.find_first_of("}", openBracket);
	if (closeBracket < 0)
		return false;
	if (!RBX::StringConverter<G3D::Vector3>::convertToValue(text.substr(openBracket+1, closeBracket-openBracket-1), value.origin()))
		return false;
	int last = closeBracket+1;
	openBracket = (int)text.find_first_of("{", last);
	if (openBracket < 0)
		return false;
	closeBracket = (int)text.find_first_of("}", openBracket);
	if (closeBracket < 0)
		return false;
	if (!RBX::StringConverter<G3D::Vector3>::convertToValue(text.substr(openBracket+1, closeBracket-openBracket-1), value.direction()))
		return false;
	return true;
}

static float getComponent(const XmlElement* element, const XmlTag& name)
{
	float result = 0.0f;
	if (const XmlElement* child = element->findFirstChildByTag(name))
	{
		bool gotComponent = child->getValue(result);
		RBXASSERT(gotComponent);
	}
	return result;
}

template<>
G3D::CoordinateFrame& RBX::Reflection::Variant::convert<G3D::CoordinateFrame>(void)
{
	return genericConvert<G3D::CoordinateFrame>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<G3D::CoordinateFrame>()
{
	static TType<G3D::CoordinateFrame> type("CoordinateFrame");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(G3D::CoordinateFrame);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>::hasStringValue() const {
	return false;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>::getStringValue(const DescribedBase* instance) const {
	return Super::getStringValue(instance);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>::setStringValue(DescribedBase* instance, const std::string& text) const {
	return Super::setStringValue(instance, text);
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		G3D::CoordinateFrame c;
		c.translation.x = getComponent(element, *tag_X);
		c.translation.y = getComponent(element, *tag_Y);
		c.translation.z = getComponent(element, *tag_Z);
		c.rotation[0][0] = getComponent(element, *tag_R00);
		c.rotation[0][1] = getComponent(element, *tag_R01);
		c.rotation[0][2] = getComponent(element, *tag_R02);
		c.rotation[1][0] = getComponent(element, *tag_R10);
		c.rotation[1][1] = getComponent(element, *tag_R11);
		c.rotation[1][2] = getComponent(element, *tag_R12);
		c.rotation[2][0] = getComponent(element, *tag_R20);
		c.rotation[2][1] = getComponent(element, *tag_R21);
		c.rotation[2][2] = getComponent(element, *tag_R22);
		setValue(instance, c);
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	G3D::CoordinateFrame c = getValue(instance);
	element->addChild(new XmlElement(*tag_X, c.translation.x));
	element->addChild(new XmlElement(*tag_Y, c.translation.y));
	element->addChild(new XmlElement(*tag_Z, c.translation.z));
	element->addChild(new XmlElement(*tag_R00, c.rotation[0][0]));
	element->addChild(new XmlElement(*tag_R01, c.rotation[0][1]));
	element->addChild(new XmlElement(*tag_R02, c.rotation[0][2]));
	element->addChild(new XmlElement(*tag_R10, c.rotation[1][0]));
	element->addChild(new XmlElement(*tag_R11, c.rotation[1][1]));
	element->addChild(new XmlElement(*tag_R12, c.rotation[1][2]));
	element->addChild(new XmlElement(*tag_R20, c.rotation[2][0]));
	element->addChild(new XmlElement(*tag_R21, c.rotation[2][1]));
	element->addChild(new XmlElement(*tag_R22, c.rotation[2][2]));
}

template<>
RBX::RbxRay& RBX::Reflection::Variant::convert<RBX::RbxRay>(void)
{
	return genericConvert<RBX::RbxRay>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<RBX::RbxRay>()
{
	static TType<RBX::RbxRay> type("Ray");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::RbxRay>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::RbxRay);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::RbxRay>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::RbxRay>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<RBX::RbxRay>::convertToString(getValue(instance));
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::RbxRay>::setStringValue(DescribedBase* instance, const std::string& text) const {
	RBX::RbxRay value;
	if (RBX::StringConverter<RBX::RbxRay>::convertToValue(text, value))
	{
		setValue(instance, value);
		return true;
	}
	return false;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::RbxRay>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		const XmlElement* originElement = element->findFirstChildByTag(*tag_Origin);
		const XmlElement* directionElement = element->findFirstChildByTag(*tag_Direction);
		RBX::RbxRay v;
		if (originElement && directionElement && ReadValue(v.origin(), originElement) && ReadValue(v.direction(), directionElement))
		{
			setValue(instance, v);
		}
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::RbxRay>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	XmlElement* originElement = element->addChild(*tag_Origin);
	XmlElement* directionElement = element->addChild(*tag_Direction);
	RBX::RbxRay v = getValue(instance);
	WriteValue(originElement, v.origin());
	WriteValue(directionElement, v.direction());
}

//////////////////////////////////////////////////////////////////////////////////
// NormalId + Vector3::Axis string converters (debts called out at the bottom
// of our NormalId.cpp; 2016 homes EnumProperty.cpp/Axes.cpp — carried here).
// Bodies verbatim from March 2016 (first-party); TODO-verify vs 2013 EAs.

template<>
bool RBX::StringConverter<RBX::NormalId>::convertToValue(const std::string& text, RBX::NormalId& value)
{
	if (text.find("Top") != std::string::npos || text.find("top") != std::string::npos)
	{
		value = RBX::NORM_Y;
		return true;
	}
	if (text.find("Bottom") != std::string::npos || text.find("bottom") != std::string::npos)
	{
		value = RBX::NORM_Y_NEG;
		return true;
	}
	if (text.find("Back") != std::string::npos || text.find("back") != std::string::npos)
	{
		value = RBX::NORM_Z;
		return true;
	}
	if (text.find("Front") != std::string::npos || text.find("front") != std::string::npos)
	{
		value = RBX::NORM_Z_NEG;
		return true;
	}
	if (text.find("Right") != std::string::npos || text.find("right") != std::string::npos)
	{
		value = RBX::NORM_X;
		return true;
	}
	if (text.find("Left") != std::string::npos || text.find("left") != std::string::npos)
	{
		value = RBX::NORM_X_NEG;
		return true;
	}
	return false;
}

template<>
bool RBX::StringConverter<G3D::Vector3::Axis>::convertToValue(const std::string& text, G3D::Vector3::Axis& value)
{
	if (text.find("Y") != std::string::npos || text.find("Top") != std::string::npos || text.find("Bottom") != std::string::npos)
	{
		value = G3D::Vector3::Y_AXIS;
		return true;
	}
	if (text.find("Z") != std::string::npos || text.find("Back") != std::string::npos || text.find("Front") != std::string::npos)
	{
		value = G3D::Vector3::Z_AXIS;
		return true;
	}
	if (text.find("X") != std::string::npos || text.find("Right") != std::string::npos || text.find("Left") != std::string::npos)
	{
		value = G3D::Vector3::X_AXIS;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
// UDim + UDim2 backing.
// convert/getSingleton/6-specs carried from 2016 v8tree/EnumProperty.cpp.
// Read/write TODO-verify vs 2013 EAs (shapes match the 0x6FDC57 reader).
// NOTE 2026-10-07: the four StringConverter<UDim/UDim2> convertToString/
// convertToValue definitions used to live here as 2016 carries, but they
// duplicate the EA-backed versions in App/util/UDim.cpp (0x2580CA/
// mac: 0x2581B0/0x2582C4/0x258618) -- both TUs in one exe link LNK2005'd, so the
// copies here were deleted; this TU calls into UDim.cpp's.

template<>
RBX::UDim& RBX::Reflection::Variant::convert<RBX::UDim>(void)
{
	return genericConvert<RBX::UDim>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<RBX::UDim>()
{
	static TType<RBX::UDim> type("UDim");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::UDim>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::UDim);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::UDim>::hasStringValue() const {
	return true;
}

template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::UDim>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<RBX::UDim>::convertToString(getValue(instance));
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::UDim>::setStringValue(DescribedBase* instance, const std::string& text) const {
	return false;
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::UDim>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		const XmlElement* sElement = element->findFirstChildByTag(*tag_S);
		const XmlElement* oElement = element->findFirstChildByTag(*tag_O);
		float s;
		int o;
		sElement->getValue(s);
		oElement->getValue(o);
		setValue(instance, RBX::UDim(s, (G3D::int16)o));
	}
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::UDim>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	XmlElement* sElement = element->addChild(*tag_S);
	XmlElement* oElement = element->addChild(*tag_O);
		RBX::UDim v = getValue(instance);
	sElement->setValue(v.scale);
	oElement->setValue((int)v.offset);
}

template<>
RBX::UDim2& RBX::Reflection::Variant::convert<RBX::UDim2>(void)
{
	return genericConvert<RBX::UDim2>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<RBX::UDim2>()
{
	static TType<RBX::UDim2> type("UDim2");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::UDim2>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::UDim2);
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::UDim2>::hasStringValue() const {
	return true;
}

template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::UDim2>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<RBX::UDim2>::convertToString(getValue(instance));
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::UDim2>::setStringValue(DescribedBase* instance, const std::string& text) const {
	return false;
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::UDim2>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		const XmlElement* xsElement = element->findFirstChildByTag(*tag_XS);
		const XmlElement* xoElement = element->findFirstChildByTag(*tag_XO);
		const XmlElement* ysElement = element->findFirstChildByTag(*tag_YS);
		const XmlElement* yoElement = element->findFirstChildByTag(*tag_YO);
		float xs, ys;
		int xo, yo;
		xsElement->getValue(xs);
		xoElement->getValue(xo);
		ysElement->getValue(ys);
		yoElement->getValue(yo);
		setValue(instance, RBX::UDim2(xs, (G3D::int16)xo, ys, (G3D::int16)yo));
	}
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::UDim2>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	XmlElement* xsElement = element->addChild(*tag_XS);
	XmlElement* xoElement = element->addChild(*tag_XO);
	XmlElement* ysElement = element->addChild(*tag_YS);
	XmlElement* yoElement = element->addChild(*tag_YO);
		RBX::UDim2 v = getValue(instance);
	xsElement->setValue(v.x.scale);
	xoElement->setValue((int)v.x.offset);
	ysElement->setValue(v.y.scale);
	yoElement->setValue((int)v.y.offset);
}

//////////////////////////////////////////////////////////////////////////////////
// Faces + Axes + Vector2 backing.
// Faces/Axes StringConverters already live in App/util/NormalId.cpp (first-party);
// convert/getSingleton/6-specs carried from 2016 v8tree/EnumProperty.cpp.
// Vector2 converters carried from 2016 v8tree/Property.cpp (RBX::Vector2 spelling
// is the same type as G3D::Vector2 via Util/G3DCore.h typedef; we spell G3D::Vector2
// to match the 0x700E3E/0x6FDC57 getSingleton<G3D::Vector2> evidence).
// Read/write TODO-verify vs 2013 EAs (shapes match the binary branches).

template<>
std::string RBX::StringConverter<G3D::Vector2>::convertToString(const G3D::Vector2& value)
{
	std::string result = RBX::StringConverter<float>::convertToString(value[0]);
	result += ", ";
	result += RBX::StringConverter<float>::convertToString(value[1]);
	return result;
}

template<>
bool RBX::StringConverter<G3D::Vector2>::convertToValue(const std::string& text, G3D::Vector2& value)
{
	int pos = (int)text.find_first_of(",;", 0);
	if (pos<0)
		return false;
	value[0] = (float)atof(text.substr(0, pos).c_str());
	int last = pos+1;
	pos = (int)text.length();
	if (pos<last)
		return false;
	value[1] = (float)atof(text.substr(last, pos-last).c_str());
	return true;
}

template<>
RBX::Faces& RBX::Reflection::Variant::convert<RBX::Faces>(void)
{
	return genericConvert<RBX::Faces>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<RBX::Faces>()
{
	static TType<RBX::Faces> type("Faces");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::Faces>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::Faces);
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::Faces>::hasStringValue() const {
	return true;
}

template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::Faces>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<RBX::Faces>::convertToString(getValue(instance));
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::Faces>::setStringValue(DescribedBase* instance, const std::string& text) const {
	return false;
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::Faces>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		const XmlElement* facesElement = element->findFirstChildByTag(*tag_faces);
		int faces;
		facesElement->getValue(faces);
		setValue(instance, RBX::Faces(faces));
	}
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::Faces>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	XmlElement* facesElement = element->addChild(*tag_faces);
		RBX::Faces v = getValue(instance);
	facesElement->setValue(v.normalIdMask);
}

template<>
RBX::Axes& RBX::Reflection::Variant::convert<RBX::Axes>(void)
{
	return genericConvert<RBX::Axes>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<RBX::Axes>()
{
	static TType<RBX::Axes> type("Axes");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::Axes>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::Axes);
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::Axes>::hasStringValue() const {
	return true;
}

template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::Axes>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<RBX::Axes>::convertToString(getValue(instance));
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::Axes>::setStringValue(DescribedBase* instance, const std::string& text) const {
	return false;
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::Axes>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		const XmlElement* axesElement = element->findFirstChildByTag(*tag_axes);
		int axes;
		axesElement->getValue(axes);
		setValue(instance, RBX::Axes(axes));
	}
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::Axes>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	XmlElement* axesElement = element->addChild(*tag_axes);
		RBX::Axes v = getValue(instance);
	axesElement->setValue(v.axisMask);
}

template<>
G3D::Vector2& RBX::Reflection::Variant::convert<G3D::Vector2>(void)
{
	return genericConvert<G3D::Vector2>();
}

template<>
const RBX::Reflection::Type& RBX::Reflection::Type::getSingleton<G3D::Vector2>()
{
	static TType<G3D::Vector2> type("Vector2");
	return type;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<G3D::Vector2>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(G3D::Vector2);
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::Vector2>::hasStringValue() const {
	return true;
}

template<>
std::string RBX::Reflection::TypedPropertyDescriptor<G3D::Vector2>::getStringValue(const DescribedBase* instance) const {
	return RBX::StringConverter<G3D::Vector2>::convertToString(getValue(instance));
}

template<>
bool RBX::Reflection::TypedPropertyDescriptor<G3D::Vector2>::setStringValue(DescribedBase* instance, const std::string& text) const {
	G3D::Vector2 value;
	if (RBX::StringConverter<G3D::Vector2>::convertToValue(text, value)) { setValue(instance, value); return true; }
	else return false;
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::Vector2>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		const XmlElement* xElement = element->findFirstChildByTag(*tag_X);
		if (!xElement) return;
		const XmlElement* yElement = element->findFirstChildByTag(*tag_Y);
		G3D::Vector2 v;
		xElement->getValue(v.x);
		yElement->getValue(v.y);
		setValue(instance, v);
	}
}

template<>
void RBX::Reflection::TypedPropertyDescriptor<G3D::Vector2>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	XmlElement* xElement = element->addChild(*tag_X);
	XmlElement* yElement = element->addChild(*tag_Y);
		G3D::Vector2 v = getValue(instance);
	xElement->setValue(v.x);
	yElement->setValue(v.y);
}

////////////////////////////////////////////////////////////////////////////////
// ProtectedString reflection backing (batches 740-770: the Script
// "Source" prop machinery pulls it into the link, Script.obj). The
// descriptor/convert member set + singletons follow the std::string /
// ContentId / Faces shapes above; bodies are 2013-conventional port
// forms, explicit-value TODO-verify vs the noted EAs (0x234/*).
// BINARY NOTE: Type::getSingleton<ProtectedString> 0x234BAC +
// Variant::convert 0x234EB6 LIVE in Script.obj's pull per the P2 link.

#include "Util/ProtectedString.h"

// (Type::getSingleton<ProtectedString> + its TypeRegistrar are defined in
// util/ProtectedString.cpp -- section 258b, binary 0x234BAC -- NOT here.)

template<>
RBX::ProtectedString& RBX::Reflection::Variant::convert<RBX::ProtectedString>(void)
{
	// TODO: full conversion matrix (0x234EB6/0x2353A2); identity + string source for now.
	if (!isType<RBX::ProtectedString>())
	{
		if (isType<std::string>())
			value = RBX::ProtectedString::fromTrustedSource(cast<std::string>());
		_type = &Type::singleton<RBX::ProtectedString>();
	}
	RBX::ProtectedString* ps = tryCast<RBX::ProtectedString>();
	if (ps == NULL)
		throw std::runtime_error("Unable to cast value to ProtectedString");
	return *ps;
}

template<>
int RBX::Reflection::TypedPropertyDescriptor<RBX::ProtectedString>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(RBX::ProtectedString) + (int)getValue(instance).getStringForImmediateUse().size();
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::ProtectedString>::hasStringValue() const {
	return true;
}
template<>
std::string RBX::Reflection::TypedPropertyDescriptor<RBX::ProtectedString>::getStringValue(const DescribedBase* instance) const {
	return getValue(instance).getStringForImmediateUse();
}
template<>
bool RBX::Reflection::TypedPropertyDescriptor<RBX::ProtectedString>::setStringValue(DescribedBase* instance, const std::string& text) const {
	setValue(instance, RBX::ProtectedString::fromTrustedSource(text));
	return true;
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::ProtectedString>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		std::string value;
		if (element->getValue(value))
			setValue(instance, RBX::ProtectedString::fromTrustedSource(value));
	}
}
template<>
void RBX::Reflection::TypedPropertyDescriptor<RBX::ProtectedString>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance).getStringForImmediateUse());
}

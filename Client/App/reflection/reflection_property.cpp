#include "stdafx.h"

#include "reflection/property.h"
#include "reflection/Object.h"
#include "util/ProtectedString.h"
#include "util/SystemAddress.h"

namespace RBX { namespace Reflection {

std::size_t hash_value(const ConstProperty& prop)
{
	std::size_t seed = boost::hash<const PropertyDescriptor*>()(&prop.getDescriptor());
	boost::hash_combine(seed, prop.getInstance());
	return seed;
}

template<>
const Type& Type::getSingleton<const Reflection::PropertyDescriptor*>()
{
	static TType<const Reflection::PropertyDescriptor*> type("Property");
	return type;
}

EnumDescriptor::EnumNameTable& EnumDescriptor::allEnumsNameLookup()
{
	static EnumNameTable m;
	return m;
}

std::vector<const EnumDescriptor*>& EnumDescriptor::allEnums()
{
	static std::vector<const EnumDescriptor*> s;
	return s;
}

EnumDescriptor::EnumTypeTable& EnumDescriptor::allEnumsTypeLookup()
{
	static EnumTypeTable m;
	return m;
}

std::vector< const EnumDescriptor* >::const_iterator EnumDescriptor::enumsBegin() { return allEnums().begin(); }
std::vector< const EnumDescriptor* >::const_iterator EnumDescriptor::enumsEnd() { return allEnums().end(); }
int EnumDescriptor::count = 0;
// mac: 0x15CBB8: per-enum type_info into Type; append, then the name and type lookups. Asserts on 2013 lines 61, 91, 92.
// FUNCTION: PLAYER 0x006fa1a0
// FUNCTION: STUDIO 0x006dcae0
EnumDescriptor::EnumDescriptor(const char* typeName, const std::type_info& info)
	:Type(typeName, "token", info, false, false)
	,enumCount(0)
	,enumCountMSB(0)
{
	allEnums().push_back(this);
	allEnumsNameLookup()[&name] = this;	// 2013: name is a const Name&, the key a temporary (0x006fa23a)
	allEnumsTypeLookup()[&type] = this;
	count++;
}

EnumDescriptor::~EnumDescriptor()
{
	RBXASSERT(std::find(allEnums().begin(), allEnums().end(), this) != allEnums().end());
	allEnums().erase(std::find(allEnums().begin(), allEnums().end(), this));
	count--;
}
PropertyDescriptor::Attributes PropertyDescriptor::Attributes::deprecated(const MemberDescriptor& preferred, Functionality flags)
{
	Attributes result(flags);
	result.Descriptor::Attributes::isDeprecated = true;
	result.preferred = &preferred;
	return result;
}
PropertyDescriptor::Attributes PropertyDescriptor::Attributes::deprecated(Functionality flags)
{
	Attributes result(flags);
	result.Descriptor::Attributes::isDeprecated = true;
	return result;
}

PropertyDescriptor::PropertyDescriptor(ClassDescriptor& classDescriptor, const Type& type, const char* name, const char* category, Attributes attributes, Security::Permissions security, bool isEnum)
	:MemberDescriptor(classDescriptor, name, category, attributes, security)
	,type(type)
	,bIsPublic((attributes.flags>>0) & 1)
	,bCanReplicate((attributes.flags>>1) & 1) 
	,bCanXmlRead((attributes.flags>>2) & 1)
	,bCanXmlWrite((attributes.flags>>3) & 1)
	,bIsScriptable((attributes.flags>>4) & 1)
	,bAlwaysClone((attributes.flags>>5) & 1)
    ,bIsEditable(1)
	,bIsEnum(isEnum)
{
	RBXASSERT(!type.tag.empty());
	RBXASSERT(!type.name.empty());

	// TODO: This could be moved up into MemberDescriptor if it were templated...
	classDescriptor.MemberDescriptorContainer<PropertyDescriptor>::declare(this);
}



XmlElement* PropertyDescriptor::write(const DescribedBase* instance, bool ignoreWriteProtection) const
{
	if (!ignoreWriteProtection)
	{
		if (isReadOnly())
			return NULL;

		if (!canXmlWrite())
			return NULL;
	}

	XmlElement* element = new XmlElement(type.tag);

	element->addAttribute(*name_name, &name);

	// Fill in the property's value
	writeValue(instance, element);

	return element;
}

void PropertyDescriptor::read(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if(canXmlRead()/* && !isReadOnly()*/){
		readValue(instance, element, binder);
	}
}

}}//namespace

#include "decomp.h"
// EnumDescriptor ctor 0x006fa1a0, dtor 0x006f9e40 (ida-sync-13).
DECOMP_SIZE_ASSERT(RBX::Reflection::EnumDescriptor, 0x34);

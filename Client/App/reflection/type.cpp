// type.cpp -- October 2013. Asserts 125, 134, 135 and 140 pin the SignatureDescriptor lines.
#include "stdafx.h"
#include "reflection/Type.h"
#include "reflection/member.h"
#include <boost/functional/hash.hpp>
#include <ostream>
namespace RBX { namespace Reflection {
	// FUNCTION: PLAYER 0x0079fd40
	// FUNCTION: STUDIO 0x0073e560
	template<>
	const Type& Type::getSingleton<shared_ptr<const ValueArray> >()
	{
		static TType<shared_ptr<const ValueArray> > type("Array");
		return type;
	}
	// FUNCTION: PLAYER 0x0079fdb0
	// FUNCTION: STUDIO 0x0073e5d0
	template<>
	const Type& Type::getSingleton<Variant>()
	{
		static TType<shared_ptr<const ValueArray> > type("Variant");
		return type;
	}
	template<>
	Variant& Variant::convert<Variant>(void)
	{
		return *this;
	}
	// FUNCTION: PLAYER 0x0079fe20
	// FUNCTION: STUDIO 0x0073e640
	template<>
	const Type& Type::getSingleton<shared_ptr<const ValueTable> >()
	{
		static TType<shared_ptr<const ValueTable> > type("Dictionary");
		return type;
	}
	// STUB: PLAYER 0x007a01d0
	// STUB: STUDIO 0x0073eac0
	template<>
	shared_ptr<const ValueArray>& Variant::convert<shared_ptr<const ValueArray> >(void)
	{
		shared_ptr<const ValueArray>* v = tryCast<shared_ptr<const ValueArray> >();
		if (v)
			return *v;
		shared_ptr<const ValueTable>* t = tryCast<shared_ptr<const ValueTable> >();
		if (t)
		{
			// an empty Lua table could be either, so an empty table converts to an empty array
			if ((*t)->size() == 0)
			{
				value = shared_ptr<const ValueArray>(new ValueArray());
				_type = &Type::getSingleton<shared_ptr<const ValueArray> >();
				return cast<shared_ptr<const ValueArray> >();
			}
		}
		throw std::runtime_error("Unable to cast to Array");
	}
	template<> TypeRegistrar<shared_ptr<const ValueTable> > TypeRegistrar<shared_ptr<const ValueTable> >::registrar(0);	// prior tree: pre-lockdown touch
	// STUB: PLAYER 0x0079f780
	// STUB: STUDIO 0x0073dfc0
	template<>
	shared_ptr<const ValueMap>& Variant::convert<shared_ptr<const ValueMap> >(void)
	{
		shared_ptr<const ValueMap>* v = tryCast<shared_ptr<const ValueMap> >();
		if (v == NULL)
			throw std::runtime_error("Unable to cast to Map");
		return *v;
	}
	// FUNCTION: PLAYER 0x0079ff00
	// FUNCTION: STUDIO 0x0073e720
	template<>
	const Type& Type::getSingleton<void>()
	{
		static TType<void> type("void");
		return type;
	}
	// STUB: PLAYER 0x007a03b0
	// STUB: STUDIO 0x0073eca0
	template<>
	shared_ptr<const ValueTable>& Variant::convert<shared_ptr<const ValueTable> >(void)
	{
		shared_ptr<const ValueTable>* v = tryCast<shared_ptr<const ValueTable> >();
		if (v)
			return *v;
		shared_ptr<const ValueArray>* a = tryCast<shared_ptr<const ValueArray> >();
		if (a)
		{
			// and an empty array converts to an empty table
			if ((*a)->size() == 0)
			{
				value = shared_ptr<const ValueTable>(new ValueTable());
				_type = &Type::getSingleton<shared_ptr<const ValueTable> >();
				return cast<shared_ptr<const ValueTable> >();
			}
		}
		throw std::runtime_error("Unable to cast to Dictionary");
	}
	static std::vector<const Type*>* allTypes;	// 0x00e0dd04, set by addToAllTypes
	// FUNCTION: PLAYER 0x0079f930
	// FUNCTION: STUDIO 0x0073e150
	void Type::addToAllTypes()
	{
		static std::vector<const Type*> types;
		allTypes = &types;
		types.push_back(this);
	}
	const std::vector<const Type*>& Type::getAllTypes()
	{
		return *allTypes;
	}
	// FUNCTION: PLAYER 0x0079ff70
	// FUNCTION: STUDIO 0x0073e790
	SignatureDescriptor::SignatureDescriptor()
		: resultType(&Type::getSingleton<void>())
	{
	}

	// STUB: PLAYER 0x0079f820
	// STUB: STUDIO 0x0073e060
	SignatureDescriptor::Item::Item(const RBX::Name* name, const Type* type, const Variant& defaultValue)
		: name(name)
		, type(type)
		, defaultValue(defaultValue)
	{
		RBXASSERT(name->c_str()[0] == tolower(name->c_str()[0]));
	}

	// FUNCTION: PLAYER 0x0079ffa0
	// FUNCTION: STUDIO 0x0073e7c0
	SignatureDescriptor::Item::Item(const RBX::Name* name, const Type* type)
		: name(name), type(type), defaultValue()
	{
		// argument names are camelCase
		RBXASSERT(name->c_str()[0] == tolower(name->c_str()[0]));
		RBXASSERT(defaultValue.isVoid());
	}
	// STUB: PLAYER 0x007a00d0
	// STUB: STUDIO 0x0073e8f0
	void SignatureDescriptor::addArgument(const RBX::Name& name, const Type& type, const Variant& defaultValue) {
		RBXASSERT(defaultValue.isVoid() || defaultValue.type()==type);
		arguments.push_back(Item(&name, &type, defaultValue));
	}

	void SignatureDescriptor::addArgument(const RBX::Name& name, const Type& type)
	{
		arguments.push_back(Item(&name, &type));
	}

	size_t StringHashPredicate::operator ()(const char* s) const
	{
		return boost::hash_range(s, s + strlen(s));
	}

	std::ostream& operator<<(std::ostream& os, const Type& type)
	{
		return os << "Type::" << type.name->str;
	}

} } // namespace RBX::Reflection

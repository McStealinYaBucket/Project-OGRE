/* type.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Refl_Type.cpp.txt)
     - getSingleton<Variant> 0x15E3DC: builds Type via TType<shared_ptr<const ValueArray>>("Variant")
       (same construction trick as 2016)
     - getSingleton<void> 0x15E8EC, addToAllTypes 0x15E964, getAllTypes 0x15E9FE
     - SignatureDescriptor::Item 0x15EA48/0x15EB28, addArgument 0x15EC64
   2013 divergence: Type stores const std::type_info* and const Name* (see Type.h). */

#include "stdafx.h"

#include "reflection/Type.h"
#include "reflection/member.h"

#include <boost/functional/hash.hpp>
#include <ostream>

namespace RBX { namespace Reflection {

	size_t StringHashPredicate::operator ()(const char* s) const
	{
		return boost::hash_range(s, s + strlen(s));
	}

	std::ostream& operator<<(std::ostream& os, const Type& type)
	{
		return os << "Type::" << type.name->str;
	}

	// mac: 0x15E9FE — global all-types vector (guard-protected static in the binary)
	static std::vector<const Type*>& allTypes()
	{
		static std::vector<const Type*> theAllTypes;
		return theAllTypes;
	}

	void Type::addToAllTypes()	// 0x15E964
	{
		allTypes().push_back(this);
	}

	const std::vector<const Type*>& Type::getAllTypes()	// 0x15E9FE
	{
		return allTypes();
	}

	// FUNCTION: PLAYER 0x0079ff00
	// FUNCTION: STUDIO 0x0073e720
	template<>
	const Type& Type::getSingleton<void>()	// 0x15E8EC
	{
		static TType<void> type("void");
		return type;
	}

	template<>
	const Type& Type::getSingleton<shared_ptr<const ValueArray> >()
	{
		static TType<shared_ptr<const ValueArray> > type("Array");
		return type;
	}

	// FUNCTION: PLAYER 0x0079fdb0
	// FUNCTION: STUDIO 0x0073e5d0
	template<>
	const Type& Type::getSingleton<Variant>()	// 0x15E3DC
	{
		static TType<shared_ptr<const ValueArray> > type("Variant");
		return type;
	}

	template<>
	Variant& Variant::convert<Variant>(void)	// 0x15E454
	{
		return *this;
	}

	template<>
	shared_ptr<const ValueArray>& Variant::convert<shared_ptr<const ValueArray> >(void)
	{
		shared_ptr<const ValueArray>* v = tryCast<shared_ptr<const ValueArray> >();
		if (v)
			return *v;
		shared_ptr<const ValueTable>* t = tryCast<shared_ptr<const ValueTable> >();
		if (t)
		{
			// empty table is the same as an empty array. (This makes Lua easier, since an empty Lua table could be either)
			if ((*t)->size() == 0)
			{
				value = shared_ptr<const ValueArray>(new ValueArray());
				_type = &Type::getSingleton<shared_ptr<const ValueArray> >();
				return cast<shared_ptr<const ValueArray> >();
			}
		}
		throw std::runtime_error("Unable to cast to Array");
	}

	template<>
	const Type& Type::getSingleton<shared_ptr<const ValueTable> >()
	{
		static TType<shared_ptr<const ValueTable> > type("Dictionary");
		return type;
	}

	// Pre-main touch (section 246 POSTMORTEM): the Dictionary Type is
	// first referenced by the LuaArguments table-pusher, i.e.
	// post-lockdown (first DescribedBase sets lockedDown), which would
	// DebugBreak in checkLockedDown. Same prescription as the
	// EnumProperty.cpp touches (Descriptor.h / InputObject.cpp
	// precedent). ("Array" needs none: array-shaped property traffic
	// builds it pre-lockdown already -- proven by the passing push-array
	// asserts that precede any table use.)
	template<> TypeRegistrar<shared_ptr<const ValueTable> > TypeRegistrar<shared_ptr<const ValueTable> >::registrar(0);

	template<>
	shared_ptr<const ValueTable>& Variant::convert<shared_ptr<const ValueTable> >(void)
	{
		shared_ptr<const ValueTable>* v = tryCast<shared_ptr<const ValueTable> >();
		if (v)
			return *v;
		shared_ptr<const ValueArray>* a = tryCast<shared_ptr<const ValueArray> >();
		if (a)
		{
			// empty array is the same as an empty table. (This makes Lua easier, since an empty Lua table could be either)
			if ((*a)->size() == 0)
			{
				value = shared_ptr<const ValueTable>(new ValueTable());
				_type = &Type::getSingleton<shared_ptr<const ValueTable> >();
				return cast<shared_ptr<const ValueTable> >();
			}
		}
		throw std::runtime_error("Unable to cast to Dictionary");
	}

	//////////////////////////////////////////////////////////////////////////////////
	// SignatureDescriptor (0x15EA48..0x15EC64)
	//////////////////////////////////////////////////////////////////////////////////
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
	}

	SignatureDescriptor::Item::Item(const RBX::Name* name, const Type* type)
		: name(name)
		, type(type)
		, defaultValue()
	{
	}

	void SignatureDescriptor::addArgument(const RBX::Name& name, const Type& type)	// 0x15EC64 family
	{
		arguments.push_back(Item(&name, &type));
	}

	// STUB: PLAYER 0x007a00d0
	// STUB: STUDIO 0x0073e8f0
	void SignatureDescriptor::addArgument(const RBX::Name& name, const Type& type, const Variant& defaultValue)
	{
		arguments.push_back(Item(&name, &type, defaultValue));
	}

} } // namespace RBX::Reflection

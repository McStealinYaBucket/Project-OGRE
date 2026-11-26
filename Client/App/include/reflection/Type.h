/* reflection/Type.h -- October 2013. Its asserts pin the layout: operator== (line 41) and
   operator!= (45) are out of line (PLAYER 0x00447390, 0x00876ba0; 200 calls to ==), and the three
   constructors assert on 66 (name, T*), 77 (name, tag, T*) and 98 (name, tag, type_info, ...).
   Members (Type<bool> ctor 0x006fdf30): type_info& +0x10, tag +0x14, isFloat +0x18 (is_float),
   isNumber +0x19 (is_arithmetic). No 2013 ctor stores +0x1a: 2016's isEnum does not exist. */
#pragma once

#include "reflection/Descriptor.h"
#include "Util/Region3.h"
#include "rbx/boost.hpp"
#include <boost/any.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_float.hpp>
#include <boost/unordered_map.hpp>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <typeinfo>

namespace RBX
{
	namespace Reflection
	{
		template<typename T> class TypeRegistrar;
		class Type : public Descriptor
		{
			template<class T> friend class TypeRegistrar; template<typename T> friend class TType;
		public:
			// must be implemented (specialized) for each type used
			template<class T> static const Type& getSingleton();
		private:
			void addToAllTypes();
		public:
			const std::type_info& type;	// +0x10
			const Name& tag;			// +0x14 (2013 asserts read this->tag.empty(): Type.h 66/77/98)
			// FUNCTION: PLAYER 0x00447390
			// FUNCTION: STUDIO 0x00435eb0
			bool operator==(const Type& right) const {
				RBXASSERT((type==right.type) == (this==&right));
				return this==&right; }
			// FUNCTION: PLAYER 0x00876ba0
			// FUNCTION: STUDIO 0x009278d0
			bool operator!=(const Type& right) const { RBXASSERT((type!=right.type) == (this!=&right));
				return this!=&right; }
			const bool isFloat;			// +0x18
			const bool isNumber;		// +0x19
			static const std::vector<const Type*>& getAllTypes();
			template<class T>
			static inline const Type& singleton() { return getSingleton<T>(); }
			template<class T>
			bool isType() const { return this == &getSingleton<T>(); }

		protected:
			// FUNCTION: PLAYER 0x0079fc70
			// FUNCTION: STUDIO 0x0073e490
			template<class T>
			Type(const char* name, T* dummy)
				:Descriptor(name, Descriptor::Attributes())
				,type(typeid(T))
				,tag(*Name::lookup(name))
				,isFloat(boost::is_float<T>::value)
				,isNumber(boost::is_arithmetic<T>::value)
			{
				RBXASSERT(!this->tag.empty());
				addToAllTypes();
			}
			template<class T>
			Type(const char* name, const char* tag, T* dummy)
				:Descriptor(name, Descriptor::Attributes())
				,type(typeid(T))
				,tag(*Name::declare(tag))
				,isFloat(boost::is_float<T>::value)
				,isNumber(boost::is_arithmetic<T>::value)
			{
				RBXASSERT(!this->tag.empty());
				addToAllTypes();
			}


			// (name, tag, type_info, isNumber, isFloat) is defined here and emitted out of line
			// next to its only caller: EnumDescriptor's constructor (0x006fa1a0) passes its own
			// type_info, "token" and two falses. The bool order is the signature's, the store
			// order the members' (arg 5 to +0x18, arg 4 to +0x19 at 0x006f9848).
			//
			//
			//
			// FUNCTION: PLAYER 0x006f97e0
			// FUNCTION: STUDIO 0x006db8a0
			Type(const char* name, const char* tag, const std::type_info& type, bool isNumber, bool isFloat)
				:Descriptor(name, Descriptor::Attributes())
				,type(type)
				,tag(*Name::declare(tag))
				,isFloat(isFloat)
				,isNumber(isNumber)
			{
				RBXASSERT(!this->tag.empty());
				addToAllTypes();
			}
		};

		std::ostream& operator<<(std::ostream& os, const RBX::Reflection::Type& type);

		// handy macro for registering a type
#define RBX_REGISTER_TYPE(mType)		template<> RBX::Reflection::TypeRegistrar<mType> RBX::Reflection::TypeRegistrar<mType>::registrar(0)

		// this class is designed to prevent clients of the library
		// from forgetting to initialize their class descriptors
		template<class T>
		class TypeRegistrar : boost::noncopyable
		{
			int x;

			// GCC does not generate the registrar variable definition & fails at link time.
			// Force construct by passing a dummy arg to ctor.
			TypeRegistrar(int i) :x(i)
			{
				// this assertion is added to catch a nasty implicit use of boost::any with Variant objects
				BOOST_STATIC_ASSERT((!boost::is_same<T, boost::any>::value));
				// this call registers the Type descriptor in the reflection database
				Type::getSingleton<T>();
			}

		public:
			static TypeRegistrar registrar;
		};

		// helper class
		template<typename T>
		class TType : public Type
		{
			friend class Type;
		protected:
			TType(const char* name)
				:Type(name, (T*)NULL)
			{
			}
			TType(const char* name, const char* tag)
				:Type(name, tag, (T*)NULL)
			{
			}
		};

		/* Variant -- header-only in 2013 (core methods fully inlined at use sites in the binary).
		   2013 layout (binary-pinned by the getValue<Variant> + withVariantValue
		   manglings, which name placement_any<RBX::Region3> on the +4 member):
		   { const Type* _type; rbx::placement_any<RBX::Region3> value; }.
		   value is public: cross-TU readers (withVariantValue etc.) reach
		   straight through, exactly as the binary's +4 accesses do. */
		class Variant
		{
		public:
			const Type* _type;
			rbx::placement_any<RBX::Region3> value;

			inline Variant()
				: _type(&Type::singleton<void>())
				, value()
			{}

			inline Variant(const Variant& other)
				: _type(other._type)
				, value(other.value)
			{}

			inline Variant& operator=(const Variant& rhs)
			{
				_type = rhs._type;
				value = rhs.value;
				return *this;
			}

			template<typename ValueType>
			inline Variant(const ValueType& value)
				: _type(&Type::singleton<ValueType>())
				, value(value)
			{
			}

			template<typename ValueType>
			inline Variant& operator=(const ValueType& rhs)
			{
				_type = &Type::singleton<ValueType>();
				value = rhs;
				return *this;
			}

			inline const Type& type() const {
				return *_type;
			}

			inline bool isVoid() const
			{
				return *_type == Type::singleton<void>();
			}
			inline bool isFloat() const { return type().isFloat; }
			inline bool isNumber() const { return type().isNumber; }
			inline bool isString() const { return isType<std::string>(); }

			// 2013 compares type_info (Variant::get<ValueMap> PLAYER 0x005bb850: address test, then
			// typeid(T).operator==(_type->type)).
			template<class ValueType>
			inline bool isType() const {
				return &_type->type == &typeid(ValueType) || typeid(ValueType) == _type->type;
			}

			// throws an exception if unable to convert
			template<typename ValueType>
			ValueType& convert();

			// throws an exception if unable to convert
			template<typename ValueType>
			inline ValueType get() const
			{
				if (isType<ValueType>())
					return cast<ValueType>();
				else
				{
					// create a non-const copy to extract the value from
					Variant v(*this);
					return v.convert<ValueType>();
				}
			}

			// 2013 casts go through rbx::any_cast on the placement_any (its type_info, not _type):
			// get<ValueMap> 0x005bb850 calls the throwing any_cast<const ValueMap&> 0x0058ef20.
			template<typename T>
			inline const T& cast() const {
				return rbx::any_cast<const T&>(const_cast<rbx::placement_any<RBX::Region3>&>(value));
			}

			template<typename T>
			inline T& cast() {
				return rbx::any_cast<T&>(value);
			}

			// The by-value fetch EventDescImpl::fireEvent uses: CFrame's out-of-line copy is PLAYER
			// 0x0058f560 (calls the by-value any_cast<CoordinateFrame> 0x0058ebd0); bool inlines it and
			// copies through a stack temp (GameBasicSettings fireEvent 0x00643ea0). No symbol names it
			// (inline on iOS), so the name is a placeholder.
			template<typename T>
			inline T cast_unk0x0058f560() const {
				return rbx::any_cast<T>(value);
			}

			template<typename T>
			inline const T* tryCast() const {
				return rbx::any_cast<T>(&value);
			}

			template<typename T>
			inline T* tryCast() {
				return rbx::any_cast<T>(&value);
			}

		private:
			template<class ValueType>
			ValueType& genericConvert();
		};

		// equivalent to an array in Lua
		typedef std::vector<Variant> ValueArray;

		// a limited table in Lua (keys must be strings for now)
		typedef boost::unordered_map<std::string, Variant> ValueTable;

		// The same as a ValueTable for now, but will always have a string key.
		typedef std::map<std::string, Variant> ValueMap;

		struct Tuple
		{
			ValueArray values;
			Tuple() {}
			Tuple(size_t count):values(count) {}
			Tuple(const Tuple& other):values(other.values) {}
			Variant& at(size_t i) { return values[i]; }
			const Variant& at(size_t i) const { return values[i]; }
		};

		// describes a function's signature
		class SignatureDescriptor
		{
		public:
			struct Item {
				friend class SignatureDescriptor;
			public:
				Item(const RBX::Name* name, const Type* type, const Variant& defaultValue);	// 0x15EA48
				Item(const RBX::Name* name, const Type* type);								// 0x15EB28
				const RBX::Name* name;
				const Type* type;
				const Variant defaultValue;
				bool hasDefaultValue() const
				{
					return defaultValue.type() == *type;
				}
			};
			typedef std::list<Item> Arguments;

			const Type* resultType;
			Arguments arguments;

			void addArgument(const RBX::Name& name, const Type& type);						// 0x15EC64 family
			void addArgument(const RBX::Name& name, const Type& type, const Variant& defaultValue);

			SignatureDescriptor();
		};

		// FUNCTION: PLAYER 0x0066f7f0
		// FUNCTION: STUDIO 0x00780350
		template<class ValueType>
		ValueType& RBX::Reflection::Variant::genericConvert()
		{
			ValueType* id = tryCast<ValueType>();
			if (id != NULL)
				return *id;

			if (isType<std::string>())
			{
				ValueType v;
				if (StringConverter<ValueType>::convertToValue(rbx::any_cast<std::string>(value), v))
				{
					value = v;
					_type = &Type::singleton<ValueType>();
					return cast<ValueType>();
				}
			}

			throw RBX::runtime_error("Unable to cast %s to %s", _type->tag.str.c_str(), Type::singleton<ValueType>().tag.str.c_str());
		}
	}
}

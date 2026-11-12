/* reflection/Type.h — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Refl_Type.cpp.txt)
     - Type(const char* name, const char* tag, const std::type_info&, bool a5, bool a6) 0x15D260
       stores type_info* at +16 and Name* tag at +20, bools at +24 (a6) and +25 (a5)
     - Type<void>(name, void*) 0x15F0D8: typeinfo for'void, Name::lookup, all bools zero,
       assert "!this->tag.empty()" at include/reflection/type.h:66
     - generic ctor asserts at include/reflection/Type.h:98
     - addToAllTypes 0x15E964, getAllTypes 0x15E9FE (global std::vector<const Type*>)
   2013 divergences from 2016:
     - keeps a const std::type_info* member (2016 removed it)
     - tag is const Name* (2016: const Name&)
     - bool order in memory: isFloat at +24, isNumber at +25 (per ctor arg mapping a6->+24, a5->+25);
       isEnum bool lives in the Type(bool,bool,bool) full ctor used by enums
     - isReplicable/isOutdated do NOT exist (2016-only) */

#pragma once

#include "reflection/Descriptor.h"
#include "Util/Region3.h"
#include "rbx/boost.hpp"
#include <boost/any.hpp>
#include <boost/static_assert.hpp>
#include <typeinfo>

#include <boost/unordered_map.hpp>
#include <list>
#include <map>
#include <vector>
#include <string>

namespace RBX
{
	namespace Reflection
	{
		template<typename T> class TypeRegistrar;
		// Types supported by the Reflection framework
		class Type : public Descriptor
		{
			template<class T>
			friend class TypeRegistrar;

			template<typename T>
			friend class TType;

		public:
			// must be implemented (specialized) for each type used
			template<class T>
			static const Type& getSingleton();

		private:
			void addToAllTypes();					// 0x15E964

		public:
			const std::type_info* const typeInfo;	// +16 (2013-only; removed by 2016)
			const Name* const tag;					// +20
			const bool isFloat;						// +24
			const bool isNumber;					// +25
			const bool isEnum;						// +26

			static const std::vector<const Type*>& getAllTypes();	// 0x15E9FE

			template<class T>
			static inline const Type& singleton()
			{
				return getSingleton<T>();
			}

			bool operator==(const Type& right) const { return this == &right; }	// inlined in binary
			// STUB: PLAYER 0x00876ba0
			// STUB: STUDIO 0x009278d0
			bool operator!=(const Type& right) const { return this != &right; }	// 0x162994

			template<class T>
			bool isType() const {
				return this == &getSingleton<T>();
			}

		protected:
			// mac: 0x15F0D8 pattern: (name, T* dummy) -> lookup tag, type_info of T, all bools false
			// STUB: PLAYER 0x0079fc70
			// STUB: STUDIO 0x0073e490
			template<class T>
			Type(const char* name, T* dummy)
				:Descriptor(name, Descriptor::Attributes())
				,typeInfo(&typeid(T))
				,tag(Name::lookup(name))
				,isFloat(false)
				,isNumber(false)
				,isEnum(false)
			{
				RBXASSERT(!this->tag->str.empty());
				addToAllTypes();
			}

			// 2016-compat: (name, tag, T* dummy) — used by RefType etc.
			Type(const char* name, const char* tag, void* dummy)
				:Descriptor(name, Descriptor::Attributes())
				,typeInfo(&typeid(void))
				,tag(Name::declare(tag))
				,isFloat(false)
				,isNumber(false)
				,isEnum(false)
			{
				RBXASSERT(!this->tag->str.empty());
				addToAllTypes();
			}

			// mac: 0x15D260 pattern: (name, tag, type_info, isNumber, isFloat)
			// STUB: PLAYER 0x006f97e0
			// STUB: STUDIO 0x006db8a0
			Type(const char* name, const char* tag, const std::type_info& info, bool isNumber, bool isFloat)
				:Descriptor(name, Descriptor::Attributes())
				,typeInfo(&info)
				,tag(Name::declare(tag))
				,isFloat(isFloat)
				,isNumber(isNumber)
				,isEnum(false)
			{
				RBXASSERT(!this->tag->str.empty());
				addToAllTypes();
			}

			// enum types: (name, tag, type_info, isNumber=false, isFloat=false, isEnum=true)
			Type(const char* name, const char* tag, const std::type_info& info, bool isNumber, bool isFloat, bool isEnum)
				:Descriptor(name, Descriptor::Attributes())
				,typeInfo(&info)
				,tag(Name::declare(tag))
				,isFloat(isFloat)
				,isNumber(isNumber)
				,isEnum(isEnum)
			{
				RBXASSERT(!this->tag->str.empty());
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
				:Type(name, tag, typeid(T), false, false)
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

			template<class ValueType>
			inline bool isType() const {
				return _type->isType<ValueType>();
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

			template<typename T>
			inline const T& cast() const {
				if (!isType<T>())
					throw std::runtime_error("Variant cast failed");
				return *reinterpret_cast<const T*>(value.getData());
			}

			template<typename T>
			inline T& cast() {
				if (!isType<T>())
					throw std::runtime_error("Variant cast failed");
				return *reinterpret_cast<T*>(value.getData());
			}

			template<typename T>
			inline const T* tryCast() const {
				if (!isType<T>())
					return NULL;
				return reinterpret_cast<const T*>(value.getData());
			}

			template<typename T>
			inline T* tryCast() {
				if (!isType<T>())
					return NULL;
				return reinterpret_cast<T*>(value.getData());
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

		// STUB: PLAYER 0x0066f7f0
		// STUB: STUDIO 0x00780350
		template<class ValueType>
		ValueType& RBX::Reflection::Variant::genericConvert()
		{
			ValueType* id = tryCast<ValueType>();
			if (id != NULL)
				return *id;

			if (_type->isType<std::string>())
			{
				ValueType v;
				if (StringConverter<ValueType>::convertToValue(cast<std::string>(), v))
				{
					value = v;
					_type = &Type::singleton<ValueType>();
					return cast<ValueType>();
				}
			}

			throw RBX::runtime_error("Unable to cast %s to %s", _type->tag->str.c_str(), Type::singleton<ValueType>().tag->str.c_str());
		}
	}
}

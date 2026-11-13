/* script/LuaEnum.h -- October 2013 (first import, section 239).
   Evidence: AllEnumDescriptors/EnumDescriptor/Item bridge registerClasses
     (0x172214/0x172476/0x1726D8), on_index/newindex, gc/eq/tostring,
     GetEnumItems helper (0x1730D6), declareAllEnums (0x173386),
     wrappers (0x1A28EE-0x1A2AD2), classNames "Enums"/"Enum"/"EnumItem"
     (batches 408-419). 2016 LuaEnum.h carried for the skeleton with
     binary-pinned DELTAS: __eq=true throughout (2016 hardcodes false;
     binary has on_eq for all three and Lb1 manglings) and
     SingletonBridge<T, bool> two-arg form (same reason).
   Original file: Client/App/script/LuaEnum.cpp (.h companion). */

#pragma once

#include "lua/luabridge.h"
#include "Reflection/EnumConverter.h"
#include "RbxFormat.h"

namespace RBX { namespace Lua {

	class AllEnumDescriptors
	{
	};

	typedef const AllEnumDescriptors* AllEnumDescriptorsPtr;

	// The global "Enum" table: pushes the singleton holder.
	class Enums : public SingletonBridge<AllEnumDescriptorsPtr, true>
	{
	public:
		static void declareAllEnums(lua_State* L);
		static bool getValue(lua_State* L, unsigned int index, RBX::Reflection::Variant& value);
	};

	typedef const Reflection::EnumDescriptor* EnumDescriptorPtr;

	class Enum : public SingletonBridge<EnumDescriptorPtr, true>
	{
	public:
	};

	typedef const Reflection::EnumDescriptor::Item* EnumDescriptorItemPtr;

	class EnumItem : public SingletonBridge<EnumDescriptorItemPtr, true>
	{
	public:
		static EnumDescriptorItemPtr getItem(lua_State* L, unsigned int index)
		{
			return getObject(L, index);
		}
		static bool getItem(lua_State* L, unsigned int index, EnumDescriptorItemPtr& value)
		{
			return getValue(L, index, value);
		}
	};

	// Specialization declarations (definitions in script/LuaEnum.cpp).
	template<> void Bridge<AllEnumDescriptorsPtr, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<AllEnumDescriptorsPtr, true>::on_index(
		const AllEnumDescriptorsPtr& object, const char* name, lua_State* L);
	template<> void Bridge<AllEnumDescriptorsPtr, true>::on_newindex(
		AllEnumDescriptorsPtr& object, const char* name, lua_State* L);
	template<> int Bridge<AllEnumDescriptorsPtr, true>::on_index(lua_State* L);
	template<> int Bridge<AllEnumDescriptorsPtr, true>::on_gc(lua_State* L);
	template<> int Bridge<AllEnumDescriptorsPtr, true>::on_eq(lua_State* L);
	template<> int Bridge<AllEnumDescriptorsPtr, true>::on_tostring(lua_State* L);
	template<> int Bridge<AllEnumDescriptorsPtr, true>::on_tostring(
		const AllEnumDescriptorsPtr& object, lua_State* L);
	template<> void Bridge<EnumDescriptorPtr, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<EnumDescriptorPtr, true>::on_index(
		const EnumDescriptorPtr& object, const char* name, lua_State* L);
	template<> void Bridge<EnumDescriptorPtr, true>::on_newindex(
		EnumDescriptorPtr& object, const char* name, lua_State* L);
	template<> int Bridge<EnumDescriptorPtr, true>::on_index(lua_State* L);
	template<> int Bridge<EnumDescriptorPtr, true>::on_gc(lua_State* L);
	template<> int Bridge<EnumDescriptorPtr, true>::on_eq(lua_State* L);
	template<> int Bridge<EnumDescriptorPtr, true>::on_tostring(lua_State* L);
	template<> int Bridge<EnumDescriptorPtr, true>::on_tostring(
		const EnumDescriptorPtr& object, lua_State* L);
	template<> void Bridge<EnumDescriptorItemPtr, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<EnumDescriptorItemPtr, true>::on_index(
		const EnumDescriptorItemPtr& object, const char* name, lua_State* L);
	template<> void Bridge<EnumDescriptorItemPtr, true>::on_newindex(
		EnumDescriptorItemPtr& object, const char* name, lua_State* L);
	template<> int Bridge<EnumDescriptorItemPtr, true>::on_index(lua_State* L);
	template<> int Bridge<EnumDescriptorItemPtr, true>::on_gc(lua_State* L);
	template<> int Bridge<EnumDescriptorItemPtr, true>::on_eq(lua_State* L);
	template<> int Bridge<EnumDescriptorItemPtr, true>::on_tostring(lua_State* L);
	template<> int Bridge<EnumDescriptorItemPtr, true>::on_tostring(
		const EnumDescriptorItemPtr& object, lua_State* L);

} }

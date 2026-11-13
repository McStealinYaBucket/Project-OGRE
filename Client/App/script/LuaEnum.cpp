/* script/LuaEnum.cpp -- October 2013 (first import, section 239).
   Evidence: see include/script/LuaEnum.h header block.
   Bodies transcribed from batches 408-419; member order, failure texts,
   and the GetEnumItems stack shape replicated exactly.
   Original file: Client/App/script/LuaEnum.cpp. */

#include "stdafx.h"

#include "script/LuaEnum.h"

namespace RBX { namespace Lua {

	template<>
	// GLOBAL: PLAYER 0x00d8ee28
	// GLOBAL: STUDIO 0x00f2b620
	// Bridge<RBX::Lua::AllEnumDescriptors const *,1>::className
	const char* Bridge<AllEnumDescriptorsPtr, true>::className = "Enums";
	template<>
	// GLOBAL: PLAYER 0x00d8ee2c
	// GLOBAL: STUDIO 0x00f2b624
	// Bridge<RBX::Reflection::EnumDescriptor const *,1>::className
	const char* Bridge<EnumDescriptorPtr, true>::className = "Enum";
	template<>
	// GLOBAL: PLAYER 0x00d8ee30
	// GLOBAL: STUDIO 0x00f2b628
	// Bridge<RBX::Reflection::EnumDescriptor::Item const *,1>::className
	const char* Bridge<EnumDescriptorItemPtr, true>::className = "EnumItem";

	static int pushEnumList(lua_State* L);

	// mac: 0x1A28EE (96B, batch413): checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x006701a0
	// FUNCTION: STUDIO 0x007d8470
	template<>
	int Bridge<EnumDescriptorItemPtr, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x1A2A72 (96B, batch413): checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00672070
	// FUNCTION: STUDIO 0x007d9fd0
	template<>
	int Bridge<AllEnumDescriptorsPtr, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x1A29B0 (96B, batch413): checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x006721c0
	// FUNCTION: STUDIO 0x007da120
	template<>
	int Bridge<EnumDescriptorPtr, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x172FAC (95B, batch409).
	// FUNCTION: PLAYER 0x0087cf60
	// FUNCTION: STUDIO 0x009e15e0
	template<>
	void Bridge<AllEnumDescriptorsPtr, true>::on_newindex(
		AllEnumDescriptorsPtr& object, const char* name, lua_State* L)
	{
		throw RBX::runtime_error("Enums cannot be modified");
	}


	// mac: 0x17239E (50B, batch410): checkudata + return 0.
	template<>
	int Bridge<AllEnumDescriptorsPtr, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x1723D0 (103B, batch410): pointer compare.
	template<>
	int Bridge<AllEnumDescriptorsPtr, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x172438 (62B, batch410): checkudata + forward.
	template<>
	int Bridge<AllEnumDescriptorsPtr, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x173418 (43B, batch411): pushes "Enums".
	template<>
	int Bridge<AllEnumDescriptorsPtr, true>::on_tostring(
		const AllEnumDescriptorsPtr& object, lua_State* L)
	{
		lua_pushstring(L, "Enums");
		return 1;
	}

	// mac: 0x1731EA (95B, batch409).
	// FUNCTION: PLAYER 0x0087cf90
	// FUNCTION: STUDIO 0x009e1610
	template<>
	void Bridge<EnumDescriptorPtr, true>::on_newindex(
		EnumDescriptorPtr& object, const char* name, lua_State* L)
	{
		throw RBX::runtime_error("Enum cannot be modified");
	}

	// mac: 0x17324A (219B, batch409): Name (owner-name string chain) + Value
	// (int +20); plain failure otherwise.
	// FUNCTION: PLAYER 0x0087cfc0
	// FUNCTION: STUDIO 0x009e1640
	template<>
	int Bridge<EnumDescriptorItemPtr, true>::on_index(
		const EnumDescriptorItemPtr& object, const char* name, lua_State* L)
	{
		if (strcmp(name, "Name") == 0)
		{
			const std::string& s = object->name->str;
			lua_pushlstring(L, s.c_str(), s.size());
			return 1;
		}
		if (strcmp(name, "Value") == 0)
		{
			lua_pushnumber(L, object->value);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x173326 (95B, batch409).
	// FUNCTION: PLAYER 0x0087d0b0
	// FUNCTION: STUDIO 0x009e1730
	template<>
	void Bridge<EnumDescriptorItemPtr, true>::on_newindex(
		EnumDescriptorItemPtr& object, const char* name, lua_State* L)
	{
		throw RBX::runtime_error("EnumItem cannot be modified");
	}

	// mac: 0x17346E (194B, batch411): "Enum.<Owner>.<Item>" via RBX::format.
	// FUNCTION: PLAYER 0x0087d140
	// FUNCTION: STUDIO 0x009e17c0
	template<>
	int Bridge<EnumDescriptorItemPtr, true>::on_tostring(
		const EnumDescriptorItemPtr& object, lua_State* L)
	{
		std::string s = RBX::format("Enum.%s.%s",
			object->owner.name->str.c_str(), object->name->str.c_str());
		::lua_pushstring(L, s.c_str());
		return 1;
	}

	// mac: 0x1730D6 (276B, batch411): array of Item pushes (1-based). Tail
	// DELTA (0x17317D+): dev-only stack-balance assert triple (flag
	// global + hook + ReleaseAssert) -- unresolvable flag global, fires
	// only on pre-existing imbalance; the table loop + return are
	// verbatim. Loud per policy.
	// STUB: PLAYER 0x0087d230
	// STUB: STUDIO 0x009e18b0
	static int pushEnumList(lua_State* L)
	{
		const EnumDescriptorPtr& object = Bridge<EnumDescriptorPtr, true>::getObject(L, 1);
		lua_createtable(L, (int)object->getEnumCount(), 0);
		int pos = 1;
		for (std::vector<EnumDescriptorItemPtr>::const_iterator iter = object->begin();
			iter != object->end(); ++iter)
		{
			lua_pushnumber(L, pos++);
			EnumItem::push(L, *iter);
			lua_rawset(L, -3);
		}
		return 1;
	}


	// mac: 0x172600 (50B-pattern): checkudata + return 0.
	template<>
	int Bridge<EnumDescriptorPtr, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x172632 (eq-pattern): pointer compare.
	template<>
	int Bridge<EnumDescriptorPtr, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x17269A (62B-pattern): checkudata + forward.
	template<>
	int Bridge<EnumDescriptorPtr, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x173444 (42B, batch411): pushes the descriptor's own name
	// (Descriptor::name string).
	template<>
	int Bridge<EnumDescriptorPtr, true>::on_tostring(
		const EnumDescriptorPtr& object, lua_State* L)
	{
		const std::string& s = object->name->str;
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x17300C (202B, batch409): "GetEnumItems" -> sub_1730D6 closure;
	// else virtual lookup(name) -> SingletonBridge<Item>::push; NULL ->
	// "%s is not a valid EnumItem".
	// FUNCTION: PLAYER 0x0087d310
	// FUNCTION: STUDIO 0x009e1990
	template<>
	int Bridge<EnumDescriptorPtr, true>::on_index(
		const EnumDescriptorPtr& object, const char* name, lua_State* L)
	{
		if (strcmp(name, "GetEnumItems") == 0)
		{
			lua_pushcfunction(L, pushEnumList);
			return 1;
		}
		const Reflection::EnumDescriptor::Item* item = object->lookup(name);
		if (item != NULL) {	// push first, throw on the fall-through (0x0087d366)
			EnumItem::push(L, item); return 1; }
		throw RBX::runtime_error("%s is not a valid EnumItem", name);

	}

	// mac: 0x172F0E (158B, batch409): Name::lookup + lookupDescriptor(Name),
	// pushed via SingletonBridge<EnumDescriptor>; NULL -> "%s is not a
	// valid EnumItem".
	// STUB: PLAYER 0x0087d3f0
	// STUB: STUDIO 0x009e1a70
	template<>
	int Bridge<AllEnumDescriptorsPtr, true>::on_index(
		const AllEnumDescriptorsPtr& object, const char* name, lua_State* L)
	{
		const Reflection::EnumDescriptor* desc =
			Reflection::EnumDescriptor::lookupDescriptor(*RBX::Name::lookup(name));
		if (desc != NULL) {	// push first, throw on the fall-through (0x0087d42c)
			Enum::push(L, desc); return 1; }
		throw RBX::runtime_error("%s is not a valid EnumItem", name);

	}

	// mac: 0x1728FC (62B, batch410): checkudata + forward.
	// FUNCTION: PLAYER 0x0087f6f0
	// FUNCTION: STUDIO 0x009e3d70
	template<>
	int Bridge<EnumDescriptorItemPtr, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x1726D8 (394B-pattern): same metatable set.
	// FUNCTION: PLAYER 0x0087f8f0
	// FUNCTION: STUDIO 0x009e3f70
	template<>
	void Bridge<EnumDescriptorItemPtr, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*))
	{
		luaL_newmetatable(L, className);
		protect_metatable(L, -1);
		lua_pushstring(L, "__index");
		lua_pushcfunction(L, on_indexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, on_newindexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, on_gc);
		lua_settable(L, -3);
		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, on_eq);
		lua_settable(L, -3);
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, on_tostring);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}


	// mac: 0x172862 (50B-pattern): checkudata + return 0.
	template<>
	int Bridge<EnumDescriptorItemPtr, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x172894 (103B, batch410): pointer compare.
	template<>
	int Bridge<EnumDescriptorItemPtr, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x172214 (394B, batch409): index/newindex/gc/eq/tostring + readonly.
	// FUNCTION: PLAYER 0x008802b0
	// FUNCTION: STUDIO 0x009e4930
	template<>
	void Bridge<AllEnumDescriptorsPtr, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*))
	{
		luaL_newmetatable(L, className);
		protect_metatable(L, -1);
		lua_pushstring(L, "__index");
		lua_pushcfunction(L, on_indexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, on_newindexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, on_gc);
		lua_settable(L, -3);
		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, on_eq);
		lua_settable(L, -3);
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, on_tostring);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x172476 (394B, batch409): same metatable set as AllEnums.
	// FUNCTION: PLAYER 0x00880390
	// FUNCTION: STUDIO 0x009e4a10
	template<>
	void Bridge<EnumDescriptorPtr, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*))
	{
		luaL_newmetatable(L, className);
		protect_metatable(L, -1);
		lua_pushstring(L, "__index");
		lua_pushcfunction(L, on_indexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, on_newindexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, on_gc);
		lua_settable(L, -3);
		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, on_eq);
		lua_settable(L, -3);
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, on_tostring);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// Binary singleton holder (BSS unk_FAD891): one static instance.
	static AllEnumDescriptors g_allEnumDescriptors;

	// mac: 0x173386 (68B, batch409): push holder + setfield(GLOBALS, "Enum").
	void Enums::declareAllEnums(lua_State* L)
	{
		Enums::push(L, &g_allEnumDescriptors);
		lua_setfield(L, LUA_GLOBALSINDEX, "Enum");
	}

	// 2016 LuaEnum.cpp shape (needed by EnumItem::getItem consumers):
	// convert the item at index to a Variant.
	bool Enums::getValue(lua_State* L, unsigned int index, RBX::Reflection::Variant& value)
	{
		EnumDescriptorItemPtr item;
		if (EnumItem::getItem(L, index, item))
			return item->convertToValue(value);
		return false;
	}

	// VS2012 odr-use holders (openState names these at 0x18B80B/0x18B827/
	// mac: 0x18B843).
	int (*volatile g_bridgeAllEnumsIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<AllEnumDescriptorsPtr, true>::on_index;
	int (*volatile g_bridgeEnumDescIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<EnumDescriptorPtr, true>::on_index;
	int (*volatile g_bridgeEnumItemIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<EnumDescriptorItemPtr, true>::on_index;

} }

/* script/LuaArguments.cpp -- October 2013 (first import, section 246).
   Evidence: push 0x162624 + pushTuple 0x1996EA + 5 ArgumentPusher
     overloads (0x1643A6/0x164402/0x164548/0x164666/0x164794) +
     pushArray pair (0x164E22/0x165016, via the header template) +
     withVariantValue/getValues in include/script/LuaArguments.h
     (batches 460-464). Original file: Client/App/script/LuaArguments.cpp.
   Map/unordered iteration is spelled with iterators (the binary walks
     its own tree/bucket nodes; observable key order is identical for
     std::map and unspecified-but-complete for unordered -- tests assert
     membership, never order).
   Queued: Instance + Instances-vector pushers (Instance bridge era);
     GenericFunction pushers + closures + overloads (Tuple era);
     get/getValues + getX family (247). */

#include "stdafx.h"

#include "script/LuaArguments.h"
#include "script/LuaEnum.h"
#include "V8Tree/Instance.h"
#include "G3D/g3dmath.h"

namespace RBX { namespace Lua {

	// mac: 0x162624 (35B, batch460): dispatch through the switch.
	int LuaArguments::push(const Reflection::Variant& value, lua_State* const L)
	{
		return withVariantValue<int, ArgumentPusher>(value, L);
	}

	// getValues (0x00683f90) is inline in include/script/LuaArguments.h (2013 assert line 146).

	// mac: 0x164318 (26B, batch465): top - 1 (slot 0 is the function).
	// FUNCTION: PLAYER 0x00877b30
	// FUNCTION: STUDIO 0x009dc230
	size_t LuaArguments::size() const
	{
		return (size_t)(lua_gettop(L) - 1);
	}

	// mac: 0x1996EA (57B, batch460): push each element, summing counts.
	int LuaArguments::pushTuple(const Reflection::Tuple& arguments, lua_State* L)
	{
		int count = 0;
		for (Reflection::ValueArray::const_iterator iter = arguments.values.begin();
			iter != arguments.values.end(); ++iter)
		{
			count += withVariantValue<int, ArgumentPusher>(*iter, L);
		}
		return count;
	}

	// mac: 0x17B95C (71B, batch465; re-dumped batch726): getDouble, rounded.
	// The binary calls getDouble virtually through the Arguments base
	// (+16 slot = vtable slot 4); the YFSO era derived LuaArguments from
	// Reflection::FunctionDescriptor::Arguments (2026-10-16), so this
	// member call is once again a virtual dispatch -- the batch465
	// "direct call" DELTA is resolved.  Later eras drop getLong (the 2016
	// interface carries getRect/getPhysicalProperties instead); the 2013
	// vtable is the authority here.  Rounding: the binary is lrint
	// (ties-to-even, the default rounding mode; 42.5 -> 42, 43.5 -> 44).
	// The MSVC CRT (VC12) has no lrint/rint, so the same result is
	// spelled out by hand below.
	// STUB: PLAYER 0x00877bb0
	// STUB: STUDIO 0x009dc2b0
	bool LuaArguments::getLong(int index, long& value) const
	{
		double v;
		if (getDouble(index, v))
		{
			double rounded = ::floor(v);
			double fraction = v - rounded;	// exact for |v| < 2^52
			if (fraction > 0.5
				|| (fraction == 0.5 && ::fmod(rounded, 2.0) != 0.0))
			{
				rounded += 1.0;		// a tie rounds to the even neighbour
			}
			value = (long)rounded;
			return true;
		}
		return false;
	}

	// mac: 0x161804 (112B, batch465): string slot only.
	// FUNCTION: PLAYER 0x008a4110
	// FUNCTION: STUDIO 0x00927940
	bool LuaArguments::getString(int index, std::string& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		if (lua_type(L, index) != LUA_TSTRING)
			return false;
		size_t len;
		const char* s = lua_tolstring(L, index, &len);
		value.assign(s, len);
		return true;
	}

	// mac: 0x164548 (193B, batch463): same shape over the unordered map.
	int ArgumentPusher::operator()(const boost::shared_ptr<const Reflection::ValueTable>& value)
	{
		if (value.get() != NULL)
		{
			lua_createtable(L, 0, (int)value->size());
			for (Reflection::ValueTable::const_iterator iter = value->begin();
				iter != value->end(); ++iter)
			{
				lua_pushlstring(L, iter->first.c_str(), iter->first.size());
				withVariantValue<int, ArgumentPusher>(iter->second, L);
				lua_settable(L, -3);
			}
		}
		else
			lua_createtable(L, 0, 0);
		return 1;
	}

	// mac: 0x164666 (73B, batch463): pushes each element, summing counts
	// (null pushes nothing and returns 0 -- NOT an empty table).
	int ArgumentPusher::operator()(const boost::shared_ptr<const Reflection::Tuple>& value)
	{
		int count = 0;
		if (value.get() != NULL)
		{
			for (Reflection::ValueArray::const_iterator iter = value->values.begin();
				iter != value->values.end(); ++iter)
			{
				count += withVariantValue<int, ArgumentPusher>(*iter, L);
			}
		}
		return count;
	}

	// mac: 0x1646B0 (113B, batch463): shared-copy + lua_pushfunction (the
	// async twin at 0x164722 queues behind scheduleResume).
	int ArgumentPusher::operator()(boost::shared_ptr<Lua::GenericFunction> value)
	{
		Lua::lua_pushfunction(L, value);
		return 1;
	}

	// mac: 0x164722 (113B, batch463): same shape for async.
	int ArgumentPusher::operator()(boost::shared_ptr<Lua::GenericAsyncFunction> value)
	{
		Lua::lua_pushfunction(L, value);
		return 1;
	}

	// mac: 0x161A36 (88B, batch465): number slot only.
	// FUNCTION: PLAYER 0x008a4170
	// FUNCTION: STUDIO 0x009279a0
	bool LuaArguments::getDouble(int index, double& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		if (lua_type(L, index) != LUA_TNUMBER)
			return false;
		value = lua_tonumber(L, index);
		return true;
	}

	// mac: 0x161A8E (91B, batch465): boolean slot only.
	// STUB: PLAYER 0x008a41c0
	// STUB: STUDIO 0x009279f0
	bool LuaArguments::getBool(int index, bool& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		if (lua_type(L, index) != LUA_TBOOLEAN)
			return false;
		value = lua_toboolean(L, index) != 0;
		return true;
	}

	// mac: 0x161874/0x1618B8/0x1618FC/0x161940 (68B each, batch465): top-check
	// then the concrete Bridge::getValue (V=T, not Variant).
	// FUNCTION: PLAYER 0x008a4720
	// FUNCTION: STUDIO 0x00928630
	bool LuaArguments::getVector3int16(int index, G3D::Vector3int16& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		return Bridge<G3D::Vector3int16>::getValue(L, index, value);
	}

	// FUNCTION: PLAYER 0x008a4760
	// FUNCTION: STUDIO 0x00928670
	bool LuaArguments::getRegion3int16(int index, RBX::Region3int16& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		return Bridge<RBX::Region3int16>::getValue(L, index, value);
	}

	// FUNCTION: PLAYER 0x008a47a0
	// FUNCTION: STUDIO 0x009286b0
	bool LuaArguments::getVector3(int index, G3D::Vector3& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		return Bridge<G3D::Vector3>::getValue(L, index, value);
	}

	// mac: 0x1643A6 (91B, batch463): null -> empty table, else array push.
	// STUB: PLAYER 0x008a4ed0
	// STUB: STUDIO 0x00928de0
	int ArgumentPusher::operator()(const boost::shared_ptr<const Reflection::ValueArray>& value)
	{
		if (value.get() != NULL)
			LuaArguments::pushArray(value->begin(), value->end(), L);
		else
			lua_createtable(L, 0, 0);
		return 1;
	}

	// FUNCTION: PLAYER 0x008a5270
	// FUNCTION: STUDIO 0x00929180
	bool LuaArguments::getRegion3(int index, RBX::Region3& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		return Bridge<RBX::Region3>::getValue(L, index, value);
	}

	// mac: 0x161984 (178B, batch465): userdata -> shared Instance (nil reads
	// as null); other types are NOT converted.
	// STUB: PLAYER 0x008a52b0
	// STUB: STUDIO 0x009291c0
	bool LuaArguments::getObject(int index, boost::shared_ptr<Reflection::DescribedBase>& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		int t = lua_type(L, index);
		if (t == LUA_TUSERDATA)
			return SharedPtrBridge<Instance>::getPtr<boost::shared_ptr<Reflection::DescribedBase>>(L, index, value);
		if (t == LUA_TNIL)
		{
			value = boost::shared_ptr<Instance>();
			return true;
		}
		return false;
	}

	// mac: 0x164402 (325B, batch463): null -> empty table, else string-keyed
	// table with the empty-key assert gate (LuaArguments.cpp:436).
	// STUB: PLAYER 0x008a53b0
	// STUB: STUDIO 0x009292c0
	int ArgumentPusher::operator()(const boost::shared_ptr<const Reflection::ValueMap>& value)
	{
		if (value.get() != NULL)
		{
			lua_createtable(L, 0, (int)value->size());
			for (Reflection::ValueMap::const_iterator iter = value->begin();
				iter != value->end(); ++iter)
			{
				if (FLog::Asserts != 0 && iter->first.empty())
				{
					if (RBX::_internal::_debugHook == 0
						|| RBX::_internal::_debugHook("!_First->first.empty()",
							"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaArguments.cpp", 436) == 0)
					{
						ReleaseAssert(FLog::Asserts,
							"!_First->first.empty() file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaArguments.cpp line: 436");
					}
				}
				lua_pushlstring(L, iter->first.c_str(), iter->first.size());
				withVariantValue<int, ArgumentPusher>(iter->second, L);
				lua_settable(L, -3);
			}
		}
		else
			lua_createtable(L, 0, 0);
		return 1;
	}

	// getVariant (0x00877b40) is inline in include/script/LuaArguments.h (2013 assert line 178).

	// mac: 0x161AEA (240B, batch465): number (isValue-gated) or EnumItem
	// userdata of the SAME descriptor (owner-identity via Type::operator!=).
	// STUB: PLAYER 0x008a5dc0
	// STUB: STUDIO 0x00929cd0
	bool LuaArguments::getEnum(int index, const Reflection::EnumDescriptor& desc, int& value) const
	{
		index += offset;
		if (index > lua_gettop(L))
			return false;
		int t = lua_type(L, index);
		if (t == LUA_TNUMBER)
		{
			value = (int)lua_tonumber(L, index);
			return desc.isValue(value);
		}
		if (t == LUA_TUSERDATA)
		{
			const Reflection::EnumDescriptor::Item* item;
			if (Bridge<const Reflection::EnumDescriptor::Item*>::getValue(L, index, item))
			{
				if (item->owner != desc)
					return false;
				value = item->value;
				return true;
			}
		}
		return false;
	}

	// mac: 0x161BDA (2601B, batch460): stack slot -> Variant by lua_type.
	// Userdata tries Enums, then shared_ptr<Instance>, then the 16 value
	// bridges in binary order (no Rect2D/Physical/sequences in 2013;
	// InputObject last), and settles void-but-true on total failure.
	// 2013-vs-2016 DELTAs: 4 args, no TablesCollection/getRec (no cyclic
	// guard -- "tables cannot be cyclic" does NOT exist in 2013); TNONE
	// honors treatNilAsMissing (2016's guard returns false first, which
	// can only produce the same observable for real indices).
	// STUB: PLAYER 0x008a7100
	// STUB: STUDIO 0x0092af80
	bool LuaArguments::get(lua_State* L, int luaIndex, Reflection::Variant& value, bool treatNilAsMissing)
	{
		if (luaIndex > lua_gettop(L))
			return false;

		switch (lua_type(L, luaIndex))
		{
		case LUA_TNIL:
			// (Binary case 0 is LUA_TNIL, not TNONE: lua_type yields -1
			// only for invalid indices, which the `ja default` takes.)
			if (treatNilAsMissing)
				return false;
			value = Reflection::Variant();
			return true;

		case LUA_TBOOLEAN:
			value = (lua_toboolean(L, luaIndex) != 0);
			return true;

		case LUA_TNUMBER:
			value = lua_tonumber(L, luaIndex);
			return true;

		case LUA_TSTRING:
			{
				size_t len;
				const char* s = lua_tolstring(L, luaIndex, &len);
				value = std::string(s, len);
			}
			return true;

		case LUA_TTABLE:
			{
				int n = (int)lua_objlen(L, luaIndex);
				if (n > 0)
				{
					boost::shared_ptr<Reflection::ValueArray> values(
						rbx::make_shared<Reflection::ValueArray>(n));
					for (int i = 1; i <= n; ++i)
					{
						Reflection::Variant& v = (*values)[i - 1];
						lua_rawgeti(L, luaIndex, i);
						LuaArguments::get(L, -1, v, false);
						lua_pop(L, 1);
					}
					value = boost::shared_ptr<const Reflection::ValueArray>(values);
				}
				else
				{
					boost::shared_ptr<Reflection::ValueTable> values;
					int tableIndex = luaIndex > 0 ? luaIndex : lua_gettop(L) + luaIndex + 1;
					lua_pushnil(L);
					while (lua_next(L, tableIndex) != 0)
					{
						if (lua_type(L, -2) != LUA_TSTRING)
							throw std::runtime_error("keys must be strings");
						std::string key = lua_tostring(L, -2);
						Reflection::Variant v;
						LuaArguments::get(L, -1, v, false);
						if (!values)
							values = rbx::make_shared<Reflection::ValueTable>();
						(*values)[key] = v;
						lua_pop(L, 1);
					}
					if (values)
						value = boost::shared_ptr<const Reflection::ValueTable>(values);
					else
						value = rbx::make_shared<const Reflection::ValueArray>();
				}
			}
			return true;

		case LUA_TFUNCTION:
			// DELTA vs binary (documents a missing-body wall): the binary
			// assigns through placement_any::operator=<WeakFunctionRef>,
			// whose same-holder path needs WeakFunctionRef::operator=
			// (declared ThreadRef.h:171, body unswept -- queued with the
			// yield era). Construct-then-copy through a temporary Variant
			// is behavior-identical (one balanced transient registry
			// ref; indices unobservable) and odr-uses only the defined
			// Variant copy-assign.
			value = Reflection::Variant(lua_tofunction(L, luaIndex));
			return true;

		case LUA_TUSERDATA:
			{
				if (Enums::getValue(L, luaIndex, value))
					return true;
				if (SharedPtrBridge<Instance>::getPtr<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<G3D::CoordinateFrame>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::Region3>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::Region3int16>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<G3D::Vector3int16>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<G3D::Vector2int16>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<G3D::Vector3>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<G3D::Vector2>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::RbxRay>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<G3D::Color3>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::BrickColor>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::UDim>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::UDim2>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::Faces>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::Axes>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::CellID>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				if (Bridge<RBX::InputObject>::getValue<Reflection::Variant>(L, luaIndex, value))
					return true;
				value = Reflection::Variant();
			}
			return true;

		default:
			value = Reflection::Variant();
			return true;
		}
	}

} }

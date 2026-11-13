/* script/LuaAtomicClasses.cpp -- October 2013 Vector3 slice (section 222).
   Evidence: see include/script/LuaAtomicClasses.h header block.
   Bodies transcribed from the batch347/batch351 disassembly; lane order
   and coercion behavior replicated exactly (see DELTAs below).
   Original file: Client/App/script/LuaAtomicClasses.cpp. */

#include "stdafx.h"

#include "script/LuaAtomicClasses.h"
#include "script/LuaEnum.h"
#include "RbxFormat.h"
#include "Util/Math.h"
#include "Util/Utilities.h"
#include "Util/FastFlags.h"
#include "util/exception.h"
#include <cmath>
#include <cctype>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace RBX { namespace Lua {

	// mac: 0x166071 (151B, batch354; 2016 LuaAtomicClasses.cpp carried the
	// shape but got NaN wrong). DELTA vs 2016: NaN maps to +inf (the
	// `!(v < +inf)` arm catches unordered), NOT (float)NaN. Order:
	// +inf (incl. NaN) -> -inf -> +FLT_MAX -> -FLT_MAX -> plain narrow.
	float lua_tofloat(lua_State* L, int idx)
	{
		double value = lua_tonumber(L, idx);
		if (!(value < std::numeric_limits<double>::infinity()))
			return std::numeric_limits<float>::infinity();
		if (!(-std::numeric_limits<double>::infinity() < value))
			return -std::numeric_limits<float>::infinity();
		if (value > std::numeric_limits<float>::max())
			return std::numeric_limits<float>::max();
		if (value < -std::numeric_limits<float>::max())
			return -std::numeric_limits<float>::max();
		return (float)value;
	}

	// FUNCTION: PLAYER 0x0066fad0
	// FUNCTION: STUDIO 0x007d7e90
	void protect_metatable(lua_State* thread, int index)
	{
		// mac: 0x18994C (batch349): push the lock string, then set __metatable
		// on index-1 (the metatable sits below the pushed string).
		lua_pushstring(thread, "The metatable is locked");
		lua_setfield(thread, index - 1, "__metatable");
	}

	// mac: 0x165EB0 (58B, batch435): tolstring-or-empty (carried 2016 location).
	const char* safe_lua_tostring(lua_State* L, int idx)
	{
		static const char* empty = "";
		const char* s = lua_tostring(L, idx);
		return s ? s : empty;
	}

	// mac: 0x165EEA (391B, batch539): tolstring-or-throw, with the
	// super-long-string guard (200000 chars) behind DFFlag::
	// ShutdownScriptsThatGenerateSuperLongStrings (BSS-false default).
	// FUNCTION: PLAYER 0x00881850
	// FUNCTION: STUDIO 0x0092b920
	const char* throwable_lua_tostring(lua_State* L, int idx)
	{
		const char* s = lua_tolstring(L, idx, NULL);
		if (s == NULL)
			throw std::runtime_error("String expected");
		if (DFFlag::ShutdownScriptsThatGenerateSuperLongStrings
			&& strlen(s) >= 0x30D40)
			throw std::runtime_error("String too long");
		return s;
	}

	// Owner-type gate shared by the enum-consuming constructors (0x166F5C,
	// mac: 0x16704C, 0x16BD06, 0x16C07E, 0x16C826): the item's owner descriptor
	// must carry the expected typeinfo (pointer compare first, then the GCC
	// mangled-name fallback the binary performs -- that arm can only fire
	// in GCC builds since MSVC decorates names differently, but the shape
	// is kept binary-faithful).
	static bool enumItemIsA(const RBX::Reflection::EnumDescriptor::Item* item,
		const std::type_info& ti, const char* gccName)
	{
		const std::type_info* owner = item->owner.typeInfo;
		return owner == &ti || strcmp(owner->name(), gccName) == 0;
	}

	// mac: 0x16D168 (692B, batch347). No "__type" entry (2016 adds one); __gc
	// IS registered (2016 Vector3 specialization omits it); __index /
	// __newindex are the caller-passed lua_CFunctions (openState passes
	// Bridge<Vector3>::on_index/on_newindex 0x19855C/0x1985BC); arithmetic
	// is this type's Vector3Bridge metamethods. Ends readonly + pop.
	// STUB: PLAYER 0x00888460
	// STUB: STUDIO 0x00932460
	void Bridge<G3D::Vector3, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, Vector3Bridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, Vector3Bridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, Vector3Bridge::on_mul);
		lua_settable(L, -3);
		lua_pushstring(L, "__div");
		lua_pushcfunction(L, Vector3Bridge::on_div);
		lua_settable(L, -3);
		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, Vector3Bridge::on_unm);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x16719E (154B, batch351): lane-wise add via luaL_checkudata pair.
	int Vector3Bridge::on_add(lua_State* L)
	{
		const G3D::Vector3& a = getObject(L, 1);
		const G3D::Vector3& b = getObject(L, 2);
		pushVector3(L, G3D::Vector3(a.x + b.x, a.y + b.y, a.z + b.z));
		return 1;
	}

	// mac: 0x167238 (154B, batch351): lane-wise subtract.
	int Vector3Bridge::on_sub(lua_State* L)
	{
		const G3D::Vector3& a = getObject(L, 1);
		const G3D::Vector3& b = getObject(L, 2);
		pushVector3(L, G3D::Vector3(a.x - b.x, a.y - b.y, a.z - b.z));
		return 1;
	}

	// mac: 0x1672D2 (431B, batch351). DELTA vs 2016 (no isnumber/throw arms):
	// Vector3 op non-Vector3 coerces via lua_tofloat; scalar op
	// non-Vector3 is a luaL_checkudata hard error (getObject throws).
	int Vector3Bridge::on_mul(lua_State* L)
	{
		G3D::Vector3 a(0, 0, 0);
		if (getValue(L, 1, a))
		{
			G3D::Vector3 b(0, 0, 0);
			if (getValue(L, 2, b))
				pushVector3(L, G3D::Vector3(a.x * b.x, a.y * b.y, a.z * b.z));
			else
			{
				float c = lua_tofloat(L, 2);
				pushVector3(L, G3D::Vector3(c * a.x, a.y * c, a.z * c));
			}
		}
		else
		{
			const G3D::Vector3& b = getObject(L, 2);
			float c = lua_tofloat(L, 1);
			pushVector3(L, G3D::Vector3(b.x * c, b.y * c, c * b.z));
		}
		return 1;
	}

	// mac: 0x167482 (437B, batch351). DELTAs vs 2016: same no-throw shape as
	// on_mul; Vector3/scalar computes (1.0/c)*v lane-wise (reciprocal
	// multiply -- NOT operator/(v, c); assoc differs, replicated
	// explicitly); scalar/Vector3 divides lane-wise (b/a.x, b/a.y, b/a.z).
	int Vector3Bridge::on_div(lua_State* L)
	{
		G3D::Vector3 a(0, 0, 0);
		if (getValue(L, 1, a))
		{
			G3D::Vector3 b(0, 0, 0);
			if (getValue(L, 2, b))
				pushVector3(L, G3D::Vector3(a.x / b.x, a.y / b.y, a.z / b.z));
			else
			{
				float c = lua_tofloat(L, 2);
				float r = 1.0f / c;
				pushVector3(L, G3D::Vector3(a.x * r, a.y * r, a.z * r));
			}
		}
		else
		{
			const G3D::Vector3& b = getObject(L, 2);
			float c = lua_tofloat(L, 1);
			pushVector3(L, G3D::Vector3(c / b.x, c / b.y, c / b.z));
		}
		return 1;
	}

	// mac: 0x167638 (127B, batch351): lane negate (binary xorps sign-bit).
	int Vector3Bridge::on_unm(lua_State* L)
	{
		const G3D::Vector3& a = getObject(L, 1);
		pushVector3(L, G3D::Vector3(-a.x, -a.y, -a.z));
		return 1;
	}

	template<>
	// GLOBAL: PLAYER 0x00d8f2f8
	// GLOBAL: STUDIO 0x00f0b980
	// Bridge<G3D::Vector3,1>::className
	const char* Bridge<G3D::Vector3, true>::className = "Vector3";

	// File-static helpers (no binary names; EAs 0x1679C8/0x167AA0/0x167B64/
	// mac: 0x167BF2, batch353). Lane assoc replicated exactly: lerp is
	// ((b-a)*t)+a (NOT G3D::lerp's a+(b-a)*t); dot sums ((x*x)+(y*y))+(z*z).
	static int lerpVector3(lua_State* L)
	{
		const G3D::Vector3& a = Bridge<G3D::Vector3>::getObject(L, 1);
		const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, 2);
		float t = lua_tofloat(L, 3);
		Bridge<G3D::Vector3>::pushNewObject(L, G3D::Vector3(
			(b.x - a.x) * t + a.x,
			(b.y - a.y) * t + a.y,
			(b.z - a.z) * t + a.z));
		return 1;
	}

	static int crossVector3(lua_State* L)
	{
		const G3D::Vector3& a = Bridge<G3D::Vector3>::getObject(L, 1);
		const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, 2);
		Bridge<G3D::Vector3>::pushNewObject(L, G3D::Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x));
		return 1;
	}

	static int dotVector3(lua_State* L)
	{
		const G3D::Vector3& a = Bridge<G3D::Vector3>::getObject(L, 1);
		const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, 2);
		lua_pushnumber(L, (a.x * b.x + a.y * b.y) + a.z * b.z);
		return 1;
	}

	static int isCloseVector3(lua_State* L)
	{
		int count = lua_gettop(L);
		const G3D::Vector3& a = Bridge<G3D::Vector3>::getObject(L, 1);
		const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, 2);
		// Binary default epsilon bits 0x3727C5AC == 1.0e-5f (== the
		// Math::fuzzyEq default); explicit arg passes fabsf(arg3).
		bool result;
		if (count < 3)
			result = Math::fuzzyEq(a, b, 1.0e-5f);
		else
			result = Math::fuzzyEq(a, b, fabsf(lua_tofloat(L, 3)));
		lua_pushboolean(L, result);
		return 1;
	}

	// mac: 0x19855C (96B, batch352): lua_CFunction wrapper. DELTA vs the
	// header's generic wrapper (lua_checkstring_secure): the binary uses
	// luaL_checklstring verbatim.
	// FUNCTION: PLAYER 0x00671940
	// FUNCTION: STUDIO 0x007d98a0
	template<>
	int Bridge<G3D::Vector3, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x167CB2 (102B, batch352): always throws, with the name. The original's linker folded every
	// "%s cannot be assigned to" thrower onto this one body (the generic on_newindex wrappers all call it).
	// FUNCTION: PLAYER 0x00881b80
	// FUNCTION: STUDIO 0x0092c410
	template<>
	void Bridge<G3D::Vector3, true>::on_newindex(G3D::Vector3& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x1676B8 (784B, batch352). Member order + case variants replicated:
	// x/X y/Y z/Z unit/Unit magnitude/Magnitude lerp/Lerp Cross Dot
	// isClose, else runtime_error with the name. unit/magnitude use
	// sqrtf(z*z + ((x*x) + (y*y))) lane order -- NOT G3D::magnitude()'s
	// ((x*x)+(y*y))+(z*z).
	// STUB: PLAYER 0x00884400
	// STUB: STUDIO 0x0092e400
	template<>
	int Bridge<G3D::Vector3, true>::on_index(const G3D::Vector3& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "x") == 0 || strcmp(name, "X") == 0)
		{
			lua_pushnumber(L, object.x);
			return 1;
		}
		if (strcmp(name, "y") == 0 || strcmp(name, "Y") == 0)
		{
			lua_pushnumber(L, object.y);
			return 1;
		}
		if (strcmp(name, "z") == 0 || strcmp(name, "Z") == 0)
		{
			lua_pushnumber(L, object.z);
			return 1;
		}
		if (strcmp(name, "unit") == 0 || strcmp(name, "Unit") == 0)
		{
			float r = 1.0f / sqrtf(object.z * object.z +
				(object.x * object.x + object.y * object.y));
			pushNewObject(L, G3D::Vector3(object.x * r, object.y * r, object.z * r));
			return 1;
		}
		if (strcmp(name, "magnitude") == 0 || strcmp(name, "Magnitude") == 0)
		{
			lua_pushnumber(L, sqrtf(object.z * object.z +
				(object.x * object.x + object.y * object.y)));
			return 1;
		}
		if (strcmp(name, "lerp") == 0 || strcmp(name, "Lerp") == 0)
		{
			lua_pushcfunction(L, lerpVector3);
			return 1;
		}
		if (strcmp(name, "Cross") == 0)
		{
			lua_pushcfunction(L, crossVector3);
			return 1;
		}
		if (strcmp(name, "Dot") == 0)
		{
			lua_pushcfunction(L, dotVector3);
			return 1;
		}
		if (strcmp(name, "isClose") == 0)
		{
			lua_pushcfunction(L, isCloseVector3);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}


	// VS2012 RULE (proven by dumpbin 2026-10-07): an explicit member
	// specialization that is never odr-used in-TU is silently dropped from
	// the obj despite a clean compile (no error, no symbol) -- the L-arity
	// pair vanished through four definition spellings while all odr-used
	// siblings emitted. These volatile holders take the addresses at
	// namespace scope, forcing emission. They mirror the openState arg
	// loads (0x18B92E/0x18B932) that name these two symbols; when the
	// openState bridge wiring lands (ScriptContext.cpp is in this lib),
	// the lib will odr-use them itself and these holders can go.
	int (*volatile g_bridgeV3Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector3>::on_index;

	// mac: 0x16EB12 (48B, batch352): checkudata + return 0. No dtor call
	// (POD; the header's generic on_gc runs ~Class() instead).
	template<>
	int Bridge<G3D::Vector3, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16EB42 (144B, batch352): NaN-aware lane == (ucomiss+jp per lane;
	// plain == chain is exactly equivalent).
	template<>
	int Bridge<G3D::Vector3, true>::on_eq(lua_State* L)
	{
		const G3D::Vector3& a = getObject(L, 1);
		const G3D::Vector3& b = getObject(L, 2);
		lua_pushboolean(L, a.x == b.x && a.y == b.y && a.z == b.z);
		return 1;
	}

	// mac: 0x16EBD2 (60B, batch352): checkudata + forward.
	template<>
	int Bridge<G3D::Vector3, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F59E (170B, batch352): StringConverter + pushlstring (already
	// ported in v8tree/EnumProperty.cpp).
	template<>
	int Bridge<G3D::Vector3, true>::on_tostring(const G3D::Vector3& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<G3D::Vector3>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// ---- Vector2 (section 223) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f304
	// GLOBAL: STUDIO 0x00f0b98c
	// Bridge<G3D::Vector2,1>::className
	const char* Bridge<G3D::Vector2, true>::className = "Vector2";

	// mac: 0x16D41C (692B, batch357): same 3-arg shape as Vector3 (no __type,
	// __gc kept, arithmetic is Vector2Bridge's).
	// STUB: PLAYER 0x008885e0
	// STUB: STUDIO 0x009325e0
	void Bridge<G3D::Vector2, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, Vector2Bridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, Vector2Bridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, Vector2Bridge::on_mul);
		lua_settable(L, -3);
		lua_pushstring(L, "__div");
		lua_pushcfunction(L, Vector2Bridge::on_div);
		lua_settable(L, -3);
		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, Vector2Bridge::on_unm);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x168B94 (128B, batch356): lane-wise add.
	int Vector2Bridge::on_add(lua_State* L)
	{
		const G3D::Vector2& a = getObject(L, 1);
		const G3D::Vector2& b = getObject(L, 2);
		pushVector2(L, G3D::Vector2(a.x + b.x, a.y + b.y));
		return 1;
	}

	// mac: 0x168C14 (128B, batch356): lane-wise subtract.
	int Vector2Bridge::on_sub(lua_State* L)
	{
		const G3D::Vector2& a = getObject(L, 1);
		const G3D::Vector2& b = getObject(L, 2);
		pushVector2(L, G3D::Vector2(a.x - b.x, a.y - b.y));
		return 1;
	}

	// mac: 0x168C94 (299B, batch356): same no-throw shape as Vector3. Lane
	// order replicated (vec*vec: y then x; vec*scalar: x=c*x, y=y*c;
	// scalar*vec: x=x*c, y=c*y -- commuted products are bitwise identical
	// for finite floats; order kept for fidelity).
	int Vector2Bridge::on_mul(lua_State* L)
	{
		G3D::Vector2 a(0, 0);
		if (getValue(L, 1, a))
		{
			G3D::Vector2 b(0, 0);
			if (getValue(L, 2, b))
				pushVector2(L, G3D::Vector2(a.x * b.x, a.y * b.y));
			else
			{
				float c = lua_tofloat(L, 2);
				pushVector2(L, G3D::Vector2(c * a.x, a.y * c));
			}
		}
		else
		{
			const G3D::Vector2& b = getObject(L, 2);
			float c = lua_tofloat(L, 1);
			pushVector2(L, G3D::Vector2(b.x * c, c * b.y));
		}
		return 1;
	}

	// mac: 0x168DC0 (297B, batch356). DELTA vs Vector3::on_div: the
	// Vector2/scalar arm calls G3D::Vector2::operator/(float) DIRECTLY
	// (0x168EB1) -- no reciprocal-multiply. Replicated via a / c.
	int Vector2Bridge::on_div(lua_State* L)
	{
		G3D::Vector2 a(0, 0);
		if (getValue(L, 1, a))
		{
			G3D::Vector2 b(0, 0);
			if (getValue(L, 2, b))
				pushVector2(L, G3D::Vector2(a.x / b.x, a.y / b.y));
			else
			{
				float c = lua_tofloat(L, 2);
				pushVector2(L, a / c);
			}
		}
		else
		{
			const G3D::Vector2& b = getObject(L, 2);
			float c = lua_tofloat(L, 1);
			pushVector2(L, G3D::Vector2(c / b.x, c / b.y));
		}
		return 1;
	}

	// mac: 0x168EEA (103B, batch356): lane negate.
	int Vector2Bridge::on_unm(lua_State* L)
	{
		const G3D::Vector2& a = getObject(L, 1);
		pushVector2(L, G3D::Vector2(-a.x, -a.y));
		return 1;
	}

	// mac: 0x169136 (178B, batch358): ((b-a)*t)+a per lane.
	static int lerpVector2(lua_State* L)
	{
		const G3D::Vector2& a = Bridge<G3D::Vector2>::getObject(L, 1);
		const G3D::Vector2& b = Bridge<G3D::Vector2>::getObject(L, 2);
		float t = lua_tofloat(L, 3);
		Bridge<G3D::Vector2>::pushNewObject(L, G3D::Vector2(
			(b.x - a.x) * t + a.x,
			(b.y - a.y) * t + a.y));
		return 1;
	}

	// mac: 0x168F52 (484B, batch356). Member set matches 2016 (x/X y/Y unit
	// magnitude lerp -- all lowercase-only past y); unit/magnitude lane
	// order x*x+y*y matches G3D::direction()/length() exactly.
	template<>
	int Bridge<G3D::Vector2, true>::on_index(const G3D::Vector2& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "x") == 0 || strcmp(name, "X") == 0)
		{
			lua_pushnumber(L, object.x);
			return 1;
		}
		if (strcmp(name, "y") == 0 || strcmp(name, "Y") == 0)
		{
			lua_pushnumber(L, object.y);
			return 1;
		}
		if (strcmp(name, "unit") == 0)
		{
			pushNewObject(L, object.direction());
			return 1;
		}
		if (strcmp(name, "magnitude") == 0)
		{
			lua_pushnumber(L, object.length());
			return 1;
		}
		if (strcmp(name, "lerp") == 0)
		{
			lua_pushcfunction(L, lerpVector2);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x1691E8 (102B, batch356): always throws, with the name.
	template<>
	void Bridge<G3D::Vector2, true>::on_newindex(G3D::Vector2& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x19861E (96B, batch357): luaL_checklstring wrapper verbatim.
	template<>
	int Bridge<G3D::Vector2, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x19867E (98B, batch357): same verbatim.
	template<>
	int Bridge<G3D::Vector2, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x16EC0E (48B, batch357): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<G3D::Vector2, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16EC3E (131B, batch357): 2-lane NaN-aware == (plain && chain
	// is exactly equivalent).
	template<>
	int Bridge<G3D::Vector2, true>::on_eq(lua_State* L)
	{
		const G3D::Vector2& a = getObject(L, 1);
		const G3D::Vector2& b = getObject(L, 2);
		lua_pushboolean(L, a.x == b.x && a.y == b.y);
		return 1;
	}

	// mac: 0x16ECC2 (60B, batch357): checkudata + forward.
	template<>
	int Bridge<G3D::Vector2, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F648 (170B, batch357): StringConverter (already ported in
	// v8tree/EnumProperty.cpp) + pushlstring.
	template<>
	int Bridge<G3D::Vector2, true>::on_tostring(const G3D::Vector2& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<G3D::Vector2>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// VS2012 odr-use holders (same rule as Vector3; openState names these
	// at 0x18B944-0x18B94A).
	int (*volatile g_bridgeV2Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector2>::on_index;
	int (*volatile g_bridgeV2NewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector2>::on_newindex;

	// ---- UDim (section 224) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f310
	// GLOBAL: STUDIO 0x00f0b998
	// Bridge<RBX::UDim,1>::className
	const char* Bridge<RBX::UDim, true>::className = "UDim";

	// mac: 0x16D948 (512B, batch360): index/newindex/gc/eq/add/sub/unm only.
	// NO __tostring, NO __mul/__div (verified by body, not by absence).
	// STUB: PLAYER 0x00888760
	// STUB: STUDIO 0x00932760
	void Bridge<RBX::UDim, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, UDimBridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, UDimBridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, UDimBridge::on_unm);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x16B59C/0x16B616/0x16B690 (122B/122B/86B, batch360): thin
	// forwarders to UDim::operator+/-/-() (already ported, EAs banked in
	// UDim.h).
	int UDimBridge::on_add(lua_State* L)
	{
		const RBX::UDim& a = getObject(L, 1);
		const RBX::UDim& b = getObject(L, 2);
		pushUDim(L, a + b);
		return 1;
	}

	int UDimBridge::on_sub(lua_State* L)
	{
		const RBX::UDim& a = getObject(L, 1);
		const RBX::UDim& b = getObject(L, 2);
		pushUDim(L, a - b);
		return 1;
	}

	int UDimBridge::on_unm(lua_State* L)
	{
		pushUDim(L, -getObject(L, 1));
		return 1;
	}

	// mac: 0x16B6E6 (298B, batch360). offset is int16 @+4 (movsx + pushinteger;
	// matches UDim.h G3D::int16). Failure arms are isupper-gated exactly
	// like 2016 (NULL or uppercase-first -> plain; lowercase-first ->
	// capitalize hint).
	// FUNCTION: PLAYER 0x00881ff0
	// FUNCTION: STUDIO 0x0092c090
	template<>
	int Bridge<RBX::UDim, true>::on_index(const RBX::UDim& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "Scale") == 0)
		{
			lua_pushnumber(L, object.scale);
			return 1;
		}
		if (strcmp(name, "Offset") == 0)
		{
			lua_pushinteger(L, object.offset);
			return 1;
		}
		if (name && !isupper(name[0]))
			throw RBX::runtime_error(
				"%s is not a valid member, did you forget to capitalize the first letter?", name);
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x16B810 (102B, batch360): always throws, with the name.
	template<>
	void Bridge<RBX::UDim, true>::on_newindex(RBX::UDim& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198926 (96B, batch360): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671b80
	// FUNCTION: STUDIO 0x007d9ae0
	template<>
	int Bridge<RBX::UDim, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x16EE06 (48B, batch360): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::UDim, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16EE36 (124B, batch360): scale== (NaN-aware) && offset int16==.
	// UDim::operator== is exactly this; reuse it.
	template<>
	int Bridge<RBX::UDim, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// ---- UDim2 (section 224) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f314
	// GLOBAL: STUDIO 0x00f0b99c
	// Bridge<RBX::UDim2,1>::className
	const char* Bridge<RBX::UDim2, true>::className = "UDim2";

	// mac: 0x16BB50 (336B, batch361): X/Width -> UDim copy @+0, Y/Height ->
	// UDim @+8 (UDim2 is two packed UDims per UDim.h); same isupper-gated
	// errors as UDim.
	// STUB: PLAYER 0x00887be0
	// STUB: STUDIO 0x00931be0
	template<>
	int Bridge<RBX::UDim2, true>::on_index(const RBX::UDim2& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "X") == 0 || strcmp(name, "Width") == 0)
		{
			Bridge<RBX::UDim>::pushNewObject(L, object.x);
			return 1;
		}
		if (strcmp(name, "Y") == 0 || strcmp(name, "Height") == 0)
		{
			Bridge<RBX::UDim>::pushNewObject(L, object.y);
			return 1;
		}
		if (name && !isupper(name[0]))
			throw RBX::runtime_error(
				"%s is not a valid member, did you forget to capitalize the first letter?", name);
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x16DB48 (512B, batch361): same set as UDim (add/sub/unm; no
	// tostring/mul/div).
	// STUB: PLAYER 0x00888880
	// STUB: STUDIO 0x00932880
	void Bridge<RBX::UDim2, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, UDim2Bridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, UDim2Bridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, UDim2Bridge::on_unm);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x16BCA0 (102B, batch361): always throws, with the name.
	template<>
	void Bridge<RBX::UDim2, true>::on_newindex(RBX::UDim2& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x1989E8 (96B, batch361): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671be0
	// FUNCTION: STUDIO 0x007d9b40
	template<>
	int Bridge<RBX::UDim2, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x16EEB2 (48B, batch361): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::UDim2, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16EEE2 (147B, batch361): 4-lane (scaleX/offsetX/scaleY/offsetY,
	// scales NaN-aware). UDim2::operator== chains UDim::operator== and is
	// exactly this; reuse it.
	template<>
	int Bridge<RBX::UDim2, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x16B9D6/0x16BA60/0x16BAEA (137B/137B/101B, batch363): thin
	// forwarders to UDim2::operator+/-/-() (already ported, EAs banked in
	// UDim.h). Binary calls luaL_checkudata directly; getObject is the
	// same call plus deref.
	int UDim2Bridge::on_add(lua_State* L)
	{
		const RBX::UDim2& a = getObject(L, 1);
		const RBX::UDim2& b = getObject(L, 2);
		pushUDim2(L, a + b);
		return 1;
	}

	int UDim2Bridge::on_sub(lua_State* L)
	{
		const RBX::UDim2& a = getObject(L, 1);
		const RBX::UDim2& b = getObject(L, 2);
		pushUDim2(L, a - b);
		return 1;
	}

	int UDim2Bridge::on_unm(lua_State* L)
	{
		pushUDim2(L, -getObject(L, 1));
		return 1;
	}

	// VS2012 odr-use holders (openState names these at 0x18B9CB-0x18B9E7).
	int (*volatile g_bridgeUdimIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::UDim>::on_index;
	int (*volatile g_bridgeUdim2Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::UDim2>::on_index;

	// ---- Faces (section 225) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f318
	// GLOBAL: STUDIO 0x00f0b9a0
	// Bridge<RBX::Faces,1>::className
	const char* Bridge<RBX::Faces, true>::className = "Faces";

	// mac: 0x17023C (394B, batch365): index/newindex/gc/eq/tostring only.
	// No arithmetic (Faces has none in the bridge).
	// STUB: PLAYER 0x00880710
	// STUB: STUDIO 0x009e4d90
	void Bridge<RBX::Faces, true>::registerClass(lua_State* L,
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

	// mac: 0x16BE5C (443B, batch365). NormalId immediates match NormalId.h
	// (X=0 Y=1 Z=2 X_NEG=3 Y_NEG=4 Z_NEG=5). Failure text lists the valid
	// members (unlike UDim's plain form).
	// FUNCTION: PLAYER 0x00882160
	// FUNCTION: STUDIO 0x0092c200
	template<>
	int Bridge<RBX::Faces, true>::on_index(const RBX::Faces& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "Top") == 0)
		{
			lua_pushboolean(L, object.getNormalId(RBX::NORM_Y));
			return 1;
		}
		if (strcmp(name, "Bottom") == 0)
		{
			lua_pushboolean(L, object.getNormalId(RBX::NORM_Y_NEG));
			return 1;
		}
		if (strcmp(name, "Back") == 0)
		{
			lua_pushboolean(L, object.getNormalId(RBX::NORM_Z));
			return 1;
		}
		if (strcmp(name, "Front") == 0)
		{
			lua_pushboolean(L, object.getNormalId(RBX::NORM_Z_NEG));
			return 1;
		}
		if (strcmp(name, "Right") == 0)
		{
			lua_pushboolean(L, object.getNormalId(RBX::NORM_X));
			return 1;
		}
		if (strcmp(name, "Left") == 0)
		{
			lua_pushboolean(L, object.getNormalId(RBX::NORM_X_NEG));
			return 1;
		}
		if (name && !isupper(name[0]))
			throw RBX::runtime_error(
				"%s is not a valid member, did you forget to capitalize the first letter?", name);
		throw RBX::runtime_error(
			"%s is not a valid member. Valid members are Top,Bottom,Left,Right,Back,Front", name);
	}

	// mac: 0x16C018 (102B, batch365): always throws, with the name.
	template<>
	void Bridge<RBX::Faces, true>::on_newindex(RBX::Faces& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198AAA (96B, batch365): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671c40
	// FUNCTION: STUDIO 0x007d9ba0
	template<>
	int Bridge<RBX::Faces, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x1703C6 (50B, batch365): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::Faces, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x1703F8 (105B, batch365): dword mask compare. Faces::operator==
	// is exactly this; reuse it.
	template<>
	int Bridge<RBX::Faces, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x170462 (67B, batch365): checkudata + forward.
	template<>
	int Bridge<RBX::Faces, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F846 (170B, batch365): StringConverter (EA-backed in
	// util/NormalId.cpp 0x20197C) + pushlstring.
	// FUNCTION: PLAYER 0x0087f4d0
	template<>
	int Bridge<RBX::Faces, true>::on_tostring(const RBX::Faces& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::Faces>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x16BD06 (251B, batch421): up-to-6 Enum.NormalId items ORed into a
	// mask. Non-Item args are SKIPPED (a failed getValue is not an
	// error); a mistyped Item throws. Extra args past 6 are ignored
	// (count = top<7 ? top : 6).
	// STUB: PLAYER 0x00887d80
	// STUB: STUDIO 0x00931d80
	int FacesBridge::newFaces(lua_State* L)
	{
		int count = lua_gettop(L);
		if (count > 6)
			count = 6;
		int mask = 0;
		for (int i = 1; i <= count; ++i)
		{
			EnumDescriptorItemPtr item = NULL;
			if (!Bridge<EnumDescriptorItemPtr, true>::getValue(L, i, item))
				continue;
			if (!enumItemIsA(item, typeid(RBX::NormalId), "N3RBX8NormalIdE"))
				throw RBX::runtime_error("Faces.new expects Enum.NormalId inputs");
			mask |= RBX::normalIdToMask((RBX::NormalId)item->value);
		}
		pushFaces(L, RBX::Faces(mask));
		return 1;
	}

	const luaL_reg FacesBridge::classLibrary[] = {
		{ "new", newFaces },
		{ NULL, NULL }
	};

	// mac: 0x16BE02 (90B, batch421): standard library-table registration.
	void FacesBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x163560 (178B, batch423): Faces userdata -> Variant (type +
	// placement_any). DELTA vs binary (documents a VS2012 wall): the
	// binary emits this as an explicit getValue<Variant> specialization,
	// which VS2012 rejects (C2768/C2244 -- member-template explicit
	// specialization of an implicitly-instantiated class). The primary
	// getValue<V> template takes the identical path -- metatable check,
	// then Variant::operator=(const Faces&) which sets singleton<Faces>
	// + the value copy -- so callers use the primary (live-covered in
	// parsetest 240; the getSingleton<Faces> spec lives in
	// v8tree/EnumProperty.cpp).
	// (No specialization body is emitted; nothing takes its address.)

	// VS2012 odr-use holders (openState names these at 0x18BA03).
	int (*volatile g_bridgeFacesIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::Faces>::on_index;

	// ---- Axes (section 226) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f31c
	// GLOBAL: STUDIO 0x00f0b9a4
	// Bridge<RBX::Axes,1>::className
	const char* Bridge<RBX::Axes, true>::className = "Axes";

	// mac: 0x16FD5C (394B, batch366): index/newindex/gc/eq/tostring only.
	// STUB: PLAYER 0x008807f0
	// STUB: STUDIO 0x009e4e70
	void Bridge<RBX::Axes, true>::registerClass(lua_State* L,
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

	// mac: 0x16C21A (589B, batch366). X/Y/Z go through getAxis (axis 0/1/2);
	// the face names through getAxisByNormalId. Failure text uses
	// lowercase "valid members" (Faces' uses uppercase "Valid").
	// FUNCTION: PLAYER 0x008823a0
	// FUNCTION: STUDIO 0x0092c470
	template<>
	int Bridge<RBX::Axes, true>::on_index(const RBX::Axes& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "X") == 0)
		{
			lua_pushboolean(L, object.getAxis(G3D::Vector3::X_AXIS));
			return 1;
		}
		if (strcmp(name, "Y") == 0)
		{
			lua_pushboolean(L, object.getAxis(G3D::Vector3::Y_AXIS));
			return 1;
		}
		if (strcmp(name, "Z") == 0)
		{
			lua_pushboolean(L, object.getAxis(G3D::Vector3::Z_AXIS));
			return 1;
		}
		if (strcmp(name, "Top") == 0)
		{
			lua_pushboolean(L, object.getAxisByNormalId(RBX::NORM_Y));
			return 1;
		}
		if (strcmp(name, "Bottom") == 0)
		{
			lua_pushboolean(L, object.getAxisByNormalId(RBX::NORM_Y_NEG));
			return 1;
		}
		if (strcmp(name, "Back") == 0)
		{
			lua_pushboolean(L, object.getAxisByNormalId(RBX::NORM_Z));
			return 1;
		}
		if (strcmp(name, "Front") == 0)
		{
			lua_pushboolean(L, object.getAxisByNormalId(RBX::NORM_Z_NEG));
			return 1;
		}
		if (strcmp(name, "Right") == 0)
		{
			lua_pushboolean(L, object.getAxisByNormalId(RBX::NORM_X));
			return 1;
		}
		if (strcmp(name, "Left") == 0)
		{
			lua_pushboolean(L, object.getAxisByNormalId(RBX::NORM_X_NEG));
			return 1;
		}
		if (name && !isupper(name[0]))
			throw RBX::runtime_error(
				"%s is not a valid member, did you forget to capitalize the first letter?", name);
		throw RBX::runtime_error(
			"%s is not a valid member, valid members are X,Y,Z,Top,Bottom,Left,Right,Front,Back", name);
	}

	// mac: 0x16C468 (102B, batch366): always throws, with the name.
	template<>
	void Bridge<RBX::Axes, true>::on_newindex(RBX::Axes& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198B6C (96B, batch366): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671ca0
	// FUNCTION: STUDIO 0x007d9c00
	template<>
	int Bridge<RBX::Axes, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x16FEE6 (48B, batch366): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::Axes, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16FF18 (eq; batch366): mask dword compare. Axes::operator== is
	// exactly this; reuse it.
	template<>
	int Bridge<RBX::Axes, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x16F8F0 (170B, batch366): StringConverter (EA-backed in
	// util/NormalId.cpp 0x1E7FEA; single home -- EnumProperty only calls)
	// + pushlstring.
	// FUNCTION: PLAYER 0x0087f550
	// FUNCTION: STUDIO 0x009e3bd0
	template<>
	int Bridge<RBX::Axes, true>::on_tostring(const RBX::Axes& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::Axes>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x16FF82 (67B, batch366): checkudata + forward.
	// FUNCTION: PLAYER 0x0087ff50
	// FUNCTION: STUDIO 0x009e45d0
	template<>
	int Bridge<RBX::Axes, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16C07E (321B, batch422): up-to-6 Enum.Axis-or-NormalId items ORed
	// into a mask. Axis values pass straight through; NormalId values
	// convert via normalIdToAxis. Non-Items skipped; mistyped Items throw.
	// STUB: PLAYER 0x00887e70
	// STUB: STUDIO 0x00931e70
	int AxesBridge::newAxes(lua_State* L)
	{
		int count = lua_gettop(L);
		if (count > 6)
			count = 6;
		int mask = 0;
		for (int i = 1; i <= count; ++i)
		{
			EnumDescriptorItemPtr item = NULL;
			if (!Bridge<EnumDescriptorItemPtr, true>::getValue(L, i, item))
				continue;
			G3D::Vector3::Axis axis;
			if (enumItemIsA(item, typeid(G3D::Vector3::Axis), "N3G3D7Vector34AxisE"))
				axis = (G3D::Vector3::Axis)item->value;
			else if (enumItemIsA(item, typeid(RBX::NormalId), "N3RBX8NormalIdE"))
				axis = RBX::Axes::normalIdToAxis((RBX::NormalId)item->value);
			else
				throw RBX::runtime_error("Axes.new expects Enum.Axis or Enum.NormalId inputs");
			mask |= RBX::Axes::axisToMask(axis);
		}
		pushAxes(L, RBX::Axes(mask));
		return 1;
	}

	const luaL_reg AxesBridge::classLibrary[] = {
		{ "new", newAxes },
		{ NULL, NULL }
	};

	// mac: 0x16C1C0 (90B, batch422): standard library-table registration.
	void AxesBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x163612 (178B, batch423): Axes userdata -> Variant. Same VS2012
	// DELTA as the Faces note above: callers use the primary getValue<V>
	// template (identical path; getSingleton<Axes> lives in
	// v8tree/EnumProperty.cpp).

	// VS2012 odr-use holders (openState names these at 0x18BA1F).
	int (*volatile g_bridgeAxesIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::Axes>::on_index;

	// ---- CellID (section 227) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f320
	// GLOBAL: STUDIO 0x00f0b9a8
	// Bridge<RBX::CellID,1>::className
	const char* Bridge<RBX::CellID, true>::className = "CellId";

	// mac: 0x16FFC6 (394B, batch367): index/newindex/gc/eq/tostring only.
	// STUB: PLAYER 0x008808d0
	// STUB: STUDIO 0x009e4f50
	void Bridge<RBX::CellID, true>::registerClass(lua_State* L,
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

	// mac: 0x16C67E (322B, batch367). TerrainPart goes through
	// SharedPtrBridge<Instance>::push (by-value shared_ptr: same
	// addref/push/release dance as the binary's manual copy).
	// STUB: PLAYER 0x008892b0
	// STUB: STUDIO 0x00933180
	template<>
	int Bridge<RBX::CellID, true>::on_index(const RBX::CellID& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "IsNil") == 0)
		{
			lua_pushboolean(L, object.getIsNil());
			return 1;
		}
		if (strcmp(name, "Location") == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L, object.getLocation());
			return 1;
		}
		if (strcmp(name, "TerrainPart") == 0)
		{
			SharedPtrBridge<Instance>::push(L, object.getTerrainPart());
			return 1;
		}
		throw RBX::runtime_error(
			"%s is not a valid member, valid members are IsNil,Location,TerrainPart", name);
	}

	// mac: 0x16C7C0 (102B, batch367): always throws, with the name.
	template<>
	void Bridge<RBX::CellID, true>::on_newindex(RBX::CellID& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198C2E (96B, batch367): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671d60
	// FUNCTION: STUDIO 0x007d9cc0
	template<>
	int Bridge<RBX::CellID, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x17018A (110B, batch367): CellID::operator== (carried in the
	// CellID import); reuse it.
	template<>
	int Bridge<RBX::CellID, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x1701F8 (67B, batch367): checkudata + forward.
	template<>
	int Bridge<RBX::CellID, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F99A (170B, batch367): StringConverter (imported in
	// util/CellID.cpp; 2016 text adapted to G3D::Vector3) + pushlstring.
	template<>
	int Bridge<RBX::CellID, true>::on_tostring(const RBX::CellID& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::CellID>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// VS2012 odr-use holders (openState names these at 0x18BA3B).
	int (*volatile g_bridgeCellIDIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::CellID>::on_index;

	// ---- InputObject (section 228) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f330
	// GLOBAL: STUDIO 0x00f0b9b8
	// Bridge<RBX::InputObject,1>::className
	const char* Bridge<RBX::InputObject, true>::className = "InputObject";

	// Nested-enum + KeyCode classNames (batch371 rodata; metatable
	// registration for these three queues behind the enum/keycode bridge
	// eras -- pushes land as bare userdata until then, exactly like the
	// binary before its own registrations... see on_index note).
	template<>
	// GLOBAL: PLAYER 0x00d8f328
	// GLOBAL: STUDIO 0x00f0b9b0
	// Bridge<RBX::InputObject::UserInputType,1>::className
	const char* Bridge<RBX::InputObject::UserInputType, true>::className = "UserInputType";
	template<>
	// GLOBAL: PLAYER 0x00d8f324
	// GLOBAL: STUDIO 0x00f0b9ac
	// Bridge<RBX::InputObject::UserInputState,1>::className
	const char* Bridge<RBX::InputObject::UserInputState, true>::className = "UserInputState";
	template<>
	// GLOBAL: PLAYER 0x00d8f32c
	// GLOBAL: STUDIO 0x00f0b9b4
	// Bridge<RBX::KeyCode,1>::className
	const char* Bridge<RBX::KeyCode, true>::className = "KeyCode";

	// mac: 0x16FAEE (394B, batch370): index/newindex/gc/eq/tostring only.
	// STUB: PLAYER 0x008809b0
	// STUB: STUDIO 0x009e5030
	void Bridge<RBX::InputObject, true>::registerClass(lua_State* L,
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

	// mac: 0x16CA6A (304B, batch370). Members pushed by value through their
	// own Bridges (nested enums + KeyCode land as bare userdata until
	// their registerClass calls land -- pushNewObject with an
	// unregistered metatable name leaves the userdata bare, no crash).
	// FUNCTION: PLAYER 0x00887fc0
	// FUNCTION: STUDIO 0x00931fc0
	template<>
	int Bridge<RBX::InputObject, true>::on_index(const RBX::InputObject& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "UserInputType") == 0)
		{
			Bridge<InputObject::UserInputType>::pushNewObject(L, object.inputType);
			return 1;
		}
		if (strcmp(name, "UserInputState") == 0)
		{
			Bridge<InputObject::UserInputState>::pushNewObject(L, object.inputState);
			return 1;
		}
		if (strcmp(name, "ScreenPosition") == 0)
		{
			Bridge<G3D::Vector2>::pushNewObject(L, object.screenPosition);
			return 1;
		}
		if (strcmp(name, "KeyCode") == 0)
		{
			Bridge<RBX::KeyCode>::pushNewObject(L, object.keyCode);
			return 1;
		}
		throw RBX::runtime_error(
			"%s is not a valid member, valid members are UserInputType, UserInputState, ScreenPosition, KeyCode", name);
	}

	// mac: 0x16CB9A (102B, batch370): always throws, with the name.
	template<>
	void Bridge<RBX::InputObject, true>::on_newindex(RBX::InputObject& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198CF0 (96B, batch370): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671dc0
	// FUNCTION: STUDIO 0x007d9d20
	template<>
	int Bridge<RBX::InputObject, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x16FC78 (50B, batch370): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::InputObject, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16FCAA (110B, batch370): InputObject::operator== (member-wise
	// below); reuse it.
	template<>
	int Bridge<RBX::InputObject, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x16FD18 (67B, batch370): checkudata + forward.
	template<>
	int Bridge<RBX::InputObject, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16FA44-via-0x6F05F8 (batches 370-371): EnumDesc pair + Vector2 +
	// int composition (ported in v8datamodel/InputObject.cpp) + pushlstring.
	template<>
	int Bridge<RBX::InputObject, true>::on_tostring(const RBX::InputObject& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::InputObject>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x16C826 (490B, batch423): (type, state, [pos], [keycode]). A failed
	// arg-1/2 getValue is NOT an error (type/state default to 0) -- only a
	// SUCCESSFUL read runs the owner-type check. The Vector2 arg is strict
	// checkudata; keycode is lua_tointeger. The binary memcpys the 20B
	// struct straight into the userdata (pointer overload, batch424); the
	// by-value push below copies the identical bits through the copy ctor.
	// STUB: PLAYER 0x00888ef0
	// STUB: STUDIO 0x00932dc0
	int InputObjectBridge::newInputObject(lua_State* L)
	{
		int count = lua_gettop(L);
		if (count > 4)
			count = 4;
		if (count <= 1)
			throw RBX::runtime_error("InputObject.new expects at least two args! (Enum.UserInputType, Enum.UserInputState)");
		int type = 0;
		EnumDescriptorItemPtr item = NULL;
		if (Bridge<EnumDescriptorItemPtr, true>::getValue(L, 1, item))
		{
			if (!enumItemIsA(item, typeid(RBX::InputObject::UserInputType), "N3RBX11InputObject13UserInputTypeE"))
				throw RBX::runtime_error("InputObject.new expects Enum.UserInputType as first arg");
			type = item->value;
		}
		int state = 0;
		item = NULL;
		if (Bridge<EnumDescriptorItemPtr, true>::getValue(L, 2, item))
		{
			if (!enumItemIsA(item, typeid(RBX::InputObject::UserInputState), "N3RBX11InputObject14UserInputStateE"))
				throw RBX::runtime_error("InputObject.new expects Enum.UserInputState as second arg");
			state = item->value;
		}
		G3D::Vector2 pos(0, 0);
		int keyCode = 0;
		if (count >= 3)
		{
			pos = Bridge<G3D::Vector2, true>::getObject(L, 3);
			if (count >= 4)
				keyCode = lua_tointeger(L, 4);
		}
		pushInputObject(L, RBX::InputObject((RBX::InputObject::UserInputType)type,
			(RBX::InputObject::UserInputState)state, pos, (RBX::KeyCode)keyCode));
		return 1;
	}

	const luaL_reg InputObjectBridge::classLibrary[] = {
		{ "new", newInputObject },
		{ NULL, NULL }
	};

	// mac: 0x16CA10 (90B, batch423): standard library-table registration.
	void InputObjectBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x163776 (178B, batch423): InputObject userdata -> Variant. Same
	// VS2012 DELTA as the Faces note above: callers use the primary
	// getValue<V> template (identical path); the getSingleton<InputObject>
	// spec exists for it.

	// VS2012 odr-use holders (openState names these at 0x18BA57).
	int (*volatile g_bridgeInputObjectIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::InputObject>::on_index;

	// ---- Color3 (section 229) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f2e8
	// GLOBAL: STUDIO 0x00f0b970
	// Bridge<G3D::Color3,1>::className
	const char* Bridge<G3D::Color3, true>::className = "Color3";

	// mac: 0x170EE6 (394B, batch373): index/newindex/gc/eq/tostring only.
	// STUB: PLAYER 0x00880470
	// STUB: STUDIO 0x009e4af0
	void Bridge<G3D::Color3, true>::registerClass(lua_State* L,
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

	// pushColor3 lives inline in the header (like pushVector3); the
	// binary's out-of-line 0x166236 is the same one-liner. No cpp body
	// (would be C2084 against the inline definition).

	// mac: 0x166274 (229B, batch373): r/g/b lowercase-only, plain failure.
	template<>
	int Bridge<G3D::Color3, true>::on_index(const G3D::Color3& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "r") == 0)
		{
			lua_pushnumber(L, object.r);
			return 1;
		}
		if (strcmp(name, "g") == 0)
		{
			lua_pushnumber(L, object.g);
			return 1;
		}
		if (strcmp(name, "b") == 0)
		{
			lua_pushnumber(L, object.b);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x16635A (102B, batch373): always throws, with the name.
	template<>
	void Bridge<G3D::Color3, true>::on_newindex(G3D::Color3& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x1987A2 (96B, batch373): luaL_checklstring wrapper verbatim.
	template<>
	int Bridge<G3D::Color3, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x198802 (98B, batch373): same verbatim.
	template<>
	int Bridge<G3D::Color3, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x171070 (50B, batch373): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<G3D::Color3, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x1710A2 (142B, batch373): 3-lane NaN-aware == (plain && chain is
	// exactly equivalent).
	template<>
	int Bridge<G3D::Color3, true>::on_eq(lua_State* L)
	{
		const G3D::Color3& a = getObject(L, 1);
		const G3D::Color3& b = getObject(L, 2);
		lua_pushboolean(L, a.r == b.r && a.g == b.g && a.b == b.b);
		return 1;
	}

	// mac: 0x171130 (67B, batch373): checkudata + forward.
	template<>
	int Bridge<G3D::Color3, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F2F6 (170B, batch373): StringConverter (already ported in
	// v8tree/EnumProperty.cpp) + pushlstring.
	// FUNCTION: PLAYER 0x0087f0d0
	// FUNCTION: STUDIO 0x009e3750
	template<>
	int Bridge<G3D::Color3, true>::on_tostring(const G3D::Color3& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<G3D::Color3>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// VS2012 odr-use holders (openState names these at 0x18B993).
	int (*volatile g_bridgeColor3Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Color3>::on_index;
	int (*volatile g_bridgeColor3NewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Color3>::on_newindex;

	// ---- BrickColor (section 230) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f308
	// GLOBAL: STUDIO 0x00f0b990
	// Bridge<RBX::BrickColor,1>::className
	const char* Bridge<RBX::BrickColor, true>::className = "BrickColor";

	// mac: 0x1704A6 (394B, batch375): index/newindex/gc/eq/tostring only.
	// STUB: PLAYER 0x00880630
	// STUB: STUDIO 0x009e4cb0
	void Bridge<RBX::BrickColor, true>::registerClass(lua_State* L,
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

	// mac: 0x1696AA (526B, batch375): number/Number (int@0), Color (Color3
	// push via the Color3 bridge -- register it first), r/g/b (via
	// color3()), name/Name (pushlstring), plain failure. Matches 2016
	// member-for-member.
	// STUB: PLAYER 0x00885fa0
	// STUB: STUDIO 0x0092ffa0
	template<>
	int Bridge<RBX::BrickColor, true>::on_index(const RBX::BrickColor& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "number") == 0 || strcmp(name, "Number") == 0)
		{
			lua_pushinteger(L, object.asInt());
			return 1;
		}
		if (strcmp(name, "Color") == 0)
		{
			Bridge<G3D::Color3>::pushNewObject(L, object.color3());
			return 1;
		}
		if (strcmp(name, "r") == 0)
		{
			lua_pushnumber(L, object.color3().r);
			return 1;
		}
		if (strcmp(name, "g") == 0)
		{
			lua_pushnumber(L, object.color3().g);
			return 1;
		}
		if (strcmp(name, "b") == 0)
		{
			lua_pushnumber(L, object.color3().b);
			return 1;
		}
		if (strcmp(name, "name") == 0 || strcmp(name, "Name") == 0)
		{
			const std::string& n = object.name();
			lua_pushlstring(L, n.c_str(), n.size());
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x1698B8 (102B, batch375): always throws, with the name.
	template<>
	void Bridge<RBX::BrickColor, true>::on_newindex(RBX::BrickColor& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198864 (96B, batch375): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671d00
	// FUNCTION: STUDIO 0x007d9c60
	template<>
	int Bridge<RBX::BrickColor, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x1988C4 (98B, batch375): same verbatim.
	template<>
	int Bridge<RBX::BrickColor, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x170630 (50B, batch375): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::BrickColor, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x170662 (eq; batch375): number dword compare. BrickColor::
	// operator== is exactly this; reuse it.
	template<>
	int Bridge<RBX::BrickColor, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x1706CC (67B, batch375): checkudata + forward.
	template<>
	int Bridge<RBX::BrickColor, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F6F2 (170B, batch375): StringConverter (already ported in
	// v8tree/EnumProperty.cpp) + pushlstring.
	template<>
	int Bridge<RBX::BrickColor, true>::on_tostring(const RBX::BrickColor& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::BrickColor>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x1692A8 (450B, batch403). count==1: number/string/Color3 arms;
	// count==0: pushes BrickColor(194) dead FIRST, then falls through to
	// the float path (double push, return 1); count>=2: up-to-3 floats +
	// alpha 1.0 -> closest(Color4) -> push.
	int BrickColorBridge::newBrickColor(lua_State* L)
	{
		int count = std::min(3, lua_gettop(L));
		if (count == 1)
		{
			if (lua_isnumber(L, 1) != 0)
				pushBrickColor(L, RBX::BrickColor(lua_tointeger(L, 1)));
			else if (lua_isstring(L, 1) != 0)
				pushBrickColor(L, RBX::BrickColor::parse(lua_tostring(L, 1)));
			else
				pushBrickColor(L, RBX::BrickColor::closest(
					Bridge<G3D::Color3>::getObject(L, 1)));
			return 1;
		}
		if (count == 0)
			pushBrickColor(L, RBX::BrickColor(194));
		float rgb[3] = { 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < count; ++i)
			rgb[i] = lua_tofloat(L, i + 1);
		pushBrickColor(L, RBX::BrickColor::closest(
			G3D::Color4(rgb[0], rgb[1], rgb[2], 1.0f)));
		return 1;
	}

	// mac: 0x16946A (42B, batch403): push(random()).
	int BrickColorBridge::randomBrickColor(lua_State* L)
	{
		pushBrickColor(L, RBX::BrickColor::random());
		return 1;
	}

	// mac: 0x169494 (206B, batch403): index = top>0 ? tointeger(1) : 0;
	// throw "palette index out of bounds (%d)" when negative or past the
	// end; push(palette.at(index)).
	// STUB: PLAYER 0x00889200
	// STUB: STUDIO 0x009330d0
	int BrickColorBridge::paletteBrickColor(lua_State* L)
	{
		int index = 0;
		if (lua_gettop(L) > 0)
			index = lua_tointeger(L, 1);
		const RBX::BrickColor::Colors& palette = RBX::BrickColor::colorPalette();
		if (index < 0 || index >= (int)palette.size())
			throw RBX::runtime_error("palette index out of bounds (%d)", index);
		pushBrickColor(L, palette.at(index));
		return 1;
	}

	// Color thunks (batch404: pushBrickColor(constant); White=1 Gray=194
	// DarkGray=199 Black=26 Red=21 Yellow=24 Green=28 Blue=23).
	static int pushWhiteBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(1));
		return 1;
	}
	static int pushGrayBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(194));
		return 1;
	}
	static int pushDarkGrayBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(199));
		return 1;
	}
	static int pushBlackBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(26));
		return 1;
	}
	static int pushRedBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(21));
		return 1;
	}
	static int pushYellowBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(24));
		return 1;
	}
	static int pushGreenBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(28));
		return 1;
	}
	static int pushBlueBrickColor(lua_State* L)
	{
		BrickColorBridge::pushBrickColor(L, RBX::BrickColor(23));
		return 1;
	}

	const luaL_reg BrickColorBridge::classLibrary[] = {
		{ "new", newBrickColor },
		{ "random", randomBrickColor },
		{ "palette", paletteBrickColor },
		{ "New", newBrickColor },
		{ "Random", randomBrickColor },
		{ "White", pushWhiteBrickColor },
		{ "Gray", pushGrayBrickColor },
		{ "DarkGray", pushDarkGrayBrickColor },
		{ "Black", pushBlackBrickColor },
		{ "Red", pushRedBrickColor },
		{ "Yellow", pushYellowBrickColor },
		{ "Green", pushGreenBrickColor },
		{ "Blue", pushBlueBrickColor },
		{ NULL, NULL }
	};

	void BrickColorBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x16C4CE (341B, batch403): count=min(4,top); isNil = count>0 ?
	// toboolean(L,0) [index ZERO verbatim -- same quirk as 2016]; floats
	// for i in 1..count-1 into v[i-1] (missing zero); fromParameters +
	// push + local dtor.
	int CellIDBridge::newCellID(lua_State* L)
	{
		int count = std::min(4, lua_gettop(L));
		bool isNil = (count > 0) ? (lua_toboolean(L, 0) != 0) : false;
		float v[3] = { 0.0f, 0.0f, 0.0f };
		for (int i = 1; i < count; ++i)
			v[i - 1] = lua_tofloat(L, i);
		pushCellID(L, RBX::CellID::fromParameters(isNil, v,
			boost::shared_ptr<RBX::Instance>()));
		return 1;
	}

	const luaL_reg CellIDBridge::classLibrary[] = {
		{ "new", newCellID },
		{ NULL, NULL }
	};

	void CellIDBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// VS2012 odr-use holders (openState names these at 0x18B9AF).
	int (*volatile g_bridgeBrickColorIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::BrickColor>::on_index;
	int (*volatile g_bridgeBrickColorNewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::BrickColor>::on_newindex;

	// ---- CoordinateFrame (section 231) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f30c
	// GLOBAL: STUDIO 0x00f0b994
	// Bridge<G3D::CoordinateFrame,1>::className
	const char* Bridge<G3D::CoordinateFrame, true>::className = "CFrame";

	// mac: 0x16D6D0 (632B, batch377): index/newindex/gc/eq/tostring/add/sub/
	// mul + named "inverse" entry (NOT __unm -- body-proven DELTA).
	// STUB: PLAYER 0x008890a0
	// STUB: STUDIO 0x00932f70
	void Bridge<G3D::CoordinateFrame, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, CoordinateFrameBridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, CoordinateFrameBridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, CoordinateFrameBridge::on_mul);
		lua_settable(L, -3);
		lua_pushstring(L, "inverse");
		lua_pushcfunction(L, CoordinateFrameBridge::on_inverse);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x169F2A (217B, batch377): CFrame + Vector3 (translation add,
	// rotation copied). Matches G3D operator+(CFrame, Vector3).
	int CoordinateFrameBridge::on_add(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, 2);
		pushCoordinateFrame(L, a + b);
		return 1;
	}

	// mac: 0x16A004 (217B, batch377): same with subtract.
	int CoordinateFrameBridge::on_sub(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, 2);
		pushCoordinateFrame(L, a - b);
		return 1;
	}

	// mac: 0x16A0DE (451B, batch377). CFrame*CFrame via operator* (called
	// directly by the binary); else CFrame*Vector3 = pointToWorldSpace
	// (binary does the R*v+t lanes explicitly -- bitwise identical to the
	// G3D call for finite floats since FP add commutes).
	// STUB: PLAYER 0x00888c70
	// STUB: STUDIO 0x00932b40
	int CoordinateFrameBridge::on_mul(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		G3D::CoordinateFrame b;
		if (getValue(L, 2, b))
		{
			pushCoordinateFrame(L, a * b);
			return 1;
		}
		const G3D::Vector3& v = Bridge<G3D::Vector3>::getObject(L, 2);
		Bridge<G3D::Vector3>::pushNewObject(L, a.pointToWorldSpace(v));
		return 1;
	}

	// mac: 0x16A2A2 (345B, batch377): rotation^T + -(R^T t). Matches
	// G3D::CoordinateFrame::inverse() (binary inlines it).
	int CoordinateFrameBridge::on_inverse(lua_State* L)
	{
		pushCoordinateFrame(L, getObject(L, 1).inverse());
		return 1;
	}

	// Space helpers (batch378): self is arg 1 (colon-call convention;
	// the upvalue closure carries the same self). Count-loop shape with
	// count==0 fast paths, verbatim vs 2016.
	int CoordinateFrameBridge::on_toWorldSpace(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		int count = lua_gettop(L) - 1;
		if (count == 0)
		{
			pushCoordinateFrame(L, a);
			return 1;
		}
		for (int i = 0; i < count; ++i)
		{
			const G3D::CoordinateFrame& b = getObject(L, i + 2);
			pushCoordinateFrame(L, a * b);
		}
		return count;
	}

	int CoordinateFrameBridge::on_toObjectSpace(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		int count = lua_gettop(L) - 1;
		if (count == 0)
		{
			pushCoordinateFrame(L, a.inverse());
			return 1;
		}
		for (int i = 0; i < count; ++i)
		{
			const G3D::CoordinateFrame& b = getObject(L, i + 2);
			pushCoordinateFrame(L, a.toObjectSpace(b));
		}
		return count;
	}

	int CoordinateFrameBridge::on_pointToWorldSpace(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		int count = lua_gettop(L) - 1;
		if (count == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L,
				a.pointToWorldSpace(G3D::Vector3::zero()));
			return 1;
		}
		for (int i = 0; i < count; ++i)
		{
			const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, i + 2);
			Bridge<G3D::Vector3>::pushNewObject(L, a.pointToWorldSpace(b));
		}
		return count;
	}

	int CoordinateFrameBridge::on_pointToObjectSpace(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		int count = lua_gettop(L) - 1;
		if (count == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L,
				a.pointToObjectSpace(G3D::Vector3::zero()));
			return 1;
		}
		for (int i = 0; i < count; ++i)
		{
			const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, i + 2);
			Bridge<G3D::Vector3>::pushNewObject(L, a.pointToObjectSpace(b));
		}
		return count;
	}

	int CoordinateFrameBridge::on_vectorToWorldSpace(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		int count = lua_gettop(L) - 1;
		if (count == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L,
				a.vectorToWorldSpace(G3D::Vector3::zero()));
			return 1;
		}
		for (int i = 0; i < count; ++i)
		{
			const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, i + 2);
			Bridge<G3D::Vector3>::pushNewObject(L, a.vectorToWorldSpace(b));
		}
		return count;
	}

	int CoordinateFrameBridge::on_vectorToObjectSpace(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		int count = lua_gettop(L) - 1;
		if (count == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L,
				a.vectorToObjectSpace(G3D::Vector3::zero()));
			return 1;
		}
		for (int i = 0; i < count; ++i)
		{
			const G3D::Vector3& b = Bridge<G3D::Vector3>::getObject(L, i + 2);
			Bridge<G3D::Vector3>::pushNewObject(L, a.vectorToObjectSpace(b));
		}
		return count;
	}

	// mac: 0x16AD9E (173B, batch378): translation xyz then rotation rows.
	// FUNCTION: PLAYER 0x00886d90
	// FUNCTION: STUDIO 0x00930d90
	int CoordinateFrameBridge::on_components(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		lua_pushnumber(L, a.translation.x);
		lua_pushnumber(L, a.translation.y);
		lua_pushnumber(L, a.translation.z);
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				lua_pushnumber(L, a.rotation[i][j]);
		return 12;
	}

	// mac: 0x16AE4C (139B, batch378): Matrix3::toEulerAnglesXYZ.
	// FUNCTION: PLAYER 0x00886e20
	// FUNCTION: STUDIO 0x00930e20
	int CoordinateFrameBridge::on_toEulerAnglesXYZ(lua_State* L)
	{
		const G3D::CoordinateFrame& a = getObject(L, 1);
		float x, y, z;
		a.rotation.toEulerAnglesXYZ(x, y, z);
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		lua_pushnumber(L, z);
		return 3;
	}

	// mac: 0x16B044 (971B, batch377). lookVector = -rotation.column(2);
	// methods bind as 1-upvalue closures over self; x/X y/Y z/Z read the
	// translation lanes; NO lerp member (unlike 2016's static).
	// STUB: PLAYER 0x008871a0
	// STUB: STUDIO 0x009311a0
	template<>
	int Bridge<G3D::CoordinateFrame, true>::on_index(const G3D::CoordinateFrame& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "p") == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L, object.translation);
			return 1;
		}
		if (strcmp(name, "lookVector") == 0)
		{
			G3D::Vector3 col = object.rotation.column(2);
			Bridge<G3D::Vector3>::pushNewObject(L, G3D::Vector3(-col.x, -col.y, -col.z));
			return 1;
		}
#define CF_METHOD(m) \
		if (strcmp(name, #m) == 0) \
		{ \
			lua_pushvalue(L, -1); \
			lua_pushcclosure(L, CoordinateFrameBridge::on_##m, 1); \
			return 1; \
		}
		CF_METHOD(inverse)
		CF_METHOD(toWorldSpace)
		CF_METHOD(toObjectSpace)
		CF_METHOD(pointToWorldSpace)
		CF_METHOD(pointToObjectSpace)
		CF_METHOD(vectorToWorldSpace)
		CF_METHOD(vectorToObjectSpace)
		CF_METHOD(toEulerAnglesXYZ)
		CF_METHOD(components)
#undef CF_METHOD
		if (strcmp(name, "x") == 0 || strcmp(name, "X") == 0)
		{
			lua_pushnumber(L, object.translation.x);
			return 1;
		}
		if (strcmp(name, "y") == 0 || strcmp(name, "Y") == 0)
		{
			lua_pushnumber(L, object.translation.y);
			return 1;
		}
		if (strcmp(name, "z") == 0 || strcmp(name, "Z") == 0)
		{
			lua_pushnumber(L, object.translation.z);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x16B410 (batch377: 102B-pattern newindex, always throws).
	template<>
	void Bridge<G3D::CoordinateFrame, true>::on_newindex(G3D::CoordinateFrame& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198192 (96B, batch378): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671820
	// FUNCTION: STUDIO 0x007d9780
	template<>
	int Bridge<G3D::CoordinateFrame, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x16ECFE (48B, batch378): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<G3D::CoordinateFrame, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16ED2E (155B, batch378): translation lanes == plus
	// Matrix3::operator==. CoordinateFrame::operator== is exactly this;
	// reuse it.
	template<>
	int Bridge<G3D::CoordinateFrame, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x16EDCA (60B, batch378): checkudata + forward.
	template<>
	int Bridge<G3D::CoordinateFrame, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F79C (170B, batch378): StringConverter (already ported in
	// v8tree/EnumProperty.cpp) + pushlstring.
	template<>
	int Bridge<G3D::CoordinateFrame, true>::on_tostring(const G3D::CoordinateFrame& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<G3D::CoordinateFrame>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// VS2012 odr-use holders (openState names these at 0x18B8B3).
	int (*volatile g_bridgeCFrameIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::CoordinateFrame>::on_index;

	// ---- Region3 / Region3int16 (section 232) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f2f0
	// GLOBAL: STUDIO 0x00f0b978
	// Bridge<RBX::Region3,1>::className
	const char* Bridge<RBX::Region3, true>::className = "Region3";
	template<>
	// GLOBAL: PLAYER 0x00d8f2f4
	// GLOBAL: STUDIO 0x00f0b97c
	// Bridge<RBX::Region3int16,1>::className
	const char* Bridge<RBX::Region3int16, true>::className = "Region3int16";
	// (Vector3int16 className lives with its full bridge in section 234
	// below; it was first defined here for the Region3int16 Min/Max link
	// and moved to avoid C2086.)

	// mac: 0x170986 (Region3) / 0x170C46 (Region3int16): index/newindex/gc/eq/
	// tostring only (both batches 380-381).
	// STUB: PLAYER 0x00880a90
	// STUB: STUDIO 0x009e5110
	void Bridge<RBX::Region3, true>::registerClass(lua_State* L,
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

	void Bridge<RBX::Region3int16, true>::registerClass(lua_State* L,
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

	// mac: 0x166A58 (240B, batch380): CFrame (full 48B copy) + Size (lanes
	// +48/+52/+56); plain failure. NO ExpandToGrid (2016-only).
	// STUB: PLAYER 0x00883780
	// STUB: STUDIO 0x0092d780
	template<>
	int Bridge<RBX::Region3, true>::on_index(const RBX::Region3& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "CFrame") == 0)
		{
			Bridge<G3D::CoordinateFrame>::pushNewObject(L, object.getCFrame());
			return 1;
		}
		if (strcmp(name, "Size") == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L, object.getSize());
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x166B48 (102B-pattern): always throws, with the name.
	template<>
	void Bridge<RBX::Region3, true>::on_newindex(RBX::Region3& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198254 (96B, batch380): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671880
	// FUNCTION: STUDIO 0x007d97e0
	template<>
	int Bridge<RBX::Region3, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}


	// mac: 0x170B10 (48B, batch380): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::Region3, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x170B42 (192B, batch380): size + translation lanes, then
	// Matrix3::operator==. Region3::operator== is exactly this; reuse it.
	template<>
	int Bridge<RBX::Region3, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x170C02 (60B-pattern): checkudata + forward.
	template<>
	int Bridge<RBX::Region3, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x172BF0 (170B-pattern): StringConverter (imported in
	// util/Region3.cpp) + pushlstring.
	template<>
	int Bridge<RBX::Region3, true>::on_tostring(const RBX::Region3& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::Region3>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x166D26 (252B, batch381): Min/Max via getMinPos/getMaxPos, pushed
	// through the Vector3int16 bridge (bare userdata until that bridge's
	// registerClass lands); plain failure.
	template<>
	int Bridge<RBX::Region3int16, true>::on_index(const RBX::Region3int16& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "Min") == 0)
		{
			Bridge<G3D::Vector3int16>::pushNewObject(L, object.getMinPos());
			return 1;
		}
		if (strcmp(name, "Max") == 0)
		{
			Bridge<G3D::Vector3int16>::pushNewObject(L, object.getMaxPos());
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x166E22 (102B-pattern): always throws, with the name.
	template<>
	void Bridge<RBX::Region3int16, true>::on_newindex(RBX::Region3int16& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x198316 (96B-pattern): luaL_checklstring wrapper verbatim.
	template<>
	int Bridge<RBX::Region3int16, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x198376 (98B-pattern): same verbatim.
	template<>
	int Bridge<RBX::Region3int16, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x170DD0 (48B-pattern): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<RBX::Region3int16, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x170E02 (159B, batch381): 6 int16 lanes. Region3int16::operator==
	// is exactly this; reuse it.
	template<>
	int Bridge<RBX::Region3int16, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x170EA2 (60B-pattern): checkudata + forward.
	template<>
	int Bridge<RBX::Region3int16, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x172B46 (170B-pattern): StringConverter (imported in
	// util/Region3int16.cpp) + pushlstring.
	template<>
	int Bridge<RBX::Region3int16, true>::on_tostring(const RBX::Region3int16& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::Region3int16>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// VS2012 odr-use holders (openState names these at 0x18B8CF/0x18B8EB).
	int (*volatile g_bridgeRegion3Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::Region3>::on_index;
	int (*volatile g_bridgeRegion3int16Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::Region3int16>::on_index;
	int (*volatile g_bridgeRegion3int16NewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::Region3int16>::on_newindex;

	// ---- RbxRay (section 233) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f2ec
	// GLOBAL: STUDIO 0x00f0b974
	// Bridge<RBX::RbxRay,1>::className
	const char* Bridge<RBX::RbxRay, true>::className = "Ray";

	// mac: 0x170710 (394B, batch387): index/newindex/gc/eq/tostring only.
	// STUB: PLAYER 0x00880550
	// STUB: STUDIO 0x009e4bd0
	void Bridge<RBX::RbxRay, true>::registerClass(lua_State* L,
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

	// File-static helpers (no binary names; EAs 0x166764/0x1667E9,
	// batch389). Pushed as 0-upvalue closures (unlike the CFrame
	// methods) -- call args start at index 1 with no self upvalue.
	static int closestPointVector3(lua_State* L)
	{
		const RBX::RbxRay& self = Bridge<RBX::RbxRay>::getObject(L, 1);
		const G3D::Vector3& point = Bridge<G3D::Vector3>::getObject(L, 2);
		Bridge<G3D::Vector3>::pushNewObject(L, self.closestPoint(point));
		return 1;
	}

	static int distanceVector3(lua_State* L)
	{
		const RBX::RbxRay& self = Bridge<RBX::RbxRay>::getObject(L, 1);
		const G3D::Vector3& point = Bridge<G3D::Vector3>::getObject(L, 2);
		lua_pushnumber(L, self.distance(point));
		return 1;
	}

	// mac: 0x166538 (556B, batch387). RbxRay carries a vtable (binary writes
	// vptr into the unit-copy); Origin @+4, Direction @+16. unit arm
	// pushes object.unit() (same bytes: vptr + origin + normalized dir).
	// STUB: PLAYER 0x00883480
	// STUB: STUDIO 0x0092d480
	template<>
	int Bridge<RBX::RbxRay, true>::on_index(const RBX::RbxRay& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "Origin") == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L, object.origin());
			return 1;
		}
		if (strcmp(name, "Direction") == 0)
		{
			Bridge<G3D::Vector3>::pushNewObject(L, object.direction());
			return 1;
		}
		if (strcmp(name, "unit") == 0 || strcmp(name, "Unit") == 0)
		{
			RbxRayBridge::pushRbxRay(L, object.unit());
			return 1;
		}
		if (strcmp(name, "ClosestPoint") == 0)
		{
			lua_pushcfunction(L, closestPointVector3);
			return 1;
		}
		if (strcmp(name, "Distance") == 0)
		{
			lua_pushcfunction(L, distanceVector3);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x166856 (102B-pattern): always throws, with the name.
	template<>
	void Bridge<RBX::RbxRay, true>::on_newindex(RBX::RbxRay& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x1986E0 (96B-pattern): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671a00
	// FUNCTION: STUDIO 0x007d9960
	template<>
	int Bridge<RBX::RbxRay, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x198740 (98B-pattern): same verbatim.
	template<>
	int Bridge<RBX::RbxRay, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x1708D4 (110B, batch387): RbxRay::operator== (header-inline);
	// reuse it.
	template<>
	int Bridge<RBX::RbxRay, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x170942 (67B-pattern): checkudata + forward.
	template<>
	int Bridge<RBX::RbxRay, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F3A0 (170B-pattern): converter + pushlstring.
	template<>
	int Bridge<RBX::RbxRay, true>::on_tostring(const RBX::RbxRay& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<RBX::RbxRay>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// VS2012 odr-use holders (openState names these at 0x18B977).
	int (*volatile g_bridgeRbxRayIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::RbxRay>::on_index;
	int (*volatile g_bridgeRbxRayNewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<RBX::RbxRay>::on_newindex;

	// ---- Vector3int16 / Vector2int16 (section 234) ----

	template<>
	// GLOBAL: PLAYER 0x00d8f2fc
	// GLOBAL: STUDIO 0x00f0b984
	// Bridge<G3D::Vector3int16,1>::className
	const char* Bridge<G3D::Vector3int16, true>::className = "Vector3int16";
	template<>
	// GLOBAL: PLAYER 0x00d8f300
	// GLOBAL: STUDIO 0x00f0b988
	// Bridge<G3D::Vector2int16,1>::className
	const char* Bridge<G3D::Vector2int16, true>::className = "Vector2int16";

	// mac: 0x16CC00 / 0x16CEB4: index/newindex/gc/eq/tostring/add/sub/mul/
	// div/unm + readonly + pop (both batches 394/396).
	// STUB: PLAYER 0x00888160
	// STUB: STUDIO 0x00932160
	void Bridge<G3D::Vector3int16, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, Vector3int16Bridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, Vector3int16Bridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, Vector3int16Bridge::on_mul);
		lua_settable(L, -3);
		lua_pushstring(L, "__div");
		lua_pushcfunction(L, Vector3int16Bridge::on_div);
		lua_settable(L, -3);
		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, Vector3int16Bridge::on_unm);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	void Bridge<G3D::Vector2int16, true>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, Vector2int16Bridge::on_add);
		lua_settable(L, -3);
		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, Vector2int16Bridge::on_sub);
		lua_settable(L, -3);
		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, Vector2int16Bridge::on_mul);
		lua_settable(L, -3);
		lua_pushstring(L, "__div");
		lua_pushcfunction(L, Vector2int16Bridge::on_div);
		lua_settable(L, -3);
		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, Vector2int16Bridge::on_unm);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x167E46/0x167ED6 (143B each, batch393): 16-bit wrapping lanes.
	int Vector3int16Bridge::on_add(lua_State* L)
	{
		const G3D::Vector3int16& a = getObject(L, 1);
		const G3D::Vector3int16& b = getObject(L, 2);
		pushVector3int16(L, a + b);
		return 1;
	}

	int Vector3int16Bridge::on_sub(lua_State* L)
	{
		const G3D::Vector3int16& a = getObject(L, 1);
		const G3D::Vector3int16& b = getObject(L, 2);
		pushVector3int16(L, a - b);
		return 1;
	}

	// mac: 0x167F66 (310B, batch393). No-throw shape; scalar converts via
	// (int)lua_tofloat (cvttss2si truncation); int lane products narrowed
	// to int16 (wrap). Matches G3D operator*(int) lane-for-lane.
	// FUNCTION: PLAYER 0x008849c0
	// FUNCTION: STUDIO 0x0092e9c0
	int Vector3int16Bridge::on_mul(lua_State* L)
	{
		G3D::Vector3int16 a;
		if (getValue(L, 1, a))
		{
			G3D::Vector3int16 b;
			if (getValue(L, 2, b))
				pushVector3int16(L, a * b);
			else
			{
				int c = (int)lua_tofloat(L, 2);
				pushVector3int16(L, G3D::Vector3int16(a.x * c, a.y * c, a.z * c));
			}
		}
		else
		{
			a = getObject(L, 2);
			int c = (int)lua_tofloat(L, 1);
			pushVector3int16(L, G3D::Vector3int16(c * a.x, c * a.y, c * a.z));
		}
		return 1;
	}

	// mac: 0x16809C (362B, batch393). DELTA vs 2016 (which throws on zero
	// divisors): the binary idivs directly (SIGFPE on zero, like the
	// binary). vec/vec: int idiv lanes; vec/scalar: (int)((float)lane /
	// c) float-div-truncate (NOT G3D integer division); scalar/vec:
	// lua_tointeger + idiv lanes.
	// STUB: PLAYER 0x00884b40
	// STUB: STUDIO 0x0092eb40
	int Vector3int16Bridge::on_div(lua_State* L)
	{
		G3D::Vector3int16 a;
		if (getValue(L, 1, a))
		{
			G3D::Vector3int16 b;
			if (getValue(L, 2, b))
				pushVector3int16(L, G3D::Vector3int16(a.x / b.x, a.y / b.y, a.z / b.z));
			else
			{
				float c = lua_tofloat(L, 2);
				pushVector3int16(L, G3D::Vector3int16(
					(G3D::int16)(int)((float)a.x / c),
					(G3D::int16)(int)((float)a.y / c),
					(G3D::int16)(int)((float)a.z / c)));
			}
		}
		else
		{
			a = getObject(L, 2);
			int c = lua_tointeger(L, 1);
			pushVector3int16(L, G3D::Vector3int16(c / a.x, c / a.y, c / a.z));
		}
		return 1;
	}

	// mac: 0x168206 (108B, batch393): 16-bit lane negate.
	int Vector3int16Bridge::on_unm(lua_State* L)
	{
		const G3D::Vector3int16& a = getObject(L, 1);
		pushVector3int16(L, G3D::Vector3int16(-a.x, -a.y, -a.z));
		return 1;
	}

	// mac: 0x168272 (281B, batch393): x/X y/Y z/Z via pushinteger (movsx);
	// plain failure.
	template<>
	int Bridge<G3D::Vector3int16, true>::on_index(const G3D::Vector3int16& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "x") == 0 || strcmp(name, "X") == 0)
		{
			lua_pushinteger(L, object.x);
			return 1;
		}
		if (strcmp(name, "y") == 0 || strcmp(name, "Y") == 0)
		{
			lua_pushinteger(L, object.y);
			return 1;
		}
		if (strcmp(name, "z") == 0 || strcmp(name, "Z") == 0)
		{
			lua_pushinteger(L, object.z);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x16838C (102B-pattern): always throws, with the name.
	template<>
	void Bridge<G3D::Vector3int16, true>::on_newindex(G3D::Vector3int16& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x1983D8 (96B-pattern): luaL_checklstring wrapper verbatim.
	template<>
	int Bridge<G3D::Vector3int16, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x198438 (98B-pattern): same verbatim.
	template<>
	int Bridge<G3D::Vector3int16, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x16E94A (48B-pattern): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<G3D::Vector3int16, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16E97A (131B, batch394): 3 int16 lanes (== operator==).
	template<>
	int Bridge<G3D::Vector3int16, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x16E9FE (60B-pattern): checkudata + forward.
	template<>
	int Bridge<G3D::Vector3int16, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F44A (170B-pattern): StringConverter (imported in
	// util/Region3int16.cpp) + pushlstring.
	template<>
	int Bridge<G3D::Vector3int16, true>::on_tostring(const G3D::Vector3int16& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<G3D::Vector3int16>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x168520/0x16859E (128B each, batch395): 16-bit wrapping lanes.
	int Vector2int16Bridge::on_add(lua_State* L)
	{
		const G3D::Vector2int16& a = getObject(L, 1);
		const G3D::Vector2int16& b = getObject(L, 2);
		pushVector2int16(L, a + b);
		return 1;
	}

	int Vector2int16Bridge::on_sub(lua_State* L)
	{
		const G3D::Vector2int16& a = getObject(L, 1);
		const G3D::Vector2int16& b = getObject(L, 2);
		pushVector2int16(L, a - b);
		return 1;
	}

	// mac: 0x16861C (336B, batch395): same no-throw int-cast-scalar shape as
	// Vector3int16 (scalar*vec arm builds via the Vector2int16(Vector2)
	// ctor in-binary; explicit lanes here are bitwise-identical for
	// finite inputs).
	// STUB: PLAYER 0x00884f30
	// STUB: STUDIO 0x0092ef30
	int Vector2int16Bridge::on_mul(lua_State* L)
	{
		G3D::Vector2int16 a;
		if (getValue(L, 1, a))
		{
			G3D::Vector2int16 b;
			if (getValue(L, 2, b))
				pushVector2int16(L, a * b);
			else
			{
				int c = (int)lua_tofloat(L, 2);
				pushVector2int16(L, G3D::Vector2int16(a.x * c, a.y * c));
			}
		}
		else
		{
			const G3D::Vector2int16& b = getObject(L, 2);
			int c = (int)lua_tofloat(L, 1);
			pushVector2int16(L, G3D::Vector2int16(c * b.x, c * b.y));
		}
		return 1;
	}

	// mac: 0x16876C (337B, batch395): vec/vec idiv; vec/scalar
	// float-div-truncate; scalar/vec lua_tointeger + idiv. No throws.
	// STUB: PLAYER 0x008850b0
	// STUB: STUDIO 0x0092f0b0
	int Vector2int16Bridge::on_div(lua_State* L)
	{
		G3D::Vector2int16 a;
		if (getValue(L, 1, a))
		{
			G3D::Vector2int16 b;
			if (getValue(L, 2, b))
				pushVector2int16(L, G3D::Vector2int16(a.x / b.x, a.y / b.y));
			else
			{
				float c = lua_tofloat(L, 2);
				pushVector2int16(L, G3D::Vector2int16(
					(G3D::int16)(int)((float)a.x / c),
					(G3D::int16)(int)((float)a.y / c)));
			}
		}
		else
		{
			a = getObject(L, 2);
			int c = lua_tointeger(L, 1);
			pushVector2int16(L, G3D::Vector2int16(c / a.x, c / a.y));
		}
		return 1;
	}

	// mac: 0x1688BE (90B, batch395): 16-bit lane negate.
	int Vector2int16Bridge::on_unm(lua_State* L)
	{
		const G3D::Vector2int16& a = getObject(L, 1);
		pushVector2int16(L, G3D::Vector2int16(-a.x, -a.y));
		return 1;
	}

	// mac: 0x168918 (231B, batch395): x/X y/Y via pushinteger; plain failure.
	template<>
	int Bridge<G3D::Vector2int16, true>::on_index(const G3D::Vector2int16& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "x") == 0 || strcmp(name, "X") == 0)
		{
			lua_pushinteger(L, object.x);
			return 1;
		}
		if (strcmp(name, "y") == 0 || strcmp(name, "Y") == 0)
		{
			lua_pushinteger(L, object.y);
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x168A00 (102B-pattern): always throws, with the name.
	template<>
	void Bridge<G3D::Vector2int16, true>::on_newindex(G3D::Vector2int16& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x19849A (96B-pattern): luaL_checklstring wrapper verbatim.
	template<>
	int Bridge<G3D::Vector2int16, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x1984FA (98B-pattern): same verbatim.
	template<>
	int Bridge<G3D::Vector2int16, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x16EA3A (48B-pattern): checkudata + return 0 (no dtor, POD).
	template<>
	int Bridge<G3D::Vector2int16, true>::on_gc(lua_State* L)
	{
		luaL_checkudata(L, 1, className);
		return 0;
	}

	// mac: 0x16EA6A (107B, batch396): whole-dword compare (== operator==).
	template<>
	int Bridge<G3D::Vector2int16, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x16EAD6 (60B-pattern): checkudata + forward.
	template<>
	int Bridge<G3D::Vector2int16, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x16F4F4 (170B-pattern): StringConverter (imported in
	// util/Region3int16.cpp) + pushlstring.
	// FUNCTION: PLAYER 0x0087f250
	// FUNCTION: STUDIO 0x009e38d0
	template<>
	int Bridge<G3D::Vector2int16, true>::on_tostring(const G3D::Vector2int16& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<G3D::Vector2int16>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x168A66 (211B, batch401): up-to-2 floats (tofloat), zero-fill,
	// pushNewObject<float*>.
	int Vector2Bridge::newVector2(lua_State* L)
	{
		int count = std::min(2, lua_gettop(L));
		float v[2] = { 0.0f, 0.0f };
		for (int i = 0; i < count; ++i)
			v[i] = lua_tofloat(L, i + 1);
		pushNewObject(L, v);
		return 1;
	}

	const luaL_reg Vector2Bridge::classLibrary[] = {
		{ "new", newVector2 },
		{ NULL, NULL }
	};

	// mac: 0x168B3A (batch355-banked shape): luaL_register + readonly + pop.
	void Vector2Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x166108 (211B, batch401): up-to-3 floats (tofloat), zero-fill,
	// pushNewObject<float*>.
	int Color3Bridge::newColor3(lua_State* L)
	{
		int count = std::min(3, lua_gettop(L));
		float v[3] = { 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < count; ++i)
			v[i] = lua_tofloat(L, i + 1);
		pushNewObject(L, v);
		return 1;
	}

	const luaL_reg Color3Bridge::classLibrary[] = {
		{ "new", newColor3 },
		{ NULL, NULL }
	};

	void Color3Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x1663C0 (285B, batch401): up-to-2 Vector3 args (origin, direction;
	// missing become zero); builds RbxRay(origin, dir) + pushes.
	int RbxRayBridge::newRbxRay(lua_State* L)
	{
		int count = std::min(2, lua_gettop(L));
		G3D::Vector3 v[2] = { G3D::Vector3::zero(), G3D::Vector3::zero() };
		for (int i = 0; i < count; ++i)
			v[i] = Bridge<G3D::Vector3>::getObject(L, i + 1);
		pushRbxRay(L, RBX::RbxRay(v[0], v[1]));
		return 1;
	}

	const luaL_reg RbxRayBridge::classLibrary[] = {
		{ "new", newRbxRay },
		{ NULL, NULL }
	};

	void RbxRayBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x1668BC (321B, batch401): up-to-2 Vector3 args (min, max;
	// zero-fill); Region3(min, max) + push.
	// STUB: PLAYER 0x00883680
	// STUB: STUDIO 0x0092d680
	int Region3Bridge::newRegion3(lua_State* L)
	{
		int count = std::min(2, lua_gettop(L));
		G3D::Vector3 v[2] = { G3D::Vector3::zero(), G3D::Vector3::zero() };
		for (int i = 0; i < count; ++i)
			v[i] = Bridge<G3D::Vector3>::getObject(L, i + 1);
		pushRegion3(L, RBX::Region3(v[0], v[1]));
		return 1;
	}

	const luaL_reg Region3Bridge::classLibrary[] = {
		{ "new", newRegion3 },
		{ NULL, NULL }
	};

	void Region3Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x166BAE (285B-pattern, batch401): up-to-2 Vector3int16 args;
	// Region3int16(min, max) + push.
	int Region3int16Bridge::newRegion3int16(lua_State* L)
	{
		int count = std::min(2, lua_gettop(L));
		G3D::Vector3int16 v[2];
		for (int i = 0; i < count; ++i)
			v[i] = Bridge<G3D::Vector3int16>::getObject(L, i + 1);
		pushRegion3int16(L, RBX::Region3int16(v[0], v[1]));
		return 1;
	}

	const luaL_reg Region3int16Bridge::classLibrary[] = {
		{ "new", newRegion3int16 },
		{ NULL, NULL }
	};

	void Region3int16Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x167D18 (211B, batch401): up-to-3 lua_tointeger, zero-fill.
	// Binary pushes via pushNewObject<int*> (Vector3int16(int*) ctor,
	// whose TU is not in the build); lanes here are bitwise-identical.
	int Vector3int16Bridge::newVector3int16(lua_State* L)
	{
		int count = std::min(3, lua_gettop(L));
		int v[3] = { 0, 0, 0 };
		for (int i = 0; i < count; ++i)
			v[i] = lua_tointeger(L, i + 1);
		pushVector3int16(L, G3D::Vector3int16(v[0], v[1], v[2]));
		return 1;
	}

	const luaL_reg Vector3int16Bridge::classLibrary[] = {
		{ "new", newVector3int16 },
		{ NULL, NULL }
	};

	void Vector3int16Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x1683F2 (211B, batch401): up-to-2 lua_tointeger, zero-fill
	// (same int*-ctor note as newVector3int16).
	int Vector2int16Bridge::newVector2int16(lua_State* L)
	{
		int count = std::min(2, lua_gettop(L));
		int v[2] = { 0, 0 };
		for (int i = 0; i < count; ++i)
			v[i] = lua_tointeger(L, i + 1);
		pushVector2int16(L, G3D::Vector2int16(v[0], v[1]));
		return 1;
	}

	const luaL_reg Vector2int16Bridge::classLibrary[] = {
		{ "new", newVector2int16 },
		{ NULL, NULL }
	};

	void Vector2int16Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x166E88 (211B, batch350): up-to-3 floats (tofloat), zero-fill,
	// pushNewObject<float*>.
	int Vector3Bridge::newVector3(lua_State* L)
	{
		int count = std::min(3, lua_gettop(L));
		float v[3] = { 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < count; ++i)
			v[i] = lua_tofloat(L, i + 1);
		pushNewObject(L, v);
		return 1;
	}

	// mac: 0x166F5C (240B, batch421): Enum.NormalId item -> unit vector.
	// STUB: PLAYER 0x00884000
	// STUB: STUDIO 0x0092e000
	int Vector3Bridge::newVector3FromNormalId(lua_State* L)
	{
		EnumDescriptorItemPtr item = NULL;
		if (!Bridge<EnumDescriptorItemPtr, true>::getValue(L, 1, item)
			|| !enumItemIsA(item, typeid(RBX::NormalId), "N3RBX8NormalIdE"))
			throw RBX::runtime_error("Vector3.FromNormalId expects Enum.NormalId input");
		pushVector3(L, RBX::normalIdToVector3((RBX::NormalId)item->value));
		return 1;
	}

	// mac: 0x16704C (248B, batch421): Enum.Axis item -> NormalId -> unit vector.
	// STUB: PLAYER 0x008840c0
	// STUB: STUDIO 0x0092e0c0
	int Vector3Bridge::newVector3FromAxis(lua_State* L)
	{
		EnumDescriptorItemPtr item = NULL;
		if (!Bridge<EnumDescriptorItemPtr, true>::getValue(L, 1, item)
			|| !enumItemIsA(item, typeid(G3D::Vector3::Axis), "N3G3D7Vector34AxisE"))
			throw RBX::runtime_error("Vector3.FromAxis expects Enum.Axis input");
		pushVector3(L, RBX::normalIdToVector3(
			RBX::Axes::axisToNormalId((G3D::Vector3::Axis)item->value)));
		return 1;
	}

	const luaL_reg Vector3Bridge::classLibrary[] = {
		{ "new", newVector3 },
		{ "FromNormalId", newVector3FromNormalId },
		{ "FromAxis", newVector3FromAxis },
		{ NULL, NULL }
	};

	void Vector3Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// VS2012 odr-use holders (openState names these at 0x18B907/0x18B923).
	int (*volatile g_bridgeV3int16Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector3int16>::on_index;
	int (*volatile g_bridgeV3int16NewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector3int16>::on_newindex;
	int (*volatile g_bridgeV2int16Index)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector2int16>::on_index;
	int (*volatile g_bridgeV2int16NewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<G3D::Vector2int16>::on_newindex;

	// ---- Library batch 2 (section 237) ----

	// mac: 0x16991E (868B, batch402). Arg-count dispatch, verbatim vs 2016
	// (0/1/2/3/7/12/default-throw with the count).
	// STUB: PLAYER 0x00886260
	// STUB: STUDIO 0x00930260
	int CoordinateFrameBridge::newCoordinateFrame(lua_State* L)
	{
		G3D::CoordinateFrame cf;
		const int count = lua_gettop(L);
		switch (count)
		{
		case 0:
			break;
		case 1:
			cf.translation = Bridge<G3D::Vector3>::getObject(L, 1);
			break;
		case 2:
			cf.translation = Bridge<G3D::Vector3>::getObject(L, 1);
			cf.lookAt(Bridge<G3D::Vector3>::getObject(L, 2));
			break;
		case 3:
			for (int i = 0; i < 3; ++i)
				cf.translation[i] = lua_tofloat(L, i + 1);
			break;
		case 7:
			{
				for (int i = 0; i < 3; ++i)
					cf.translation[i] = lua_tofloat(L, i + 1);
				G3D::Quat q;
				q.x = lua_tofloat(L, 3 + 1);
				q.y = lua_tofloat(L, 4 + 1);
				q.z = lua_tofloat(L, 5 + 1);
				q.w = lua_tofloat(L, 6 + 1);
				cf.rotation = G3D::Matrix3(q);
			}
			break;
		case 12:
			for (int i = 0; i < 3; ++i)
				cf.translation[i] = lua_tofloat(L, i + 1);
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					cf.rotation[i][j] = lua_tofloat(L, 3 + 3 * i + j + 1);
			break;
		default:
			throw RBX::runtime_error("Invalid number of arguments: %d", count);
		}
		pushNewObject(L, cf);
		return 1;
	}

	// mac: 0x169CB8 (300B, batch402): identity + rotation from 3 checknumbers.
	int CoordinateFrameBridge::fromEulerAnglesXYZ(lua_State* L)
	{
		G3D::CoordinateFrame cf;
		cf.rotation = G3D::Matrix3::fromEulerAnglesXYZ(
			(float)luaL_checknumber(L, 1),
			(float)luaL_checknumber(L, 2),
			(float)luaL_checknumber(L, 3));
		pushNewObject(L, cf);
		return 1;
	}

	// mac: 0x169DE4 (236B, batch402): identity + fromAxisAngle(Vector3, angle).
	// FUNCTION: PLAYER 0x00887100
	// FUNCTION: STUDIO 0x00931100
	int CoordinateFrameBridge::fromAxisAngle(lua_State* L)
	{
		G3D::CoordinateFrame cf;
		cf.rotation = G3D::Matrix3::fromAxisAngle(
			Bridge<G3D::Vector3>::getObject(L, 1),
			(float)luaL_checknumber(L, 2));
		pushNewObject(L, cf);
		return 1;
	}

	const luaL_reg CoordinateFrameBridge::classLibrary[] = {
		{ "new", newCoordinateFrame },
		{ "fromEulerAnglesXYZ", fromEulerAnglesXYZ },
		{ "Angles", fromEulerAnglesXYZ },
		{ "fromAxisAngle", fromAxisAngle },
		{ NULL, NULL }
	};

	void CoordinateFrameBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x16B476 (203B, batch402). Quirk (body-proven): count==0 pushes a
	// dead UDim(0,0) FIRST, then falls through and pushes again (stack
	// grows by 2, return 1). Replicated exactly.
	int UDimBridge::newUDim(lua_State* L)
	{
		int count = std::min(3, lua_gettop(L));
		if (count == 0)
			pushUDim(L, RBX::UDim(0.0f, 0));
		float scale = lua_tofloat(L, 1);
		if (count == 1)
		{
			pushUDim(L, RBX::UDim(scale, 0));
			return 1;
		}
		pushUDim(L, RBX::UDim(scale, (G3D::int16)lua_tointeger(L, 2)));
		return 1;
	}

	const luaL_reg UDimBridge::classLibrary[] = {
		{ "new", newUDim },
		{ NULL, NULL }
	};

	void UDimBridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x16B876 (237B, batch402): fallthrough switch (5/4 -> 3 -> 2 -> 1),
	// verbatim shape; default is unreachable (count <= 5).
	int UDim2Bridge::newUDim2(lua_State* L)
	{
		int count = std::min(5, lua_gettop(L));
		float sx = 0.0f;
		G3D::int16 ox = 0;
		float sy = 0.0f;
		G3D::int16 oy = 0;
		switch (count)
		{
		default:
			break;
		case 5:
		case 4:
			oy = (G3D::int16)lua_tointeger(L, 4);
		case 3:
			sy = lua_tofloat(L, 3);
		case 2:
			ox = (G3D::int16)lua_tointeger(L, 2);
		case 1:
			sx = lua_tofloat(L, 1);
		case 0:
			break;
		}
		pushUDim2(L, RBX::UDim2(sx, ox, sy, oy));
		return 1;
	}

	const luaL_reg UDim2Bridge::classLibrary[] = {
		{ "new", newUDim2 },
		{ NULL, NULL }
	};

	void UDim2Bridge::registerClassLibrary(lua_State* L)
	{
		luaL_register(L, className, classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

} }

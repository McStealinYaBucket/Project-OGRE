/* script/LuaArguments.h -- October 2013 (first import, section 246).
   Evidence: withVariantValue<int,ArgumentPusher> 0x163828 + ArgumentPusher overloads
     (0x164334-0x164794) + push 0x162624 + pushTuple 0x1996EA + pushArray pair + getValues
     0x1B0348 + get 0x161BDA (batches 460-464). 2013 asserts in this header (PLAYER strings,
     "include\script/LuaArguments.h"): "0" line 114 (withVariantValue), "success" line 146
     (getValues), "luaIndex>0" line 178 (getVariant), "count == 1" line 213 (pushArray).
     getValues and getVariant sit on their lines; withVariantValue (defined last, after the
     class) and pushArray still carry the Mac-form literals.
   2013-vs-2016 DELTAs: the withVariantValue SET is 2013-pinned (no Rect2D/PhysicalProperties/
     sequences/Instances/connection; +InputObject +PropertyDescriptor const*); dual
     typeinfo-pointer/GCC-name checks; 3 template params (value, L); only shared/alias types
     dispatch through F. */

#pragma once

#include "Reflection/Type.h"
#include "Reflection/EnumConverter.h"
#include "reflection/Function.h"
#include "Util/ProtectedString.h"
#include "script/ThreadRef.h"
#include "script/LuaEnum.h"
#include "script/LuaAtomicClasses.h"
#include "RbxAssert.h"

#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <typeinfo>
#include <boost/type_traits.hpp>

namespace RBX {

	// Defined after the classes below (dispatcher 0x163828).
	template<typename R, typename F, typename T>
	R withVariantValue(const Reflection::Variant& value, T L);

	namespace Lua {

		// Pushes one value per Variant (returns values pushed). Holds the
		// lua_State for the delegated arms.
		class ArgumentPusher
		{
			lua_State* L;
		public:
			ArgumentPusher(lua_State* L) : L(L) {}
			int operator()(const boost::shared_ptr<const Reflection::ValueArray>& value);
			int operator()(const boost::shared_ptr<const Reflection::ValueMap>& value);
			int operator()(const boost::shared_ptr<const Reflection::ValueTable>& value);
			int operator()(const boost::shared_ptr<const Reflection::Tuple>& value);
			// Sync-function pusher (0x1646B0, batch463: by-value shared,
			// matching the mangling). Async twin queues (yield era).
			int operator()(boost::shared_ptr<Lua::GenericFunction> value);
			// Async-function pusher (0x164722, batch463: same by-value
			// shape as sync).
			int operator()(boost::shared_ptr<Lua::GenericAsyncFunction> value);
			// (Instance + Instances-vector queue behind the Instance
			// bridge era; decls land with it.)
			template<class T>
			int operator()(const T& value,
				typename boost::disable_if<boost::is_arithmetic<T>, void>::type* = NULL)
			{
				// mac: 0x164794 (122B, batch463): local copy (shared-aware) +
				// push + scope-end destruction. Only CellID instantiates
				// this in the binary.
				T copy(value);
				Bridge<T>::pushNewObject(L, copy);
				return 1;
			}
		};

		// Stack <-> Variant marshaling.  Base derived in the YFSO era: the
		// first caller (YieldFunctionStateObject::execute 0x178C06) passes
		// *this to YieldFunctionDescriptor::execute(
		// FunctionDescriptor::Arguments&, ...).  The 12-slot 2013 interface
		// (batch727 vtable 0xED1B00-0xED1B38) is satisfied 1:1 by the member
		// fns below: size/getVariant/getBool/getLong/getDouble/getString/
		// getVector3int16/getRegion3int16/getVector3/getRegion3/getObject/
		// getEnum.
		class LuaArguments : public Reflection::FunctionDescriptor::Arguments
		{
		public:
			// Binary layout (all getX EAs; re-confirmed batch726 --
			// getString/getVariant read this+72 / this+76): offset at
			// +72, L at +76.  The ported base carries the interface +
			// returnValue only, so offset/L sit at +20/+24 here (DELTA,
			// behavior-neutral: every access is by name; the rest of
			// the binary's 72-byte base is a Function-era item).
			const int offset;
			lua_State* const L;
			LuaArguments(lua_State* L, int offset):L(L),offset(offset) {}











































			// Inline here in 2013: its assert is include/script/LuaArguments.h line 146.
			// FUNCTION: PLAYER 0x00683f90
			// FUNCTION: STUDIO 0x007e9d50
			static boost::shared_ptr<Reflection::Tuple> getValues(lua_State* L)
			{
				int argCount = lua_gettop(L);
				boost::shared_ptr<Reflection::Tuple> args(rbx::make_shared<Reflection::Tuple>(argCount));
				for (int i = 0; i < argCount; ++i)
				{
					Reflection::Variant& v = args->values.at(i);
					bool success = LuaArguments::get(L, i + 1, v, false);
					RBXASSERT(success);
				}
				return args;
			}
			static int pushTuple(const Reflection::Tuple& arguments, lua_State* L);
			static bool get(lua_State* L, int luaIndex, Reflection::Variant& value, bool treatNilAsMissing);
			bool getString(int index, std::string& value) const;
			bool getVector3int16(int index, G3D::Vector3int16& value) const;
			bool getRegion3int16(int index, RBX::Region3int16& value) const;
			bool getVector3(int index, G3D::Vector3& value) const;
			bool getRegion3(int index, RBX::Region3& value) const;
			bool getObject(int index, boost::shared_ptr<Reflection::DescribedBase>& value) const;
			bool getDouble(int index, double& value) const;
			bool getBool(int index, bool& value) const;
			bool getEnum(int index, const Reflection::EnumDescriptor& desc, int& value) const;










			// Inline here in 2013: its assert is include/script/LuaArguments.h line 178.
			// Offset slot, then a nil-tolerant get (treatNilAsMissing=true).
			// FUNCTION: PLAYER 0x00877b40
			// FUNCTION: STUDIO 0x009dc240
			bool getVariant(int index, Reflection::Variant& value) const
			{
				int luaIndex = index + offset;
				RBXASSERT(luaIndex>0);
				return LuaArguments::get(L, luaIndex, value, true);
			}
			bool getLong(int index, long& value) const;
			size_t size() const;
			// STUB: PLAYER 0x008a6460
			// STUB: STUDIO 0x0092a2e0
			template<class Iterator>
			static int pushArray(Iterator first, Iterator last, lua_State* const L)
			{
				lua_createtable(L, (int)(last - first), 0);
				unsigned int i = 0;
				while (first != last)
				{
					int count = RBX::withVariantValue<int, ArgumentPusher>(*first, L);
					if (FLog::Asserts != 0 && count != 1)
					{
						if (RBX::_internal::_debugHook == 0
							|| RBX::_internal::_debugHook("count == 1", "include/Script/LuaArguments.h", 213) == 0)
						{
							ReleaseAssert(FLog::Asserts,
								"count == 1 file: include/Script/LuaArguments.h line: 213");
						}
					}
					lua_rawseti(L, -2, ++i);
					++first;
				}
				return 1;
			}
			static int push(const Reflection::Variant& value, lua_State* const L);
		};

	}

	// Variant-to-Lua dispatcher (0x163828). First match wins; the order
	// below is the binary's. Only the shared/alias arms dispatch through
	// F (constructed from L); everything else pushes directly.
	// STUB: PLAYER 0x008a67b0
	// STUB: STUDIO 0x0092a630
	template<typename R, typename F, typename T>
	R withVariantValue(const Reflection::Variant& value, T L)
	{
		F f(L);
		const std::type_info* ti = value.type().typeInfo;
		if (ti == &typeid(void) || !strcmp(ti->name(), "v"))
			return 0;
		if (ti == &typeid(bool) || !strcmp(ti->name(), "b"))
		{
			lua_pushboolean(L, *rbx::any_cast<const bool, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(int) || !strcmp(ti->name(), "i"))
		{
			lua_pushnumber(L, (double)*rbx::any_cast<const int, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(long) || !strcmp(ti->name(), "l"))
		{
			lua_pushnumber(L, (double)*rbx::any_cast<const long, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(float) || !strcmp(ti->name(), "f"))
		{
			lua_pushnumber(L, (double)*rbx::any_cast<const float, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(double) || !strcmp(ti->name(), "d"))
		{
			lua_pushnumber(L, *rbx::any_cast<const double, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(std::string) || !strcmp(ti->name(), "Ss"))
		{
			const std::string& s = *rbx::any_cast<const std::string, RBX::Region3>(value.value);
			lua_pushlstring(L, s.c_str(), s.size());
			return 1;
		}
		if (ti == &typeid(RBX::ProtectedString) || !strcmp(ti->name(), "N3RBX15ProtectedStringE"))
		{
			const std::string& s = rbx::any_cast<const RBX::ProtectedString, RBX::Region3>(
				value.value)->getStringForImmediateUse();
			lua_pushlstring(L, s.c_str(), s.size());
			return 1;
		}
		// (shared_ptr<Instance> queues behind the Instance bridge era.)
		if (const Reflection::EnumDescriptor* desc =
			Reflection::EnumDescriptor::lookupDescriptor(value.type()))
		{
			const Reflection::EnumDescriptor::Item* item = desc->lookup(value);
			if (item == NULL)
				throw RBX::runtime_error("Invalid value for enum %s", desc->name->str.c_str());
			Lua::EnumItem::push(L, item);
			return 1;
		}
		if (ti == &typeid(Lua::WeakFunctionRef) || !strcmp(ti->name(), "N3RBX3Lua15WeakFunctionRefE"))
		{
			Lua::lua_pushfunction(L, *rbx::any_cast<const Lua::WeakFunctionRef, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(boost::shared_ptr<const Reflection::ValueArray>)
			|| !strcmp(ti->name(), "N5boost10shared_ptrIKSt6vectorIN3RBX10Reflection7VariantESaIS4_EEEE"))
			return f(*rbx::any_cast<const boost::shared_ptr<const Reflection::ValueArray>, RBX::Region3>(value.value));
		if (ti == &typeid(boost::shared_ptr<const Reflection::ValueMap>)
			|| !strcmp(ti->name(), "N5boost10shared_ptrIKSt3mapISsN3RBX10Reflection7VariantESt4lessISsESaISt4pairIKSsS4_EEEEE"))
			return f(*rbx::any_cast<const boost::shared_ptr<const Reflection::ValueMap>, RBX::Region3>(value.value));
		if (ti == &typeid(boost::shared_ptr<const Reflection::ValueTable>)
			|| !strcmp(ti->name(), "N5boost10shared_ptrIKNS_9unordered13unordered_mapISsN3RBX10Reflection7VariantENS_4hashISsEESt8equal_toISsESaISt4pairIKSsS5_EEEEEE"))
			return f(*rbx::any_cast<const boost::shared_ptr<const Reflection::ValueTable>, RBX::Region3>(value.value));
		// (Instances-vector queues behind the Instance bridge era.)
		if (ti == &typeid(boost::shared_ptr<const Reflection::Tuple>)
			|| !strcmp(ti->name(), "N5boost10shared_ptrIKN3RBX10Reflection5TupleEEE"))
			return f(*rbx::any_cast<const boost::shared_ptr<const Reflection::Tuple>, RBX::Region3>(value.value));
		// mac: 0x163828 sync-function arm (batch462: non-const shared_ptr
		// element, GCC name pinned). The async twin queues behind
		// scheduleResume (yield era).
		if (ti == &typeid(boost::shared_ptr<Lua::GenericFunction>)
			|| !strcmp(ti->name(), "N5boost10shared_ptrINS_8functionIFNS0_IKN3RBX10Reflection5TupleEEES6_EEEEE"))
			return f(*rbx::any_cast<const boost::shared_ptr<Lua::GenericFunction>, RBX::Region3>(value.value));
		// Async-function arm (batch462: same non-const shape; the closure
		// needs onAsyncResult/scheduleResume, both landed).
		if (ti == &typeid(boost::shared_ptr<Lua::GenericAsyncFunction>)
			|| !strcmp(ti->name(), "N5boost10shared_ptrINS_8functionIFvNS0_IKN3RBX10Reflection5TupleEEENS1_IFvPNS2_3Lua12IAsyncResultEEEEEEEEE"))
			return f(*rbx::any_cast<const boost::shared_ptr<Lua::GenericAsyncFunction>, RBX::Region3>(value.value));
		if (ti == &typeid(G3D::Vector3int16) || !strcmp(ti->name(), "N3G3D12Vector3int16E"))
		{
			Lua::Vector3int16Bridge::pushVector3int16(L,
				*rbx::any_cast<const G3D::Vector3int16, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(G3D::Vector2int16) || !strcmp(ti->name(), "N3G3D12Vector2int16E"))
		{
			Lua::Vector2int16Bridge::pushVector2int16(L,
				*rbx::any_cast<const G3D::Vector2int16, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(G3D::Vector3) || !strcmp(ti->name(), "N3G3D7Vector3E"))
		{
			Lua::Vector3Bridge::pushVector3(L,
				*rbx::any_cast<const G3D::Vector3, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(G3D::Vector2) || !strcmp(ti->name(), "N3G3D7Vector2E"))
		{
			Lua::Vector2Bridge::pushVector2(L,
				*rbx::any_cast<const G3D::Vector2, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::RbxRay) || !strcmp(ti->name(), "N3RBX6RbxRayE"))
		{
			Lua::RbxRayBridge::pushRbxRay(L,
				*rbx::any_cast<const RBX::RbxRay, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(G3D::CoordinateFrame) || !strcmp(ti->name(), "N3G3D15CoordinateFrameE"))
		{
			Lua::CoordinateFrameBridge::pushCoordinateFrame(L,
				*rbx::any_cast<const G3D::CoordinateFrame, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(G3D::Color3) || !strcmp(ti->name(), "N3G3D6Color3E"))
		{
			Lua::Color3Bridge::pushColor3(L,
				*rbx::any_cast<const G3D::Color3, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::BrickColor) || !strcmp(ti->name(), "N3RBX10BrickColorE"))
		{
			Lua::BrickColorBridge::pushBrickColor(L,
				*rbx::any_cast<const RBX::BrickColor, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::Region3) || !strcmp(ti->name(), "N3RBX7Region3E"))
		{
			Lua::Region3Bridge::pushRegion3(L,
				*rbx::any_cast<const RBX::Region3, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::Region3int16) || !strcmp(ti->name(), "N3RBX12Region3int16E"))
		{
			Lua::Region3int16Bridge::pushRegion3int16(L,
				*rbx::any_cast<const RBX::Region3int16, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::UDim) || !strcmp(ti->name(), "N3RBX4UDimE"))
		{
			Lua::UDimBridge::pushUDim(L,
				*rbx::any_cast<const RBX::UDim, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::UDim2) || !strcmp(ti->name(), "N3RBX5UDim2E"))
		{
			Lua::UDim2Bridge::pushUDim2(L,
				*rbx::any_cast<const RBX::UDim2, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::Faces) || !strcmp(ti->name(), "N3RBX5FacesE"))
		{
			Lua::FacesBridge::pushFaces(L,
				*rbx::any_cast<const RBX::Faces, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::Axes) || !strcmp(ti->name(), "N3RBX4AxesE"))
		{
			Lua::AxesBridge::pushAxes(L,
				*rbx::any_cast<const RBX::Axes, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(RBX::CellID) || !strcmp(ti->name(), "N3RBX6CellIDE"))
			return f(*rbx::any_cast<const RBX::CellID, RBX::Region3>(value.value));
		if (ti == &typeid(RBX::ContentId) || !strcmp(ti->name(), "N3RBX9ContentIdE"))
		{
			const std::string& s = rbx::any_cast<const RBX::ContentId, RBX::Region3>(
				value.value)->toString();
			lua_pushlstring(L, s.c_str(), s.size());
			return 1;
		}
		if (ti == &typeid(RBX::InputObject) || !strcmp(ti->name(), "N3RBX11InputObjectE"))
		{
			Lua::InputObjectBridge::pushInputObject(L,
				*rbx::any_cast<const RBX::InputObject, RBX::Region3>(value.value));
			return 1;
		}
		if (ti == &typeid(const RBX::Reflection::PropertyDescriptor*)
			|| !strcmp(ti->name(), "PKN3RBX10Reflection18PropertyDescriptorE"))
		{
			const std::string& s = (*rbx::any_cast<const RBX::Reflection::PropertyDescriptor* const, RBX::Region3>(
				value.value))->name->str;
			lua_pushlstring(L, s.c_str(), s.size());
			return 1;
		}

		if (FLog::Asserts != 0)
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("0", "include/Script/LuaArguments.h", 114) == 0)
			{
				ReleaseAssert(FLog::Asserts, "0 file: include/Script/LuaArguments.h line: 114");
			}
		}
		return 0;
	}
}

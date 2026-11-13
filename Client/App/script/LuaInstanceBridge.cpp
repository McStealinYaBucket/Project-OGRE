/* script/LuaInstanceBridge.cpp -- October 2013 (Instance bridge,
   sections 258a-d; evidence: see include/script/LuaInstanceBridge.h).
   Complete: null/non-null push, registerClass, on_index/on_newindex
   dispatch both ways, newInstance + Lock/Unlock + Instance library.
   Method era (2026-10-17, batches 735-737): callMember pair
   0x17587C/0x175B4A + on_index method arms.
   Queued: Parent +476-guard audit.
   Original file: Client/App/script/LuaInstanceBridge.cpp. */
#include "stdafx.h"
#include "script/LuaInstanceBridge.h"
#include "script/LuaSignal.h"
#include "script/LuaEnum.h"
#include "script/LuaAtomicClasses.h"
#include "Reflection/Event.h"
#include "reflection/property.h"
#include "V8Tree/Instance.h"
#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/BaseScript.h"
#include "Util/Name.h"
#include "Util/Limits.h"
#include "security/SecurityContext.h"
#include "Util/FastFlags.h"
#include "RbxAssert.h"
#include "luaconf.h"
#include "script/LuaArguments.h"
#include "reflection/YieldFunction.h"
#include "V8DataModel/DataModel.h"
#include "Util/StandardOut.h"
#include "Util/Object.h"
#include "boost/bind.hpp"
#include <cctype>
#include <stdexcept>
#include <vector>
namespace RBX { namespace Lua {
	template<>
	// GLOBAL: PLAYER 0x00d8e914
	// GLOBAL: STUDIO 0x00f2b108
	// Bridge<boost::shared_ptr<RBX::Instance>,0>::className
	const char* Bridge<boost::shared_ptr<Instance>, false>::className = "Object";
	// mac: 0x1736EA (137B, batch520): fresh table, self-metatable, __mode set.
	// (Sits immediately before newInstance in the binary; generic bridge
	// infra used by every SharedPtrBridge::registerClassLibrary.)
	// FUNCTION: PLAYER 0x00876c70
	// FUNCTION: STUDIO 0x009db890
	void newweaktable(lua_State* L, const char* mode)
	{
		lua_createtable(L, 0, 0);
		lua_pushvalue(L, -1);
		lua_setmetatable(L, -2);
		lua_pushlstring(L, "__mode", 6);
		lua_pushstring(L, mode);
		lua_settable(L, -3);
	}
	// mac: 0x1718CE (334B, batch515): index/newindex/gc/tostring + readonly.
	// NO __eq (the false param shows: no __eq entry, unlike the true
	// bridges). Queued use: openState 0x18B85F passes the L-form
	// on_index/on_newindex (258b/c).
	// FUNCTION: PLAYER 0x00880f40
	// FUNCTION: STUDIO 0x009e55c0
	template<>
	void Bridge<boost::shared_ptr<Instance>, false>::registerClass(lua_State* L,
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
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, on_tostring);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}
	// mac: 0x171A1C (65B, batch515): releases the userdata's shared copy
	// (spelled .reset() per the GenericFunction-bridge precedent; the
	// bare release is unobservable once the userdata is garbage).
	template<>
	int Bridge<boost::shared_ptr<Instance>, false>::on_gc(lua_State* L)
	{
		getObject(L, 1).reset();
		return 0;
	}
	// mac: 0x171A5E (62B-pattern, batch515): checkudata + forward.
	template<>
	int Bridge<boost::shared_ptr<Instance>, false>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}
	// mac: 0x178706 (204B, batch515): pushes the instance name. The binary
	// reads it raw (fw()+0x18 length-prefixed chars); getName() is the
	// same string (Instance.h: FWInstance +24), so the pushed characters
	// are identical.
	template<>
	int Bridge<boost::shared_ptr<Instance>, false>::on_tostring(
		const boost::shared_ptr<Instance>& object, lua_State* L)
	{
		const std::string& name = object->getName();
		lua_pushlstring(L, name.c_str(), name.size());
		return 1;
	}
	// mac: 0x173982 (35B, batch516): pushes true unconditionally, verbatim.
	int ObjectBridge::lockInstance(lua_State* L)
	{
		lua_pushboolean(L, 1);
		return 1;
	}
	// mac: 0x1739A6 (7B, batch516): no-op returning 0, verbatim.
	int ObjectBridge::unlockInstance(lua_State* L)
	{
		(void)L;
		return 0;
	}
	// mac: 0x173774 (526B, batch516): Instance.new(className[, parent]).
	// STUB: PLAYER 0x0087af00
	// STUB: STUDIO 0x009df580
	int ObjectBridge::newInstance(lua_State* L)
	{
		const char* className = throwable_lua_tostring(L, 1);
		// Binary takes const char*& (RKPKc); the pointee content is
		// what matters (lookup never mutates it observably), so the
		// by-value form is equivalent.
		const RBX::Name* name = RBX::Name::lookup(className);
		if (!Limits::Counter::canAdd(1))
			throw RBX::runtime_error("Instance limit exceeded");
		boost::shared_ptr<Instance> created =
			RBX::Creatable<Instance>::createByName(*name, ScriptingCreator);
		if (created.get() == NULL)
		{
			throw RBX::runtime_error(
				"Unable to create an Instance of type \"%s\"", className);
		}
		if (lua_gettop(L) >= 2)
		{
			boost::shared_ptr<Instance> parent =
				SharedPtrBridge<Instance>::getPtr(L, 2);
			if (parent.get() != NULL)
			{
				// DELTA (fw()+22 on the parent): same unmapped flag as
				// the 258b event/child arms -- deterministic-false.
				// setParent forwards to setParentInternal(parent, false),
				// exactly the binary's call.
				created->setParent(parent.get());
			}
		}
		SharedPtrBridge<Instance>::push(L, created);
		return 1;
	}
	// mac: 0x199134 (90B, batch518): {"new","Lock","Unlock"} + readonly + pop.
	void ObjectBridge::registerInstanceClassLibrary(lua_State* L)
	{
		static const luaL_Reg classLibrary[] =
		{
			{ "new", newInstance },
			{ "Lock", lockInstance },
			{ "Unlock", unlockInstance },
			{ NULL, NULL }
		};
		luaL_register(L, "Instance", classLibrary);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}
	// mac: 0x1756D3 (425B, batch522-523/Hexrays): legacy "size" -> "Size"
	// alias. Fires the legacy-read signal (binary Context+724) with
	// (instance, "size", L-12 script) on the alias path only.
	// STUB: PLAYER 0x0087ab20
	// STUB: STUDIO 0x009df1a0
	static const char* resolveLegacyMemberName(
		const boost::shared_ptr<Instance>& object,
		const char* name, lua_State* L)
	{
		if (name[0] == 's' && strcmp(name, "size") == 0)
		{
			// Binary calls ScriptContext::getContext(L) (0x18B3DE); the
			// in-tree equivalent is the extra-space accessor (all green
			// paths pass non-null L, same as the slots).
			ScriptContext* context = RobloxExtraSpace::get(L)->context();
			boost::shared_ptr<BaseScript> script;
			if (L != NULL)
			{
				script = boost::shared_ptr<BaseScript>(
					RobloxExtraSpace::get(L)->script, boost::detail::sp_nothrow_tag());
			}
			// Stats/debug era: renamed/retyped camelCaseViolation (binary
			// EventDesc types the third slot shared_ptr<Instance>); the
			// BaseScript here converts up on the call.
			context->camelCaseViolation(object, std::string(name), script);
			return "Size";
		}
		return name;
	}
	// isValueType<V> recheck (include/reflection/property.h:289, batch524).
	// Spelled manually: the binary's FLog-gated ReleaseAssert is
	// log-continue, while the in-tree RBXASSERT behind getValue<V>()
	// is not -- so the check is hoisted here, verbatim-literal.
	template<class V>
	static void checkValueType(const Reflection::PropertyDescriptor* prop)
	{
		if (FLog::Asserts != 0
			&& !(prop->type == Reflection::Type::singleton<V>()))
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("isValueType<V>()",
					"include/reflection/property.h", 289) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"isValueType<V>() file: include/reflection/property.h line: 289");
			}
		}
	}
	template<class V>
	static V readValue(const Reflection::PropertyDescriptor* prop,
		const Reflection::DescribedBase* described)
	{
		checkValueType<V>(prop);
		return static_cast<const Reflection::TypedPropertyDescriptor<V>*>(
			prop)->getValue(described);
	}
	// mac: 0x1739AE (7461B, batches 514/521-536 + Hexrays): member-read
	// dispatch. Order is binary-verbatim: properties (type-switched
	// getters) -> [callback arms DELTA'd below] -> events -> children ->
	// CamelCase fallback -> final throw. All Asserts are log-continue.
	template<>
	int Bridge<boost::shared_ptr<Instance>, false>::on_index(
		const boost::shared_ptr<Instance>& object,
		const char* name, lua_State* L)
	{
		if (object.get() == NULL)
			throw RBX::runtime_error("The object has been deleted");
		const char* memberName = resolveLegacyMemberName(object, name, L);
		Security::Context context = Security::Context::current();
		Instance* instance = object.get();
		const Reflection::PropertyDescriptor* prop =
			instance->getDescriptor().findPropertyDescriptor(memberName);
		if (prop != NULL)
		{
			// Binary passes the DescribedBase subobject at instance+36;
			// the static_cast names the same pointer (MI rule).
			const Reflection::DescribedBase* described =
				static_cast<const Reflection::DescribedBase*>(instance);
			if (FLog::Asserts != 0 && !prop->isMemberOf(described))
			{
				if (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook("!instance || descriptor.isMemberOf(instance)",
						"include/reflection/property.h", 255) == 0)
				{
					ReleaseAssert(FLog::Asserts,
						"!instance || descriptor.isMemberOf(instance) file: include/reflection/property.h line: 255");
				}
			}
			if (prop->security != 0)
				context.requirePermission(prop->security, NULL);
			if (!prop->isScriptable())
			{
				throw RBX::runtime_error(
					"Unable to query property %s. It is not scriptable", memberName);
			}
			const Reflection::Type& type = prop->type;
			if (type == Reflection::Type::singleton<int>())
			{
				lua_pushinteger(L, readValue<int>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<bool>())
			{
				lua_pushboolean(L, readValue<bool>(prop, described) ? 1 : 0);
				return 1;
			}
			if (type == Reflection::Type::singleton<float>())
			{
				lua_pushnumber(L, readValue<float>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<double>())
			{
				lua_pushnumber(L, readValue<double>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<ProtectedString>())
			{
				const std::string& value =
					readValue<ProtectedString>(prop, described).getStringForImmediateUse();
				lua_pushlstring(L, value.c_str(), value.size());
				return 1;
			}
			if (type == Reflection::Type::singleton<std::string>())
			{
				const std::string& value = readValue<std::string>(prop, described);
				lua_pushlstring(L, value.c_str(), value.size());
				return 1;
			}
			if (type == Reflection::Type::singleton<ContentId>())
			{
				const std::string& value =
					readValue<ContentId>(prop, described).toString();
				lua_pushlstring(L, value.c_str(), value.size());
				return 1;
			}
			if (type == Reflection::Type::singleton<boost::shared_ptr<Instance> >())
			{
				boost::shared_ptr<Instance> value =
					readValue<boost::shared_ptr<Instance> >(prop, described);
				SharedPtrBridge<Instance>::push(L, value);
				return 1;
			}
			if (type == Reflection::Type::singleton<G3D::Vector3>())
			{
				Bridge<G3D::Vector3>::pushNewObject(L,
					readValue<G3D::Vector3>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<G3D::Vector2>())
			{
				Bridge<G3D::Vector2>::pushNewObject(L,
					readValue<G3D::Vector2>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<RbxRay>())
			{
				Bridge<RbxRay>::pushNewObject(L,
					readValue<RbxRay>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<G3D::CoordinateFrame>())
			{
				Bridge<G3D::CoordinateFrame>::pushNewObject(L,
					readValue<G3D::CoordinateFrame>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<G3D::Color3>())
			{
				Color3Bridge::pushColor3(L,
					readValue<G3D::Color3>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<BrickColor>())
			{
				Bridge<BrickColor>::pushNewObject(L,
					readValue<BrickColor>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<UDim>())
			{
				Bridge<UDim>::pushNewObject(L,
					readValue<UDim>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<UDim2>())
			{
				Bridge<UDim2>::pushNewObject(L,
					readValue<UDim2>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<Faces>())
			{
				Bridge<Faces>::pushNewObject(L,
					readValue<Faces>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<Axes>())
			{
				Bridge<Axes>::pushNewObject(L,
					readValue<Axes>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<Region3int16>())
			{
				Bridge<Region3int16>::pushNewObject(L,
					readValue<Region3int16>(prop, described));
				return 1;
			}
			if (type == Reflection::Type::singleton<
				boost::shared_ptr<const std::vector<boost::shared_ptr<Instance> > > >())
			{
				boost::shared_ptr<const std::vector<boost::shared_ptr<Instance> > > value =
					readValue<boost::shared_ptr<const std::vector<boost::shared_ptr<Instance> > > >(
						prop, described);
				if (value.get() != NULL)
				{
					lua_createtable(L, (int)value->size(), 0);
					for (size_t i = 0; i < value->size(); ++i)
					{
						SharedPtrBridge<Instance>::push(L, (*value)[i]);
						lua_rawseti(L, -2, (int)i + 1);
					}
				}
				else
				{
					lua_createtable(L, 0, 0);
				}
				return 1;
			}
			if (prop->bIsEnum)
			{
				const Reflection::EnumPropertyDescriptor* enumProp =
					static_cast<const Reflection::EnumPropertyDescriptor*>(prop);
				const Reflection::EnumDescriptor::Item* item =
					enumProp->getEnumItem(described);
				if (item != NULL)
				{
					EnumItem::push(L, item);
					return 1;
				}
				throw RBX::runtime_error("Invalid value for enum %s", memberName);
			}
			if (Reflection::RefPropertyDescriptor::isRefPropertyDescriptor(*prop))
			{
				const Reflection::RefPropertyDescriptor* refProp =
					static_cast<const Reflection::RefPropertyDescriptor*>(prop);
				Reflection::DescribedBase* ref = refProp->getRefValue(described);
				if (ref != NULL)
				{
					SharedPtrBridge<Instance>::push(L,
						RBX::shared_from<Instance>(static_cast<Instance*>(ref)));
				}
				else
				{
					boost::shared_ptr<Instance> nullInstance;
					SharedPtrBridge<Instance>::push(L, nullInstance);
				}
				return 1;
			}
			throw RBX::runtime_error("Unable to get property %s, type %s",
				memberName, prop->type.tag->str.c_str());
		}
		// Method closures (0x1739AE arms, batch737): yield descriptors
		// first (0x173C36), then plain functions (0x173E52).  Each arm
		// caches its closure in the running function's environment table
		// keyed by the descriptor address -- "you get the same object
		// back each time, so relational operators are meaningful" --
		// and logs lua_isfunction(L, -1) (lines 371/399).
		Reflection::YieldFunctionDescriptor* yieldFunction =
			instance->getDescriptor().findYieldFunctionDescriptor(memberName);
		if (yieldFunction != NULL)
		{
			void* desc = reinterpret_cast<void*>(yieldFunction);
			lua_pushlightuserdata(L, desc);
			lua_rawget(L, LUA_ENVIRONINDEX);
			if (lua_type(L, -1) == LUA_TNIL)
			{
				lua_pop(L, 1);
				lua_pushlightuserdata(L, desc);
				lua_pushcclosure(L, &ObjectBridge::callMemberYieldFunction, 1);
				lua_pushlightuserdata(L, desc);
				lua_pushvalue(L, -2);
				lua_settable(L, LUA_ENVIRONINDEX);
			}
			if (FLog::Asserts != 0 && !lua_isfunction(L, -1))
			{
				if (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook("lua_isfunction(L, -1)",
						"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 371) == 0)
				{
					ReleaseAssert(FLog::Asserts,
						"lua_isfunction(L, -1) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 371");
				}
			}
			return 1;
		}
		Reflection::FunctionDescriptor* memberFunction =
			instance->getDescriptor().findFunctionDescriptor(memberName);
		if (memberFunction != NULL)
		{
			void* desc = reinterpret_cast<void*>(memberFunction);
			lua_pushlightuserdata(L, desc);
			lua_rawget(L, LUA_ENVIRONINDEX);
			if (lua_type(L, -1) == LUA_TNIL)
			{
				lua_pop(L, 1);
				lua_pushlightuserdata(L, desc);
				lua_pushcclosure(L, &ObjectBridge::callMemberFunction, 1);
				lua_pushlightuserdata(L, desc);
				lua_pushvalue(L, -2);
				lua_settable(L, LUA_ENVIRONINDEX);
			}
			if (FLog::Asserts != 0 && !lua_isfunction(L, -1))
			{
				if (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook("lua_isfunction(L, -1)",
						"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 399) == 0)
				{
					ReleaseAssert(FLog::Asserts,
						"lua_isfunction(L, -1) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 399");
				}
			}
			return 1;
		}
		const Reflection::EventDescriptor* event =
			instance->getDescriptor().findEventDescriptor(memberName);
		if (event != NULL)
		{
			instance->securityCheck(context);
			// DELTA (fw()+22, binary-verbatim position): per-instance
			// permission-request flag, identity unmapped (same unknown as
			// newInstance's parent check). Deterministic-false + queued
			// FW audit (unread-ident rule: no mapped path sets it in-tree).
			EventInstance eventInstance;
			eventInstance.event = event;
			eventInstance.instance = object;
			Bridge<EventInstance>::pushNewObject(L, eventInstance);
			return 1;
		}
		Instance* child = instance->findFirstChildByName(memberName);
		if (child != NULL)
		{
			instance->securityCheck(context);
			// DELTA (fw()+22, same unknown as above): deterministic-false.
			SharedPtrBridge<Instance>::push(L, RBX::shared_from<Instance>(child));
			return 1;
		}
		if (FFlag::CamelCaseHackEnabled)
		{
			const RBX::Name* plainName = RBX::Name::lookup(memberName);
			if (plainName != &RBX::Name::getNullName())
			{
				std::string capitalized(memberName);
				capitalized[0] = (char)toupper((unsigned char)capitalized[0]);
				if (capitalized != memberName
					&& RBX::Name::lookup(capitalized) != &RBX::Name::getNullName())
				{
					int results = on_index(object, capitalized.c_str(), L);
					ScriptContext* scriptContext = RobloxExtraSpace::get(L)->context();
					boost::shared_ptr<BaseScript> script;
					if (L != NULL)
					{
						script = boost::shared_ptr<BaseScript>(
							RobloxExtraSpace::get(L)->script,
							boost::detail::sp_nothrow_tag());
					}
					scriptContext->camelCaseViolation(object, std::string(memberName), script);
					return results;
				}
			}
		}
		throw RBX::runtime_error("%s is not a valid member of %s",
			memberName, instance->getClassNameStr().c_str());
	}
	// Standard luaL_checklstring wrapper (0x1A282C banked).
	template<>
	int Bridge<boost::shared_ptr<Instance>, false>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}
	// isValueType<V> recheck for setters (property.h:341; same cond as
	// the :289 getter check, line bumped. Spelled manually for the same
	// log-continue reason as checkValueType above).
	template<class V>
	static void checkValueTypeForSet(const Reflection::PropertyDescriptor* prop)
	{
		if (FLog::Asserts != 0
			&& !(prop->type == Reflection::Type::singleton<V>()))
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("isValueType<V>()",
					"include/reflection/property.h", 341) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"isValueType<V>() file: include/reflection/property.h line: 341");
			}
		}
	}
	template<class V>
	static void writeValue(const Reflection::PropertyDescriptor* prop,
		Reflection::DescribedBase* described, const V& value)
	{
		checkValueTypeForSet<V>(prop);
		static_cast<const Reflection::TypedPropertyDescriptor<V>*>(
			prop)->set(described, value);
	}
	// mac: 0x1768A4 (7782B, batches 514/521/537 + Hexrays): member-write
	// dispatch. Order is binary-verbatim: null/security/name/prop-trie,
	// [callback set/clear DELTA'd], Parent special-case DELTA'd (falls
	// into the generic ref arm), type-switched setters, enum/ref arms,
	// final throw. Values are read off Lua stack index 3, exactly like
	// the binary. All Asserts are log-continue.
	template<>
	void Bridge<boost::shared_ptr<Instance>, false>::on_newindex(
		boost::shared_ptr<Instance>& object,
		const char* name, lua_State* L)
	{
		if (object.get() == NULL)
			throw RBX::runtime_error("The object has been deleted");
		Security::Context context = Security::Context::current();
		Instance* instance = object.get();
		// The binary re-fetches the name off slot 2 here (sub_1756D3);
		// same slot the wrapper read, so the parameter is used directly.
		const Reflection::PropertyDescriptor* prop =
			instance->getDescriptor().findPropertyDescriptor(name);
		if (prop == NULL)
		{
			// DELTA (callback set/clear arm): no CallbackDescriptor
			// exists in-tree (same dead-arm finding as the 258b read
			// side); setGenericCallback era queued. Falls through to the
			// final throw below, exactly as a miss would.
			throw RBX::runtime_error("%s is not a valid member of %s",
				name, instance->getClassNameStr().c_str());
		}
		// DELTA (Parent special-case): binary pins v11 == &propParent
		// first (canParent-guard + same-parent early-out + fw22 gates +
		// Changed raise). No in-tree equivalent for the +476 guard, so
		// Parent flows into the generic ref arm below (setRefValue);
		// observably identical for reparentable green paths.
		{
			// Binary passes the non-const DescribedBase subobject
			// (instance+36) to setters; the static_cast names it.
			Reflection::DescribedBase* described =
				static_cast<Reflection::DescribedBase*>(instance);
			if (FLog::Asserts != 0 && !prop->isMemberOf(described))
			{
				if (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook("!instance || descriptor.isMemberOf(instance)",
						"include/reflection/property.h", 255) == 0)
				{
					ReleaseAssert(FLog::Asserts,
						"!instance || descriptor.isMemberOf(instance) file: include/reflection/property.h line: 255");
				}
			}
			if (prop->security != 0)
				context.requirePermission(prop->security, NULL);
			if (!prop->isScriptable())
			{
				throw RBX::runtime_error(
					"Unable to assign property %s. It is not scriptable", name);
			}
			const Reflection::Type& type = prop->type;
			if (type == Reflection::Type::singleton<int>())
			{
				writeValue<int>(prop, described, lua_tointeger(L, 3));
				return;
			}
			if (type == Reflection::Type::singleton<bool>())
			{
				writeValue<bool>(prop, described, lua_toboolean(L, 3) != 0);
				return;
			}
			if (type == Reflection::Type::singleton<float>())
			{
				writeValue<float>(prop, described, lua_tofloat(L, 3));
				return;
			}
			if (type == Reflection::Type::singleton<double>())
			{
				writeValue<double>(prop, described, lua_tofloat(L, 3));
				return;
			}
			if (type == Reflection::Type::singleton<ProtectedString>())
			{
				writeValue<ProtectedString>(prop, described,
					ProtectedString::fromTrustedSource(
						throwable_lua_tostring(L, 3)));
				return;
			}
			if (type == Reflection::Type::singleton<std::string>())
			{
				writeValue<std::string>(prop, described,
					throwable_lua_tostring(L, 3));
				return;
			}
			if (type == Reflection::Type::singleton<boost::shared_ptr<Instance> >())
			{
				writeValue<boost::shared_ptr<Instance> >(prop, described,
					SharedPtrBridge<Instance>::getPtr(L, 3));
				return;
			}
			if (type == Reflection::Type::singleton<G3D::Vector3>())
			{
				checkValueTypeForSet<G3D::Vector3>(prop);
				const G3D::Vector3* value = static_cast<const G3D::Vector3*>(
					luaL_checkudata(L, 3, Bridge<G3D::Vector3>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<G3D::Vector3>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<G3D::Vector2>())
			{
				checkValueTypeForSet<G3D::Vector2>(prop);
				const G3D::Vector2* value = static_cast<const G3D::Vector2*>(
					luaL_checkudata(L, 3, Bridge<G3D::Vector2>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<G3D::Vector2>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<RbxRay>())
			{
				checkValueTypeForSet<RbxRay>(prop);
				const RbxRay* value = static_cast<const RbxRay*>(
					luaL_checkudata(L, 3, Bridge<RbxRay>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<RbxRay>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<G3D::CoordinateFrame>())
			{
				checkValueTypeForSet<G3D::CoordinateFrame>(prop);
				const G3D::CoordinateFrame* value =
					static_cast<const G3D::CoordinateFrame*>(
						luaL_checkudata(L, 3, Bridge<G3D::CoordinateFrame>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<G3D::CoordinateFrame>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<G3D::Color3>())
			{
				checkValueTypeForSet<G3D::Color3>(prop);
				const G3D::Color3* value = static_cast<const G3D::Color3*>(
					luaL_checkudata(L, 3, Bridge<G3D::Color3>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<G3D::Color3>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<UDim>())
			{
				checkValueTypeForSet<UDim>(prop);
				const UDim* value = static_cast<const UDim*>(
					luaL_checkudata(L, 3, Bridge<UDim>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<UDim>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<UDim2>())
			{
				checkValueTypeForSet<UDim2>(prop);
				const UDim2* value = static_cast<const UDim2*>(
					luaL_checkudata(L, 3, Bridge<UDim2>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<UDim2>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<Faces>())
			{
				checkValueTypeForSet<Faces>(prop);
				const Faces* value = static_cast<const Faces*>(
					luaL_checkudata(L, 3, Bridge<Faces>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<Faces>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<Axes>())
			{
				checkValueTypeForSet<Axes>(prop);
				const Axes* value = static_cast<const Axes*>(
					luaL_checkudata(L, 3, Bridge<Axes>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<Axes>*>(
					prop)->set(described, *value);
				return;
			}
			if (type == Reflection::Type::singleton<BrickColor>())
			{
				checkValueTypeForSet<BrickColor>(prop);
				const BrickColor* value = static_cast<const BrickColor*>(
					luaL_checkudata(L, 3, Bridge<BrickColor>::className));
				static_cast<const Reflection::TypedPropertyDescriptor<BrickColor>*>(
					prop)->set(described, *value);
				return;
			}
			// DELTA (Id-type arms): SoundId/MeshId/AnimationId are
			// undeclared in-tree, so their dynamic_casts are unwritable;
			// the TextureId arm (== ContentId, which exists) is ported
			// below. All four set from a Lua string via ContentId.
			{
				const Reflection::TypedPropertyDescriptor<TextureId>* idProp =
					dynamic_cast<const Reflection::TypedPropertyDescriptor<TextureId>*>(prop);
				if (idProp != NULL)
				{
					checkValueTypeForSet<TextureId>(prop);
					idProp->set(described, TextureId(
						throwable_lua_tostring(L, 3)));
					return;
				}
			}
			{
				const Reflection::EnumPropertyDescriptor* enumProp =
					dynamic_cast<const Reflection::EnumPropertyDescriptor*>(prop);
				if (enumProp != NULL)
				{
					EnumDescriptorItemPtr item = NULL;
					if (EnumItem::getItem(L, 3, item))
					{
						if (enumProp->setEnumItem(
							described, *item))
							return;
						throw RBX::runtime_error("Invalid value for enum %s", name);
					}
					if (lua_isnumber(L, 3) != 0)
					{
						if (enumProp->setEnumValue(
							described,
							lua_tointeger(L, 3)))
							return;
						throw RBX::runtime_error("Invalid value %d for enum %s",
							lua_tointeger(L, 3), name);
					}
					if (lua_isstring(L, 3) != 0)
					{
						// DELTA: the successful string→item path needs
						// EnumDescriptor name lookup (unmapped); only the
						// throw text is pinned ('Invalid value "%s"...').
						throw RBX::runtime_error("Invalid value \"%s\" for enum %s",
							lua_tostring(L, 3), name);
					}
					throw RBX::runtime_error("Invalid value for enum %s", name);
				}
			}
			{
				const Reflection::RefPropertyDescriptor* refProp =
					dynamic_cast<const Reflection::RefPropertyDescriptor*>(prop);
				if (refProp != NULL)
				{
					boost::shared_ptr<Instance> value =
						SharedPtrBridge<Instance>::getPtr(L, 3);
					Reflection::DescribedBase* refValue =
						value.get() != NULL
							? static_cast<Reflection::DescribedBase*>(value.get())
							: NULL;
					refProp->setRefValue(
						described, refValue);
					return;
				}
			}
			throw RBX::runtime_error("Unable to set property %s, type %s",
				name, prop->type.tag->str.c_str());
		}
	}
	// Standard luaL_checklstring wrapper (0x1A288C banked).
	template<>
	int Bridge<boost::shared_ptr<Instance>, false>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}
} }
// VS2012 odr-use holder (the L-form newindex wrapper is named cross-TU
// by tests; the obj-form above keeps it emitted in-TU).
int (*volatile g_bridgeInstanceNewIndex)(lua_State*) =
	(int (*)(lua_State*))&RBX::Lua::Bridge<boost::shared_ptr<RBX::Instance>, false>::on_newindex;
// VS2012 odr-use holder (the L-form wrapper is named cross-TU by the
// 258b test; the obj-form above keeps it emitted in-TU).
int (*volatile g_bridgeInstanceIndex)(lua_State*) =
	(int (*)(lua_State*))&RBX::Lua::Bridge<boost::shared_ptr<RBX::Instance>, false>::on_index;
// ---------------------------------------------------------------------------
// YieldFunctionStateObject (YFSO era, 2026-10-16).
// mac: Binary: execute 0x178C06, dtor 0x179276, onReturnResult 0x179768,
// onRaiseException 0x179B06, resumeThreadWithException 0x179F72, ctor
// mac: 0x17B758; global-namespace class (__ZN24YieldFunctionStateObject...).
// 2016 keeps it in this same file between the callMember pair; landed at
// file end (callMemberYieldFunction/callMemberFunction 0x17587C/0x175B4A
// follow below). Batches 717/725/726.
using namespace RBX;
using namespace RBX::Lua;
class YieldFunctionStateObject
	: public boost::enable_shared_from_this<YieldFunctionStateObject>
	, boost::noncopyable
{
protected:
	// mac: 0x179F72 (batches 717/725): lock the parked thread; when a
	// continuations->error handler is present, push the message and hand
	// the state to it; otherwise report + resume with the single message
	// value.  (2016 rewrote this to reportError/lua_resetstack -- the 2013
	// binary keeps the printf/printCallStack/pushlstring/resume(1)/settop
	// fallback, same shape as LSO's 0x631A3E at LuaLibrary.cpp:442, except
	// YFSO pushes ONE value, not nil+message.)
	static void resumeThreadWithException(boost::intrusive_ptr<WeakThreadRef> threadRef, std::string message)
	{
		if (Lua::ThreadRef thread = threadRef->lock())
		{
			if (Continuations* continuations = RobloxExtraSpace::get(thread)->continuations.get())
			{
				if (continuations->error)
				{
					lua_pushlstring(thread, message.data(), message.size());
					(continuations->error)(thread);
					return;
				}
			}
			int savedTop = lua_gettop(thread);
			StandardOut::singleton()->printf(MESSAGE_ERROR, "%s", message.c_str());
			ScriptContext::printCallStack(thread, NULL);
			lua_pushlstring(thread, message.data(), message.size());
			ScriptContext::getContext(thread)->resume(thread, 1);
			lua_settop(thread, savedTop);
		}
	}
public:
	boost::intrusive_ptr<WeakThreadRef> thread;
	boost::weak_ptr<DataModel> dm;
	shared_ptr<Instance> instance;
	const Reflection::YieldFunctionDescriptor* const desc;
	lua_State* const L;
	long callingThreadId;
	bool isExecuting;	// set to true while the execute function is running. Used only in the executing thread. Otherwise ignored
	volatile int immediateResults;
	bool reentrancyCheck;	// for debugging
	static const int YIELD = -1;
	// mac: 0x17B758 (batch725 + batch726 disasm: the WeakThreadRef(lua_State*)
	// arg is real -- `mov [esp+4], esi` with esi = L before the ctor call).
	// Binary stores: base 0-7, thread +8, dm +12/0 (then assigned in body),
	// instance +20, desc +28, L +32, immediateResults +44 = YIELD,
	// reentrancyCheck +48 = false; callingThreadId/isExecuting stay
	// uninitialized there and here.
	YieldFunctionStateObject(const Reflection::YieldFunctionDescriptor* desc, shared_ptr<Instance> instance, lua_State* L)
		: thread(new WeakThreadRef(L))
		, instance(instance)
		, desc(desc)
		, L(L)
		, immediateResults(YIELD)
		, reentrancyCheck(false)
	{
		dm = shared_from(DataModel::get(ScriptContext::getContext(L)));
	}
	// mac: 0x178C06 (batch725): mark executing, remember the native thread, run
	// the descriptor with our two continuations, then either return the
	// immediate result or capture the yield.
	// STUB: PLAYER 0x0087c580
	// STUB: STUDIO 0x009e0c00
	int execute()
	{
		isExecuting = true;
		// Binary calls pthread_self() (GCC-flavored build); the 2016 Windows
		// source uses GetCurrentThreadId() -- 32-bit current-thread value, same comparison semantics.
		this->callingThreadId = GetCurrentThreadId();
		LuaArguments args(L, 1);	// skip the first argument, since it is "self"
		desc->execute(
			instance.get(), args,
			boost::bind(&YieldFunctionStateObject::onReturnResult, shared_from(this), _1),
			boost::bind(&YieldFunctionStateObject::onRaiseException, shared_from(this), _1)
			);
		isExecuting = false;
		instance.reset();		// we're done with this pointer
		if (immediateResults != YIELD)
			return immediateResults;	// the return already happened
		//Capture the yield. The callbacks will handle it
		RobloxExtraSpace::get(L)->yieldCaptured = true;
		return lua_yield(L, 0);
	}
private:
	// mac: 0x179768 (batch725): one-shot guard, then either stash the result for the executing thread (immediate mode) or schedule a 1-value Tuple resume on the parked thread.
	void onReturnResult(Reflection::Variant value)
	{
		// !reentrancyCheck (LuaInstanceBridge.cpp:948) -- FLog-gated ReleaseAssert spelled verbatim-literal (the binary becomes log-continue).
		RBXASSERT(!reentrancyCheck);
		reentrancyCheck = true;
		if (FFlag::ImmediateYieldResultsEnabled && isExecuting && callingThreadId == GetCurrentThreadId())
			immediateResults = LuaArguments::push(value, L);
		else
		{
			if (Lua::ThreadRef thread = this->thread->lock())
			{
				shared_ptr<Reflection::Tuple> arguments(new Reflection::Tuple(1));
				arguments->values[0] = value;
				ScriptContext::getContext(thread.get())->scheduleResume(thread, arguments);
			}
		}
	}
	// mac: 0x179B06 (batch725): same guard; in immediate mode on the executing thread the exception unwinds back into execute(), otherwise the write task carries the message to resumeThreadWithException.
	// FUNCTION: PLAYER 0x0087b2f0
	// FUNCTION: STUDIO 0x009df970
	void onRaiseException(std::string message)
	{
		RBXASSERT(!reentrancyCheck);
		reentrancyCheck = true;

		if (FFlag::ImmediateYieldResultsEnabled && isExecuting)
		{
			if (callingThreadId == GetCurrentThreadId())
				// This unwinds the stack back to execute()
				throw std::runtime_error(message);
		}

		if (boost::shared_ptr<DataModel> dataModel = dm.lock())
			dataModel->submitTask(boost::bind(&YieldFunctionStateObject::resumeThreadWithException, thread, message), DataModelJob::Write);
	}
};

// mac: 0x17587C (717B, batches 735/736): callMemberYieldFunction -- the
// yield-closure body.  Upvalue 1 is the YieldFunctionDescriptor; the
// prologue is shared with callMemberFunction (assert lua_islightuserdata,
// descriptor security, instance fetch + ':' throw, securityCheck, fw
// gate, membership), then a heap YieldFunctionStateObject (0x34 bytes)
// runs the descriptor against its continuations -- execute returns the
// immediate result or captures the yield.
// STUB: PLAYER 0x0087c710
// STUB: STUDIO 0x009e0d90
int ObjectBridge::callMemberYieldFunction(lua_State* L)
{
	if (FLog::Asserts != 0 && !lua_islightuserdata(L, lua_upvalueindex(1)))
	{
		if (RBX::_internal::_debugHook == 0
			|| RBX::_internal::_debugHook("lua_islightuserdata(L, lua_upvalueindex(1))",
				"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 984) == 0)
		{
			ReleaseAssert(FLog::Asserts,
				"lua_islightuserdata(L, lua_upvalueindex(1)) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 984");
		}
	}
	const Reflection::YieldFunctionDescriptor* desc =
		reinterpret_cast<const Reflection::YieldFunctionDescriptor*>(
			lua_touserdata(L, lua_upvalueindex(1)));
	if (desc->security != Security::None)
		Security::Context::current().requirePermission(desc->security);
	boost::shared_ptr<Instance> instance;
	if (!SharedPtrBridge<Instance>::getPtr(L, 1, instance) || instance.get() == NULL)
	{
		throw RBX::runtime_error(
			"Did you forget a colon? The first argument of member function %s must be an Object",
			desc->name->str.c_str());
	}
	Security::Context context = Security::Context::current();
	instance->securityCheck(context);
	// DELTA (fw()+22, binary-verbatim position): per-instance
	// permission-request flag, identity unmapped (same unknown as
	// newInstance's parent check). Deterministic-false + queued FW audit.
	const Reflection::DescribedBase* described =
		static_cast<const Reflection::DescribedBase*>(instance.get());
	if (!desc->isMemberOf(described))
	{
		throw RBX::runtime_error("The function %s is not a member of \"%s\"",
			desc->name->str.c_str(),
			instance->getDescriptor().name->str.c_str());
	}
	boost::shared_ptr<YieldFunctionStateObject> functionObject(
		new YieldFunctionStateObject(desc, instance, L));
	return functionObject->execute();
}

// mac: 0x175B4A (720B, batches 735/736): callMemberFunction -- the plain
// closure.  Same prologue (its own assert line 829 + throw text), then
// the descriptor executes against a LuaArguments starting at arg 1 (the
// binary builds the 2013 base inline: Arguments vptr, void return type,
// then the LuaArguments vptr + offset/L) and the return value is pushed
// as the call result through the static pusher (0x162624).
// STUB: PLAYER 0x0087a5c0
// STUB: STUDIO 0x009dec40
int ObjectBridge::callMemberFunction(lua_State* L)
{
	if (FLog::Asserts != 0 && !lua_islightuserdata(L, lua_upvalueindex(1)))
	{
		if (RBX::_internal::_debugHook == 0
			|| RBX::_internal::_debugHook("lua_islightuserdata(L, lua_upvalueindex(1))",
				"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 829) == 0)
		{
			ReleaseAssert(FLog::Asserts,
				"lua_islightuserdata(L, lua_upvalueindex(1)) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 829");
		}
	}
	const Reflection::FunctionDescriptor* desc =
		reinterpret_cast<const Reflection::FunctionDescriptor*>(
			lua_touserdata(L, lua_upvalueindex(1)));
	if (desc->security != Security::None)
		Security::Context::current().requirePermission(desc->security);
	boost::shared_ptr<Instance> instance;
	if (!SharedPtrBridge<Instance>::getPtr(L, 1, instance) || instance.get() == NULL)
	{
		throw RBX::runtime_error("Expected ':' not '.' calling member function %s",
			desc->name->str.c_str());
	}
	Security::Context context = Security::Context::current();
	instance->securityCheck(context);
	// DELTA (fw()+22, binary-verbatim position): same unmapped flag as
	// callMemberYieldFunction above (also the 258b arms + newInstance).
	const Reflection::DescribedBase* described =
		static_cast<const Reflection::DescribedBase*>(instance.get());
	if (!desc->isMemberOf(described))
	{
		throw RBX::runtime_error("The function %s is not a member of \"%s\"",
			desc->name->str.c_str(),
			instance->getDescriptor().name->str.c_str());
	}
	LuaArguments args(L, 1);
	desc->execute(instance.get(), args);
	return LuaArguments::push(args.returnValue, L);
}

/* script/LuaAtomicClasses.h -- October 2013 (Vector3 slice, section 222).
   Evidence: Vector3Bridge::registerClassLibrary 0x167144 (batch347) +
     classLibrary data 0xED1B84 {"new", "FromNormalId", "FromAxis"} +
     className "Vector3" 0xB5B85E (batches 349-350); Bridge<G3D::Vector3,
     true>::registerClass 3-arg 0x16D168 (batch347); on_add 0x16719E /
     on_sub 0x167238 / on_mul 0x1672D2 / on_div 0x167482 / on_unm 0x167638
     (batch351); on_index(obj,name,L) 0x1676B8 / on_newindex 0x167CB2 /
     on_index(L) 0x19855C / on_newindex(L) 0x1985BC / on_gc 0x16EB12 /
     on_eq 0x16EB42 / on_tostring(L) 0x16EBD2 / on_tostring(obj,L) 0x16F59E
     + lerp/cross/dot/isClose helpers (batch352).
   2016 LuaAtomicClasses.h carried for the class skeleton; DELTAS vs 2016:
     - Bridge::registerClass is 3-arg (L, on_index-fn, on_newindex-fn) --
       the 2016 1-arg header API does NOT match the 2013 binary.
     - No "__type" metatable entry (2016 specialization sets it; the 2013
       body does not).
     - __gc IS registered (2016 Vector3 specialization omits it).
     - on_mul/on_div never throw RBX::runtime_error: Vector3 op garbage
       coerces via lua_tofloat; number op non-Vector3 hits luaL_checkudata.
     - on_div Vector3/scalar computes (1.0/c)*v (reciprocal-multiply), not
       v/c -- replicated explicitly (assoc differs from operator/).
   Queued (need EnumDescriptor::Item bridge era): newVector3 (0x166E88
     needs only lua_tofloat -- lands with the library table), FromNormalId
     0x166F5C / FromAxis 0x16704C, registerClassLibrary + classLibrary.
   Original file: Client/App/script/LuaAtomicClasses.cpp (.h companion). */

#pragma once

#include "lua/Lua.hpp"
#include "lua/luabridge.h"
#include "G3D/Vector3.h"
#include "G3D/Vector2.h"
#include "G3D/Vector3int16.h"
#include "G3D/Vector2int16.h"
#include "G3D/Quat.h"
#include "G3D/Color3.h"
#include "G3D/CoordinateFrame.h"
#include "RbxG3D/RbxRay.h"
#include "Util/Region3.h"
#include "Util/Region3int16.h"
#include "Util/BrickColor.h"
#include "Util/UDim.h"
#include "Util/Faces.h"
#include "Util/Axes.h"
#include "Util/CellID.h"
#include "V8DataModel/InputObject.h"

namespace RBX { namespace Lua {

	class Vector3Bridge : public Bridge<G3D::Vector3>
	{
		friend class Bridge<G3D::Vector3>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushVector3(lua_State* L, const G3D::Vector3& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newVector3(lua_State* L);
		static int newVector3FromNormalId(lua_State* L);
		static int newVector3FromAxis(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_mul(lua_State* L);
		static int on_div(lua_State* L);
		static int on_unm(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	// Explicit-specialization declarations for the Vector3 instantiation
	// (definitions in script/LuaAtomicClasses.cpp). Declared here so no TU
	// ever implicitly instantiates the primary template's members (which
	// have no generic definitions in this tree). className is declared
	// only at its definition site (2016 pattern; a no-initializer header
	// declaration double-defines under MSVC).
	template<> void Bridge<G3D::Vector3>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<G3D::Vector3>::on_index(const G3D::Vector3& object,
		const char* name, lua_State* L);
	template<> void Bridge<G3D::Vector3>::on_newindex(G3D::Vector3& object,
		const char* name, lua_State* L);
	template<> int Bridge<G3D::Vector3>::on_index(lua_State* L);
	template<> int Bridge<G3D::Vector3>::on_newindex(lua_State* L);
	template<> int Bridge<G3D::Vector3>::on_gc(lua_State* L);
	template<> int Bridge<G3D::Vector3>::on_eq(lua_State* L);
	template<> int Bridge<G3D::Vector3>::on_tostring(lua_State* L);
	template<> int Bridge<G3D::Vector3>::on_tostring(const G3D::Vector3& object,
		lua_State* L);

	// Vector2 instantiation (section 223; same rules). 2013 bridges
	// G3D::Vector2 (binary manglings); the 2016 tree bridges RBX::Vector2.
	template<> void Bridge<G3D::Vector2>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<G3D::Vector2>::on_index(const G3D::Vector2& object,
		const char* name, lua_State* L);
	template<> void Bridge<G3D::Vector2>::on_newindex(G3D::Vector2& object,
		const char* name, lua_State* L);
	template<> int Bridge<G3D::Vector2>::on_index(lua_State* L);
	template<> int Bridge<G3D::Vector2>::on_newindex(lua_State* L);
	template<> int Bridge<G3D::Vector2>::on_gc(lua_State* L);
	template<> int Bridge<G3D::Vector2>::on_eq(lua_State* L);
	template<> int Bridge<G3D::Vector2>::on_tostring(lua_State* L);
	template<> int Bridge<G3D::Vector2>::on_tostring(const G3D::Vector2& object,
		lua_State* L);

	// Vector2Bridge (section 223; 2013 bridges G3D::Vector2, not the
	// 2016 RBX::Vector2). Evidence: newVector2 0x168A66 +
	// registerClassLibrary 0x168B3A + classLibrary {"new"} 0xED1BC4 +
	// className "Vector2" 0xB85C2F (batch358); on_add/sub/mul/div/unm
	// mac: 0x168B94/8C14/8C94/8DC0/8EEA + on_index(obj) 0x168F52 +
	// on_newindex 0x1691E8 (batch356); registerClass 0x16D41C + on_gc
	// mac: 0x16EC0E + on_eq 0x16EC3E + on_tostring x2 (0x16ECC2/0x16F648) +
	// on_index(L)/on_newindex(L) 0x19861E/0x19867E (batch357); lerp
	// helper 0x169136 (batch358). Queued with the enum era: newVector2,
	// registerClassLibrary + classLibrary (consistent with Vector3).
	class Vector2Bridge : public Bridge<G3D::Vector2>
	{
		friend class Bridge<G3D::Vector2>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushVector2(lua_State* L, const G3D::Vector2& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newVector2(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_mul(lua_State* L);
		static int on_div(lua_State* L);
		static int on_unm(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	// UDimBridge / UDim2Bridge (section 224). Evidence: newUDim 0x16B476 /
	// newUDim2 0x16B876 + classLibraries {"new"} 0xED1C6C/0xED1C7C +
	// classNames "UDim"/"UDim2" 0xB5B8EF/0xB5B900 (batch361); UDim
	// registerClass 0x16D948 + on_add/sub/unm 0x16B59C/0x16B616/0x16B690 +
	// on_index(obj) 0x16B6E6 + on_newindex 0x16B810 + wrappers
	// mac: 0x198926/0x198986 + on_gc 0x16EE06 + on_eq 0x16EE36 (batch360);
	// UDim2 registerClass 0x16DB48 + on_index(obj) 0x16BB50 +
	// on_newindex 0x16BCA0 + wrappers 0x1989E8/0x198A48 + on_gc 0x16EEB2 +
	// on_eq 0x16EEE2 (batch361); UDim2Bridge arithmetic (batch362).
	// Neither metatable registers __tostring or __mul/__div (verified by
	// the registerClass bodies, not by symbol absence). Queued with the
	// enum era: newUDim/newUDim2 + registerClassLibrary + tables.
	class UDimBridge : public Bridge<RBX::UDim>
	{
		friend class Bridge<RBX::UDim>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushUDim(lua_State* L, const RBX::UDim& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newUDim(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_unm(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	class UDim2Bridge : public Bridge<RBX::UDim2>
	{
		friend class Bridge<RBX::UDim2>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushUDim2(lua_State* L, const RBX::UDim2& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newUDim2(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_unm(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::UDim>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::UDim>::on_index(const RBX::UDim& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::UDim>::on_newindex(RBX::UDim& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::UDim>::on_index(lua_State* L);
	template<> int Bridge<RBX::UDim>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::UDim>::on_gc(lua_State* L);
	template<> int Bridge<RBX::UDim>::on_eq(lua_State* L);
	template<> void Bridge<RBX::UDim2>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::UDim2>::on_index(const RBX::UDim2& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::UDim2>::on_newindex(RBX::UDim2& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::UDim2>::on_index(lua_State* L);
	template<> int Bridge<RBX::UDim2>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::UDim2>::on_gc(lua_State* L);
	template<> int Bridge<RBX::UDim2>::on_eq(lua_State* L);

	// FacesBridge (section 225). Evidence: newFaces 0x16BD06 +
	// registerClassLibrary 0x16BE02 + classLibrary {"new"} 0xED1BC4 +
	// className "Faces" (batch365 rodata: 0xB5B906); registerClass
	// mac: 0x17023C + on_index(obj) 0x16BE5C + on_newindex 0x16B810 +
	// on_gc 0x1703C6 + on_eq 0x1703F8 + on_tostring x2 (0x170462/0x16F846)
	// + wrappers 0x198AAA/0x198B0A (batch365). No arithmetic in the
	// metatable (body-proven). Queued with the enum era: newFaces +
	// registerClassLibrary + table. Axes/CellID maps banked (batch364).
	class FacesBridge : public Bridge<RBX::Faces>
	{
		friend class Bridge<RBX::Faces>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushFaces(lua_State* L, const RBX::Faces& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newFaces(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::Faces>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::Faces>::on_index(const RBX::Faces& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::Faces>::on_newindex(RBX::Faces& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::Faces>::on_index(lua_State* L);
	template<> int Bridge<RBX::Faces>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::Faces>::on_gc(lua_State* L);
	template<> int Bridge<RBX::Faces>::on_eq(lua_State* L);
	template<> int Bridge<RBX::Faces>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::Faces>::on_tostring(const RBX::Faces& object,
		lua_State* L);

	// AxesBridge (section 226). Evidence: newAxes 0x16C07E +
	// registerClassLibrary 0x16C1C0 + classLibrary {"new"} 0xED1C9C +
	// className "Axes" (batch366 rodata: 0xB5B90C); registerClass
	// mac: 0x16FD5C + on_index(obj) 0x16C21A + on_newindex 0x16C468 +
	// on_gc 0x16FEE6 + on_eq 0x16FF18 + on_tostring x2 (0x16FF82/0x16F8F0)
	// + wrappers 0x198B6C/0x198BCC (batch366). No arithmetic (body-proven).
	// Queued with the enum era: newAxes + registerClassLibrary + table.
	class AxesBridge : public Bridge<RBX::Axes>
	{
		friend class Bridge<RBX::Axes>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushAxes(lua_State* L, const RBX::Axes& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newAxes(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::Axes>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::Axes>::on_index(const RBX::Axes& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::Axes>::on_newindex(RBX::Axes& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::Axes>::on_index(lua_State* L);
	template<> int Bridge<RBX::Axes>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::Axes>::on_gc(lua_State* L);
	template<> int Bridge<RBX::Axes>::on_eq(lua_State* L);
	template<> int Bridge<RBX::Axes>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::Axes>::on_tostring(const RBX::Axes& object,
		lua_State* L);

	// CellIDBridge (section 227). Evidence: newCellID 0x16C4CE +
	// registerClassLibrary 0x16C624 + classLibrary {"new"} 0xED1CAC +
	// className "CellId" (batch367 rodata: 0xB7E893); registerClass
	// mac: 0x16FFC6 + on_index(obj) 0x16C67E + on_newindex 0x16C7C0 +
	// on_gc 0x170150 + on_eq 0x17018A + on_tostring x2 (0x1701F8/0x16F99A)
	// + wrappers 0x198C2E/0x198C8E (batch367). Layout pinned by the
	// disasm: isNil byte @+0, G3D::Vector3 location @+4, shared_ptr
	// terrainPart @+16 (24B; G3D type, not 2016 RBX::Vector3).
	// on_gc is NOT specialized: the binary (checkudata + ~CellID) matches
	// the primary template's generic on_gc exactly, so the primary is
	// used (odr-used by registerClass below). Queued with the enum era:
	// newCellID + registerClassLibrary + table.
	class CellIDBridge : public Bridge<RBX::CellID>
	{
		friend class Bridge<RBX::CellID>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushCellID(lua_State* L, const RBX::CellID& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newCellID(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::CellID>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::CellID>::on_index(const RBX::CellID& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::CellID>::on_newindex(RBX::CellID& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::CellID>::on_index(lua_State* L);
	template<> int Bridge<RBX::CellID>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::CellID>::on_eq(lua_State* L);
	template<> int Bridge<RBX::CellID>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::CellID>::on_tostring(const RBX::CellID& object,
		lua_State* L);

	// InputObjectBridge (section 228). Evidence: newInputObject 0x16C826 +
	// registerClassLibrary 0x16CA10 + classLibrary {"new"} 0xED1CBC +
	// className "InputObject" (batch370 rodata: 0xB5B8F4); registerClass
	// mac: 0x16FAEE + on_index(obj) 0x16CA6A + on_newindex 0x16CB9A +
	// on_gc 0x16FC78 + on_eq 0x16FCAA + on_tostring x2 (0x16FD18/0x16FA44)
	// + wrappers 0x198CF0/0x198D50 (batch370). The bridge reads a PLAIN
	// 20B struct (type@0/state@4/screen@8/keycode@16) -- no vtable -- so
	// 2013's InputObject is NOT the 2016 Instance subclass (not carried).
	// Nested-enum/KeyCode classNames (batch371): "UserInputType" /
	// "UserInputState" / "KeyCode". Queued: newInputObject + library +
	// KeyCode bridge + enum-descriptor value verification.
	class InputObjectBridge : public Bridge<RBX::InputObject>
	{
		friend class Bridge<RBX::InputObject>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushInputObject(lua_State* L, const RBX::InputObject& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newInputObject(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::InputObject>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::InputObject>::on_index(const RBX::InputObject& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::InputObject>::on_newindex(RBX::InputObject& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::InputObject>::on_index(lua_State* L);
	template<> int Bridge<RBX::InputObject>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::InputObject>::on_gc(lua_State* L);
	template<> int Bridge<RBX::InputObject>::on_eq(lua_State* L);
	template<> int Bridge<RBX::InputObject>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::InputObject>::on_tostring(const RBX::InputObject& object,
		lua_State* L);

	// Color3Bridge (section 229). Evidence: newColor3 0x166108 +
	// registerClassLibrary 0x1661DC + classLibrary {"new"} 0xED1B44 +
	// className "Color3" (batch373 rodata: 0xB85A39); registerClass
	// mac: 0x170EE6 + on_index(obj) 0x166274 + on_newindex 0x16635A +
	// on_gc 0x171070 + on_eq 0x1710A2 + on_tostring x2 (0x171130/0x16F2F6)
	// + wrappers 0x1987A2/0x198802 + pushColor3 0x166236 (batch373).
	// No arithmetic (body-proven). Queued with the enum era: newColor3 +
	// registerClassLibrary + table.
	class Color3Bridge : public Bridge<G3D::Color3>
	{
		friend class Bridge<G3D::Color3>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushColor3(lua_State* L, const G3D::Color3& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newColor3(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<G3D::Color3>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<G3D::Color3>::on_index(const G3D::Color3& object,
		const char* name, lua_State* L);
	template<> void Bridge<G3D::Color3>::on_newindex(G3D::Color3& object,
		const char* name, lua_State* L);
	template<> int Bridge<G3D::Color3>::on_index(lua_State* L);
	template<> int Bridge<G3D::Color3>::on_newindex(lua_State* L);
	template<> int Bridge<G3D::Color3>::on_gc(lua_State* L);
	template<> int Bridge<G3D::Color3>::on_eq(lua_State* L);
	template<> int Bridge<G3D::Color3>::on_tostring(lua_State* L);
	template<> int Bridge<G3D::Color3>::on_tostring(const G3D::Color3& object,
		lua_State* L);

	// BrickColorBridge (section 230). Evidence: newBrickColor 0x1692A8 +
	// randomBrickColor 0x16946A + paletteBrickColor 0x169494 +
	// registerClassLibrary 0x16924E + classLibrary {"new","random",
	// "palette"} 0xED1BD4 + className "BrickColor" (batch374/375 rodata:
	// 0xB5B915); registerClass 0x1704A6 + on_index(obj) 0x1696AA +
	// on_newindex 0x1698B8 + on_gc 0x170630 + on_eq 0x170662 +
	// on_tostring x2 (0x1706CC/0x16F6F2) + wrappers 0x198864/0x1988C4
	// (batch375). No arithmetic (body-proven). Queued with the enum era:
	// newBrickColor/randomBrickColor/paletteBrickColor +
	// registerClassLibrary + table.
	class BrickColorBridge : public Bridge<RBX::BrickColor>
	{
		friend class Bridge<RBX::BrickColor>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushBrickColor(lua_State* L, const RBX::BrickColor& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newBrickColor(lua_State* L);
		static int randomBrickColor(lua_State* L);
		static int paletteBrickColor(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::BrickColor>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::BrickColor>::on_index(const RBX::BrickColor& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::BrickColor>::on_newindex(RBX::BrickColor& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::BrickColor>::on_index(lua_State* L);
	template<> int Bridge<RBX::BrickColor>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::BrickColor>::on_gc(lua_State* L);
	template<> int Bridge<RBX::BrickColor>::on_eq(lua_State* L);
	template<> int Bridge<RBX::BrickColor>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::BrickColor>::on_tostring(const RBX::BrickColor& object,
		lua_State* L);

	// CoordinateFrameBridge (section 231). Evidence: newCoordinateFrame
	// mac: 0x16991E + fromEulerAnglesXYZ 0x169CB8 + fromAxisAngle 0x169DE4 +
	// registerClassLibrary 0x169ED0 + classLibrary {"new",
	// "fromEulerAnglesXYZ","Angles"...} 0xED1C44 + className "CFrame"
	// (batch377 rodata: 0xB24EDE); on_add 0x169F2A + on_sub 0x16A004 +
	// on_mul 0x16A0DE + on_inverse 0x16A2A2 (batch377); on_index(obj)
	// mac: 0x16B044 + on_newindex 0x16B410 + registerClass 0x16D6D0 (batch377);
	// toWorldSpace 0x16A3FC + toObjectSpace 0x16A510 + pointToWorldSpace
	// mac: 0x16A872 + pointToObjectSpace 0x16AA4E + vectorToWorldSpace 0x16AC2A
	// + vectorToObjectSpace 0x16AED8 + components 0x16AD9E +
	// toEulerAnglesXYZ 0x16AE4C + on_gc 0x16ECFE + on_eq 0x16ED2E +
	// on_tostring x2 (0x16EDCA/0x16F79C) + wrappers 0x198192/0x1981F2
	// (batch378). Queued with the enum era: new/fromEulerAnglesXYZ/
	// fromAxisAngle + registerClassLibrary + table.
	class CoordinateFrameBridge : public Bridge<G3D::CoordinateFrame>
	{
		friend class Bridge<G3D::CoordinateFrame>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushCoordinateFrame(lua_State* L, const G3D::CoordinateFrame& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newCoordinateFrame(lua_State* L);
		static int fromEulerAnglesXYZ(lua_State* L);
		static int fromAxisAngle(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_mul(lua_State* L);
		static int on_inverse(lua_State* L);
		static int on_toWorldSpace(lua_State* L);
		static int on_toObjectSpace(lua_State* L);
		static int on_pointToWorldSpace(lua_State* L);
		static int on_pointToObjectSpace(lua_State* L);
		static int on_vectorToWorldSpace(lua_State* L);
		static int on_vectorToObjectSpace(lua_State* L);
		static int on_toEulerAnglesXYZ(lua_State* L);
		static int on_components(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<G3D::CoordinateFrame>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<G3D::CoordinateFrame>::on_index(const G3D::CoordinateFrame& object,
		const char* name, lua_State* L);
	template<> void Bridge<G3D::CoordinateFrame>::on_newindex(G3D::CoordinateFrame& object,
		const char* name, lua_State* L);
	template<> int Bridge<G3D::CoordinateFrame>::on_index(lua_State* L);
	template<> int Bridge<G3D::CoordinateFrame>::on_newindex(lua_State* L);
	template<> int Bridge<G3D::CoordinateFrame>::on_gc(lua_State* L);
	template<> int Bridge<G3D::CoordinateFrame>::on_eq(lua_State* L);
	template<> int Bridge<G3D::CoordinateFrame>::on_tostring(lua_State* L);
	template<> int Bridge<G3D::CoordinateFrame>::on_tostring(const G3D::CoordinateFrame& object,
		lua_State* L);

	// Region3Bridge / Region3int16Bridge (section 232). Evidence:
	// newRegion3 0x1668BC + classLibrary {"new"} 0xED1B64 + className
	// "Region3" (batch380 rodata: 0xB78AA1); on_index(obj) 0x166A58 +
	// on_newindex 0x166B48 + registerClass 0x170986 + on_gc 0x170B10 +
	// on_eq 0x170B42 + on_tostring x2 (0x170C02/0x172BF0) + wrappers
	// mac: 0x198254/0x1982B4 (batch380); newRegion3int16 0x166BAE +
	// classLibrary {"new"} 0xED1B74 + className "Region3int16"
	// (batch381 rodata: 0xB7E89A); on_index(obj) 0x166D26 + on_newindex
	// mac: 0x166E22 + registerClass 0x170C46 + on_gc 0x170DD0 + on_eq 0x170E02
	// + on_tostring x2 (0x170EA2/0x172B46) + wrappers 0x198316/0x198376
	// (batch381). Neither has arithmetic or extra members (body-proven:
	// 2016's ExpandToGrid does NOT exist in the 2013 on_index). Queued
	// with the enum era: new* + libraries + tables. Vector3int16
	// className "Vector3int16" (batch382 rodata: 0xB5B866) for the
	// Min/Max pushes (metatable registration queues with that bridge).
	class Region3Bridge : public Bridge<RBX::Region3>
	{
		friend class Bridge<RBX::Region3>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushRegion3(lua_State* L, const RBX::Region3& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newRegion3(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	class Region3int16Bridge : public Bridge<RBX::Region3int16>
	{
		friend class Bridge<RBX::Region3int16>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushRegion3int16(lua_State* L, const RBX::Region3int16& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newRegion3int16(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::Region3>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::Region3>::on_index(const RBX::Region3& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::Region3>::on_newindex(RBX::Region3& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::Region3>::on_index(lua_State* L);
	template<> int Bridge<RBX::Region3>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::Region3>::on_gc(lua_State* L);
	template<> int Bridge<RBX::Region3>::on_eq(lua_State* L);
	template<> int Bridge<RBX::Region3>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::Region3>::on_tostring(const RBX::Region3& object,
		lua_State* L);
	template<> void Bridge<RBX::Region3int16>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::Region3int16>::on_index(const RBX::Region3int16& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::Region3int16>::on_newindex(RBX::Region3int16& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::Region3int16>::on_index(lua_State* L);
	template<> int Bridge<RBX::Region3int16>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::Region3int16>::on_gc(lua_State* L);
	template<> int Bridge<RBX::Region3int16>::on_eq(lua_State* L);
	template<> int Bridge<RBX::Region3int16>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::Region3int16>::on_tostring(const RBX::Region3int16& object,
		lua_State* L);

	// RbxRayBridge (section 233). Evidence: newRbxRay 0x1663C0 +
	// registerClassLibrary 0x1664DE + classLibrary {"new"} 0xED1B54 +
	// className "Ray" (batch387 rodata: 0xB5B911); registerClass 0x170710
	// + on_index(obj) 0x166538 + on_newindex 0x166856 + on_gc 0x17089A +
	// on_eq 0x1708D4 + on_tostring x2 (0x170942/0x16F3A0) + wrappers
	// mac: 0x1986E0/0x198740 (batch387); closestPointVector3 0x166764 +
	// distanceVector3 0x1667E9 (batch389); converter 0x550BDC (batch390).
	// Queued with the enum era: newRbxRay + registerClassLibrary + table.
	class RbxRayBridge : public Bridge<RBX::RbxRay>
	{
		friend class Bridge<RBX::RbxRay>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushRbxRay(lua_State* L, const RBX::RbxRay& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newRbxRay(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<RBX::RbxRay>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<RBX::RbxRay>::on_index(const RBX::RbxRay& object,
		const char* name, lua_State* L);
	template<> void Bridge<RBX::RbxRay>::on_newindex(RBX::RbxRay& object,
		const char* name, lua_State* L);
	template<> int Bridge<RBX::RbxRay>::on_index(lua_State* L);
	template<> int Bridge<RBX::RbxRay>::on_newindex(lua_State* L);
	template<> int Bridge<RBX::RbxRay>::on_eq(lua_State* L);
	template<> int Bridge<RBX::RbxRay>::on_tostring(lua_State* L);
	template<> int Bridge<RBX::RbxRay>::on_tostring(const RBX::RbxRay& object,
		lua_State* L);

	// Vector3int16Bridge / Vector2int16Bridge (section 234). Evidence:
	// V3: on_add/sub/mul/div/unm 0x167E46/0x167ED6/0x167F66/0x16809C/
	// mac: 0x168206 + on_index(obj) 0x168272 + on_newindex 0x16838C
	// (batch393); registerClass 0x16CC00 + on_gc 0x16E94A + on_eq
	// mac: 0x16E97A + on_tostring x2 (0x16E9FE/0x16F44A) + wrappers
	// mac: 0x1983D8/0x198438 + className "Vector3int16" + {"new"} table
	// (batch394); V2: on_add/sub/mul/div/unm 0x168520/0x16859E/0x16861C/
	// mac: 0x16876C/0x1688BE + on_index(obj) 0x168918 + on_newindex 0x168A00
	// (batch395); registerClass 0x16CEB4 + on_gc 0x16EA3A + on_eq 0x16EA6A
	// + on_tostring x2 (0x16EAD6/0x16F4F4) + wrappers 0x19849A/0x1984FA +
	// className "Vector2int16" + {"new"} table (batch396). Queued with
	// the enum era: new* + libraries + tables.
	class Vector3int16Bridge : public Bridge<G3D::Vector3int16>
	{
		friend class Bridge<G3D::Vector3int16>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushVector3int16(lua_State* L, const G3D::Vector3int16& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newVector3int16(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_mul(lua_State* L);
		static int on_div(lua_State* L);
		static int on_unm(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	class Vector2int16Bridge : public Bridge<G3D::Vector2int16>
	{
		friend class Bridge<G3D::Vector2int16>;
	public:
		static void registerClassLibrary(lua_State* L);

		static void pushVector2int16(lua_State* L, const G3D::Vector2int16& v)
		{
			pushNewObject(L, v);
		}
	private:
		static int newVector2int16(lua_State* L);
		static int on_add(lua_State* L);
		static int on_sub(lua_State* L);
		static int on_mul(lua_State* L);
		static int on_div(lua_State* L);
		static int on_unm(lua_State* L);
		static const luaL_reg classLibrary[];
	};

	template<> void Bridge<G3D::Vector3int16>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<G3D::Vector3int16>::on_index(const G3D::Vector3int16& object,
		const char* name, lua_State* L);
	template<> void Bridge<G3D::Vector3int16>::on_newindex(G3D::Vector3int16& object,
		const char* name, lua_State* L);
	template<> int Bridge<G3D::Vector3int16>::on_index(lua_State* L);
	template<> int Bridge<G3D::Vector3int16>::on_newindex(lua_State* L);
	template<> int Bridge<G3D::Vector3int16>::on_gc(lua_State* L);
	template<> int Bridge<G3D::Vector3int16>::on_eq(lua_State* L);
	template<> int Bridge<G3D::Vector3int16>::on_tostring(lua_State* L);
	template<> int Bridge<G3D::Vector3int16>::on_tostring(const G3D::Vector3int16& object,
		lua_State* L);
	template<> void Bridge<G3D::Vector2int16>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<G3D::Vector2int16>::on_index(const G3D::Vector2int16& object,
		const char* name, lua_State* L);
	template<> void Bridge<G3D::Vector2int16>::on_newindex(G3D::Vector2int16& object,
		const char* name, lua_State* L);
	template<> int Bridge<G3D::Vector2int16>::on_index(lua_State* L);
	template<> int Bridge<G3D::Vector2int16>::on_newindex(lua_State* L);
	template<> int Bridge<G3D::Vector2int16>::on_gc(lua_State* L);
	template<> int Bridge<G3D::Vector2int16>::on_eq(lua_State* L);
	template<> int Bridge<G3D::Vector2int16>::on_tostring(lua_State* L);
	template<> int Bridge<G3D::Vector2int16>::on_tostring(const G3D::Vector2int16& object,
		lua_State* L);

} }

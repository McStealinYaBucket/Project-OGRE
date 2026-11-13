/* App/v8datamodel/ScriptContext.cpp -- October 2013 (sName; reflection block ported 2026-10-17, rest stub-only).
   TODO: full decompile. */

#include "stdafx.h"
#include "decomp.h"
#include <string.h>		// strchr/strcmp (LuaProfiler bodies, TENTH ERA)

#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/BaseScript.h"
#include "V8DataModel/LuaSettings.h"
#include "V8DataModel/CoreScript.h"
#include "V8DataModel/StarterScript.h"
#include "V8DataModel/Script.h"
#include "RbxFormat.h"
#include "V8DataModel/DebugSettings.h"
#include "script/LuaAtomicClasses.h"
#include "script/LuaEnum.h"
#include "script/LuaLibrary.h"
#include "script/LuaSignal.h"
#include "script/ThreadRef.h"
#include "script/ScriptStats.h"
#include "script/LuaProfiler.h"
#include "Util/Object.h"
#include "Util/ProtectedString.h"
#include "V8DataModel/StatsService.h"
#include "V8DataModel/DataModel.h"
#include "Network/Players.h"
#include "V8DataModel/RunService.h"
#include "V8DataModel/IScriptFilter.h"	// RuntimeScriptService (RSS tail, era C1)
#include "V8DataModel/Workspace.h"
#include "V8DataModel/OverlayDataModel.h"
#include "V8DataModel/GlobalAdvancedSettings.h"
#include "V8DataModel/GlobalBasicSettings.h"
#include "V8DataModel/PluginManager.h"
#include "V8DataModel/DebuggerManager.h"
#include "V8DataModel/ContentProvider.h"
#include "script/LuaArguments.h"
#include "script/ExitHandlers.h"
#include "script/LuaInstanceBridge.h"
#include "Util/FastFlags.h"
#include "Util/RbxStringTable.h"
#include "V8DataModel/HackDefines.h"
#include "security/SecurityContext.h"
#include "RbxPlatform.h"
#include "util/standardout.h"
#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "G3D/System.h"
#include "rbx/rbxtime.h"
#include "rbx/Log.h"
#include "rbx/Thread.hpp"
#include "Util/Math.h"

#include <boost/bind.hpp>

#include <sstream>
#include <stdexcept>
#include <algorithm>

// 2013 asserts with the line the PLAYER strings record ("<expr> file: <build path>/Client/App/
// script/ScriptContext.cpp line: N"): RBX_LOG_ASSERT's shape, __FILE__ for the path.
#define RBXASSERT_LINE(expr, line) \
	((void) (FLog::Asserts && (!!(expr) || \
	((RBX::_internal::_debugHook != NULL) && (RBX::_internal::_debugHook(#expr, __FILE__, line))) || \
	(ReleaseAssert(FLog::Asserts, #expr " file: " __FILE__ " line: " #line), 0))))

LOGGROUP(UseLuaMemoryPool);
LOGVARIABLE(ScriptContext, 0);
LOGVARIABLE(LuaProfiler, 0);
// 0xFA9B20 byte = 0xFF in the 2013 PE (batch703; registration InitFunc_480
// mac: 0x670B59): gates the loadfile/executeInNewThread source dumps.
LOGVARIABLE(FullScriptCode, 255)
// 0xFA9B44 byte = 0xFF in the 2013 PE (batch720; Initer slot 0xFA9B48,
// registration InitFunc_480 0x670C67; read through the pointer-table
// slot off_EADEC8): lua script-timeout span in seconds (Channel =
// unsigned char; the decompiler shows the [0] registration-array form).
// 0xFF = 255 -- the shipped default arms a 255 s monitor at every
// provider attach (the arm gates on != 0 and passes the byte value as
// the span).
LOGVARIABLE(LuaScriptTimeoutSeconds, 255)
// 0xFA9AFC/0xFA9B04 bytes = 0xFF in the 2013 PE (batch723/724; Initer
// slots 0xFA9B00/0xFA9B08, registration InitFunc_480): attach/detach
// script logging; declared byte-faithful for the provider-connection era.
LOGVARIABLE(ScriptContextAdd, 255)
// 0xF88531 byte = 1 in the 2013 PE (batch723/724; Initer slot 0xFA9B14,
// registration InitFunc_480 0x670AFD): closeStates logging is ON at rest
// (closeStates 0x18E9BC gates its ScriptCount/WaitingThreads/heap logs on
// this).
LOGVARIABLE(ScriptContextClose, 1)
LOGVARIABLE(ScriptContextRemove, 255)

// Start-script settings-byte gates read through the pointer-table
// slots (batch741: dword_D1A07D/09D/0A5 regions -- the pointed bytes
// never resolved to named variables in the dumped material; rest
// values unknown). Port stand-ins, default OFF so the deferred arms
// stay inert; LOUD DELTA (stand-in names, cited at each use).
LOGVARIABLE(StartScriptSourceDump, 0);         // batch741:229 -- gates the start-script source printf
LOGVARIABLE(StartScriptSourceTransform, 0);    // batch741:348-351 -- gates the sourceTransform apply
LOGVARIABLE(StartScriptErrorContinuation, 0);  // batch741:387-391 -- gates the error-continuation call

// 0xFA56C8/0xFA56CC (batch710: both 0xFFFFFFFF at rest; read through the
// pointer tables off_EADE8C/off_EADE90): RBX::Debugable::scriptCounter /
// scriptDocCounter low-bit gates around the two early HATE_ILLEGAL_SCRIPTS
// inserts in executeInNewThread. RBX::Debugable is unported (no other 2013
// xrefs recovered); these mirrors preserve the load-time gate state.
// LOUD DELTA: writers unevidenced in the dumped material (gates ON).
static unsigned int debugableScriptCounter = 0xFFFFFFFFu;
static unsigned int debugableScriptDocCounter = 0xFFFFFFFFu;;

namespace RBX {
	const char* const sScriptContext = "ScriptContext";
}

// LuaProfiler (TENTH ERA, batches 772/773/774/775): real bodies landed.
// The class is GLOBAL-namespace per the __ZN11LuaProfiler* mangles -- the
// hook-era RBX{Lua{instance}} block is retired (that spelling had no
// binary anchor and no consumer besides this file, updated below in the
// same landing). Binary statics: __ZN11LuaProfiler8instanceE @0xFA6420
// (read by ScriptContext::hook @18B2E7/18B36B, written only by ctor
// mac: 0x1A8172 / dtor 0x1A80D4) and __ZN11LuaProfiler11stringCacheE @0xFA6424
// (constructed empty as two std::maps by static-init InitFunc_123 @1AA4F0
// with both maps' __cxa_atexit teardown registration, batch774 decompile
// -- reproduced by defining the static here and letting the compiler emit
// the init/teardown pair). The 2013 ctor/dtor asserts cite
// /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/
// ScriptContext.cpp lines 321/356 (assert strings @B2DC75 @B2DBF0, file
// path @B2BEC3, batch774 reads) -- the class bodies were compiled in this
// TU, so the method definitions follow here.
LuaProfiler* LuaProfiler::instance = NULL;
LuaProfiler::StringCache LuaProfiler::stringCache;

// mac: 0x1AA4F0 InitFunc_123 constructs the static's two empty maps; the port
// needs no explicit member init (default construction reaches the same
// state). The binary does carry real StringCache ctor/dtor symbols whose
// only content is map construction/teardown (D2 @1979D0: two _M_erase,
// batch772) -- kept as empty out-of-line definitions so the shape is
// explicit (LOUD: no added behavior; our compiler still emits the same
// atexit pair from the static definition above).
LuaProfiler::StringCache::StringCache()
{
}

LuaProfiler::StringCache::~StringCache()
{
}

// mac: 0x1A8C0A Function::operator< (batch775 asm 1A8C0A-8C3C): ascending
// UNSIGNED pointer order on {name, namewhat, source} (setb at 1A8C29),
// then SIGNED line (setl at 1A8C37). Pointer compare == unsigned dword
// compare on this ABI; the C++ spellings below match setb/setl exactly.
bool LuaProfiler::StringCache::Function::operator<(const Function& rhs) const
{
	if (name != rhs.name)
		return name < rhs.name;
	if (namewhat != rhs.namewhat)
		return namewhat < rhs.namewhat;
	if (source != rhs.source)
		return source < rhs.source;
	return line < rhs.line;
}

// mac: 0x1A8502 getBegin (batch773 asm 1A8502-85C1; map operator[] via slot
// 96BCDA @1A851E; empty-cache concat = operator+(const char*, const
// string&) 95EAD4 @1A8541). Returns the cached buffer pointer (the binary
// returns the std::string's data dword directly, 1A8523-27).
// FUNCTION: PLAYER 0x006772e0
// FUNCTION: STUDIO 0x007defa0
const char* LuaProfiler::StringCache::getBegin(const std::string& position)
{
	std::string& cached = beginMap[position];
	if (cached.empty())
		cached = "LUABEG" + position;
	return cached.c_str();
}

// mac: 0x1A8608 getCall (batch773 asm 1A8608-875C; the Function-keyed map
// operator[] @95A118 targets binary this+24 @1A8636-38). Key = the four
// lua_Debug-derived params; the cache entry is written only when empty
// (1A8649-D jnz past the build). Vararg order restored from the push
// order (1A86A0-D2): arg1 = source ? source : "?" (fallback string
// @asc_B1DBA2; cmovnz 1A86CF), arg2 = line, arg3 = op, arg4 = name ?
// name : "?" (cmovz 1A86C3). op: strcmp(namewhat, "index" @B2DD07)==0 ->
// "get " @B2DD0D; strcmp(namewhat, "newindex" @B2DD12)==0 -> "set "
// @B2DD1B; else "" (incl. namewhat==NULL, 1A8653-868B). The
// 'index'/'newindex' namewhat tokens are the Lua fork's funcname tokens
// for metamethod-driven calls; the ported lua.h passes them through
// untouched.
// STUB: PLAYER 0x00677380
// STUB: STUDIO 0x007df040
const char* LuaProfiler::StringCache::getCall(const char* name, const char* namewhat,
	const char* source, int line)
{
	Function key;
	key.name = name;
	key.namewhat = namewhat;
	key.source = source;
	key.line = line;
	std::string& cached = callMap[key];
	if (cached.empty())
	{
		const char* op = "";
		if (namewhat != NULL)
		{
			if (strcmp(namewhat, "index") == 0)
				op = "get ";
			else if (strcmp(namewhat, "newindex") == 0)
				op = "set ";
		}
		cached = RBX::format("LUACALL%s(%d): %s%s",
			source != NULL ? source : "?",
			line,
			op,
			name != NULL ? name : "?");
	}
	return cached.c_str();
}

// mac: 0x1A8416 getResumePosition (batch773 asm 1A8416-84FF). The reads hit the
// 2013 Lua fork's private thread structures; no App-side header names
// those fields, so the offsets are ported VERBATIM (loud DELTA-lite:
// fork-internal layout, offset math identical to the binary):
//   byte (char*)L+6 set -> "(resume)" @B2DCFA (1A842B-8445);
//   table = *(void**)((char*)L+8); record = table + 12*(~level)
//   (signed imul 0Ch, 1A8454-59 -- ~level makes the stride negative);
//   record tag *(int*)(record+8) != 6 -> "??" @B2DD3B (1A845C-61);
//   pos = *(char**)record; byte pos+6 set -> "=[C](-1)" @B2DD3E
//   (1A8463-84AE);
//   else: chunk = *(char**)(pos+0x20) + 0x10 (1A84B5-84BA);
//         number = *(*(int*)( *(int*)(pos+0x10) + 0x14 ))
//         (1A84AF-B8: eax=*(pos+10h); ecx=*(eax+14h); esi=*ecx);
//         strchr(chunk, '\n') (1A84BD-C8), result used only as a test:
//         name = found ? "<string>" @B2DD4E : chunk (cmovz 1A84D9);
//         return RBX::format("%s(%d)" @B2DD47, name, number)
//         (pushes [esp+8]=name, [esp+0Ch]=number at 1A84CD-84DC).
// STUB: PLAYER 0x0066fbe0
// STUB: STUDIO 0x007d7fa0
std::string LuaProfiler::getResumePosition(lua_State* L, int level)
{
	if (*((const char*)L + 6) != 0)
		return "(resume)";
	const void* table = *(void* const*)((const char*)L + 8);
	char* record = (char*)table + 12 * (~level);
	if (*(int*)(record + 8) != 6)
		return "??";
	const char* pos = *(char* const*)record;
	if (*(pos + 6) != 0)
		return "=[C](-1)";
	const int* tagPtr = *(int* const*)(pos + 0x10);						// 1A84AF
	const void* numberPtr = *(void* const*)((const char*)tagPtr + 0x14);	// 1A84B2
	const int number = *(const int*)numberPtr;							// 1A84B8
	char* chunk = *(char* const*)(pos + 0x20) + 0x10;					// 1A84B5-BA
	const char* name = strchr(chunk, '\n') != NULL ? "<string>" : chunk;
	return RBX::format("%s(%d)", name, number);
}









































// mac: 0x1A8172 ctor (batch773 asm 1A8172-8415). Assert "instance == NULL"
// @B2DC75 -- binary ScriptContext.cpp line 321 (0x141; full ReleaseAssert
// text read @B2DC86, batch774). Sequence: instance = this (1A81F8);
// position = getResumePosition(L, level) (sret pushes (out, L=arg_4,
// level=arg_8), call slot 959A6A @1A820C);
// FastLogS(FLog::LuaProfiler, stringCache.getBegin(position), NULL)
// (getBegin slot 959A64 @1A8227; FastLogS 958A8C @1A824A -- flag byte
// through off_EADEAC, the FLog::LuaProfiler channel; the begin text is
// the cached "LUABEG<pos>" buffer). If position == "(resume)"
// (@B2DCFA, std::string::compare 973A74 @1A825C): walk frames from level
// 0 with lua_getstack 63D857 (1A82B8) / lua_getinfo 63D9E0 with the flag
// "Sln" @B2DD03 (flag address computed once at 1A8292-98; byte-verified
// b'Sln' in batch775); per frame getCall(name, namewhat, source,
// linedefined -- lua_Debug reads debug+4/+8/+0x10/+0x1C at 1A82D7-82ED)
// and vector<const char*>::push_back @95A1B4 (1A832B); then the frames
// log in REVERSE order (last frame first: element [size-1] first,
// [begin + size*4 - 4] at 1A834B), each
// FastLogS(FLog::LuaProfiler, frame, NULL) @1A8368; the vector buffer is
// operator delete'd after the logs (973DDA @1A8385-8C -- the port's
// vector destructor reaches the same end state).
// STUB: PLAYER 0x00677ec0
// STUB: STUDIO 0x007dfde0
LuaProfiler::LuaProfiler(lua_State* L, int level)
{
	RBXASSERT(instance == NULL);	// binary line 321 (@B2DC75)
	LuaProfiler::instance = this;
	const std::string position = getResumePosition(L, level);
	FLog::FastLogS(FLog::LuaProfiler, stringCache.getBegin(position), NULL);
	if (position.compare("(resume)") == 0)
	{
		std::vector<const char*> frames;
		lua_Debug debug;
		for (int i = 0; lua_getstack(L, i, &debug) != 0; ++i)
		{
			lua_getinfo(L, "Sln", &debug);
			frames.push_back(stringCache.getCall(
				debug.name,
				debug.namewhat,
				debug.source,
				debug.linedefined));
		}
		for (std::size_t j = frames.size(); j != 0; --j)	// reverse: last frame logs first
			FLog::FastLogS(FLog::LuaProfiler, frames[j - 1], NULL);
	}
}




// mac: 0x1A80D4 dtor (batch773 asm 1A80D4-8171): FastLogS(FLog::LuaProfiler,
// "LUAEND", NULL) (text @B2DBE9, sarg 0, 1A80E0-84); assert
// "instance == this" @B2DBF0 -- binary ScriptContext.cpp line 356 (0x164;
// full ReleaseAssert text read @B2DC01, batch774); instance = NULL
// (1A8114).
// FUNCTION: PLAYER 0x0066fca0
// FUNCTION: STUDIO 0x007d8060
LuaProfiler::~LuaProfiler()
{
	FLog::FastLogS(FLog::LuaProfiler, "LUAEND", NULL);
	RBXASSERT(instance == this);	// binary line 356 (@B2DBF0)
	LuaProfiler::instance = NULL;
}

// mac: 0x197D28 hookCall (batch773 asm 197D28-7D9F): lua_getinfo(L, "nS"
// @B2DF48, ar) via 63D9E0 (pushes (L, "nS", ar) at 197D39-4D), then
// FastLogS(FLog::LuaProfiler, stringCache.getCall(ar->name [ar+4],
// ar->namewhat [ar+8], ar->source [ar+0x10], ar->linedefined [ar+0x1C]),
// NULL) (getCall slot 959A5E @197D77; FastLogS 958A8C @197D90 -- the
// getCall result is the cached c_str; sarg NULL). this is unused in the
// binary body (the member shape merely carries it).
// FUNCTION: PLAYER 0x006774e0
// FUNCTION: STUDIO 0x007df1a0
void LuaProfiler::hookCall(lua_State* L, lua_Debug* ar)
{
	lua_getinfo(L, "nS", ar);
	const char* call = stringCache.getCall(ar->name, ar->namewhat, ar->source, ar->linedefined);
	FLog::FastLogS(FLog::LuaProfiler, call, NULL);	// NULL pushed after the call (0x00677519)
}

using namespace RBX;

REFLECTION_BEGIN();
// ScriptContext reflection block: registrations with exact binary args.
// Binary home: the ScriptContext TU static init InitFunc_123 (0x1AA4F0);
// the order below is the binary's construction order (events per batches
// 728-731; the rest per batches 733/734).  Security values are
// binary-verbatim -- 2016 drift noted per slot.  Attributes are the
// TU-wide default (the identical stack temp every descriptor in that init
// passes).  QUEUED at their in-order slots (loud DELTAs; dependencies
// unported -- see ScriptContext.h for the member-side notes):
//   -- GetScriptStats landed this era (func_GetScriptStats 0xFA6630
//      wraps getScriptStatsNew 0x18A6AC; batch757:704-716 pins
//      security 4 + arity-0; the reader + ScriptStats internals per
//      batch733/716/719 -- see ScriptStats.h and the era comments).


// mac: 0x1AA664 -- EventDesc "CamelCaseViolation": member offset 724, args
// "object"/"member"/"script", security 4 (WritePlayer); the third signal
// slot is shared_ptr<Instance> per the mangled EventDesc instantiation.
static Reflection::EventDesc<ScriptContext, void(boost::shared_ptr<Instance>, std::string, boost::shared_ptr<Instance>)> event_CamelCaseViolation(&ScriptContext::camelCaseViolation, "CamelCaseViolation", "object", "member", "script", Security::WritePlayer);
// 0xFA6454 -- func_AddStarterScript wrapping addStarterScript 0x189990
// (C1 landed): "assetId" int arg; security 4 (WritePlayer).
static Reflection::BoundFuncDesc<ScriptContext, boost::shared_ptr<Instance>(int)> func_AddStarterScript(&ScriptContext::addStarterScript, "AddStarterScript", "assetId", Security::WritePlayer);

// mac: 0x189BD8 -- func_RegisterRobloxLibrary: LibraryService::registerLibrary
// (... /*force=*/true); security 4 (WritePlayer).
static Reflection::BoundFuncDesc<ScriptContext, void(std::string, std::string)> func_RegisterRobloxLibrary(&ScriptContext::registerRobloxLibrary, "RegisterRobloxLibrary", "libraryName", "assetId", Security::WritePlayer);
// mac: 0x189C0A -- func_RegisterLibrary: force = false (batch734); security 4.
static Reflection::BoundFuncDesc<ScriptContext, void(std::string, std::string)> func_RegisterLibrary(&ScriptContext::registerLibrary, "RegisterLibrary", "libraryName", "assetId", Security::WritePlayer);
// mac: 0x189C3C -- func_RegisterDevelopmentLibrary; security 4.
static Reflection::BoundFuncDesc<ScriptContext, void(std::string, boost::shared_ptr<Instance>)> func_RegisterDevelopmentLibrary(&ScriptContext::registerDevelopmentLibrary, "RegisterDevelopmentLibrary", "libraryName", "scriptInstance", Security::WritePlayer);
// member markLibrariesRegistered 0x189DFE (LibraryService::markLibrariesLoaded); security 4.
static Reflection::BoundFuncDesc<ScriptContext, void()> func_MarkLibrariesRegistered(&ScriptContext::markLibrariesRegistered, "LibraryRegistrationComplete", Security::WritePlayer);
// 0xFA6560 -- func_AddCoreScript wrapping addCoreScript 0x189E1A
// (C1 landed): assetId int, parent Object, name string; security 4.
static Reflection::BoundFuncDesc<ScriptContext, boost::shared_ptr<Instance>(int, boost::shared_ptr<Instance>, std::string)> func_AddCoreScript(&ScriptContext::addCoreScript, "AddCoreScript", "assetId", "parent", "name", Security::WritePlayer);
// 0xFA659C -- propScriptsDisabled "ScriptsDisabled" (bound to the +246
// bool; onChangedScriptEnabled = startPendingScripts, 0x18A304). Class
// static; definition sits next to the GC props below (the binary's UI
// flags byte is 21 -- encoded Attributes(21) at the definition, loud
// comment there).

// mac: 0x1AAB0F -- func_SetTimeout ("seconds"; wrapper 0xFA65C8).  Security 2
// = RobloxPlace (2016 moved it to Plugin -- binary is authority).
static Reflection::BoundFuncDesc<ScriptContext, void(double)> func_SetTimeout(&ScriptContext::setTimeout, "SetTimeout", "seconds", Security::RobloxPlace);
// func_GetHeapStats 0xFA65FC: "clearHighwaterMark" default true (binary
// passes 1), security 4 (2016: RobloxScript -- binary is authority).
// Result type is shared_ptr<const Tuple> per the binary's mangled
// signature (the getSingleton spec lives at v8tree/EnumProperty.cpp:309).
static Reflection::BoundFuncDesc<ScriptContext, shared_ptr<const Reflection::Tuple>(bool)> func_GetHeapStats(&ScriptContext::getHeapStats, "GetHeapStats", "clearHighwaterMark", true, Security::WritePlayer);
// func_GetScriptStats 0xFA6630 (batch757:704-716): arity-0 member
// getScriptStatsNew 0x18A6AC, name string verbatim, security 4
// (WritePlayer), descriptors templated <..., 0>; Statistics return
// shared_ptr<const vector<Variant>> per the mangled ctor args.
static Reflection::BoundFuncDesc<ScriptContext, boost::shared_ptr<const std::vector<Reflection::Variant> >()> func_GetScriptStats(&ScriptContext::getScriptStatsNew, "GetScriptStats", Security::WritePlayer);

// (propLuaGcLimit/propLuaGcFrequency are class statics -- definitions in
// the namespace section below, next to the class registrar.)
// func_CollectScriptStats 0xFA66B8: "enable" default false, security 4.
static Reflection::BoundFuncDesc<ScriptContext, void(bool)> func_CollectScriptStats(&ScriptContext::setCollectScriptStats, "SetCollectScriptStats", "enable", false, Security::WritePlayer);

// mac: 0x1AAE44 -- EventDesc "Error": member offset 720, args
// "message"/"stackTrace"/"script", security 1 (Plugin; 2016 carries None
// -- 2013 binary is authority).
static Reflection::EventDesc<ScriptContext, void(std::string, std::string, boost::shared_ptr<Instance>)> event_Error(&ScriptContext::errorSignal, "Error", "message", "stackTrace", "script", Security::Plugin);
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ScriptContext);
}

namespace RBX {

// propLuaGcLimit/propLuaGcFrequency (class statics; symbols @0xFA6660/
// 0xFA668C = RBX::ScriptContext::{propLuaGcLimit,propLuaGcFrequency}):
// BoundProp<int, READWRITE> bound to the +0xF8/+0xFC ints (InitFunc_123
// member offsets 248/252, batch733).  Category string "Performance"
// verbatim; flags 21 = PropertyDescriptor::UI; security 4 (WritePlayer).
Reflection::BoundProp<int> ScriptContext::propLuaGcLimit("GarbageCollectionLimit", "Performance", &ScriptContext::gcFullStepThreshold, Reflection::PropertyDescriptor::UI, Security::WritePlayer);
Reflection::BoundProp<int> ScriptContext::propLuaGcFrequency("GarbageCollectionFrequency", "Performance", &ScriptContext::gcFrequencyBase, Reflection::PropertyDescriptor::UI, Security::WritePlayer);

// propScriptsDisabled 0xFA659C ("ScriptsDisabled", C1 landed): binary UI
// flags byte = 21 (the same value the GC props' "Flags are 21" note
// documents; passed as the PropertyDescriptor::UI constant like the two
// above). Security 2 = RobloxPlace. LOUD DELTA: the category string was
// not captured in the batch733/734 prop entries -- category_Data stands
// in (deterministic, cosmetic-only). The 4th ctor arg is the onChanged
// hook (onChangedScriptEnabled = tail-call startPendingScripts 0x18A304).
Reflection::BoundProp<bool> ScriptContext::propScriptsDisabled("ScriptsDisabled", category_Data, &ScriptContext::scriptsDisabled, &ScriptContext::onChangedScriptEnabled, Reflection::PropertyDescriptor::UI, Security::RobloxPlace);

// Open-state counter (the binary's dword_FADA34, batch723/724): InitFunc_123
// mac: 0x1AA72B writes the rest 0, openState 0x18B638 xadd +1 after
// lua_newstate, closeState 0x18EEC8 xadd -1 after lua_close; closeStates
// reads it for the leak-report gate below.
static volatile LONG g_openLuaStates = 0;

// mac: 0x18E9BC-0x18EE84 (batch722 FULL body; callers: SC detach arm 0x194FFD +
// DataModel::clearContents 0x2CFD89).  Sets +247 outside the reset gate,
// early-outs on resettingSimulation, then tears the Lua world down:
// commandLineSandbox reset (+0x6C WeakThreadRef::reset); ScriptCount/
// WaitingThreads logs on FLog::ScriptContextClose (0xF88531 default 1);
// scripts tree erase-all (+0x84); the LeakedWaitingThreads log +
// yieldEvent.reset() (+0xC4); waitingThreads.clear() (+0xC8); closeState
// + NULL on both globalStates slots (+0x64/+0x68); allocator heap stats
// (allocatorView +0x2E0) with the ScriptContextClose heap logs,
// clearHeapMax and the "Script memory leaks" MESSAGE_ERROR when no other
// state is open and the heap is non-empty.
// DELTAs (loud): gcJob/waitingScriptsJob removeBlocking blocks
// (members +0x2E4/+0x2EC unported -- scheduler era; nothing schedules
// them, so the guarded blocks have no state to act on); the
// FFlag::LuaDebugger branch shape (loop vs unrolled close -- both
// variants close exactly the two slots, the port uses one loop).
void ScriptContext::closeStates(bool resettingSimulation)
{
	contentClearing = true;              // +247 (first write, outside the gate)
	if (resettingSimulation)
		return;

	commandLineSandbox.reset();          // +0x6C

	if (FLog::ScriptContextClose != 0)
	{
		FASTLOG1(FLog::ScriptContextClose, "ScriptContext::closeState -- ScriptCount=%u", scripts.size());
		if (yieldEvent != NULL)
			FASTLOG1(FLog::ScriptContextClose, "ScriptContext::closeState -- WaitingThreads=%u", yieldEvent->waiterCount());
	}

	scripts.clear();                     // +0x84 (_M_erase + head rebind + size 0)

	if (yieldEvent != NULL)
		FASTLOG1(FLog::ScriptContextClose, "ScriptContext::closeState -- LeakedWaitingThreads=%u", yieldEvent->waiterCount());
	yieldEvent.reset();                  // +0xC4

	// gcJob/waitingScriptsJob removeBlocking: DELTA (members unported).

	waitingThreads.clear();              // +0xC8

	for (size_t i = 0; i < 2; ++i)
	{
		if (globalStates[i] != NULL)
		{
			closeState(globalStates[i]); // 0x18EE84 (both slots)
			globalStates[i] = NULL;
		}
	}

	if (luaAllocator.get() != NULL)
	{
		size_t heapSize, heapCount, maxHeapSize, maxHeapCount;
		luaAllocator->getHeapStats(heapSize, heapCount, maxHeapSize, maxHeapCount);
		if (FLog::ScriptContextClose != 0)
		{
			FASTLOG2(FLog::ScriptContextClose, "ScriptContext::closeState -- heapSize=%u heapCount=%u", heapSize, heapCount);
			FASTLOG2(FLog::ScriptContextClose, "ScriptContext::closeState -- maxHeapSize=%u maxHeapCount=%u", maxHeapSize, maxHeapCount);
		}
		luaAllocator->clearHeapMax();
		if (g_openLuaStates == 0 && (heapSize != 0 || heapCount != 0))
			StandardOut::singleton()->printf(MESSAGE_ERROR, "Script memory leaks: %d bytes, %d blocks", (int)heapSize, (int)heapCount);
	}
}

// mac: 0x18B4C2 (233B, batch313).
// STUB: PLAYER 0x00670d20
// STUB: STUDIO 0x007d8c80
lua_State* ScriptContext::getGlobalState(Security::Identities identity)
{
	lua_State** slot;
	if (FFlag::SegregatedLuaEnvironments)
	{
		slot = &globalStates[Security::Context::isInRole(identity,
			(Security::Permissions)4) ? 1 : 0];  // 4 = WritePlayer tier.
		RBXASSERT(*slot != NULL);  // ScriptContext.cpp:564
	}
	else
	{
		slot = &globalStates[0];
		RBXASSERT(*slot != NULL);  // ScriptContext.cpp:552
	}
	return *slot;
}

// iOS name; the ctor connects it to camelCaseViolation. After 100 reports it disconnects.
// STUB: PLAYER 0x00670df0
// STUB: STUDIO 0x007d8d50
void ScriptContext::onCamelCaseViolation(boost::shared_ptr<Instance> object, std::string memberName, boost::shared_ptr<Instance> script)
{
	if (++camelCaseViolationCount > 100)
	{
		camelCaseViolationConnection.disconnect();
		return;
	}
	std::string trueName = memberName;
	trueName[0] = toupper(memberName[0]);

	if (script)
		StandardOut::singleton()->printf(MESSAGE_WARNING, "\"%s.%s\" should be \"%s\" in %s", object->getClassNameStr().c_str(), memberName.c_str(), trueName.c_str(), script->getFullName().c_str());
	else
		StandardOut::singleton()->printf(MESSAGE_WARNING, "\"%s.%s\" should be \"%s\"", object->getClassNameStr().c_str(), memberName.c_str(), trueName.c_str());
}

// mac: 0x190A9E (101B, batch313/455). Binary-pinned STATIC (called with no
// instance); the context comes from the thread's own extra space
// (NULL pre-open -> NULL, matching the tested pre-open delegation).
// The debugger fast path (+104 slot vs thread+16) queues behind the
// debugger-state member -- loud tripwire if ever reached.
lua_State* ScriptContext::getGlobalState(lua_State* thread)
{
	Security::Identities identity =
		getThreadIdentity(thread);
	if (FFlag::LuaDebugger == false || FFlag::SegregatedLuaEnvironments == false
		|| ((int)identity & 0x1F) != 0)
	{
		ScriptContext* context = RobloxExtraSpace::get(thread)->context();
		if (context == NULL)
			return NULL;
		return context->getGlobalState(identity);
	}
	// Debugger fast path (context+104 slot vs thread+16 field) queues
	// behind the debugger-state member -- loud if the flags ever flip.
	RBXASSERT(false && "getGlobalState(thread): debugger-state era");
	return NULL;
}

// mac: 0x18EE84 (310B, batch314: ref dumps + extra-space ref wipe +
// lua_close + counter decrement. The NULL-state call matches the binary
// (unconditional wipe call); never exercised with NULL in-tree).
// STUB: PLAYER 0x00672e60
// STUB: STUDIO 0x007dad30
int ScriptContext::closeState(lua_State* L)
{
	RBX::Lua::dumpThreadRefCounts();
	RobloxExtraSpace* extra = (L != NULL)
		? RobloxExtraSpace::get(L) : NULL;
	if (extra != NULL)
		extra->eraseRefsFromAllNodes();
	RBX::Lua::dumpThreadRefCounts();
	RBX::Lua::dumpThreadRefCounts();
	if (L != NULL)
		lua_close(L);
	RBX::Lua::dumpThreadRefCounts();
	return InterlockedDecrement(&g_openLuaStates);
}

// mac: 0x190A7A (35B, batch314: summed Shared.threadCount over both slots).
int ScriptContext::getThreadCount() const
{
	int result = 0;
	for (int i = 0; i < 2; ++i)
	{
		lua_State* state = globalStates[i];
		if (state != NULL)
			result += (int)RobloxExtraSpace::get(state)->getThreadCount();
	}
	return result;
}

// mac: 0x190900 (206B, batch313: two fresh tables; the second gets
// __metatable = "The metatable is locked" plus __index -> globals,
// is set as the first's metatable, then replaces LUA_GLOBALSINDEX).
// Pseudoindices spelled verbatim (-10002 globals, -3, -2).
// FUNCTION: PLAYER 0x0066fd40
// FUNCTION: STUDIO 0x007d8100
void ScriptContext::sandboxThread(lua_State* thread)
{
	lua_createtable(thread, 0, 0);
	lua_createtable(thread, 0, 0);
	lua_pushstring(thread, "The metatable is locked");
	lua_setfield(thread, -2, "__metatable");
	lua_pushlstring(thread, "__index", 7);
	lua_pushvalue(thread, LUA_GLOBALSINDEX);
	lua_settable(thread, -3);
	lua_setmetatable(thread, -2);
	lua_replace(thread, LUA_GLOBALSINDEX);
}






















































































































































































































































































































































































































































































































































































// mac: 0x1909CE (171B, batch316: sandbox + identity/script stash in the
// thread extra space + readback assert :1231).
// STUB: PLAYER 0x00672f70
// STUB: STUDIO 0x007dae40
void ScriptContext::setThreadIdentity(lua_State* thread,
	Security::Identities identity, boost::shared_ptr<BaseScript> script)
{
	sandboxThread(thread);
	RobloxExtraSpace* extra = RobloxExtraSpace::get(thread);
	// Low 5 bits take the identity, high 3 preserved (binary
	// read-modify-write; the bitfield assignment compiles to the same).
	extra->identity = (Security::Identities)(identity & 0x1F);
	extra->script = script;
	RBXASSERT(getThreadIdentity(thread)==identity);  // ScriptContext.cpp:1231
}

// Identity readback (binary getThreadIdentity helper).
Security::Identities ScriptContext::getThreadIdentity(lua_State* thread)
{
	return (Security::Identities)(RobloxExtraSpace::get(thread)->identity & 0x1F);
}

// mac: 0x1978E9 (176B, batch321: throws runtime_error, caught by the
// script error machinery). Free function in the binary; file-local
// here (only referenced by the openLib proxy below).
// FUNCTION: PLAYER 0x0066fa60
// FUNCTION: STUDIO 0x007d7e20
static int libraryWriteGuard(lua_State* L)
{
	(void)L;
	throw std::runtime_error("can't modify this library");
}

namespace Lua {
	// 2013 sits here, between libraryWriteGuard 0x0066fa60 and openScriptLibrary 0x0066fb00,
	// so the other TUs (LuaAtomicClasses registerClass 0x00880484) call it out of line.
	// FUNCTION: PLAYER 0x0066fad0
	// FUNCTION: STUDIO 0x007d7e90
	void protect_metatable(lua_State* thread, int index)
	{
		lua_pushstring(thread, "The metatable is locked");
		lua_setfield(thread, index - 1, "__metatable");
	}
}

// mac: 0x18C76B (352B, batch320: call open + metatable lock + locked
// proxy with __index/__newindex/__metatable into globals).
// Static free function in the binary (sub_); file-local here.
// STUB: PLAYER 0x0066fb00
// STUB: STUDIO 0x007d7ec0
static void openScriptLibrary(lua_State* L, const char* libName, lua_CFunction open)
{
	int count = open(L);
	if (lua_getmetatable(L, -1) == 1)
	{
		lua_pushstring(L, "The metatable is locked");
		lua_setfield(L, -2, "__metatable");
		lua_settop(L, -2);
	}
	lua_settop(L, -(count + 1));
	lua_newuserdata(L, 0);
	lua_createtable(L, 0, 0);
	lua_getfield(L, LUA_GLOBALSINDEX, libName);
	lua_setfield(L, -2, "__index");
	lua_pushcclosure(L, libraryWriteGuard, 0);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "The metatable is locked");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_GLOBALSINDEX, libName);
}

// mac: 0x2D5790 callee: trusted-core-script spawn (lands via the 3-arg
// forwarder; identity RobloxGameScript_, name getStringById(4)).
// mac: 0x190DE7 (6B leaf): zero-argument pusher for scripts that take no
// arguments (3-arg forwarder + addLibrary). Binary body is `xor eax,eax;
// ret`; Darwin size_t = unsigned long (bind_t 'm' in the mangling) -- on
// MSVC size_t is 32-bit and convert-to-unsigned-long keeps the same
// function1<unsigned long, lua_State*> target signature.
static size_t pushNoArguments(lua_State* thread)
{
	return 0;
}

// mac: 0x190DEE (426B, batch279/710): forwarder -- default pusher, no result
// callback, empty continuations, no parent state.
void ScriptContext::executeInNewThread(RBX::Security::Identities identity, const ProtectedString& script, const char* name)
{
	executeInNewThread(identity, script, name,
		boost::bind(&pushNoArguments, _1),
		NULL,
		Scripts::Continuations(),
		NULL);
}

// mac: 0x190F98 (batch279/708/709/710): identity-gated spawn. CmdLine_ runs
// under the persistent commandLineSandbox member (+0x6C, live field
// +0x7C); every other identity runs in a fresh child of the parent state
// (or the identity's global state). Tamper telemetry: HATE_ILLEGAL_SCRIPTS
// (0x10) into the DataModel hack-flag set -- gates 1/2 mirror the image
// values of the RBX::Debugable counters (0xFA56C8/0xFA56CC, both
// 0xFFFFFFFF at rest; see the file-scope mirrors). Binary notes:
// DataModel::get is dereferenced unguarded at the two early inserts
// (0x191078) but null-checked at the write-lock insert (batch279 699-701);
// assert sites are lines 1476/1490/1517 verbatim (batch712 strings).
void ScriptContext::executeInNewThread(RBX::Security::Identities identity, ProtectedString script, const char* name,
	boost::function1<unsigned long, lua_State*> pushArguments,
	boost::function2<void, lua_State*, unsigned long> readImmediateResults,
	Scripts::Continuations continuations,
	lua_State* globalStateToExecuteIn)
{
	Lua::WeakThreadRef thread;
	lua_State* owner = NULL;

	if ((debugableScriptCounter & 1) != 0 && identity != RBX::Security::CmdLine_)
		DataModel::get(this)->addHackFlag(HATE_ILLEGAL_SCRIPTS);
	if ((debugableScriptDocCounter & 1) != 0 && identity != RBX::Security::LocalGUI_)
		DataModel::get(this)->addHackFlag(HATE_ILLEGAL_SCRIPTS);

	if (FLog::FullScriptCode != 0)
	{
		// (6) name + identity (disasm passes both varargs; hexrays dropped
		// the second), then (7) the script body (batch279 0x1910E9/0x19114A).
		StandardOut::singleton()->printf(MESSAGE_INFO, getStringById(ExecScriptNewThread),
			name ? name : "NULL", (unsigned int)identity);
		StandardOut::singleton()->printf(MESSAGE_INFO, getStringById(FullScriptCode),
			script.getStringForImmediateUse().c_str());
	}

	if (identity == RBX::Security::CmdLine_)
	{
		if (commandLineSandbox.empty())
		{
			lua_State* globalState = getGlobalState(RBX::Security::CmdLine_);
			commandLineSandbox = Lua::WeakThreadRef(lua_newthread(globalState));
			{
				Lua::ThreadRef safeSandbox = commandLineSandbox.lock();
				if (!safeSandbox)
					throw std::runtime_error(getStringById(EnableToCreateSBThread));
				setThreadIdentity(safeSandbox.get(), RBX::Security::CmdLine_, shared_ptr<BaseScript>());
			}
			lua_settop(globalState, -2);
		}
		{
			Lua::ThreadRef safeCommandlineSandbox = commandLineSandbox.lock();
			RBXASSERT_LINE(safeCommandlineSandbox, 1476);
			owner = safeCommandlineSandbox.get();
			thread = Lua::WeakThreadRef(lua_newthread(owner));
		}
	}
	else
	{
		owner = globalStateToExecuteIn ? globalStateToExecuteIn : getGlobalState(identity);
		thread = Lua::WeakThreadRef(lua_newthread(owner));
		{
			Lua::ThreadRef safeThread = thread.lock();
			RBXASSERT_LINE(safeThread, 1490);
			setThreadIdentity(safeThread.get(), identity, shared_ptr<BaseScript>());
		}
	}

	Lua::ThreadRef safeThread = thread.lock();
	if (!safeThread)
	{
		StandardOut::singleton()->print(MESSAGE_ERROR, getStringById(EnableToCreateNewThread));
		throw std::runtime_error(getStringById(EnableToCreateNewThread));
	}

	std::string sName = "=";
	if (name)
		sName += name;
	else
		sName += getStringById(ScriptStr);

	if (luaL_loadprotected(safeThread.get(), script, sName.c_str()) != 0)
	{
		std::string err = Lua::safe_lua_tostring(safeThread.get(), -1);
		lua_settop(safeThread.get(), -2);
		throw std::runtime_error(err);
	}

	RBXASSERT_LINE(continuations.empty(), 1517);
	RobloxExtraSpace::get(safeThread.get())->continuations.reset(new Lua::Continuations(continuations));

	// "Check for somebody using CheatEngine to inject a direct call."
	// (batch279 699-713: guarded, unlike the two early inserts.)
	if (DataModel* dataModel = DataModel::get(this))
	{
		if (!dataModel->currentThreadHasWriteLock())
			dataModel->addHackFlag(HATE_ILLEGAL_SCRIPTS);
	}

	resume(safeThread, pushArguments, readImmediateResults);
	lua_settop(owner, -2);
}

// mac: 0x19271A (938B, batch707/710): hand the library source to each live
// global state as a new GameScript thread; onResults files the library
// via LibraryBridge::saveLibraryResult. parent = globalStates[i] only
// under FFlag::SegregatedLuaEnvironments (off_EADED0 gate). Callers:
// LibraryService::contentReady / contentReadyLocal (the only 2 xrefs).
void ScriptContext::addLibrary(const std::string& library, const ProtectedString& code)
{
	for (int i = 0; i < 2; ++i)
	{
		if (globalStates[i] == NULL)
			continue;
		executeInNewThread(RBX::Security::GameScript_, code, library.c_str(),
			boost::bind(&pushNoArguments, _1),
			boost::bind(&Lua::LibraryBridge::saveLibraryResult, _1, _2, library),
			Scripts::Continuations(),
			FFlag::SegregatedLuaEnvironments ? globalStates[i] : NULL);
	}
}

// mac: 0x189BD8: Roblox-trusted library registration (force = true).
void ScriptContext::registerRobloxLibrary(std::string name, std::string url)
{
	libraryService->registerLibrary(name, url, true);
}

// mac: 0x189C0A: normal registration -- force = false (batch734; the earlier
// "trusted-source (force = true)" reading was wrong).
void ScriptContext::registerLibrary(std::string name, std::string url)
{
	libraryService->registerLibrary(name, url, false);
}

// mac: 0x189D7E wrapper: development-library registration + validation.
// The middle of 0x189D7E is undumped (only its jnz ref in batch707); the
// validation order (null / non-Script / running script / empty source) is
// from the throw-test decode. The three instance messages are now
// binary-verbatim (batch712 string cluster @0xb2c6xx): "Second argument
// must be non-nil and a script" / "Scripts used as libraries should be
// \"Disabled\"" / "Scripts used as libraries should always have embedded
// code". Name is forwarded to the LibraryService key verbatim.
// STUB: PLAYER 0x00682220
// STUB: STUDIO 0x007e8070
void ScriptContext::registerDevelopmentLibrary(std::string name, boost::shared_ptr<Instance> instance)
{
	if (!instance)
		throw RBX::runtime_error("Second argument must be non-nil and a script");
	Script* script = Instance::fastDynamicCast<Script>(instance.get());
	if (!script)
		throw RBX::runtime_error("Second argument must be non-nil and a script");
	if (!script->getDisabled())
		throw RBX::runtime_error("Scripts used as libraries should be \"Disabled\"");
	// Binary arm (batch734:469): the wrapper reads the LinkedSource
	// ContentId at the script (+92) and refuses a LINKED script (url
	// string non-empty) -- a dev library must carry embedded code, not a
	// linked SourceCodeId. (The former "empty embedded source" reading
	// predates the batch734 decode landing: the Script ctor seeds the
	// default code per batch768:38, so emptiness cannot be the test.)
	if (script->getScriptId().toString().size() != 0)
		throw RBX::runtime_error("Scripts used as libraries should always have embedded code");
	libraryService->registerDevelopmentLibrary(name, boost::static_pointer_cast<Script>(instance));
}

void ScriptContext::markLibrariesRegistered()
{
	libraryService->markLibrariesLoaded();
}

// Heartbeat (0x19341A, batch769:216-235, 206B, binary-verbatim order):
// 1. Library pump (member +728); 2. "Script context heartbeat start"
// FLog log; 3. timeout deadline arm (+296 span > 0 -> +304 deadline);
// 4. the timedout latch exchange (+312; a set latch decrements the
// open-timeout count +292 -- swap(0) reads-and-clears); 5. pending
// pump at the +156 deadline (startPendingScripts + 1.0s re-arm);
// 6. "Script context heartbeat done" (aScriptContextH_0). Heartbeat
// parameter unused (dead in the GCC encoding). The binary's remaining
// tail (thread-scheduler internals at +0x9C/+0x124/...) has no driver
// in the port yet -- loud DELTA; no heartbeat connection exists, so
// this is currently reached only by direct calls/tests.
// STUB: PLAYER 0x00689f00
// STUB: STUDIO 0x007ef660
void ScriptContext::onHeartbeat(const Heartbeat& heartbeat)
{
	(void)heartbeat;
	if (libraryService)
		libraryService->onHeartbeat();
	if (FLog::ScriptContext)
		FLog::FastLog(FLog::ScriptContext, "Script context heartbeat start", 0);
	if (timoutSpan.seconds() > 0.0)
		timoutTime = Time::now<Time::Fast>() + timoutSpan;
	if (timedout.swap(0) == 1)
		--timedoutCount;
	if (Time::now<Time::Fast>() >= nextPendingScripts)
	{
		startPendingScripts();
		nextPendingScripts = Time::now<Time::Fast>() + Time::Interval(1.0);
	}
	if (FLog::ScriptContext)
		FLog::FastLog(FLog::ScriptContext, "Script context heartbeat done", 0);
}

// mac: 0x18E976: store + CoreScripts log 'srp: %u' (@B2C201, from 0x18E9A1,
// batch721:27-28 -- the doorlog prints the bool as %u).
// FUNCTION: PLAYER 0x0066fd10
// FUNCTION: STUDIO 0x007d80d0
void ScriptContext::setRobloxPlace(bool robloxPlace)
{
	this->robloxPlace = robloxPlace;
	if (FLog::CoreScripts)
		FLog::FastLog(FLog::CoreScripts, "srp: %u",
			reinterpret_cast<const void*>(static_cast<unsigned>(robloxPlace)));
}

// ===================================================================
// C1 script plumbing (batches 733-771). The 2013 binary's TU is
// "Client/App/script/ScriptContext.cpp" (ReleaseAssert labels + the
// IScriptFilter.h note); the port files the bodies here beside the
// other ScriptContext methods. Assert texts recovered through the
// string-era decodes carry their binary line numbers + @B2xxxx rodata
// refs (# = unrecovered rests on the D1EAE1 skip only).
// ===================================================================


























































































































































































































































































































































































































































































































































































































































































































































































// mac: 0x195672 (removeScript, batch753:259-301): scripts.erase + the
// "num == 1" RBXASSERT (binary line 2304) + ScriptContextRemove log +
// disassociateState + first-match erase from both start queues.
// STUB: PLAYER 0x00684700
// STUB: STUDIO 0x007ea160
void ScriptContext::removeScript(BaseScript* script)
{
	std::size_t num = scripts.erase(script);
	RBXASSERT(num == 1);		// binary line 2304 ("num == 1")
	if (FLog::ScriptContextRemove)
		FLog::FastLogS(FLog::ScriptContextRemove, "RemoveScript %s", script->getName().c_str());
	disassociateState(script);
	eraseScript(pendingScripts, script);
	eraseScript(loadingScripts, script);
}

// mac: 0x19553E (disassociateState, batch754:212-245): whole body inside the
// threadNode-gated arm -- ScriptContext-gated name log
// 'ScriptContext::disassociateState -- %s' (@B2D003, batch743:14), the +756
// map entry (operator[] creates-on-miss is real in the binary), the px
// erase walk (+ entry erase when the vector empties),
// Node::eraseAllRefs, the node release, and the stopped signal.
// STUB: PLAYER 0x00684610
// STUB: STUDIO 0x007ea070
void ScriptContext::disassociateState(BaseScript* script)
{
	if (!script->threadNode)
		return;

	if (FLog::ScriptContext)
		FLog::FastLogS(FLog::ScriptContext, "ScriptContext::disassociateState -- %s",
			script->getName().c_str());

	std::string fullName = script->getFullName();
	if (!fullName.empty())
	{
		ScriptStatInformation& info = scriptStatsMap[fullName];
		for (std::vector<boost::shared_ptr<Instance>>::iterator it = info.runningScripts.begin();
			it != info.runningScripts.end(); ++it)
		{
			if (it->get() == script)
			{
				info.runningScripts.erase(it);
				break;
			}
		}
		if (info.runningScripts.empty())
			scriptStatsMap.erase(fullName);
	}

	script->threadNode->eraseAllRefs();
	script->threadNode = boost::intrusive_ptr<Lua::WeakThreadRef::Node>();
	script->stopped();
}

// mac: 0x193530 (startPendingScripts, batch742:376-500): drain loadingScripts
// unconditionally; then -- with the ScriptsDisabled prop clear -- drain
// pendingScripts under the CoreScripts per-item logs and the elapsed
// watchdog (the "num left" countdown ticks after each script). The
// threshold is the double qword @C1C910 = 30.0 (batch771:23; batch742:471
// reads it through the A893CF + 0x193541 thunk).
// STUB: PLAYER 0x00687ea0
// STUB: STUDIO 0x007ed600
void ScriptContext::startPendingScripts()
{
	std::vector<ScriptStart> loading;
	loading.swap(loadingScripts);
	for (std::vector<ScriptStart>::iterator it = loading.begin(); it != loading.end(); ++it)
		startScript(*it);

	if (scriptsDisabled == 0)
	{
		std::vector<ScriptStart> pending;
		pending.swap(pendingScripts);
		Time startTime = Time::now<Time::Fast>();
		int numLeft = static_cast<int>(pending.size());
		for (std::vector<ScriptStart>::iterator it = pending.begin(); it != pending.end(); ++it)
		{
			if (FLog::CoreScripts)
			{
				FLog::FastLog(FLog::CoreScripts, "Starting pending script: %p",
					reinterpret_cast<const void*>(it->script.get()));
				// Binary quirk (batch742:453-462): the name log is FastLogS
				// with the %p fmt @B2CE48 and the name c_str as the string
				// arg (the fw(script)+24 read, batch742:457-461).
				FLog::FastLogS(FLog::CoreScripts, "Pending script name: %p",
					it->script->getName().c_str());
			}
			startScript(*it);
			if ((Time::now<Time::Fast>() - startTime).seconds() > 30.0)
				FLog::FastLog(FLog::CoreScripts, "startPendingScripts time out, num left %d",
					reinterpret_cast<const void*>(numLeft));
			--numLeft;
		}
	}
}

// mac: 0x19563A (eraseScript): erase the FIRST queue entry whose script px
// matches (removeScript calls it on both queues; later duplicates stay).
void ScriptContext::eraseScript(std::vector<ScriptStart>& queue, BaseScript* script)
{
	for (std::vector<ScriptStart>::iterator it = queue.begin(); it != queue.end(); ++it)
	{
		if (it->script.get() == script)
		{
			queue.erase(it);
			break;
		}
	}
}

// mac: 0x193E26 (startScript, batch741:45-435, 2,066B, binary-verbatim
// order): the parent/membership assert gates (lines 2549/2550 -- texts
// 'script->getRootAncestor() == getRootAncestor()' @B2D223 and
// 'scripts.find(script.get()) != scripts.end()' @B2D2E5, batch743:32-35),
// the ScriptsDisabled pend arm (+164), the hoisted
// BaseScript::Code (binary-verbatim; reads garbage in the binary when
// the provider gate skips -- the port reads the default-constructed
// holder), the provider-match requestCode warm + loading pend (+176),
// the re-entrancy guard (binary text verbatim), the logs/settings
// prints, the identity resolution (2 -> 4; non-roblox scripts without
// the +244 byte and outside RobloxPlace mode drop to (robloxPlace|2)),
// the newthread + hook mask + setHook log pair, identity stamping, the
// threadNode registry, the +756 stats entry, the "script" global +
// chunkname, the transform arm, the Continuations bag, and the
// loadprotected fail/success arms with the common stack tail. String
// stand-ins retired -- all resolved: the "script" setfield key
// (@B1C1DE, batch743:51), the "=" chunkname prefix (@B247A3,
// batch744:3/batch753:22), the printf format 'Full script code:\n %s'
// (@B7BA73 = unk_9E7C3C + 0x193E37, batch741:709 -- shared with
// loadfile), and the setHook log's name arg (Instance::fw(script) + 24
// c_str, batch771:1-5 / batch716:907). Remaining loud DELTA: the
// duplicated RbxInterlockedDecrement through the off_D1EAE1
// pointer-table slot (batch741:404/424 -- the pointed global never
// resolved in the dumped material; the x2 pair is skipped).
// STUB: PLAYER 0x00686870
// STUB: STUDIO 0x007ec1d0
void ScriptContext::startScript(ScriptStart start)
{
	BaseScript* script = start.script.get();
















































































































	// (assert texts recovered: batch743:32-35)
	RBXASSERT(script->getRootAncestor() == getRootAncestor());	// binary line 2549 (@B2D223)
	RBXASSERT(scripts.find(script) != scripts.end());			// binary line 2550 (@B2D2E5)

	// Binary logic inverted: with the prop SET, non-core scripts pend
	// instead of starting.
	if (scriptsDisabled != 0 &&
		(script == NULL || Instance::fastDynamicCast<CoreScript>(script) == NULL))
	{
		pendingScripts.push_back(start);
		return;
	}

	// (binary-verbatim: declared before the provider gate -- when the
	// providers mismatch, the binary reads it uninitialized; the port
	// reads the default holder.)
	BaseScript::Code code;
	if (ServiceProvider::findServiceProvider(start.script.get()) == ServiceProvider::findServiceProvider(this))
	{
		code = start.script->requestCode(NULL);
		if (code.script.getStringForImmediateUse().empty())
		{
			loadingScripts.push_back(start);
			return;
		}
	}

	if (reentrancy >= 4)
		throw std::runtime_error("startScript re-entrancy has exceeded 3");
	++reentrancy;

	if (FLog::ScriptContext)
		FLog::FastLog(FLog::ScriptContext, "Running script %p",
			reinterpret_cast<const void*>(start.script.get()));
	if (LuaSettings::singleton().getUnknown120())
	{
		StandardOut::singleton()->printf(MESSAGE_INFO, "Running Script \"%s\"",
			script->getFullName().c_str());
	}
	// The pointer-table byte dword_D1A07D (batch741:229) gates the source
	// dump printf; the fmt resolved to 'Full script code:\n %s' @B7BA73
	// (unk_9E7C3C + 0x193E37, batch741:709 -- the loadfile literal).
	// (Reads the default `code` when the provider gate skipped, as the
	// binary does.)
	if (FLog::StartScriptSourceDump != 0)
		StandardOut::singleton()->printf(MESSAGE_INFO, "Full script code:\n %s",
			code.script.getStringForImmediateUse().c_str());
	int identity = start.identity;
	if (identity == 2)
	{
		identity = 4;
		if (!script->isRobloxScript() && !m_unk0xf0)
			identity = (robloxPlace != 0) | 2;
	}
	lua_State* globalState = getGlobalState((RBX::Security::Identities)identity);
	lua_State* thread = lua_newthread(globalState);
	if (thread == NULL)
		throw RBX::runtime_error("Unable to create a new thread for %s",
			script->getFullName().c_str());
	// (RbxFormat.h:44: RBX::runtime_error is the printf-style
	// std::runtime_error factory; the binary's ctor call carried the
	// fmt + the getFullName result.)
	int hookMask = 11;
	if (FLog::LuaProfiler == 0)
		hookMask = 3 * (FFlag::LuaProfilerHooks != 0) + 8;
	// setHook log arg, resolved (batch771:1-5 / batch716:907): the binary
	// reads Instance::fw(script) (0x544AF8 = *(this+68)) then that
	// object's +24 c_str -- the instance name string; landed
	// getName().c_str().
	if (FLog::ScriptContext)
		FLog::FastLogS(FLog::ScriptContext, "lua_sethook %s", script->getName().c_str());
	lua_sethook(thread, &ScriptContext::hook, hookMask, 1000);
	setThreadIdentity(thread, (RBX::Security::Identities)identity, start.script);
	RBXASSERT(!script->threadNode);		// binary line 2621
	script->threadNode = Lua::WeakThreadRef::Node::create(thread);

	std::string fullName = script->getFullName();
	if (!fullName.empty())
	{
		ScriptStatInformation& info = scriptStatsMap[fullName];
		info.name = script->getName();
		info.runningScripts.push_back(start.script);
	}

	{
		Lua::ThreadRef safeThread(thread);
		RBXASSERT(lua_gettop(thread)==0);		// binary line 2634
		Lua::SharedPtrBridge<Instance>::push(thread, start.script);
		// setfield key resolved @B1C1DE (unk_9883A7 + 0x193E37, batch743:51 / batch753:21) = the Roblox running-script global "script" -- landed literal correct.
		lua_setfield(thread, LUA_GLOBALSINDEX, "script");
		// Chunkname prefix resolved @B247A3 (unk_99096C + 0x193E37, batch744:3 / batch753:22) = the standard Lua "=" prefix -- landed literal correct. The binary makes its own second getFullName call here.
		std::string chunkname = std::string("=") + script->getFullName();
		ProtectedString source;
		if (!start.sourceTransform.empty() && FLog::StartScriptSourceTransform != 0)
			source = ProtectedString::fromTrustedSource(
				start.sourceTransform(code.script.getStringForImmediateUse()));
		else
			source = code.script;
		// Continuations bag only when either handler is set (the ctor clones the start's two-function block, batch741:360-368).
		if (!start.continuation.empty() || !start.reportError.empty())
		{
			Scripts::Continuations conts;
			conts.successHandler = start.continuation;
			conts.errorHandler = start.reportError;
			RobloxExtraSpace::get(thread)->continuations.reset(new Lua::Continuations(conts));
		}
		if (luaL_loadprotected(thread, source, chunkname.c_str()) != 0)
		{
			reportError(thread);
			Lua::Continuations* c = RobloxExtraSpace::get(thread)->continuations.get();
			if (c != NULL && !c->error.empty() && FLog::StartScriptErrorContinuation != 0)
				c->error(thread);
			lua_settop(thread, -2);
		}
		else
		{
			script->starting(thread);
			resume(safeThread, 0);
		}
		lua_settop(thread, 0);
	}
	lua_settop(globalState, -2);
	--reentrancy;
	// (skipped, loud DELTA: the duplicated RbxInterlockedDecrement through the off_D1EAE1 pointer-table slot -- batch741:404 (the success arm, after resume) and batch741:424 (the common tail); the pointed global never resolved in the dumped material.)
}
// mac: 0x18B2C2 (57-line body; batch744:281-338 == batch747:538-595): the lua hook dispatcher (armed by startScript at mask 8 -- line-only -- or 11 = call+ret+line with FFlag::LuaProfilerHooks, so call/ret are the profiler surface and line is the timeout hand-off). event 3 (LUA_HOOKLINE): the owning ScriptContext arrives through the double-deref *( *(L-28) + 4 ) = extra->shared->context (the binary extra space is 40B: shared_ptr<Shared> at +12, and Shared.context = ScriptContext* at its +4 -- batch746's landed luaconf.h shapes). FASTLOG "ScriptContext::hook, timeout count %d" (@B2BE9D, batch749:13) with timedoutCount (+292), and when timedoutCount > 0, onHook(L, ar). event 1 (LUA_HOOKRET): if the profiler pointer is set, FASTLOGS(FLog::LuaProfiler, "LUARET" @B2DF41, NULL). event not in {0,1,3}: the 2013 build's line-506 guard (assert "false", batch744:319-331 -- "line: 506"). event 0 (LUA_HOOKCALL): if the profiler pointer is set, hookCall(L, ar) (real body landed TENTH ERA: lua_getinfo "nS" + stringCache.getCall + FastLogS, script\LuaProfiler.h comment). DELTA: the binary __cdecls an unsigned char up the FastLog gate results; the Lua dispatcher discards it -- landed void (the static declaration at ScriptContext.h predates the era).
// STUB: PLAYER 0x0067e3a0
// STUB: STUDIO 0x007e4320
void ScriptContext::hook(lua_State* thread, lua_Debug* ar)
{
	int event = ar->event;
	if (event == 3)		// LUA_HOOKLINE
	{
		ScriptContext* sc = RobloxExtraSpace::get(thread)->context();
		if (FLog::ScriptContext)
			FLog::FastLog(FLog::ScriptContext, "ScriptContext::hook, timeout count %d",
				reinterpret_cast<const void*>((int)sc->timedoutCount));
		if (sc->timedoutCount > 0)
			sc->onHook(thread, ar);
	}
	else if (event == 1)	// LUA_HOOKRET
	{
		if (LuaProfiler::instance != NULL)
			FLog::FastLogS(FLog::LuaProfiler, "LUARET", NULL);
	}
	else if (event != 0)
	{
		RBXASSERT(false);		// binary ScriptContext.cpp line 506
	}
	else	// event == 0: LUA_HOOKCALL
	{
		if (LuaProfiler::instance != NULL)
			LuaProfiler::instance->hookCall(thread, ar);	// 0x197D28, real body (TENTH ERA)
	}
}
// mac: 0x18B3EC (16-line body, batch746:118-134, the hook era): the script- kill half of the line hook. If the timedout latch (+312, set by the heartbeat/consumer via onCheckTimeout's swap(1) arm) is live AND the current identity is one of the two game-script permissions (2 = GameScript_ / 4 = RobloxGameScript_, the same test as resume()'s contentClearing gate), raised luaL_error "Game script timout" on the thread. The binary returns the this-pointer as int; landed void (discarded by the dispatcher).
// STUB: PLAYER 0x0067a150
// STUB: STUDIO 0x007e2070
void ScriptContext::onHook(lua_State* thread, lua_Debug* ar)
{
	(void)ar;
	if (timedout != 0
		&& (Security::Context::current().identity == Security::GameScript_
			|| Security::Context::current().identity == Security::RobloxGameScript_))
		luaL_error(thread, "Game script timout");
}
// mac: 0x189990 (AddStarterScript scriptable, batch733:28-85): 0 assetId throws (binary text verbatim); rbxassetid://%d create<StarterScript>; setParentInternal AT this; the closing CoreScripts log 'Added Starter Script, asset id: %u, script: %p' (@B2CEC5, batch743:11).
// STUB: PLAYER 0x006776c0
// STUB: STUDIO 0x007df380
boost::shared_ptr<Instance> ScriptContext::addStarterScript(int assetId)
{
	if (assetId == 0)
		throw std::runtime_error("AddStarterScript bad argument: assetId = 0");
	boost::shared_ptr<Instance> script =
		Creatable<Instance>::create<StarterScript>(ContentId(RBX::format("rbxassetid://%d", assetId)));
	// (the binary's setParentInternal(parent, false) == the public setParent form)
	script->setParent(this);
	if (FLog::CoreScripts)
		FLog::FastLog(FLog::CoreScripts, "Added Starter Script, asset id: %u, script: %p",
			reinterpret_cast<const void*>(assetId),
			reinterpret_cast<const void*>(script.get()), 0);
	return script;
}
// mac: 0x189E1A (AddCoreScript scriptable, batch744:341-500): name log, 0 assetId throws (binary text verbatim), parent defaults to this, rbxassetid://%d create<CoreScript>, setName when non-empty, setParentInternal(parent), addScript{identity 2} when not parented at this, the closing CoreScripts log, and the created script returned.
// STUB: PLAYER 0x0068a1b0
// STUB: STUDIO 0x007ef910
boost::shared_ptr<Instance> ScriptContext::addCoreScript(int assetId, boost::shared_ptr<Instance> parent, std::string name)
{
	if (FLog::CoreScripts)
		FLog::FastLogS(FLog::CoreScripts, "Adding core script name: %s", name.c_str());
	if (assetId == 0)
		throw std::runtime_error("AddCoreScript bad argument: assetId = 0");
	if (parent == NULL)
		parent = shared_from(this);
	if (FLog::CoreScripts)
		FLog::FastLogS(FLog::CoreScripts, "Parent name: %s", parent->getFullName().c_str());
	boost::shared_ptr<CoreScript> script =
		Creatable<Instance>::create<CoreScript>(ContentId(RBX::format("rbxassetid://%d", assetId)));
	if (!name.empty())
		script->setName(name);
	// (the binary's setParentInternal(parent, false) == the public setParent form)
	script->setParent(parent.get());
	if (script->getParent() != this)
	{
		ScriptStartOptions options;
		options.identity = 2;
		addScript(script.get(), options);
	}
	if (FLog::CoreScripts)
		FLog::FastLog(FLog::CoreScripts, "Added Core script, asset: %u, parent: %p, script: %p",
			reinterpret_cast<const void*>(assetId),
			reinterpret_cast<const void*>(parent.get()),
			reinterpret_cast<const void*>(script.get()));
	return script;
}
// mac: 0x195358 (addScript, batch744:341-500): scripts.insert + the RBXASSERT gate (binary text "ok", line 2256), the ScriptContextAdd log, the 60B ScriptStart field-copy build (each boost function assigns its own state), and startScript BY VALUE.
// STUB: PLAYER 0x006880e0
// STUB: STUDIO 0x007ed840
void ScriptContext::addScript(BaseScript* script, ScriptStartOptions options)
{
	bool ok = scripts.insert(script).second;
	RBXASSERT(ok);		// binary line 2256 ("ok")
	if (FLog::ScriptContextAdd)
		FLog::FastLogS(FLog::ScriptContextAdd, "ScriptContext::addScript -- %s",
			script->getName().c_str());
	ScriptStart start;
	start.script = shared_from(script);
	start.identity = options.identity;
	start.continuation = options.continuation;
	start.reportError = options.reportError;
	start.sourceTransform = options.sourceTransform;
	startScript(start);
}
// mac: 0x18A2F4 (the ScriptsDisabled prop onChanged): binary jumps to startPendingScripts when the prop byte is SET -- inverted vs the drain gate's '== 0' (batch742); the landed prop-descriptor signature is the binding contract and the binary decompiles the body as parameterless (batch742), so the parameter closes unused.
// FUNCTION: PLAYER 0x006880c0
// FUNCTION: STUDIO 0x007ed820
void ScriptContext::onChangedScriptEnabled(const Reflection::PropertyDescriptor& descriptor)
{
	(void)descriptor;
	if (scriptsDisabled != 0)
		startPendingScripts();
}
// mac: 0x1952EA (the IScriptFilter override; batch743:379-397 full decompile): live script -> CoreScript's descriptor -> isA gate -> true, else false. The return arm renders as '(RBX::BaseScript*)((char*)a2 + 36) != (RBX::BaseScript*)36' = pointer-truth on the argument (the session-133 askSetParent mac: 0x444E5A artifact family: true for any non-null post-gate) -- conventional shape confirmed by the 2016 tree (:2781-2784 'return Instance::fastDynamicCast<CoreScript>(script) != NULL', crosscheck only, binary wins). The old "+36-byte walk unrecovered" DELTA is retired -- there is no member read at +36.
/*override*/ bool ScriptContext::scriptShouldRun(BaseScript* script)
{
	return script != NULL && RBX::Instance::fastDynamicCast<CoreScript>(script) != NULL;
}
// mac: 0x18B230 (batch599: +276 store; enable creates the +280 holder via ServiceProvider::find<StatsService> -- NOT create, so with no service parented only the flag flips; re-enable with a live holder is a no-op ([+280] px check); disable releases the holder. Binary +280/+284 are the shared_ptr px/pn words (the audit's "+0x11C second holder" reading was wrong -- 0x18B28D loads and releases the pn word of the same shared_ptr).
void ScriptContext::setCollectScriptStats(bool collect)
{
	collectStats = collect;
	if (collect)
	{
		if (scriptStats.get() == NULL)
		{
			if (ServiceProvider::find<Stats::StatsService>(this) != NULL)
				scriptStats.reset(new ScriptStats());
		}
	}
	else
		scriptStats.reset();
}
// mac: 0x18A6AC getScriptStatsNew (batch733: 282-line body; the visible prefix lands binary-verbatim -- the +280 holder gate with the "Script stats collection is not enabled" runtime_error, the make_shared<vector<Variant>> result sized by the internal meter map's node count (+20), and the per-script entry whose "Hash" fill is the unordered-map operator[] onto Variant(string), then the +756-map find keyed by the SAME hash string). The tail beyond the "Name" temp is unrecovered (batch733 dump cap at line 354): the key roster below ("Count","Activity","InvocationCount") is 2016-shaped (2016 getScriptStatsNew/getScriptStats), LOUD DELTAs: -- 2016's "Scripts" per-entry key carries shared_ptr<const Instances>; Oct2013 carries no Instances class (tree-wide absence), and the truncated region shows exactly one shared_ptr temp of unrecoverable type -- realized here as "Count" = runningScripts.size() (2016 getScriptStats' shell), payload shape unrecoverable byte-exact. -- the [Unknown] name-arm and the find-not-found -1 count are 2016 conventional (2013 fail-side shape unrecovered). Registered as func_GetScriptStats 0xFA6630: "GetScriptStats", security 4, arity-0 (batch757:704-716).
// STUB: PLAYER 0x00685580
// STUB: STUDIO 0x007eaee0
boost::shared_ptr<const std::vector<Reflection::Variant> > ScriptContext::getScriptStatsNew()
{
	if (scriptStats.get() == NULL)
		throw std::runtime_error("Script stats collection is not enabled");
	const ScriptStats::ScriptActivityMeterMap& stats = scriptStats->getScriptActivityMap();
	boost::shared_ptr<std::vector<Reflection::Variant> > result(
		rbx::make_shared<std::vector<Reflection::Variant> >(
			static_cast<unsigned long>(stats.size())));
	int pos = 0;
	for (ScriptStats::ScriptActivityMeterMap::const_iterator iter = stats.begin();
		iter != stats.end(); ++iter)
	{
		std::string hash = iter->first;
		boost::shared_ptr<Reflection::ValueTable> entry(
			rbx::make_shared<Reflection::ValueTable>());
		(*entry)["Hash"] = hash;
		std::map<std::string, ScriptStatInformation>::const_iterator it =
			scriptStatsMap.find(hash);
		if (it != scriptStatsMap.end()) {
			(*entry)["Name"] = (std::string)it->second.name;
			(*entry)["Count"] = (int)it->second.runningScripts.size();
		}
		else {
			(*entry)["Name"] = std::string("[Unknown]");
			(*entry)["Count"] = (int)-1;
		}
		(*entry)["Activity"] = (double)iter->second.activity->averageValue() * 100;
		(*entry)["InvocationCount"] =
			(double)iter->second.invocations->getTotalValuePerSecond();
		(*result)[pos++] = boost::shared_ptr<const Reflection::ValueTable>(entry);
	}
	return result;
}
// mac: 0x1902A8 getScriptStats: name listed (batch755:180) but the body is unrecovered (no decompile in C:\tmp; no descriptor binds it in the InitFunc_123 window). 2016 shell (the 1077-1106 Tuple of 5) with the null-holder early return; LOUD DELTA: conventional port, the 2013 return type is unrecorded (mangled names never carry it).
boost::shared_ptr<const Reflection::Tuple> ScriptContext::getScriptStats()
{
	if (scriptStats.get() == NULL)
		return boost::shared_ptr<const Reflection::Tuple>();
	const ScriptStats::ScriptActivityMeterMap& stats = scriptStats->getScriptActivityMap();
	boost::shared_ptr<Reflection::Tuple> result(
		rbx::make_shared<Reflection::Tuple>(stats.size()));
	int pos = 0;
	for (ScriptStats::ScriptActivityMeterMap::const_iterator iter = stats.begin();
		iter != stats.end(); ++iter)
	{
		std::string hash = iter->first;
		boost::shared_ptr<Reflection::Tuple> entry(rbx::make_shared<Reflection::Tuple>(5));
		entry->values[0] = hash;												//Hash
		std::map<std::string, ScriptStatInformation>::const_iterator it =
			scriptStatsMap.find(hash);
		if (it != scriptStatsMap.end()) {
			entry->values[1] = (std::string)it->second.name;					//Name
			entry->values[2] = (int)it->second.runningScripts.size();			//Count
		}
		else {
			entry->values[1] = std::string("[Unknown]");						//Name
			entry->values[2] = (int)-1;											//Count
		}
		entry->values[3] = (double)iter->second.activity->averageValue() * 100;	//Activity
		entry->values[4] =
			(double)iter->second.invocations->getTotalValuePerSecond();			//InvocationCount
		result->values[pos++] = boost::shared_ptr<const Reflection::Tuple>(entry);
	}
	return result;
}
// Panic handler for lua_atpanic (sub_18C6D5; 2016 shape carried minus the callstack print, which queues with printCallStack).
static int scriptPanic(lua_State* L)
{
	StandardOut::singleton()->printf(MESSAGE_ERROR,
		"Unprotected error in call to Lua API (%s)\n", lua_tostring(L, -1));
	RBXCRASH();
	return 0;
}
// mac: 0x18DB92 (250B, batch324).
// STUB: PLAYER 0x0067a9b0
// STUB: STUDIO 0x007e28d0
int ScriptContext::printidentity(lua_State* L)
{
	if (lua_gettop(L) <= 0)
	{
		StandardOut::singleton()->printf(MESSAGE_OUTPUT, "Current identity is %d",
			(int)Security::Context::current().identity);
	}
	else
	{
		const char* s = lua_tolstring(L, -1, NULL);
		StandardOut::singleton()->printf(MESSAGE_OUTPUT, "%s %d", s,
			(int)Security::Context::current().identity);
	}
	return 0;
}
// mac: 0x18C8CC (653B, batch324: tostring each arg joined with spaces to StandardOut; tostring-miss throws; spacing via fputs).
int ScriptContext::print(lua_State* L)
{
	int top = lua_gettop(L);
	std::string out;
	lua_getfield(L, LUA_GLOBALSINDEX, "tostring");
	if (top > 0)
	{
		for (int i = 1; i <= top; ++i)
		{
			if (i >= 2)
			{
				out.push_back(' ');
				fputs(" ", stdout);
			}
			lua_pushvalue(L, -1);
			lua_pushvalue(L, i);
			lua_call(L, 1, 1);
			const char* s = lua_tolstring(L, -1, NULL);
			if (s == NULL)
				throw std::runtime_error("tostring failed");  // DELTA: binary message bytes
				                                              // at unk_9A0493 unrecovered.
			out.append(s, strlen(s));
			lua_settop(L, -2);
		}
	}
	StandardOut::singleton()->print(MESSAGE_OUTPUT, out);
	return 0;
}
// mac: 0x18B48A (56B, batch326).
// FUNCTION: PLAYER 0x0067a190
// FUNCTION: STUDIO 0x007e20b0
int ScriptContext::crash(lua_State* L)
{
	(void)L;
	Security::Context::current().requirePermission((Security::Permissions)2);
	RBXCRASH("User-generated crash");
	return 0;
}
// mac: 0x18CB5A (36B, batch324: pushes G3D wall time).
int ScriptContext::tick(lua_State* L)
{
	lua_pushnumber(L, G3D::System::time());
	return 1;
}
// mac: 0x18CBB0 (40B, batch324: pushes sampled epoch seconds).
int ScriptContext::rbxTime(lua_State* L)
{
	lua_pushnumber(L, RBX::Time::now<(RBX::Time::SampleMethod)0>().timestampSeconds());
	return 1;
}
// sub_193123 (73B, batch472): floor a wait timeout at the LuaSettings wait setting (~0.03); NaN/Inf also clamp there. (2016's cleanTimeout is the same shape against defaultWaitTime.)
static void clampWaitTimeout(double& timeout)
{
	double floor = LuaSettings::singleton().getDefaultWaitTime();
	if (floor > timeout || RBX::Math::isNanInf((float)timeout))
		timeout = floor;
}
// mac: 0x18CBD8 (98B, batch471): clamp + queue self + yield (caller resumes with (elapsed, now); see YieldingThreads::resume, section 250).
int ScriptContext::wait(lua_State* L)
{
	double timeout = lua_tonumber(L, 1);
	clampWaitTimeout(timeout);
	Lua::YieldingThreads* yt = RobloxExtraSpace::get(L)->context()->yieldEvent.get();
	if (yt != NULL)
		yt->queueWaiter(L, timeout);
	return lua_yield(L, 0);
}
// mac: 0x18D8A4 (499B, batch471/475): spawn = delay with arg-1 and timeout 0 ("Spawn function requires 1 argument", ... line: 1606).
// STUB: PLAYER 0x00674740
// STUB: STUDIO 0x007dc330
int ScriptContext::spawn(lua_State* L)
{
	Lua::ThreadRef threadLock(L);
	if (lua_gettop(L) <= 0)
		throw std::runtime_error("Spawn function requires 1 argument");
	lua_State* child = lua_newthread(L);
	// Fresh-child L-20 slot: see delay() -- same zeroing.
	*(void**)((char*)child - 20) = NULL;
	Lua::WeakThreadRef functor(child);
	Lua::ThreadRef safeFunctor = functor.lock();
	RBXASSERT_LINE(safeFunctor, 1606);
	lua_pushvalue(L, 1);
	lua_xmove(L, safeFunctor.get(), 1);
	Lua::YieldingThreads* yt = RobloxExtraSpace::get(L)->context()->yieldEvent.get();
	if (yt != NULL)
		yt->queueWaiter(safeFunctor.get(), 0);
	lua_settop(L, -2);
	return 0;
}
// mac: 0x18CC3A (523B, batch471/475): new thread + move arg-2 + queue with timeout; returns immediately (0 values). Error texts pinned by batch475 disasm ("delay function requires 2 arguments") and batch476 rodata ("safeFunctor .../ScriptContext.cpp line: 1651").
// STUB: PLAYER 0x0067a3b0
// STUB: STUDIO 0x007e22d0
int ScriptContext::delay(lua_State* L)
{
	Lua::ThreadRef threadLock(L);
	if (lua_gettop(L) <= 1)
		throw std::runtime_error("delay function requires 2 arguments");
	double timeout = lua_tonumber(L, 1);
	clampWaitTimeout(timeout);
	lua_State* child = lua_newthread(L);
	// Defensive: the fresh child's L-20 slot must read NULL before any WeakThreadRef touches it (the binary's states arrive NULL-slotted on fresh pages; recycled pages can carry garbage). The slot IS the fork node member (section 257 audit: 40B extra space), and the store is a no-op when construction already nulled it.
	*(void**)((char*)child - 20) = NULL;
	Lua::WeakThreadRef functor(child);
	Lua::ThreadRef safeFunctor = functor.lock();
	RBXASSERT_LINE(safeFunctor, 1651);
	lua_pushvalue(L, 2);
	lua_xmove(L, safeFunctor.get(), 1);
	Lua::YieldingThreads* yt = RobloxExtraSpace::get(L)->context()->yieldEvent.get();
	if (yt != NULL)
		yt->queueWaiter(safeFunctor.get(), timeout);
	lua_settop(L, -2);
	return 0;
}
// mac: 0x193B70 (315B, batch484): push the Tuple, resume with its size, restore the top.
void ScriptContext::resumeWithArgs(Lua::ThreadRef thread,
	boost::shared_ptr<const Reflection::Tuple> args)
{
	lua_State* live = thread.get();
	int top = lua_gettop(live);
	int count = Lua::LuaArguments::pushTuple(*args, live);
	Lua::ThreadRef pin(thread);
	resume(pin, count);
	lua_settop(live, top);
}
// mac: 0x1937F8 (887B, batch484): drain up to a snapshot size of the resume queue through resumeWithArgs, then run due waiters. Stops past the expiration time. DELTAs (loud): the drain cap is a snapshot size (the binary computes an equivalent bound from queue member diffs); the two telemetry samples (+620/+676) are skipped; now/limit for the waiter pass are sampled fresh (the binary reads them off a +188 time object).
// STUB: PLAYER 0x0068a000
// STUB: STUDIO 0x007ef760
void ScriptContext::resumeWaitingScripts(RBX::Time expirationTime)
{
	if (FLog::ScriptContext != 0)
		FLog::FastLog(FLog::ScriptContext, "ScriptContext::resumeWaitingScripts start", NULL);
	size_t n = waitingThreads.size();
	while (n-- > 0)
	{
		WaitingThread waiter;
		if (!waitingThreads.pop_if_present(waiter))
			break;
		resumeWithArgs(waiter.thread, waiter.result);
		if (RBX::Time::nowFastSec() > expirationTime.timestampSeconds())
			break;
	}
	if (yieldEvent.get() != NULL)
	{
		bool maxed = false;
		yieldEvent->resume(RBX::Time::now<RBX::Time::Fast>().timestampSeconds(),
			expirationTime, maxed);
	}
	if (FLog::ScriptContext != 0)
		FLog::FastLog(FLog::ScriptContext, "ScriptContext::resumeWaitingScripts finish", NULL);
}
// mac: 0x18E942 (52B, batch471): pushes DebugSettings::robloxVersion.
int ScriptContext::version(lua_State* L)
{
	lua_pushlstring(L, DebugSettings::robloxVersion.c_str(), DebugSettings::robloxVersion.size());
	return 1;
}
// mac: 0x18E89C (batch599: ServiceProvider::create<StatsService> from the calling thread's ScriptContext (binary reads shared.px at L-28, context at +4 -- our context() accessor) + SharedPtrBridge push; returns 1. No null guard, binary-identical: openState always links the context).
int ScriptContext::stats(lua_State* L)
{
	ScriptContext* sc = RobloxExtraSpace::get(L)->context();
	Stats::StatsService* service = ServiceProvider::create<Stats::StatsService>(sc);
	Lua::SharedPtrBridge<Instance>::push(L, shared_from(service));
	return 1;
}
	// mac: 0x18A54C (352B, batch587: allocator counters into a 4-int Tuple (heapSize, heapCount, maxHeapSize, maxHeapCount); optional clearHeapMax. Variant(int) sets getSingleton<int> + value, exactly the binary type/operator=<int> pair per element. allocatorView must exist (openState creates it) -- no null guard, binary-identical). Reflection era (batch733/734): return type const-shared per the binary mangled signature (shared_ptr<Tuple const>); the other spelling had no getSingleton spec and broke the exe link.
	shared_ptr<const Reflection::Tuple> ScriptContext::getHeapStats(bool clearHighwaterMark)
	{
		size_t a, b, c, d;
		luaAllocator->getHeapStats(a, b, c, d);
		shared_ptr<Reflection::Tuple> result(new Reflection::Tuple(4));
		result->at(0) = Reflection::Variant((int)a);
		result->at(1) = Reflection::Variant((int)b);
		result->at(2) = Reflection::Variant((int)c);
		result->at(3) = Reflection::Variant((int)d);
		if (clearHighwaterMark)
			luaAllocator->clearHeapMax();
		return result;
	}
// Exit-handler dispatch gate (resume#1/#2 read dword_D1B1CC: handlers fire only when set. BSS-false default; no setter is banked anywhere DB-wide, so production behavior with a fresh binary is off -- model exactly that. The scheduler tests (251) flip it if the drain side needs handlers live.)
static bool g_fireExitHandlers = false;
// (the old C-shell LuaProfilerShell struct was retired with the TENTH ERA: the real global-scope LuaProfiler is defined in include\script\LuaProfiler.h with its bodies above, and ScriptContext::resume constructs it directly.) mac: 0x192A50 (512B, batch471): ThreadRef + arg-pusher/results callbacks around resume#2. Used by WaitScriptSlot::operator()/callCallback (section 251); landed now so the shape is fixed. Judgment calls (loud): the null-live early return (binary blind-derefs into lua_gettop -- AV, unreachable from real callers); the count pair is net-zero (the decomp shows the Decrement; a matching Increment keeps the diagnostic count stable instead of draining it).
int ScriptContext::resume(Lua::ThreadRef thread,
	boost::function1<unsigned long, lua_State*> pushArguments,
	boost::function2<void, lua_State*, unsigned long> onResults)
{
	lua_State* live = thread.get();
	if (live == NULL)
		return 0;
	int top = lua_gettop(live);
	unsigned long nargs = pushArguments(live);
	Lua::ThreadRef pin(thread);
	int status = RobloxExtraSpace::get(live)->context()->resume(pin, (int)nargs);
	if (status == 2)
	{
		if (onResults)
		{
			std::string message = Lua::safe_lua_tostring(live, -1);
			lua_settop(live, top);
			throw std::runtime_error(message);
		}
		lua_settop(live, top);
	}
	else
	{
		int results = lua_gettop(live) - top;
		if (onResults)
			onResults(live, (unsigned long)(results + 1));
		lua_settop(live, -2 - results);
	}
	return status;
}
	// mac: 0x193CAC (394B, batch577: COUNT both globalStates, sample KB delta when non-negative, STEP full(100) iff threshold int < threshold double else single(1), sample interval + gc times. C1C900 multiplier is 1000.0 sec->ms (2016 .msec() correspondence for the same averages; byte-verification queued). Member roles per the +0xF8 family audit; all access by name.
	void ScriptContext::stepGc()
	{
		int kb = 0;
		for (int s = 0; s < 2; ++s)
		{
			lua_State* state = globalStates[s];
			if (state != NULL)
				kb += lua_gc(state, LUA_GCCOUNT, 0);
		}
		if (kb - lastGcKb >= 0)
			avgLuaGcMem.sample((double)(kb - lastGcKb));
		lastGcKb = kb;
		RBX::Time now = RBX::Time::now<RBX::Time::Fast>();
		avgLuaGcInterval.sample((now - lastGcTime).seconds() * 1000.0);
		lastGcTime = now;
		bool full = (double)gcFullStepThreshold < gcStepThreshold;
		for (int s = 0; s < 2; ++s)
		{
			lua_State* state = globalStates[s];
			if (state != NULL)
				lua_gc(state, LUA_GCSTEP, full ? 100 : 1);
		}
		RBX::Time end = RBX::Time::now<RBX::Time::Fast>();
		avgLuaGcTime.sample((end - now).seconds() * 1000.0);
	}
// mac: 0x192D02 (1057B, batch471/479): identity scope, plugin gate, stats hooks (landed this era per batch716:503-507/592-596), hook check, profiler gate, lua_resume, exit-handler dispatch, yield re-queue vs reportError. Returns 2 (blocked/error), 1 (yielded), 0 (finished). DELTA (loud, related era): the +620 running-average sample is skipped (pure telemetry); the plugin-gate member reuses contentClearing (binary +312 vs our +247 layout drift; 2016 calls the concept `timedout`).
// STUB: PLAYER 0x006863a0
// STUB: STUDIO 0x007ebd00
int ScriptContext::resume(Lua::ThreadRef thread, int nargs)
{
	lua_State* live = thread.get();
	Security::Identities identity = Security::Anonymous;
	if (live != NULL)
		identity = (Security::Identities)(RobloxExtraSpace::get(live)->identity & 0x1F);
	Security::Impersonator impersonate(identity);
	if (contentClearing
		&& (Security::Context::current().identity == Security::GameScript_
			|| Security::Context::current().identity == Security::RobloxGameScript_))
	{
		StandardOut::singleton()->print(MESSAGE_ERROR, "Timeout before resuming thread");
		return 2;
	}
	boost::shared_ptr<BaseScript> script;
	if (live != NULL)
		script = boost::shared_ptr<BaseScript>(
			RobloxExtraSpace::get(live)->script, boost::detail::sp_nothrow_tag());
	if (scriptStats.get() != NULL && script.get() != NULL)
		scriptStats->scriptResumeStarted(script->getFullName());	// batch716:503-507
	if (lua_gethook(live) == NULL && script.get() != NULL)
	{
		StandardOut::singleton()->printf(MESSAGE_ERROR, "No lua hook set for %s",
			script->getName().c_str());
		FASTLOGS(FLog::Error, "No lua hook set for %s", script->getName().c_str());
	}
	if (FLog::ScriptContext != 0)
	{
		FLog::FastLog(FLog::ScriptContext, "Resuming script: %p",
			reinterpret_cast<const void*>(script.get()));
		if (script.get() != NULL)
			FLog::FastLogS(FLog::ScriptContext, "Pending script name: %s",
				script->getName().c_str());
	}
	int status;
	// mac: 0x192D02 resume (TENTH ERA, batch774 asm): the C2 slot 959A76 call @192F52 sits behind this gate -- the FLog::LuaProfiler channel byte test (192F08-15) AND the don't-nest test, then lua_resume runs inside the ctor scope; the D2 slot 959A7C fires on scope exit (success arm @192F79, unwind arm @1930DA). Construction destroys the old LuaProfilerShell equivalent: <LuaProfiler prof> scopes the profiler over the resume so the hook events (armed at mask 8/11 in startScript) see instance != NULL.
	if (FLog::LuaProfiler != 0 && LuaProfiler::instance == NULL)
	{
		LuaProfiler prof(live, nargs);
		status = lua_resume(live, nargs);
	}
	else
	{
		status = lua_resume(live, nargs);
	}
	Lua::Continuations* continuations = NULL;
	if (live != NULL)
		continuations = RobloxExtraSpace::get(live)->continuations.get();
	if (continuations != NULL)
	{
		if (status == 0)
		{
			if (!continuations->success.empty() && g_fireExitHandlers)
				continuations->success(live);
		}
		else if (status == LUA_YIELD + 1)
		{
			if (!continuations->error.empty() && g_fireExitHandlers)
				continuations->error(live);
		}
	}
	if (scriptStats.get() != NULL && script.get() != NULL)
		scriptStats->scriptResumeStopped(script->getFullName());	// batch716:592-596
	int result;
	if (status == 0)
	{
		result = 0;
	}
	else if (status == 1)
	{
		result = 1;
		if (live != NULL && !RobloxExtraSpace::get(live)->yieldCaptured
			&& yieldEvent.get() != NULL)
		{
			yieldEvent->queueWaiter(live);
		}
	}
	else
	{
		reportError(live);
		result = 2;
	}
	return result;
}
// mac: 0x196714 (346B, batch471): pin + share into the +200 resume queue (drained by resumeWaitingScripts, section 251).
void ScriptContext::scheduleResume(Lua::ThreadRef thread,
	boost::shared_ptr<const Reflection::Tuple> result)
{
	WaitingThread waiter;
	waiter.thread = thread;
	waiter.result = result;
	waitingThreads.push(waiter);
}
// mac: 0x19316C (347B, batch472): push true + move functor results, resume the caller. Bound by ypcall for the success continuation.
void ScriptContext::on_ypcall_success(Lua::WeakThreadRef caller, lua_State* functor)
{
	if (Lua::ThreadRef thread = caller.lock())
	{
		lua_pushboolean(thread.get(), true);
		int nargs = lua_gettop(functor);
		lua_xmove(functor, thread.get(), nargs);
		Lua::ThreadRef pin(thread);
		resume(pin, 1 + nargs);
	}
}
// mac: 0x1932C8 (337B, batch472): push false + error string, resume caller.
void ScriptContext::on_ypcall_failure(Lua::WeakThreadRef caller, lua_State* functor)
{
	if (Lua::ThreadRef thread = caller.lock())
	{
		lua_pushboolean(thread.get(), false);
		const char* message = Lua::safe_lua_tostring(functor, -1);
		lua_pushstring(thread.get(), message);
		Lua::ThreadRef pin(thread);
		resume(pin, 2);
	}
}
// mac: 0x18E432 (173B, batches 645/647): permission gate + singleton push.
int ScriptContext::settings(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)2);
	boost::shared_ptr<GlobalAdvancedSettings> s = GlobalAdvancedSettings::singleton();
	Lua::SharedPtrBridge<Instance>::push(L, shared_from(s.get()));
	return 1;
}
// mac: 0x18E4E0 (batches 645/647): permission gate + singleton push.
int ScriptContext::usersettings(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)2);
	boost::shared_ptr<GlobalBasicSettings> s = GlobalBasicSettings::singleton();
	Lua::SharedPtrBridge<Instance>::push(L, shared_from(s.get()));
	return 1;
}
// mac: 0x18E58E (batches 645/647): permission gate + singleton push.
int ScriptContext::pluginmanager(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)2);
	boost::shared_ptr<PluginManager> s = PluginManager::singleton();
	Lua::SharedPtrBridge<Instance>::push(L, shared_from(s.get()));
	return 1;
}
// Ctor moved out-of-line (was a header stub): the tail resets libraryService, and the header can only fwd-declare LibraryService -- including script/LuaLibrary.h there cycles back through lua/luaconf.h, which itself includes ScriptContext.h.
bool ScriptContext::disabled = false;
double ScriptContext::gcStepThreshold = 0.0;
// STUB: PLAYER 0x0068ad20
// STUB: STUDIO 0x007f0480
ScriptContext::ScriptContext()
	: nextPendingScripts(Time::now<Time::Fast>())
	, m_unk0xf0(false), robloxPlace(false), scriptsDisabled(false), contentClearing(false)
	, lastGcKb(0)
	, collectStats(false)
	, reentrancy(0)
	, timedoutCount(0), timedout(0)
	, endTimoutThread(false), checkTimeout(false)
	, camelCaseViolationCount(0)
{
	setName("Script Context");
	// mac: 0x18F8EA: ctor-tail libraryService.reset(new LibraryService(this)).
	libraryService.reset(new LibraryService(this));
	// 2013 0x0068b0ed: each read goes through GlobalAdvancedSettingsItem<LuaSettings, sLuaSettings>::singleton() 0x00654600.
	gcFullStepThreshold = LuaSettings::singleton().getUnknown112();
	gcFrequencyBase = LuaSettings::singleton().getUnknown116();
	camelCaseViolationConnection = camelCaseViolation.connect(boost::bind(&ScriptContext::onCamelCaseViolation, this, _1, _2, _3));
}
ScriptContext::~ScriptContext()
{
}
// mac: 0x18E63C (batches 645/647): arg tostring (no permission gate) + delayed request; the thread parks until the library era loads it.
int ScriptContext::loadLibrary(lua_State* L)
{
	std::string library = lua_tostring(L, 1);
	return getContext(L)->libraryService->requestLibrary(L, library, false);
}
// mac: 0x18E70A (batches 645/647): WritePlayer gate + immediate request.
int ScriptContext::loadRobloxLibrary(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)4);
	std::string library = lua_tostring(L, 1);
	return getContext(L)->libraryService->requestLibrary(L, library, true);
}
// mac: 0x18E7EE (batches 645/647): permission gate + singleton push.
int ScriptContext::debuggermanager(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)2);
	boost::shared_ptr<DebuggerManager> s = DebuggerManager::singleton();
	Lua::SharedPtrBridge<Instance>::push(L, shared_from(s.get()));
	return 1;
}
// mac: 0x18DE94 (1261B, batch640): loadfile -- cached-string fetch + signature gate + protected load. RobloxPlace gate; the verified body (verify returns it; throws when unsigned) feeds ProtectedString::fromTrustedSource and luaL_loadprotected. Empty verified body logs via Log::current() (Warning); FLog::FullScriptCode dumps content id + body. Load failure returns nil + error (2 results); success returns the chunk (1). Registration 0x18C067 pushcclosure / "loadfile" setfield (batch637).
// STUB: PLAYER 0x00682350
// STUB: STUDIO 0x007e81a0
int ScriptContext::loadfile(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)2);
	ContentId contentId(Lua::throwable_lua_tostring(L, -1));
	ContentProvider* provider = ServiceProvider::create<ContentProvider>(RobloxExtraSpace::get(L)->context());
	std::string content(*provider->getContentString(contentId));
	std::string verified(ContentProvider::verifyScriptSignature(content.c_str(), true));
	if (verified.empty())
	{
		if (RBX::Log::current())
			RBX::Log::current()->writeEntry(RBX::Log::Warning, RBX::format("contentId %s loaded a null or unsigned script!", contentId.c_str()).c_str());
	}
	if (FLog::FullScriptCode)
	{
		StandardOut::singleton()->printf(MESSAGE_INFO, "Load script through loadfile, contentid: %s", contentId.c_str());
		StandardOut::singleton()->printf(MESSAGE_INFO, "Full script code:\n %s", verified.c_str());
	}
	ProtectedString protectedSource = ProtectedString::fromTrustedSource(verified);
	int result = 1;
	if (luaL_loadprotected(L, protectedSource, "") != 0)
	{
		lua_pushnil(L);
		result = 2;
		lua_insert(L, -2);
	}
	return result;
}
// mac: 0x18DC8C (520B, batch640): dofile -- cached-file fetch + luaL_loadfile. RobloxPlace gate; chunk errors go through lua_error (longjmp), otherwise lua_call(0, MULTRET) and the result-count delta is returned. Registration mac: 0x18C033 pushcclosure / 0x18C038 setfield "dofile" (batch637).
// STUB: PLAYER 0x006826b0
// STUB: STUDIO 0x007e8500
int ScriptContext::dofile(lua_State* L)
{
	Security::Context::current().requirePermission((Security::Permissions)2);
	const char* filename = luaL_optlstring(L, 1, NULL, NULL);
	int oldTop = lua_gettop(L);
	ContentId contentId(filename);
	ContentProvider* provider = ServiceProvider::create<ContentProvider>(RobloxExtraSpace::get(L)->context());
	std::string file = provider->getFile(contentId);
	if (luaL_loadfile(L, file.c_str()))
		lua_error(L);
	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L) - oldTop;
}
// mac: 0x18E382 (176B, batch650): unconditional not-implemented throw.
int ScriptContext::load(lua_State* L)
{
	(void)L;
	throw std::runtime_error("not implemented");
}
// mac: 0x18CB7E (50B, batches 639/663): RunService gameTime (+0x74 live clock, cf. updateDistributedGameTime@0x515A7E reading the same slot).
int ScriptContext::time(lua_State* L)
{
	// DELTA: binary reads a cached RunService* at SC+0xBC (writer unmapped; chain entry); ours resolves live via the model (identical when adopted).
	DataModel* dm = DataModel::get(RobloxExtraSpace::get(L)->context());
	RunService* rs = ServiceProvider::find<RunService>(dm);
	lua_pushnumber(L, rs->gameTime());
	return 1;
}
// mac: 0x18CF42 (2021B, batch472): run fn+args on a fresh thread via resume#2. Returns true+results (or false+error); yields the caller with a Continuations pair when the function yields.
// STUB: PLAYER 0x006878c0
// STUB: STUDIO 0x007ed020
int ScriptContext::ypcall(lua_State* L)
{
	Lua::ThreadRef threadLock(L);
	int top = lua_gettop(L);
	if (top <= 0)
		throw std::runtime_error("ypcall requires at least 1 argument");
	if (lua_iscfunction(L, 1) != 0)
		throw std::runtime_error("ypcall requires a resumable function as a first argument. Wrap your C function with a Lua function.");
	lua_State* child = lua_newthread(L);
	// Fresh-child L-20 slot: see delay() -- same zeroing.
	*(void**)((char*)child - 20) = NULL;
	Lua::WeakThreadRef functor(child);
	Lua::ThreadRef safeFunctor = functor.lock();
	RBXASSERT_LINE(safeFunctor, 1725);
	for (int i = 1; i <= top; ++i)
		lua_pushvalue(L, i);
	lua_xmove(L, safeFunctor.get(), top);
	lua_settop(L, -2);
	// Nargs excludes the function itself (binary passes top-1).
	int status = RobloxExtraSpace::get(L)->context()->resume(safeFunctor, top - 1);
	if (status != 0)
	{
		if (status == 1)
		{
			Lua::Continuations* cont = new Lua::Continuations();
			ScriptContext* context = RobloxExtraSpace::get(L)->context();
			cont->success = boost::bind(&ScriptContext::on_ypcall_success,
				context, Lua::WeakThreadRef(child), boost::arg<1>());
			cont->error = boost::bind(&ScriptContext::on_ypcall_failure,
				context, Lua::WeakThreadRef(child), boost::arg<1>());
			RBXASSERT_LINE(RobloxExtraSpace::get(safeFunctor)->continuations.get() == NULL, 1768);
			RobloxExtraSpace::get(safeFunctor.get())->continuations.reset(cont);
			RobloxExtraSpace::get(L)->yieldCaptured = true;
			// The caller resumes via the continuations (bound above); the bound temporaries die with this frame (binary clears them explicitly on the resume path back).
			return lua_yield(L, 0);
		}
		// Defensive: lua_resume only returns 0/1/2 (2 = error pair below). Literal unrecovered (rodata ref used a second PIC base); loud DELTA.
		if (status != 2)
			throw std::runtime_error("Unexpected resume status");
		lua_pushboolean(L, false);
		const char* message = Lua::safe_lua_tostring(safeFunctor.get(), -1);
		lua_pushstring(L, message);
		lua_settop(safeFunctor.get(), 0);
		return 2;
	}
	{
		lua_State* thread = L;	// the 2013 name in the assert text
		RBXASSERT_LINE(top == lua_gettop(thread), 1747);
	}
	lua_pushboolean(L, true);
	{
		int nresults = lua_gettop(safeFunctor.get());
		lua_xmove(safeFunctor.get(), L, nresults);
		return nresults + 1;
	}
}
// mac: 0x18B3DE (14B, batch544): shared->context straight off the extra space. Spelled via the accessor (identical pointer; the binary reads L-28/Shared/+4 raw).
ScriptContext* ScriptContext::getContext(lua_State* thread)
{
	return RobloxExtraSpace::get(thread)->context();
}
// Results collector for call#1 (sub_191EEC): fresh Tuple(count) + element-wise get (the binary's auto_ptr dance, RAII here).
static void collectTupleResults(std::auto_ptr<Reflection::Tuple>& out,
	lua_State* thread, unsigned long count)
{
	std::auto_ptr<Reflection::Tuple> fresh(
		new Reflection::Tuple((size_t)count));
	out = fresh;
	for (unsigned long i = 0; i < count; ++i)
		Lua::LuaArguments::get(thread, (int)i + 1, out->at(i), true);
}
// mac: 0x190B94 (342B, batches 546-549): pin + pushfunction + resume#1 with the caller's closures. Returns void; resume#1 throws on error (the onResults arm is set) and runs onResults on success AND yield. ThreadRef pins + net-zero count pairs stand in for the manual addref/release dance. Throws "Cannot call a nil function" (0xB2C86C) on a dead function.
// STUB: PLAYER 0x00689cb0
// STUB: STUDIO 0x007ef410
void ScriptContext::call(Lua::WeakFunctionRef& function,
	boost::function1<unsigned long, lua_State*> pushArguments,
	boost::function2<void, lua_State*, unsigned long> onResults)
{
	Lua::ThreadRef pin = function.lock();
	lua_State* live = pin.get();
	if (live == NULL)
	{
		throw std::runtime_error("Cannot call a nil function");
	}
	Lua::lua_pushfunction(live, function);
	Lua::ThreadRef callPin(pin);
	resume(callPin, pushArguments, onResults);
}
// mac: 0x191E3B (186B, batches 546/551): Tuple in, Tuple out -- binds pushTuple + the collector above around call#2. Errors throw out of resume#1; yields return whatever the collector gathered.
boost::shared_ptr<const Reflection::Tuple> ScriptContext::call(
	Lua::WeakFunctionRef& function,
	const Reflection::Tuple& args)
{
	std::auto_ptr<Reflection::Tuple> out;
	boost::function1<unsigned long, lua_State*> pushArguments =
		boost::bind(&Lua::LuaArguments::pushTuple, boost::cref(args),
			boost::arg<1>());
	boost::function2<void, lua_State*, unsigned long> onResults =
		boost::bind(&collectTupleResults, boost::ref(out),
			boost::arg<1>(), boost::arg<2>());
	call(function, pushArguments, onResults);
	boost::shared_ptr<const Reflection::Tuple> result(out.release());
	return result;
}
// mac: 0x19237A (927B, batches 499-503): up to 12 frames of "chunk, line N[ - namewhat name]" text; outs the level-0 script (the thread extra-space weak ref, promoted non-throwing -- same read the binary spells as L-12) and its current line (-1 with no frames). lua_Debug roles are layout-pinned (+4 name / +8 namewhat / +16 source / +20 currentline); null names fall back to the short line, null qualifiers set failbit (both binary-verbatim).
// STUB: PLAYER 0x00673b60
// STUB: STUDIO 0x007db850
std::string ScriptContext::extractCallStack(lua_State* thread,
	shared_ptr<BaseScript>& script, int& line)
{
	line = -1;
	script = shared_ptr<BaseScript>(
		RobloxExtraSpace::get(thread)->script, boost::detail::sp_nothrow_tag());
	std::stringstream stream;
	lua_Debug ar;
	if (lua_getstack(thread, 0, &ar) != 0)
	{
		int level = 0;
		for (;;)
		{
			if (level > 11)
				break;
			if (lua_getinfo(thread, "nSlu", &ar) != 0)
			{
				if (level == 0)
					line = ar.currentline;
				if (ar.currentline >= 0)
				{
					char chunk[LUA_IDSIZE];
					luaO_chunkid(chunk, ar.source, sizeof(chunk));
					if (ar.name == NULL)
					{
						stream << chunk << ", line " << ar.currentline << std::endl;
					}
					else
					{
						stream << chunk << ", line " << ar.currentline << " - ";
						if (ar.namewhat != NULL)
							stream << ar.namewhat;
						else
							stream.clear(stream.rdstate() | std::ios_base::failbit);
						stream << " ";
						if (ar.name != NULL)
							stream << ar.name;
						else
							stream.clear(stream.rdstate() | std::ios_base::failbit);
						stream << std::endl;
					}
				}
			}
			++level;
			if (lua_getstack(thread, level, &ar) == 0)
				break;
		}
	}
	return stream.str();
}
// mac: 0x1920D0 (243B, batches 497-498): assert non-empty, collect the stack values, invoke. All Asserts are log-continue (never branch).
// STUB: PLAYER 0x006844f0
// STUB: STUDIO 0x007e9f50
void Lua::Continuations::onSuccessHandler(lua_State* thread,
	Scripts::SuccessHandler handler)
{
	RBXASSERT_LINE(!handler.empty(), 1373);
	shared_ptr<const Reflection::Tuple> results = LuaArguments::getValues(thread);
	handler(results);
}
// mac: 0x1921C4 (437B, batches 497-498): message off the top (falling back to "An error occurred", 0xB2C6BE) + extractCallStack triple, then invoke.
// STUB: PLAYER 0x00677530
// STUB: STUDIO 0x007df1f0
void Lua::Continuations::onErrorHandler(lua_State* thread,
	Scripts::ErrorHandler handler)
{
	RBXASSERT_LINE(!handler.empty(), 1381);
	const char* message = lua_tolstring(thread, -1, NULL);
	if (message == NULL || message[0] == '\0')
		message = "An error occurred";
	shared_ptr<BaseScript> script;
	int line = 0;
	std::string callStack = ScriptContext::extractCallStack(thread, script, line);
	handler(message, callStack.c_str(), script, line);
}
// mac: 0x191F72 (350B, batch497): wrap the non-empty Scripts handlers in the (lua_State*) closures resume invokes (bound here; bodies above).
// STUB: PLAYER 0x00685ce0
// STUB: STUDIO 0x007eb640
Lua::Continuations::Continuations(const Scripts::Continuations& eh)
{
	if (!eh.successHandler.empty())
	{
		success = boost::bind(&Continuations::onSuccessHandler,
			boost::arg<1>(), eh.successHandler);
	}
	if (!eh.errorHandler.empty())
	{
		error = boost::bind(&Continuations::onErrorHandler,
			boost::arg<1>(), eh.errorHandler);
	}
}
// mac: 0x195764 (1346B, batch714-716): callstack printer.  Gate = the DebugSettings singleton byte (+0x5D; early return when 0).  Per frame (max 12 levels): "Script '%s', Line %d" or "... - %s %s" with namewhat/name when ar.name is set; each frame goes to StandardOut at MESSAGE_INFO and, when output != NULL, to the stream; a lowercase "stack end" trailer prints when frames remain and the cap was not reached; *output always receives the stream text last (binary-verbatim, including the double lua_getstack(L, 0) entry check).
// STUB: PLAYER 0x0067abc0
// STUB: STUDIO 0x007e2ae0
void ScriptContext::printCallStack(lua_State* thread, std::string* output)
{
	if (!DebugSettings::singleton().getStackTracingEnabled())
		return;
	std::stringstream stream;
	lua_Debug ar;
	if (lua_getstack(thread, 0, &ar))
	{
		bool moreStack = false;
		int level = 0;
		const int maxLevel = 12;
		while (level < maxLevel && lua_getstack(thread, level++, &ar))
		{
			if (lua_getinfo(thread, "nSlu", &ar))
			{
				if (ar.currentline >= 0)
				{
					moreStack = true;
					char source[LUA_IDSIZE];
					luaO_chunkid(source, ar.source, LUA_IDSIZE);
					std::string messageLine;
					if (ar.name)
					{
						messageLine = RBX::format("Script '%s', Line %d - %s %s",
							source, ar.currentline, ar.namewhat, ar.name);
					}
					else
					{
						messageLine = RBX::format("Script '%s', Line %d",
							source, ar.currentline);
					}
					StandardOut::singleton()->printf(MESSAGE_INFO, "%s",
						messageLine.c_str());
					if (output)
						stream << messageLine << std::endl;
				}
			}
		}
		if (moreStack && level < maxLevel)
		{
			StandardOut::singleton()->printf(MESSAGE_INFO, "stack end");
			if (output)
				stream << "stack end";
		}
	}
	if (output)
		*output = stream.str();
}
// mac: 0x195CA6 (2669B, batch714-716): error pipeline.  message = tostring of the failing value (non-empty) else "An error occurred" + the 35-char "Error occurred, no output from Lua." fallback.  printCallStack runs with the exception stack only when DFInt::LuaExceptionPlaceFilter equals the DataModel placeID (dm+0xD80).  StatsService::report( "LuaExceptions", {PlaceId, CallStack, Error, PlaceVersion}) fires when a StatsService exists and the filter matched.  DFFlag:: LuaExceptionSignalEnabled fires the DataModel's void() signal (DM+0xAEC).  Then the errorSignal(errorMessage, callStackString, scriptPtr) fire (guarded by non-empty + RBXASSERT(scriptPtr == source) at ScriptContext.cpp:2437) and BaseScript::extraErrorReporting; scriptPtr comes from the thread's extra-space weak_ptr.
// STUB: PLAYER 0x00685dd0
// STUB: STUDIO 0x007eb730
void ScriptContext::reportError(lua_State* thread)
{
	std::string errorMessage;
	const char* message = lua_tolstring(thread, -1, NULL);
	if (message != NULL && *message != 0)
	{
		StandardOut::singleton()->printf(MESSAGE_ERROR, "%s", message);
		errorMessage.assign(message, strlen(message));
	}
	else
	{
		StandardOut::singleton()->print(MESSAGE_ERROR, "An error occurred");
		errorMessage.assign("Error occurred, no output from Lua.", 0x23);
	}
	std::string exceptionStack;
	DataModel* dm = DataModel::get(this);
	bool reportingStack = false;
	if (DFInt::LuaExceptionPlaceFilter != 0)
		reportingStack = (DFInt::LuaExceptionPlaceFilter == dm->getPlaceID());
	printCallStack(thread, reportingStack ? &exceptionStack : NULL);
	if (dm != NULL)
	{
		Stats::StatsService* stats = ServiceProvider::find<Stats::StatsService>(dm);
		if (stats != NULL && reportingStack)
		{
			shared_ptr<Reflection::ValueTable> entry(new Reflection::ValueTable());
			(*entry)["PlaceId"] = dm->getPlaceID();
			(*entry)["CallStack"] = exceptionStack;
			(*entry)["Error"] = errorMessage;
			(*entry)["PlaceVersion"] = dm->getPlaceVersion();
			stats->report("LuaExceptions", entry,
				DFInt::LuaExceptionThrottlingPercentage);
		}
	}
	if (DFFlag::LuaExceptionSignalEnabled)
		dm->luaExceptionSignal();
	shared_ptr<BaseScript> scriptPtr(RobloxExtraSpace::get(thread)->script,
		boost::detail::sp_nothrow_tag());
	if (!errorSignal.empty())
	{
		shared_ptr<BaseScript> source;
		int line;
		std::string callStackString = extractCallStack(thread, source, line);
		RBXASSERT_LINE(scriptPtr == source, 2437);
		errorSignal(errorMessage, callStackString, scriptPtr);
	}
	if (scriptPtr != NULL)
		scriptPtr->extraErrorReporting(thread);
}
// mac: 0x194F84-0x1952E9 (batch713 full body): service attach/detach. Decoded flow: heartbeatConnection.disconnect() [0x194F99, +0x2DC]; old-provider arm [0x194FAD: endTimoutThread=1 + checkTimeout.Set + join the timeout thread + delete the LibraryService [+0x2D8] + closeStates(false)]; BOTH-arms stats resets [0x195002/0x19504A: statsItem [+0x10C] + scriptStats [+0x118]]; newProvider arm: stats = newProvider->create<Stats::StatsService>() [0x195089]; if (stats) LuaStatsItem create/parent, then the collectStats [+0x114] gate -> scriptStats.reset(new ScriptStats()) [0x1950F2-0x195136]; globalStates fill [0x19513B: Segregated- LuaEnvironments (off_EADED0 -- same flag as getGlobalState/ addLibrary, batch713 xrefs) -> openState into both slots; else slot 0 + NULL]; EADEC8-flag setTimeout arm [0x195173-0x195197]; LibraryService re-create [0x195197, old object deleted by the reset]; RunService create [0x1951CB, cached +0xBC + Heartbeat connect 0x1951DD-0x195279]. No Super call in the body (Instance's hook is empty). DELTAs (unported members, loud): heartbeatConnection disconnect [+0x2DC], statsItem create/reset [0x195002/0x19509C, LuaStatsItem parent wiring], RunService cache + Heartbeat connect tail (heartbeat era).  The detach trio [0x194FAD-0x194FD0], the closeStates(false) tail [0x194FFD] and the EADEC8 setTimeout arm [0x195173-0x195197] are LIVE (P2-503 + closeStates era).
// STUB: PLAYER 0x0068b5f0
// STUB: STUDIO 0x007f0d50
void ScriptContext::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	// this->heartbeatConnection.disconnect();   // +0x2DC DELTA
	if (oldProvider != NULL)
	{
		endTimoutThread = true;                   // +0x16C
		checkTimeout.Set();                       // +0x170 (wakes the monitor)
		if (timeoutThread)
			timeoutThread->join();                // +0x13C (bounded: Set above)
		libraryService.reset();                   // +0x2D8: delete old LS
		closeStates(false);                       // 0x194FFD (body live: 0x18E9BC)
	}
	// if (statsItem) statsItem.reset();          // +0x10C DELTA (LuaStatsItem)
	scriptStats.reset();                          // +0x118
	if (newProvider == NULL)
		return;
	Stats::StatsService* stats = newProvider->create<Stats::StatsService>();
	if (stats != NULL)
	{
		// statsItem = LuaStatsItem::create(this); statsItem->setParent(stats);          // +0x10C DELTA (unported)
		if (collectStats)                         // +0x114 gate
			scriptStats.reset(new ScriptStats());
	}
	if (FFlag::SegregatedLuaEnvironments)
	{
		for (size_t i = 0; i < 2; ++i)
			globalStates[i] = openState();
	}
	else
	{
		globalStates[0] = openState();
		globalStates[1] = NULL;
	}
	// mac: 0x195173-0x195197 (batch719/720): pointer-table slot off_EADEC8 -> FLog::LuaScriptTimeoutSeconds (storage byte 0xFF at rest); the binary reads the byte, gates on != 0 and passes it as the span.
	if (FLog::LuaScriptTimeoutSeconds != 0)
		setTimeout((double)FLog::LuaScriptTimeoutSeconds);
	libraryService.reset(new LibraryService(this));
	RunService* runService = newProvider->create<RunService>();
	(void)runService;   // cache +0xBC / Heartbeat connect: heartbeat era DELTA
}
// mac: 0x18A30C-0x18A54B (batch719; 2016 ScriptContext.cpp:1192 matches modulo branch order).  The binary tests `timeoutThread != 0` first and kicks the running monitor (checkTimeout.Set); the spawn path is identical outright.
// FUNCTION: PLAYER 0x0067eee0
// FUNCTION: STUDIO 0x007e4db0
void ScriptContext::setTimeout(double seconds)
{
	boost::mutex::scoped_lock lock(timeoutMutex);
	timoutSpan = Time::Interval(seconds);
	if (timoutSpan.isZero())
		timoutTime = Time();
	else
	{
		FASTLOG1(FLog::ScriptContext, "Set lua time out: %d", (int)seconds);
		timoutTime = Time::now<Time::Fast>() + timoutSpan;
		if (!timeoutThread)
			timeoutThread.reset(new boost::thread(RBX::thread_wrapper(
				boost::bind(&ScriptContext::onCheckTimeout, shared_from(this)),
				"ScriptContext Timeout")));
		else
			checkTimeout.Set();
	}
}
#ifndef _WIN32
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif
// mac: 0x190786-0x190900 (batch720; control flow binary-identical to 2016): the monitor.  Sleeps on checkTimeout, exits when the detach arm sets endTimoutThread, and parks at -1 while the span is zero (disabled). On expiry it re-arms the chunk to the span and latches timedout; the counter bumps only on the 0->1 transition.
void ScriptContext::onCheckTimeout()
{
	Time::Interval sleepTimeInterval = timoutSpan;
	while (true)
	{
		{
			double sleepTime = sleepTimeInterval.seconds();
			int ticks = sleepTime > 0 ? std::max(1, (int)(1010.0 * sleepTime)) : INFINITE;
			checkTimeout.Wait(ticks);
			if (endTimoutThread)
				return;	// endTimoutThread was set
		}
		{
			boost::mutex::scoped_lock lock(timeoutMutex);
			if (!timoutSpan.isZero())
			{
				sleepTimeInterval = timoutTime - Time::now<Time::Fast>();
				if (sleepTimeInterval.seconds() <= 0)
				{
					// We've timed out!
					sleepTimeInterval = timoutSpan;
					if (timedout.swap(1) == 0)
						++timedoutCount;
				}
			}
			else
				sleepTimeInterval = Time::Interval(-1);	// disabled
		}
	}
}
// mac: 0x18B5AC core (batch320: allocator + newstate + atpanic + context link + 4 locked libs + string-metatable lock + _G table; returns the state for the caller to store. gc tuning, Bridges, globals and jobs queue in later parts).
// STUB: PLAYER 0x00688370
// STUB: STUDIO 0x007edad0
lua_State* ScriptContext::openState()
{
	if (luaAllocator.get() == NULL)
		luaAllocator.reset(new LuaAllocator(FLog::UseLuaMemoryPool != 0));
	lua_State* L = lua_newstate(LuaAllocator::alloc, luaAllocator.get());
	if (L == NULL)
		throw std::runtime_error("Failed to create Lua state");
	InterlockedIncrement(&g_openLuaStates);
	lua_atpanic(L, scriptPanic);
	lua_gc(L, LUA_GCSETSTEPMUL, LuaSettings::singleton().getGcStepMul());
	lua_gc(L, LUA_GCSETPAUSE, LuaSettings::singleton().getGcPause());
	RobloxExtraSpace::get(L)->setContext(this);
	openScriptLibrary(L, "coroutine", luaopen_base);
	openScriptLibrary(L, "string", luaopen_string);
	{
		lua_pushlstring(L, "", 0);
		int hasMeta = lua_getmetatable(L, -1);
		RBXASSERT(hasMeta == 1);  // ScriptContext.cpp:613 "i==1"
		lua_pushstring(L, "The metatable is locked");
		lua_setfield(L, -2, "__metatable");
		lua_settop(L, -3);
	}
	openScriptLibrary(L, "math", luaopen_math);
	openScriptLibrary(L, "table", luaopen_table);
	lua_createtable(L, 0, 0);
	lua_setfield(L, LUA_GLOBALSINDEX, "_G");
	// Bridge metatypes (batch320 EA order; 0x18B7EF block sits between _G (0x18B7BE) and the shared/print globals). Binary order 0x18B7EF- mac: 0x18BAAB: Library, AllEnumDescriptors, EnumDescriptor, Item, shared_ptr<Instance>, EventInstance, connection (shared_ptr still queues behind the Instance era), then the 17 atomic types, then WeakThreadRef::Node + 2 GenericFunctions (thread/yield eras).
	Lua::Bridge<Lua::Library>::registerClass(L,
		Lua::Bridge<Lua::Library>::on_index, Lua::Bridge<Lua::Library>::on_newindex);
	Lua::Bridge<Lua::AllEnumDescriptorsPtr, true>::registerClass(L,
		Lua::Bridge<Lua::AllEnumDescriptorsPtr, true>::on_index,
		Lua::Bridge<Lua::AllEnumDescriptorsPtr, true>::on_newindex);
	Lua::Bridge<Lua::EnumDescriptorPtr, true>::registerClass(L,
		Lua::Bridge<Lua::EnumDescriptorPtr, true>::on_index,
		Lua::Bridge<Lua::EnumDescriptorPtr, true>::on_newindex);
	Lua::Bridge<Lua::EnumDescriptorItemPtr, true>::registerClass(L,
		Lua::Bridge<Lua::EnumDescriptorItemPtr, true>::on_index,
		Lua::Bridge<Lua::EnumDescriptorItemPtr, true>::on_newindex);
	// (shared_ptr<Instance> registerClass queues behind the Instance era, 0x18B85F.) shared_ptr<Instance> bridge (sections 258a-c; binary slot 0x18B85F).
	Lua::Bridge<boost::shared_ptr<Instance>, false>::registerClass(L,
		Lua::Bridge<boost::shared_ptr<Instance>, false>::on_index,
		Lua::Bridge<boost::shared_ptr<Instance>, false>::on_newindex);
	// EventInstance bridge (section 253; binary slot 0x18B87B).
	Lua::Bridge<Lua::EventInstance>::registerClass(L,
		Lua::Bridge<Lua::EventInstance>::on_index,
		Lua::Bridge<Lua::EventInstance>::on_newindex);
	// connection bridge (section 243; binary slot 0x18B897).
	Lua::Bridge<rbx::signals::connection>::registerClass(L,
		Lua::Bridge<rbx::signals::connection>::on_index,
		Lua::Bridge<rbx::signals::connection>::on_newindex);
	Lua::Bridge<G3D::CoordinateFrame>::registerClass(L,
		Lua::Bridge<G3D::CoordinateFrame>::on_index, Lua::Bridge<G3D::CoordinateFrame>::on_newindex);
	Lua::Bridge<RBX::Region3>::registerClass(L,
		Lua::Bridge<RBX::Region3>::on_index, Lua::Bridge<RBX::Region3>::on_newindex);
	Lua::Bridge<RBX::Region3int16>::registerClass(L,
		Lua::Bridge<RBX::Region3int16>::on_index, Lua::Bridge<RBX::Region3int16>::on_newindex);
	Lua::Bridge<G3D::Vector3int16>::registerClass(L,
		Lua::Bridge<G3D::Vector3int16>::on_index, Lua::Bridge<G3D::Vector3int16>::on_newindex);
	Lua::Bridge<G3D::Vector2int16>::registerClass(L,
		Lua::Bridge<G3D::Vector2int16>::on_index, Lua::Bridge<G3D::Vector2int16>::on_newindex);
	Lua::Bridge<G3D::Vector3>::registerClass(L,
		Lua::Bridge<G3D::Vector3>::on_index, Lua::Bridge<G3D::Vector3>::on_newindex);
	Lua::Bridge<G3D::Vector2>::registerClass(L,
		Lua::Bridge<G3D::Vector2>::on_index, Lua::Bridge<G3D::Vector2>::on_newindex);
	Lua::Bridge<RBX::RbxRay>::registerClass(L,
		Lua::Bridge<RBX::RbxRay>::on_index, Lua::Bridge<RBX::RbxRay>::on_newindex);
	Lua::Bridge<G3D::Color3>::registerClass(L,
		Lua::Bridge<G3D::Color3>::on_index, Lua::Bridge<G3D::Color3>::on_newindex);
	Lua::Bridge<RBX::BrickColor>::registerClass(L,
		Lua::Bridge<RBX::BrickColor>::on_index, Lua::Bridge<RBX::BrickColor>::on_newindex);
	Lua::Bridge<RBX::UDim>::registerClass(L,
		Lua::Bridge<RBX::UDim>::on_index, Lua::Bridge<RBX::UDim>::on_newindex);
	Lua::Bridge<RBX::UDim2>::registerClass(L,
		Lua::Bridge<RBX::UDim2>::on_index, Lua::Bridge<RBX::UDim2>::on_newindex);
	Lua::Bridge<RBX::Faces>::registerClass(L,
		Lua::Bridge<RBX::Faces>::on_index, Lua::Bridge<RBX::Faces>::on_newindex);
	Lua::Bridge<RBX::Axes>::registerClass(L,
		Lua::Bridge<RBX::Axes>::on_index, Lua::Bridge<RBX::Axes>::on_newindex);
	Lua::Bridge<RBX::CellID>::registerClass(L,
		Lua::Bridge<RBX::CellID>::on_index, Lua::Bridge<RBX::CellID>::on_newindex);
	Lua::Bridge<RBX::InputObject>::registerClass(L,
		Lua::Bridge<RBX::InputObject>::on_index, Lua::Bridge<RBX::InputObject>::on_newindex);
	// (WeakThreadRef::Node + 2 GenericFunction registerClasses queue behind thread/yield eras; 0x18BA73/0x18BA8F/0x18BAAB.) Node bridge (section 245; binary slot 0x18BA73).
	Lua::Bridge<boost::intrusive_ptr<Lua::WeakThreadRef::Node>>::registerClass(L,
		Lua::Bridge<boost::intrusive_ptr<Lua::WeakThreadRef::Node>>::on_index,
		Lua::Bridge<boost::intrusive_ptr<Lua::WeakThreadRef::Node>>::on_newindex);
	// Sync GenericFunction bridge (section 248; binary slot 0x18BA8F -- async twin at 0x18BAAB queues behind scheduleResume).
	Lua::Bridge<boost::shared_ptr<Lua::GenericFunction>>::registerClass(L,
		Lua::Bridge<boost::shared_ptr<Lua::GenericFunction>>::on_index,
		Lua::Bridge<boost::shared_ptr<Lua::GenericFunction>>::on_newindex);
	// Async GenericAsyncFunction bridge (section 251; binary slot 0x18BAAB).
	Lua::Bridge<boost::shared_ptr<Lua::GenericAsyncFunction>>::registerClass(L,
		Lua::Bridge<boost::shared_ptr<Lua::GenericAsyncFunction>>::on_index,
		Lua::Bridge<boost::shared_ptr<Lua::GenericAsyncFunction>>::on_newindex);
	// Class libraries batch 1 (section 236; binary order 0x18BAxx-0x18BB4B: the enum-gated tables -- Vector3 FromNormalId/FromAxis, Faces, Axes -- land with the EnumDescriptor::Item bridge era; CFrame/UDim/ UDim2/BrickColor/CellID/InputObject libraries queue behind their new* bodies, batch402 next). Enum singleton tables (section 239; binary slots 0x18BABB/0x18BAC3/ mac: 0x18BACB, ahead of the CFrame library) + Library table (section 241; binary slot 0x18BAB3, first of the block).
	Lua::LibraryBridge::registerClassLibrary(L);
	Lua::Enums::registerClassLibrary(L);
	Lua::Enum::registerClassLibrary(L);
	Lua::EnumItem::registerClassLibrary(L);
	Lua::Region3Bridge::registerClassLibrary(L);
	Lua::Region3int16Bridge::registerClassLibrary(L);
	Lua::Vector3int16Bridge::registerClassLibrary(L);
	Lua::Vector2int16Bridge::registerClassLibrary(L);
	Lua::Vector3Bridge::registerClassLibrary(L);
	Lua::Vector2Bridge::registerClassLibrary(L);
	Lua::RbxRayBridge::registerClassLibrary(L);
	Lua::Color3Bridge::registerClassLibrary(L);
	Lua::BrickColorBridge::registerClassLibrary(L);
	Lua::CoordinateFrameBridge::registerClassLibrary(L);
	Lua::UDimBridge::registerClassLibrary(L);
	Lua::UDim2Bridge::registerClassLibrary(L);
	// Faces/Axes/CellID/InputObject libraries (section 240; binary slots mac: 0x18BB33/0x18BB3B/0x18BB43/0x18BB4B; SharedPtr<Instance> 0x18BB53 still queues behind the Instance era).
	Lua::FacesBridge::registerClassLibrary(L);
	Lua::AxesBridge::registerClassLibrary(L);
	Lua::CellIDBridge::registerClassLibrary(L);
	Lua::InputObjectBridge::registerClassLibrary(L);
	// shared_ptr<Instance> reuse table (binary slot 0x18BB53, between the InputObject library and Node::create; the push weak-table lookup AVs without it -- stats() proved it live).
	Lua::SharedPtrBridge<Instance>::registerClassLibrary(L);
	// Instance global (binary slot 0x18BB5B, called from openState right after the SharedPtr library: luaL_register(L, "Instance")).
	Lua::ObjectBridge::registerInstanceClassLibrary(L);
	// Main-thread Node (binary slot 0x18BB63, batch477: sits between the InputObject library above and the game/app/Workspace globals + shared table below; the game/app/Workspace globals queue behind the DataModel era). closeState 0x18EE84 erases refs through every thread's node, so an openState'd state without one AVs on close (section 271 postmortem: intrusive_ptr px != 0 in eraseRefsFromAllNodes). Return discarded: the L-20 slot holds the ref, matching Node::create's contract.
	Lua::WeakThreadRef::Node::create(L);
	// game/Game/workspace/Workspace globals (0x18BB89-0x18BD7A, batches 632/668/669/671: +0x34 root-climb (getRootAncestor) pushed twice as the game/Game alias; Workspace pair via find<Workspace> (binary Instance overload -- climbs to the root provider, same call here). OverlayDataModel roots take the App-only arm instead -- queued with the overlay era. DELTA: binary pushes shared_from() unconditionally (throws/AVs for unmanaged roots); ours pushes nil when unresolvable so harness stack states stay green -- identical for heap states.
	{
		shared_ptr<Instance> rootSp;
		try { rootSp = shared_from(Instance::getRootAncestor(this)); }
		catch (const std::exception&) { }
		if (rootSp != NULL && rootSp->isA<OverlayDataModel>())
		{
			Lua::SharedPtrBridge<Instance>::push(L, rootSp);
			lua_setfield(L, LUA_GLOBALSINDEX, "App");
		}
		else if (rootSp != NULL)
		{
			Lua::SharedPtrBridge<Instance>::push(L, rootSp);
			lua_setfield(L, LUA_GLOBALSINDEX, "game");
			Lua::SharedPtrBridge<Instance>::push(L, rootSp);
			lua_setfield(L, LUA_GLOBALSINDEX, "Game");
		}
		else
		{
			lua_pushnil(L);
			lua_setfield(L, LUA_GLOBALSINDEX, "game");
			lua_pushnil(L);
			lua_setfield(L, LUA_GLOBALSINDEX, "Game");
		}
		shared_ptr<Instance> wsSp;
		try { wsSp = shared_from(ServiceProvider::find<Workspace>(static_cast<Instance*>(this))); }
		catch (const std::exception&) { }
		if (wsSp != NULL)
		{
			Lua::SharedPtrBridge<Instance>::push(L, wsSp);
			lua_setfield(L, LUA_GLOBALSINDEX, "workspace");
			Lua::SharedPtrBridge<Instance>::push(L, wsSp);
			lua_setfield(L, LUA_GLOBALSINDEX, "Workspace");
		}
		else
		{
			lua_pushnil(L);
			lua_setfield(L, LUA_GLOBALSINDEX, "workspace");
			lua_pushnil(L);
			lua_setfield(L, LUA_GLOBALSINDEX, "Workspace");
		}
	}
	// Globals batch 1 (batch320 tail: shared table + print + gated crash__ + tick + time + ElapsedTime + printidentity; the rest queue behind yield/bridge/service eras).
	lua_createtable(L, 0, 0);
	lua_setfield(L, LUA_GLOBALSINDEX, "shared");
	lua_pushcfunction(L, print);
	lua_setfield(L, LUA_GLOBALSINDEX, "print");
	if (FFlag::DebugCrashEnabled)
	{
		lua_pushcfunction(L, crash);
		lua_setfield(L, LUA_GLOBALSINDEX, "crash__");
	}
	lua_pushcfunction(L, tick);
	lua_setfield(L, LUA_GLOBALSINDEX, "tick");
	// time global (binary slot 0x18BE7C, between tick and ElapsedTime; the +188/+116 DELTA above is resolved to live lookup).
	lua_pushcfunction(L, time);
	lua_setfield(L, LUA_GLOBALSINDEX, "time");
	lua_pushcfunction(L, rbxTime);
	lua_setfield(L, LUA_GLOBALSINDEX, "ElapsedTime");
	// Globals batch 2 (section 249; binary order 0x18BED3-0x18C2BC: wait/Wait/delay/Delay/ypcall/Spawn/printidentity/dofile/loadfile/ load/settings/UserSettings/PluginManager/DebuggerManager/stats/ Stats/version/Version; LoadLibrary/LoadRobloxLibrary queue behind LibraryService -- the only unlanded batch-2 slots). DELTA: binary registers into the -10002 sandbox env; ours uses LUA_GLOBALSINDEX like batch 1 (sandbox env unlanded -- reconcile in the sandbox-era audit).
	lua_pushcfunction(L, wait);
	lua_setfield(L, LUA_GLOBALSINDEX, "wait");
	lua_pushcfunction(L, wait);
	lua_setfield(L, LUA_GLOBALSINDEX, "Wait");
	lua_pushcfunction(L, delay);
	lua_setfield(L, LUA_GLOBALSINDEX, "delay");
	lua_pushcfunction(L, delay);
	lua_setfield(L, LUA_GLOBALSINDEX, "Delay");
	// ypcall (section 250; binary slot between Delay and Spawn in the batch477 order).
	lua_pushcfunction(L, ypcall);
	lua_setfield(L, LUA_GLOBALSINDEX, "ypcall");
	lua_pushcfunction(L, spawn);
	lua_setfield(L, LUA_GLOBALSINDEX, "Spawn");
	// printidentity (batch637: binary slot between Spawn and dofile; the port moves it here from the batch-1 group -- registry writes only, order within the batch is not observable).
	lua_pushcfunction(L, printidentity);
	lua_setfield(L, LUA_GLOBALSINDEX, "printidentity");
	// dofile/loadfile/load (batch637: pushcclosure 0x18C033/0x18C067/0x18C09B + setfield 0x18C038/0x18C081/0x18C0B5; fns dofile 0x18DC8C / loadfile 0x18DE94 / notImplemented 0x18E382).
	lua_pushcfunction(L, dofile);
	lua_setfield(L, LUA_GLOBALSINDEX, "dofile");
	lua_pushcfunction(L, loadfile);
	lua_setfield(L, LUA_GLOBALSINDEX, "loadfile");
	lua_pushcfunction(L, load);
	lua_setfield(L, LUA_GLOBALSINDEX, "load");
	// settings-tree globals (0x18C0E9/0x18C11D/0x18C151/0x18C1F8).
	lua_pushcfunction(L, settings);
	lua_setfield(L, LUA_GLOBALSINDEX, "settings");
	lua_pushcfunction(L, usersettings);
	lua_setfield(L, LUA_GLOBALSINDEX, "UserSettings");
	lua_pushcfunction(L, pluginmanager);
	lua_setfield(L, LUA_GLOBALSINDEX, "PluginManager");
	// Library globals (0x18C170/0x18C1A4: "LoadLibrary" then "LoadRobloxLibrary", between PluginManager and DebuggerManager).
	lua_pushcfunction(L, loadLibrary);
	lua_setfield(L, LUA_GLOBALSINDEX, "LoadLibrary");
	lua_pushcfunction(L, loadRobloxLibrary);
	lua_setfield(L, LUA_GLOBALSINDEX, "LoadRobloxLibrary");
	lua_pushcfunction(L, debuggermanager);
	lua_setfield(L, LUA_GLOBALSINDEX, "DebuggerManager");
	// stats/Stats (section 250; binary order between Spawn and version/Version in the batch477 sequence).
	lua_pushcfunction(L, stats);
	lua_setfield(L, LUA_GLOBALSINDEX, "stats");
	lua_pushcfunction(L, stats);
	lua_setfield(L, LUA_GLOBALSINDEX, "Stats");
	lua_pushcfunction(L, version);
	lua_setfield(L, LUA_GLOBALSINDEX, "version");
	lua_pushcfunction(L, version);
	lua_setfield(L, LUA_GLOBALSINDEX, "Version");
	// Enum global (section 239; binary slot 0x18C2C4, after the version/Version globals which queue behind their era).
	Lua::Enums::declareAllEnums(L);
	// Yielding waiters (section 249; binary 0x18C2C9: guarded singleton-construct into the yieldEvent member).
	if (yieldEvent.get() == NULL)
		yieldEvent.reset(new Lua::YieldingThreads(this));
	return L;
}
// =================================================================== RuntimeScriptService (batches 730-771; the binary implements it in script/ScriptContext.cpp per IScriptFilter.h:17 -- filed here beside the ScriptContext methods instead; App/script/ScriptContext.cpp stays the empty placeholder). ===================================================================
const char* const sRuntimeScriptService = "RuntimeScriptService";
REFLECTION_BEGIN();
REFLECTION_END();
// Class registrar (see Instance.cpp/Service.cpp note).
RBX_REGISTER_CLASS(RBX::RuntimeScriptService);
// mac: 0x1848EC (batch770:1129-1161): zeroes pendingScripts/runningScripts/ isRunning (connection/sets default-construct; the binary's GCC zeroing of the set headers matches the member default state).
RuntimeScriptService::RuntimeScriptService()
	: isRunning(false)
{
}
// mac: 0x196AC4 (batch770:502-636): old provider -> disconnect; new -> the RunService runState adopt (friend-read, private +112) + assign the transition connect into the scoped connection (per HeartbeatInstance.cpp:25 form; the binary compares, disconnects, assigns).
// STUB: PLAYER 0x0068bd50
// STUB: STUDIO 0x007f14b0
void RuntimeScriptService::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	if (oldProvider != NULL)
		runTransitionConnection.disconnect();
	if (newProvider != NULL)
	{
		RunService* runService = ServiceProvider::create<RunService>(newProvider);
		onRunState(runService->runState);
		runTransitionConnection = runService->runTransitionSignal.connect(
			boost::bind(&RuntimeScriptService::onRunTransition, this, _1));
	}
}
// mac: 0x19ABCC (batch770:634-666): bridge to the run-state writer.
// FUNCTION: PLAYER 0x0068bd40
// FUNCTION: STUDIO 0x007f14a0
void RuntimeScriptService::onRunTransition(RunTransition event)
{
	onRunState(event.newState);
}
// mac: 0x196B9C (batch770:668-830): the isRunning toggle moves the two script sets into/from the ScriptContext -- range-insert BEFORE the temp swap, per the decode; arm bodies over the moved sets.
// STUB: PLAYER 0x0068b9d0
// STUB: STUDIO 0x007f1130
void RuntimeScriptService::onRunState(RunState state)
{
	if ((state != 0) != isRunning)
	{
		isRunning = (state != 0);
		ScriptContext* scriptContext = ServiceProvider::create<ScriptContext>(this);
		if (isRunning)
		{
			runningScripts.insert(pendingScripts.begin(), pendingScripts.end());
			std::set<BaseScript*> temp;
			temp.swap(pendingScripts);
			for (std::set<BaseScript*>::iterator it = temp.begin(); it != temp.end(); ++it)
			{
				ScriptContext::ScriptStartOptions options;
				options.identity = 2;
				scriptContext->addScript(*it, options);
			}
		}
		else
		{
			pendingScripts.insert(runningScripts.begin(), runningScripts.end());
			std::set<BaseScript*> temp;
			temp.swap(runningScripts);
			for (std::set<BaseScript*>::iterator it = temp.begin(); it != temp.end(); ++it)
				scriptContext->removeScript(*it);
		}
	}
}
// mac: 0x19705A (batch753:306-455): the assert contract (binary lines 2776/2780, texts verbatim in the checkpoint), ScriptContext create, the CoreScript arm (start now, identity 2), the backend/running gate (running set + start), else pend by pointer.
// STUB: PLAYER 0x0068bbd0
// STUB: STUDIO 0x007f1330
void RuntimeScriptService::runScript(BaseScript* script)
{
	RBXASSERT(runningScripts.find(script) == runningScripts.end());		// binary line 2776
	ScriptContext* scriptContext = ServiceProvider::create<ScriptContext>(this);
	RBXASSERT(!scriptContext->hasScript(script));		// binary line 2780
	if (RBX::Instance::fastDynamicCast<CoreScript>(script) != NULL)
	{
		ScriptContext::ScriptStartOptions options;
		options.identity = 2;
		scriptContext->addScript(script, options);
		return;
	}
	if (!Network::Players::backendProcessing(this) || isRunning)
	{
		runningScripts.insert(script);
		ScriptContext::ScriptStartOptions options;
		options.identity = 2;
		scriptContext->addScript(script, options);
		return;
	}
	pendingScripts.insert(script);
}

// mac: 0x19745C (batch770:794-1118 + batch771 STR list): find-or-bail, then
// the CoreScript arm = the three membership asserts (2809/2810/2811 --
// thunk texts @B2D6BE/B2D54A/B2D616) and RETURN, no mutation; the
// non-core arm: num = pendingScripts.erase(script) with the "num <= 1"
// assert (binary line 2816, @B2D8A8); num==0 (was running) asserts
// runningScripts membership (2820, @B2D91E), erases it, asserts
// scriptContext->hasScript (2823, @B2D9EA), hands to
// ScriptContext::removeScript; num==1 (was pending) asserts
// !hasScript (2828, @B2D616 short / @B2DA90 full) and STOPs -- no
// removeScript on that arm.
// STUB: PLAYER 0x00684800
// STUB: STUDIO 0x007ea260
void RuntimeScriptService::releaseScript(BaseScript* script)
{
	ScriptContext* scriptContext = ServiceProvider::find<ScriptContext>(this);
	if (!scriptContext)
		return;

	if (RBX::Instance::fastDynamicCast<CoreScript>(script) != NULL)
	{
		// CoreScript arm, assert-only (batch770:858-969): a released core
		// script must sit in none of the three sets. No writes follow.
		RBXASSERT(pendingScripts.find(script) == pendingScripts.end());		// binary line 2809 (@B2D6BE)
		RBXASSERT(runningScripts.find(script) == runningScripts.end());		// binary line 2810 (@B2D54A)
		RBXASSERT(!scriptContext->hasScript(script));						// binary line 2811 (@B2D616 / B2D822)
		return;
	}

	std::size_t num = pendingScripts.erase(script);
	RBXASSERT(num <= 1);		// binary line 2816 (@B2D8A8)

	if (num == 0)
	{
		// Was running (batch770:995-1079): must be in runningScripts;
		// erase it, confirm the ScriptContext still holds it, remove.
		RBXASSERT(runningScripts.find(script) != runningScripts.end());		// binary line 2820 (@B2D91E)
		runningScripts.erase(script);
		RBXASSERT(scriptContext->hasScript(script));						// binary line 2823 (@B2D9EA)
		scriptContext->removeScript(script);
	}
	else
	{
		// Was pending (batch770:1081-1117): never registered with the
		// ScriptContext -- assert and stop (no removeScript here).
		RBXASSERT(!scriptContext->hasScript(script));						// binary line 2828 (@B2D616 / B2DA90)
	}
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::ScriptContext, 0x288);

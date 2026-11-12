/* V8DataModel/ScriptContext.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: referenced from DataModel::clearContents 0x2CF1D0 / doCloseDataModel
     0x2D3952 (clearContents 0x2CF1D0 calls closeStates; setPlaceID wires setRobloxPlace. Big scripting module � stub only.); "ScriptContext" string check TODO per service.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ScriptContext.h */

#pragma once

#include "V8DataModel/IScriptFilter.h"	// ScriptContext is an IScriptFilter (scriptShouldRun override)
#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "reflection/property.h"  // propLuaGcLimit/propLuaGcFrequency declarations
#include "Util/ProtectedString.h"
#include "rbx/RunningAverage.h"
#include "rbx/atomic.h"    // timedout / timedoutCount (+0x138/+0x124)
#include "rbx/CEvent.h"    // checkTimeout (+0x170); brings boost/thread.hpp
#include "script/ExitHandlers.h"
#include "script/LuaMemory.h"
#include "script/ScriptEvent.h"
#include "script/ThreadRef.h"
#include "rbx/TaskScheduler.Job.h"	// gcJob / waitingScriptsJob

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <map>     // scriptStatsMap (+756)
#include <vector>  // pendingScripts/loadingScripts (+164/+176)
#include <set>   // scripts (+0x84 tree; closeStates is its first user)

LOGGROUP(CoreScripts)
// 0xFA9B20 byte = 0xFF in the 2013 PE (batch703; registration
// InitFunc_480 0x670B59): gates the loadfile/executeInNewThread dumps.
LOGGROUP(FullScriptCode)
LOGGROUP(ScriptContext)
// ScriptContextAdd / ScriptContextRemove: the 2013 queue-log groups the
// C1 add/remove plumbing logs through (addScript's 'ScriptContext::addScript
// -- %s', removeScript's 'RemoveScript %s', 'ScriptContext', the
// FastLogS-authored confirm texts).
LOGGROUP(ScriptContextAdd)
LOGGROUP(ScriptContextRemove)
// 0xFA9B44/0xFA9B48 in the 2013 PE (batches 719/720; registration
// InitFunc_480 0x670C67): lua script-timeout span in seconds, byte
// 0xFF at rest (255 s default monitor).
LOGGROUP(LuaScriptTimeoutSeconds)

struct lua_State;
struct lua_Debug;

namespace RBX {

	extern const char* const sScriptContext;

	class BaseScript;
	class Heartbeat;
	class IScriptFilter;
	class LibraryService;
	class ScriptStats;
	class LuaStatsItem;
	class RunService;

	// SIZE 0x288
	class ScriptContext
		: public DescribedNonCreatable<ScriptContext, Instance, sScriptContext, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
		, public IScriptFilter
	{
	public:
		// Defined out-of-line (ScriptContext.cpp): the ctor tail resets
		// libraryService, and this header can only fwd-declare
		// LibraryService (including script/LuaLibrary.h here cycles
		// back through lua/luaconf.h, which includes this header).
		ScriptContext();
		~ScriptContext();

		void setRobloxPlace(bool robloxPlace);  // 0x18E976: *(this+245) + CoreScripts log.

		// mac: 0x2CF1D0: clearContents sets byte +247 = 1 on entry and calls
		// closeStates(reset) at the end.
		// mac: 0x18E9BC-0x18EE84 (batch722 FULL body; callers: detach arm
		// mac: 0x194FFD + clearContents 0x2CFD89).  Sets +247 first (outside
		// the reset gate), early-outs on resettingSimulation, resets
		// commandLineSandbox, logs ScriptCount/WaitingThreads
		// (FLog::ScriptContextClose 0xF88531 default 1 = ON),
		// scripts.clear(), logs LeakedWaitingThreads + yieldEvent.reset(),
		// waitingThreads.clear(), closes + NULLs both globalStates
		// (closeState 0x18EE84 x2), then allocator heap stats
		// (allocatorView +0x2E0): ScriptContextClose heap logs,
		// clearHeapMax, and the "Script memory leaks" MESSAGE_ERROR when
		// the open-state counter (port g_openLuaStates; binary
		// mac: dword_FADA34: InitFunc_123 0x1AA72B writes rest 0, openState
		// mac: 0x18B638 xadd +1, closeState 0x18EEC8 xadd -1) is 0 and the
		// heap is non-empty.  DELTAs: gcJob/waitingScriptsJob
		// removeBlocking blocks (members +0x2E4/+0x2EC unported --
		// scheduler era; nothing schedules them); the FFlag::LuaDebugger
		// branch (loop vs unrolled close -- both variants close both
		// slots identically, the port uses one loop).
		void closeStates(bool resettingSimulation);
		// +132 (0x84) tree: std::set<BaseScript*> (_Rb_tree with
		// _Identity, _M_erase'd by closeStates 0x18E9BC; the writers are
		// this era's addScript 0x195358 / removeScript 0x195672
		// (closeStates is a consumer; the size feeds the ScriptCount log).

		// ===== C1 script-plumbing era (batches 733-771) =====
		// ScriptStart (GCC 60B, settled by addScript's copy chain --
		// binary mangle startScript(ScriptStart) passes it BY VALUE):
		//   +0  script           shared_ptr<BaseScript>
		//   +8  identity         int
		//   +12 continuation     boost::function1<void, shared_ptr<const Tuple>>
		//   +28 reportError      boost::function4<void, const char*, const char*, shared_ptr<BaseScript>, int>
		//   +44 sourceTransform  boost::function1<std::string, const std::string&>
		// ScriptStartOptions = int identity + the same three boost functions
		// (addScript's mangle takes the options BY VALUE too).
		struct ScriptStart
		{
			boost::shared_ptr<BaseScript> script;
			int identity;
			boost::function1<void, boost::shared_ptr<const Reflection::Tuple>> continuation;
			boost::function4<void, const char*, const char*, boost::shared_ptr<BaseScript>, int> reportError;
			boost::function1<std::string, const std::string&> sourceTransform;

			ScriptStart() : identity(0) {}
		};
		struct ScriptStartOptions
		{
			int identity;
			boost::function1<void, boost::shared_ptr<const Reflection::Tuple>> continuation;
			boost::function4<void, const char*, const char*, boost::shared_ptr<BaseScript>, int> reportError;
			boost::function1<std::string, const std::string&> sourceTransform;

			ScriptStartOptions() : identity(0) {}
		};

		// +156: pendingScripts retry deadline (onHeartbeat 0x19341A drains
		// when Time::now() >= nextPendingScripts and re-arms with +1.0s;
		// binary Time used its 8-byte double form here).
		// +164: the non-core start queue (startScript pends here when the
		// scriptsDisabled prop is set; startPendingScripts drains it on
		// enable and on the onHeartbeat deadline).
		// +176: the code-loading queue (startScript pends here when
		// requestCode reports not-yet-loaded; drained first, unconditionally).
		// +244: reader-side gating byte in startScript's identity==2
		// resolution (writer unported -- LOUD DELTA).
		// +246: the "ScriptsDisabled" BoundProp target (internal logic
		// inverted: set -> non-core scripts pend instead of start).
		// +288: startScript re-entrancy depth (throws
		// std::runtime_error("startScript re-entrancy has exceeded 3") past 3).

		// +756: std::map<std::string, ScriptStatInformation>.
		// startScript registers each run under the script's getFullName()
		// (name assign from the flyweight + push_back this shared_ptr);
		// disassociateState erases the script px (and the entry when empty).
		// The stats-era readers (getScriptStatsNew 0x18A6AC, landed
		// this era) touch ONLY these two fields: name (the fill
		// reassigns it from fw+24, batch716:941-945) and runningScripts
		// (push_back, batch716:946-951) -- payload pinned complete.
		struct ScriptStatInformation
		{
			std::string name;                                        // +0 (binary field 0)
			std::vector<boost::shared_ptr<Instance>> runningScripts; // +4
		};

		// ===== C1 method surface (bodies in ScriptContext.cpp) =====
		// mac: 0x195358 (485B): scripts.insert + RBXASSERT(ok) (binary line 2256)
		// + ScriptContextAdd log; builds the 60B ScriptStart from the
		// options (boost functions assign-own) and calls startScript BY
		// VALUE.
		void addScript(BaseScript* script, ScriptStartOptions options);
		// mac: 0x19563A: erase the FIRST queue entry whose script px matches
		// (removeScript calls it on both queues).
		void eraseScript(std::vector<ScriptStart>& queue, BaseScript* script);
		// mac: 0x195672: scripts.erase (RBXASSERT(num == 1), binary line 2304) +
		// ScriptContextRemove log + disassociateState + eraseScript on both
		// queues.
		void removeScript(BaseScript* script);
		// mac: 0x19553E: name log; erase this script's px from the +756 entry
		// (and the entry when empty); eraseAllRefs + release the
		// WeakThreadRef node; fire the script's stopped signal.
		void disassociateState(BaseScript* script);
		// mac: 0x193530 (711B): drain loadingScripts; then -- when the
		// scriptsDisabled prop is clear -- drain pendingScripts (FLog-gated
		// per-item fastlogs + the elapsed-time watchdog log).
		void startPendingScripts();
		// mac: 0x193E26 (432B): resolve the identity (2 -> isRobloxScript /
		// unknown244 / robloxPlace favor), re-entrancy guard, the
		// requestCode(0)-load-or-pend arm, settings logging, newthread +
		// hook mask (11 when FLog::LuaProfiler or FFlag::LuaProfilerHooks,
		// else 8) + setThreadIdentity + threadNode + the +756 registry,
		// luaL_loadprotected, the reportError path or starting(L) + resume.
		void startScript(ScriptStart start);

		// mac: 0x189990: AddStarterScript scriptable (throws
		// std::runtime_error("AddStarterScript bad argument: assetId = 0")
		// on 0; rbxassetid://%d ContentId; the created StarterScript
		// parents AT this ScriptContext).
		boost::shared_ptr<Instance> addStarterScript(int assetId);
		// mac: 0x189E1A: AddCoreScript scriptable (throws
		// std::runtime_error("AddCoreScript bad argument: assetId = 0");
		// rbxassetid://%d ContentId create<CoreScript>; setName when the
		// name is non-empty; setParentInternal(parent-or-this); when the
		// created CoreScript is NOT parented AT this, addScript{identity 2}).
		boost::shared_ptr<Instance> addCoreScript(int assetId, boost::shared_ptr<Instance> parent, std::string name);
		// mac: 0x18A2F4 (the "ScriptsDisabled" prop onChanged): jmp to
		// startPendingScripts (0x18A304 -- the only other xref is the
		// onHeartbeat deadline 0x1934DF).
		void onChangedScriptEnabled(const Reflection::PropertyDescriptor& descriptor);

		// runScript/releaseScript's assert contract (binary
		// '!scriptContext->hasScript(script)', binary line 2780): the
		// scripts-set membership test.
		bool hasScript(BaseScript* script) const { return scripts.find(script) != scripts.end(); }

	protected:
		// mac: 0x1952EA (the IScriptFilter override; batch743:379-397): isA
		// CoreScript on the live script; the '+36 != 36' return-arm
		// rendering is pointer-truth on the argument (session-133
		// askSetParent artifact family; 2016 :2781-2784 conventional
		// crosscheck) -- no member read at +36, DELTA retired in the cpp.
		/*override*/ virtual bool scriptShouldRun(BaseScript* script);
	public:

		// mac: 0x18B4C2 (233B, batch313: segregated index by
		// isInRole(identity, 4) under the flag, else slot 0; :552/:564
		// NULL asserts).
		lua_State* getGlobalState(Security::Identities identity);
		// mac: 0x18DB92 (250B, batch324: prints current identity, or
		// arg + identity, via StandardOut).
		static int printidentity(lua_State* L);
		// mac: 0x18CB5A (36B, batch324) / 0x18CBB0 (40B, batch324): time
		// globals. Static C functions (binary __cdecl lua_State*).
		static int tick(lua_State* L);
		// time global (0x18CB7E, batches 639/663: RunService gameTime,
		// binary SC+0xBC cache; ours resolves live, see body).
		static int time(lua_State* L);
		static int rbxTime(lua_State* L);
		// mac: 0x18C8CC (653B, batch324: tostring-join to StandardOut).
		// mac: 0x18B48A (56B, batch326: permission gate + user crash).
		static int print(lua_State* L);
		static int crash(lua_State* L);
		// mac: 0x18B5AC core (batch320: allocator + newstate + atpanic +
		// context link + 4 locked libs + string-metatable lock + _G;
		// gc tuning, Bridges, globals, jobs queue in later parts).
		lua_State* openState();
		// mac: 0x190A9E (101B, batch313: identity/flag gate to the
		// Identities overload, else the +104 debugger-state fast path
		// -- the debugger arm queues behind the debugger-state member
		// and thread-field mapping; loud tripwire if ever reached).
		// Binary-pinned STATIC (called with no instance: 0x1AEB6E etc.;
		// the 2016-carry decl was member).
		static lua_State* getGlobalState(lua_State* thread);
		// mac: 0x18EE84 (310B, batch314: ref dumps + extra-space ref wipe
		// + lua_close + open-state counter decrement; returns the
		// post-decrement count like the binary).
		int closeState(lua_State* L);
		// mac: 0x190A7A (35B, batch314: summed Shared.threadCount over
		// both globalStates slots).
		int getThreadCount() const;
		// mac: 0x190900 (206B, batch313: locked sandbox env table with
		// __index -> globals; pseudoindices verbatim (-10002 globals).
		// Static in spirit (no this-use; member per the mangling).
		static void sandboxThread(lua_State* thread);	// 2013: static (0x0066fd40 ends in ret, not ret 4)
		// mac: 0x1909CE (171B, batch316: sandbox + identity/ script stash
		// in the thread extra space + readback assert :1231).
		void setThreadIdentity(lua_State* thread,
			Security::Identities identity,
			boost::shared_ptr<BaseScript> script);
		// Identity readback for the assert (binary getThreadIdentity).
		static Security::Identities getThreadIdentity(lua_State* thread);
		// lua_sethook callback armed by startScript (binary 0x18B2C2;
		// body LANDED in the hook era per batch744:281-338 ==
		// batch747:538-595 -- event dispatch {line=timeout hand-off,
		// ret=profiler log, others=the binary's line-506 guard,
		// call=profiler dispatch}; see the body comment).
		static void hook(lua_State* thread, lua_Debug* ar);
		// mac: 0x18B3EC (batch746:118-134, hook era): the line-hook kill arm
		// -- see the onHook body comment in ScriptContext.cpp.
		void onHook(lua_State* thread, lua_Debug* ar);

		// mac: 0x194F84 (batch708/713 full body): service attach/detach
		// hook, fired by ServiceProvider::onDescendantAdded/Removing
		// (0x55219C). Old arm: timeout trio (endTimoutThread/Set/join)
		// + LS delete + closeStates(false); both arms: stats resets;
		// new arm: StatsService create + collectStats-gated ScriptStats
		// holder, globalStates fill (+0x64/+0x68), FLog-gated setTimeout
		// arm, LS re-create, RunService create (cache/Heartbeat tail
		// DELTA). See body.
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider,
			ServiceProvider* newProvider);

		// mac: 0x18A30C-0x18A54B (batch719 body; attach arm 0x195173-0x195197
		// batch719/720): arm the script run limit.  timeoutMutex-locked;
		// a zero span clears the deadline (timoutTime = Time()); else
		// FASTLOG "Set lua time out: %d" (0xB2C4E4, xref 0x18A37C),
		// deadline = now + span, then kick the running monitor
		// (checkTimeout.Set) or spawn onCheckTimeout through
		// RBX::thread_wrapper ("ScriptContext Timeout" 0xB2C4F9).
		// Reflection registration ("SetTimeout"/"seconds",
		// func_SetTimeout wrapper 0xFA65C8, InitFunc_123 0x1AAB0F)
		// stays queued with the reflection-era DELTA.
		void setTimeout(double seconds);
		// mac: 0x190786-0x190900 (batch720; 2016 form identical): monitor
		// thread body.  Sleeps on checkTimeout in 1.01x-span chunks
		// (min 1 ms; INFINITE when the span <= 0), exits on
		// endTimoutThread, re-locks and either parks at -1 (span zero ==
		// disabled) or measures the deadline; expiry resets the chunk to
		// the span and latches timedout (swap(1)==0 -> ++timedoutCount).
		// The timedout consumers (script-kill) are the heartbeat era.
		void onCheckTimeout();

		// Lua states (0x18B4C2: globalStates[vm] at +100, two slots;
		// filled by onServiceProvider 0x195100 -- batch570: openState
		// itself never stores +0x64/+0x68 (the +0x64 read at 0x18B657 is
		// LuaSettings::getGcStepMul). 2016 wraps these in a GlobalStates
		// struct; binary pins two lua_State* words.
		// ---- Script-timeout monitor (setTimeout 0x18A30C +
		// onCheckTimeout 0x190786; batches 719/720).  Binary member
		// block +0x124..+0x170 in 2016 header order (port layout is
		// by-name): timedoutCount +0x124 (interlocked inc on the first
		// expiry), timoutSpan +0x128 (Time::Interval; the 2013/2016
		// "timout" spelling kept; 0 == no timeouts), timoutTime +0x130
		// (deadline), timedout +0x138 (swap(1) latch), timeoutThread
		// +0x13C (scoped_ptr<boost::thread>), timeoutMutex +0x140,
		// endTimoutThread +0x16C, checkTimeout +0x170 (auto-reset CEvent).
		// mac: 0x190DEE (426B, batch279/710): spawn forwarder -- zero-argument
		// pusher (sub_190DE7 = {return 0;}), no result callback, empty
		// continuations, no parent state. DataModel::loadCoreScripts
		// mac: 0x2D5790 passes identity RobloxGameScript_ + getStringById(4).
		void executeInNewThread(RBX::Security::Identities identity, const ProtectedString& script, const char* name);
		// mac: 0x190F98 (batch279/708/709/710): identity-gated spawn. CmdLine_
		// reuses the persistent commandLineSandbox member (+0x6C); all other
		// identities run in a fresh child of globalStateToExecuteIn (or the
		// identity's global state), sandboxed via setThreadIdentity.
		// Continuations must be empty (assert line 1517); the binary takes
		// script + continuations BY VALUE and the parent may be NULL
		// (addLibrary passes FFlag::SegregatedLuaEnvironments ? slot : NULL).
		void executeInNewThread(RBX::Security::Identities identity, ProtectedString script, const char* name,
			boost::function1<unsigned long, lua_State*> pushArguments,
			boost::function2<void, lua_State*, unsigned long> readImmediateResults,
			Scripts::Continuations continuations,
			lua_State* globalStateToExecuteIn);
		// Yield globals (section 249: wait 0x18CBD8 / delay 0x18CC3A /
		// spawn 0x18D8A4 / version 0x18E942, batches 471/477). ypcall
		// queues behind Continuations (250).
		static int wait(lua_State* L);
		static int delay(lua_State* L);
		static int spawn(lua_State* L);
		static int version(lua_State* L);
		// mac: 0x18E89C (batch599: create<StatsService> from the calling
		// thread's ScriptContext + SharedPtrBridge push; returns 1).
		static int stats(lua_State* L);
		// dofile 0x18DC8C / loadfile 0x18DE94 (520B/1261B, batch640;
		// registration pushcclosure 0x18C033/0x18C067 + setfield
		// mac: 0x18C038/0x18C081 "dofile"/"loadfile", batch637). Both gate on
		// RobloxPlace; dofile = getFile -> luaL_loadfile -> lua_error on bad
		// chunk -> lua_call(0,MULTRET) -> result-count delta; loadfile =
		// getContentString -> verifyScriptSignature(body, true) ->
		// fromTrustedSource -> luaL_loadprotected (nil+err on failure).
		static int dofile(lua_State* L);
		static int loadfile(lua_State* L);
		// settings-tree cfuncs (0x18E432/0x18E4E0/0x18E58E/0x18E7EE, batches 645/647:
		// permission gate + singleton push; LoadLibrary/LoadRobloxLibrary queue
		// behind LibraryService).
		static int settings(lua_State* L);
		static int usersettings(lua_State* L);
		static int pluginmanager(lua_State* L);
		static int debuggermanager(lua_State* L);
		// Library entry points (0x18E63C/0x18E70A, batches 645/647);
		// LoadRobloxLibrary gates on WritePlayer.
		static int loadLibrary(lua_State* L);
		static int loadRobloxLibrary(lua_State* L);
		// load global (0x18E382, batch650: unconditional not-implemented throw).
		static int load(lua_State* L);
		// mac: 0x18A54C (352B, batch587: allocator counters into a 4-int
		// Tuple; optional clearHeapMax. luaAllocator must exist
		// (openState creates it) -- no null guard, binary-identical).
		shared_ptr<const Reflection::Tuple> getHeapStats(bool clearHighwaterMark);
		// mac: 0x193CAC (394B, batch577: COUNT both states, sample deltas,
		// STEP full(100)/single(1) by threshold, sample times).
		void stepGc();
		// Resume core (section 250: resume#1 0x192A50 / resume#2 0x192D02 /
		// scheduleResume 0x196714 / ypcall 0x18CF42 / on_ypcall pair
		// mac: 0x19316C/0x1932C8 / Continuations handlers, batches 459/472/479).
		// Drain (section 251): resumeWithArgs 0x193B70 /
		// resumeWaitingScripts 0x1937F8 (batch484).
		static int resume(Lua::ThreadRef thread,
			boost::function1<unsigned long, lua_State*> pushArguments,
			boost::function2<void, lua_State*, unsigned long> onResults);
		int resume(Lua::ThreadRef thread, int nargs);
		void scheduleResume(Lua::ThreadRef thread,
			boost::shared_ptr<const Reflection::Tuple> result);
		void resumeWithArgs(Lua::ThreadRef thread,
			boost::shared_ptr<const Reflection::Tuple> args);
		void resumeWaitingScripts(RBX::Time expirationTime);
		static int ypcall(lua_State* L);
		// Protected-call trio (section 259: getContext 0x18B3DE +
		// call#2 0x190B94 + call#1 0x191E3B, batches 544-551).
		static ScriptContext* getContext(lua_State* thread);
		void call(Lua::WeakFunctionRef& function,
			boost::function1<unsigned long, lua_State*> pushArguments,
			boost::function2<void, lua_State*, unsigned long> onResults);
		boost::shared_ptr<const Reflection::Tuple> call(
			Lua::WeakFunctionRef& function,
			const Reflection::Tuple& args);
		// Bound by ypcall as member-fns (mf2, batch472): non-static.
		void on_ypcall_success(Lua::WeakThreadRef caller, lua_State* functor);
		void on_ypcall_failure(Lua::WeakThreadRef caller, lua_State* functor);
		// Callstack printer (batch714-716: static 2-arg, 0x195764-0x195CA5;
		// __ZN3RBX13ScriptContext14printCallStackEP9lua_StatePSs).  Gate =
		// DebugSettings singleton +0x5D byte; prints "Script '<src>',
		// Line N[ - <namewhat> <name>]" per frame at MESSAGE_INFO, plus a
		// lowercase "stack end" trailer when frames remain under the
		// 12-level cap; a non-NULL output collects the same text.
		static void printCallStack(lua_State* thread, std::string* output);
		// Error pipeline (0x195CA6-0x196713, batch714-716 full body):
		// tostring fallbacks ("An error occurred" / 35-char "Error
		// occurred, no output from Lua."), printCallStack behind
		// DFInt::LuaExceptionPlaceFilter==placeID (dm+0xD80), the
		// StatsService "LuaExceptions" report (PlaceId/CallStack/Error/
		// PlaceVersion), the DFFlag::LuaExceptionSignalEnabled DataModel
		// void() signal (DM+0xAEC), the errorSignal(errorMessage,
		// callStack, scriptPtr) fire (RBXASSERT scriptPtr==source at
		// ScriptContext.cpp:2437) and BaseScript::extraErrorReporting.
		void reportError(lua_State* thread);
		// Lua-error signal (binary Context+720/0x2D0, adjacent to the
		// legacy-read signal at +724): reportError fires it with
		// (errorMessage, callStack, scriptPtr) when non-empty.  2016-carry
		// name/arity.  The stats/debug era (2026-10-17) retired the old
		// "inert fire" note: the signal is REGISTERED as the reflected
		// event "Error" -- binary EventDesc at InitFunc_123 0x1AAE44
		// passes member offset 720 with args "message"/"stackTrace"/
		// "script" and security 1 = Security::Plugin (batches 730/731).
		// DELTA: 2016 carries Security::None here; 2013 binary is the
		// authority.
		// Legacy-spelling member-read signal (section 258b; binary
		// Context+724, fired by sub_1756D3 + the CamelCase fallback with
		// (instance, name, script)).  Registered as the reflected event
		// "CamelCaseViolation" (binary EventDesc at InitFunc_123 0x1AA664:
		// member offset 724, args "object"/"member"/"script", security 4 =
		// Security::WritePlayer).  That registration is what gives scripts
		// their listener (stats/debug era, 2026-10-17).  Two DELTAs, both
		// binary-backed: (a) the mangled EventDesc instantiation types the
		// third slot shared_ptr<Instance>, NOT BaseScript -- retyped to
		// match (the fires' shared_ptr<BaseScript> converts up); (b) the
		// 258b-era inferred member name instanceLegacyRead is retired for
		// 2016's camelCaseViolation (identical signal + fire shape; the
		// binary event string confirms the semantic).
		// Callstack formatter (section 255: 0x19237A, batches 499-503).
		// Returns the "chunk, line N ..." stack text; outs the level-0
		// script (never throws -- null when none) and its current line
		// (-1 when no frames).
		static std::string extractCallStack(lua_State* thread,
			shared_ptr<BaseScript>& script, int& line);
		// Yielding waiters (section 249; openState constructs it after
		// declareAllEnums). Exact +196 offset queues behind the DM-era
		// layout audit; reached via this member (never raw offsets).
		// Scheduler resume queue (section 250; scheduleResume pushes
		// WaitingThreads here for resumeWaitingScripts, section 251).
		// Shape per 2016 + the 0x196714 memberwise ops (ThreadRef copy +
		// shared Tuple copy, both RAII here).
		struct WaitingThread
		{
			Lua::ThreadRef thread;
			boost::shared_ptr<const Reflection::Tuple> result;
		};
		// Script-stats holder (section 250 DELTA retired by batch599:
		// binary +280 is a shared_ptr<ScriptStats>, not a raw pointer.
		// The enable path reset()s a 64B C1 object here only when a
		// StatsService is already findable; disable releases. resume's
		// two guarded blocks now call scriptResumeStarted/
		// scriptResumeStopped with getFullName() -- landed this era
		// per batch716:503-507/592-596).
		// +276 role: collect-stats flag (setCollectScriptStats 0x18B23E
		// store; C2 false is a deterministic stand-in -- binary C2 value
		// unevidenced, same LOUD-DELTA class as the GC members below).
		// +0xF6 role: stats-disabled flag, bound by LuaStatsItem::init
		// as the "disabled" child (batch581). Writer unevidenced;
		// C2 false stand-in.
		// +0x2A4 role: threads-throttled sampler, bound by
		// LuaStatsItem::init as "ThreadsThrottled" (batch581).
		// +0x6C (24B Lua::WeakThreadRef; live field +0x7C, batch279/710):
		// persistent CmdLine_-identity sandbox thread -- created on first
		// use (getGlobalState(CmdLine_) + lua_newthread + setThreadIdentity)
		// and reused by later CmdLine_ spawns. 2016 name: commandLineSandbox.
		// +0x26C role: resume-rate sampler feeding the ThreadsResumed
		// display via LuaStatsItem::update's formatRate (batch610).
		// Writer unevidenced; default-constructed.
		// mac: 0x18B230 (batch599: +276 store + find-gated holder create/
		// release -- see body).
		void setCollectScriptStats(bool collect);
		// Bring-up views (no binary getters recovered).
		bool getCollectScriptStats() const { return collectStats; }
		bool hasScriptStats() const { return scriptStats.get() != NULL; }

		// Stats-era readers, landed per batches 733/716/757:
		// mac: 0x18A6AC getScriptStatsNew (batch733: per-script ValueTable
		// list; the visible prefix is binary-verbatim, the tail keys
		// are 2016 semantics -- LOUD DELTAs in the body comments) and
		// mac: 0x1902A8 getScriptStats (body unrecovered -- 2016 Tuple-5
		// shell, loud DELTA). The registered wrapper 0xFA6630 binds
		// the New form ("GetScriptStats", security 4, arity-0,
		// batch757:704-716).
		boost::shared_ptr<const std::vector<Reflection::Variant> > getScriptStatsNew();
		boost::shared_ptr<const Reflection::Tuple> getScriptStats();

		// LibraryService wiring (ctor 0x18F8EA; wrappers 0x189BD8/
		// mac: 0x189C0A/0x189C3C -- batch734 corrected the old 0x189C46/
		// mac: 0x189D7E reading; addLibrary 0x19289D; onHeartbeat arm).
		void addLibrary(const std::string& library, const ProtectedString& code);
		// mac: 0x189BD8: force = true (Roblox-trusted).  registerLibrary
		// (0x189C0A) passes force = false (batch734).
		void registerRobloxLibrary(std::string name, std::string url);
		void registerLibrary(std::string name, std::string url);
		// (std::string, shared_ptr<Instance>) both BY VALUE (mangled Ss +
		// shared_ptr<Instance>, batch706); the name is forwarded verbatim
		// to LibraryService::registerDevelopmentLibrary.
		void registerDevelopmentLibrary(std::string name, boost::shared_ptr<Instance> instance);
		void markLibrariesRegistered();
		void onHeartbeat(const Heartbeat& heartbeat);

		// Lua fork hook (luaconf.h RobloxExtraSpace): anchor validity for
		// thread registration. Security era owns the real check; stub
		// returns valid (true) so Lua threads register in-tree.
		bool checkSecurityAnchorValid() { return true; }

		// Lua allocator holder. Batch602 correction: openState creates
		// DIRECTLY into +0x2E0 (guarded null-check + raw store, no
		// +184 traffic), so the +184 scoped_ptr role the audit
		// carried is UNEVIDENCED -- this holder is our ownership
		// vehicle (same object, same create-once lifecycle), kept
		// for leak-free teardown; the +0x2E0-role mirror below is
		// what getHeapStats reads.
		// +0x2E0 role: raw allocator view (getHeapStats 0x18A558
		// dereferences this slot, not the +184 holder -- audit
		// reconciliation; mirrored at creation, SC-owned so it never
		// dangles).

		// +0x2D8-role: the library service (ctor 0x18F8EA; public --
		// the cfuncs + LibraryService::ContentReadyHelper reach in).

		// GC-era telemetry (stepGc 0x193CAC + GcJob frequency, batches
		// 575/577; binary offsets +0xF8/+0xFC/+0x100/+0x108/+0x1E8/
		// +0x214/+0x240/+0x258 differ -- BY-NAME access throughout, and
		// deterministic zero-init in C2 below vs uninitialized binary
		// slots (LOUD DELTA; unobservable -- values feed only the
		// LuaStatsItem UI until the stats era wires listeners).

		// Class-static prop descriptors (binary symbols __ZN3RBX13ScriptContext
		// {14propLuaGcLimit,18propLuaGcFrequency}E @0xFA6660/0xFA668C;
		// definitions in ScriptContext.cpp next to the class registrar).
		static Reflection::BoundProp<int> propLuaGcLimit;
		static Reflection::BoundProp<int> propLuaGcFrequency;
		// 0xFA659C: "ScriptsDisabled" BoundProp<bool> (member +246),
		// ReadWrite, onChanged = onChangedScriptEnabled (0x18A2F4),
		// security 2 = RobloxPlace; binary UI flags 21 (InitFunc_123 --
		// 1AA6-row; encoded raw in the .cpp definition next to the
		// class registrar).
		static Reflection::BoundProp<bool> propScriptsDisabled;

	private:

		// ---- 2013 members (PLAYER ctor 0x0068ad20 and dtor 0x0068b1a0 read store by store;
		// STUDIO ctor 0x007f0480 is the same). Bases end at 0x64: Instance (0x60), Service (empty),
		// IScriptFilter vfptr at 0x60. Size 0x288 (operator new in 0x00421700). 2013 disagrees
		// with 2016: no GlobalStates struct or securityAnchor, no preventNewConnection,
		// loadedModules, loadedAssetModules or statesClosed; a third GC average. ----
	public:
		bool m_unk0x64;													// 0x64 ctor sets true
		lua_State* globalStates[2];										// 0x68 getGlobalState 0x00670d71 (ctor leaves them)
		Lua::WeakThreadRef commandLineSandbox;							// 0x70 (vftable 0x00b348b0)
		std::set<BaseScript*> scripts;									// 0x8c
		RBX::Time nextPendingScripts;									// 0x98
		std::vector<ScriptStart> pendingScripts;						// 0xa0
		std::vector<ScriptStart> loadingScripts;						// 0xb0
		boost::shared_ptr<RunService> runService;						// 0xc0 src2016 (shared_ptr in the 2016 slot)
		boost::scoped_ptr<Lua::YieldingThreads> yieldEvent;				// 0xc8 dtor 0x0068b50f
		rbx::safe_queue<WaitingThread> waitingThreads;					// 0xcc
		bool m_unk0xf0;													// 0xf0 startScript's identity-2 gate (mac +244)
		bool robloxPlace;												// 0xf1 setRobloxPlace 0x0066fd14
		bool scriptsDisabled;											// 0xf2 (mac +246; LuaStatsItem's "disabled")
		bool contentClearing;											// 0xf3 (mac +247)
		int gcFullStepThreshold;										// 0xf4 ctor: LuaSettings +0x78
		int gcFrequencyBase;											// 0xf8 ctor: LuaSettings +0x7c (GcJob ctor reads it)
		RBX::Time lastGcTime;											// 0x100
		int lastGcKb;													// 0x108
		boost::shared_ptr<LuaStatsItem> statsItem;						// 0x10c src2016 (onServiceProvider 0x0068b6c8)
		bool collectStats;												// 0x114
		boost::shared_ptr<ScriptStats> scriptStats;						// 0x118
		int reentrancy;													// 0x120 startScript depth
		rbx::atomic<int> timedoutCount;									// 0x124
		Time::Interval timoutSpan;										// 0x128
		Time timoutTime;												// 0x130
		rbx::atomic<int> timedout;										// 0x138
		boost::scoped_ptr<boost::thread> timeoutThread;					// 0x13c
		boost::mutex timeoutMutex;										// 0x140
		volatile bool endTimoutThread;									// 0x148
		CEvent checkTimeout;											// 0x14c
		RunningAverage<double> avgLuaGcInterval;						// 0x150
		RunningAverage<double> avgLuaGcTime;							// 0x180
		RunningAverage<double> avgLuaGcMem;								// 0x1b0
		RunningAverageTimeInterval<> threadsResumedRate;				// 0x1e0
		RunningAverage<double, double> threadsThrottled;				// 0x220
		rbx::signal<void(std::string, std::string, boost::shared_ptr<Instance>)> errorSignal;					// 0x250 event "Error"
		rbx::signal<void(boost::shared_ptr<Instance>, std::string, boost::shared_ptr<Instance>)> camelCaseViolation;	// 0x254
		boost::scoped_ptr<LibraryService> libraryService;				// 0x258 ctor body 0x0068b0d1
		rbx::signals::scoped_connection heartbeatConnection;			// 0x25c src2016 (closeStates disconnects it)
		boost::scoped_ptr<LuaAllocator> luaAllocator;					// 0x260 dtor 0x0068b2b9
		boost::shared_ptr<TaskScheduler::Job> gcJob;					// 0x264 src2016
		boost::shared_ptr<TaskScheduler::Job> waitingScriptsJob;		// 0x26c src2016
		std::map<std::string, ScriptStatInformation> scriptStatsMap;	// 0x274
		int camelCaseViolationCount;									// 0x280 src2016 (ctor 0)
		rbx::signals::connection camelCaseViolationConnection;			// 0x284 src2016 (ctor connects onCamelCaseViolation)

		// No 2013 slot: statics so LuaStatsItem's bound children still compile.
		static bool disabled;
		static double gcStepThreshold;
	};

} // namespace RBX

/* Util/FastFlags.h — October 2013 (static fast-flag declarations).

   The 2013 client binary reads plain namespace-scope `bool` globals out of
   namespace FFlag; the mangled symbol proves the namespace and name:

     __ZN5FFlag43PhysicsAllowAutoJointsWithSmallParts_DE6056E
       -> namespace FFlag, 43-char name "PhysicsAllowAutoJointsWithSmallParts_DE6056"
       -> read by Joint::canBuildJoint 0x5784C7 as
          `cmp ds:(__ZN5FFlag43..._DE6056E - base)[eax], 0`

   DELTA (documented, unresolvable from the binary): a static fast flag's
   *value* is delivered at run time by the Studio fast-flag service and is not
   recoverable from the executable alone -- only the two alternative code
   paths and their constants are.  The default chosen here matches the
   2013-era shipped client (see the note at the definition site).  Dynamic
   delivery is not ported. */
#pragma once

namespace FFlag {

// Joint::canBuildJoint 0x5784C7.  ON  -> Face::hasOverlap(..., 0.1f)
// (0x3DCCCCCD);  OFF -> Face::hasOverlap(..., 0.35f) (0x3EB33333).
// The flag name says "allow auto joints with small parts", and the loose
// 0.35f overlap is the *old* tolerance, so the 2013 shipped client is taken
// to be running the flag ON (0.1f).
extern bool PhysicsAllowAutoJointsWithSmallParts_DE6056;

// ContactManager::onNewPair 0x56539A.  OFF (0) -> createContact +
// insertContact directly; ON -> the Assembly/Mechanism grounded dance
// (getConstPrimitiveAssembly/computeIsGrounded/getConstMovingAssemblyRoot --
// unported types, see ContactManager.cpp).  Same server-value DELTA as
// above; the 2013 shipped client is taken to be running it OFF (the direct
// path -- the deferred-contacts experiment postdates this build).
extern bool DeferredContacts;

// GuiBase2d::recalculateAbsoluteSizeAndPosition 0x6E4C4C.  ON -> lrintf
// pixel rounding of the incoming rect; OFF -> verbatim.  Rounded output is
// the shipped look (crisp text/borders); same server-value DELTA as above.
extern bool RoundGuiAbsoluteSizeAndPosition;

// GuiObject::mouseIsOver 0x3AD14A.  ON (shipped) -> hover honors the
// clipping-ancestor intersection; OFF -> raw getRect2D hit-test.  Same
// server-value DELTA as above.
extern bool CheckMouseOverWithClipping;

// World::getUiStepId 0x5AA47E (via Constants::worldStepsPerUiStep 0x52701A).
// ON -> 4 world steps per UI step; OFF -> 8.  The 60Hz physics mode
// postdates the October 2013 build; shipped default OFF.  Same
// server-value DELTA as above.
extern bool Physics60HZ;

// KernelData::addBodyToNewList.  OFF -> joint-list routing (freeFall
// fast path disabled); BSS-zero shipped defaults.  Same server-value
// DELTA as above.
extern bool FreeFallOptimization;
extern bool ImpulseSolverForContact;
extern bool SkipBroadPhaseWhenThrottling;
// BlockBlockContact find/load FFlag variants (0x55EE0E etc., batch308).
// BSS-zero shipped default (OFF -> legacy paths); server-value DELTA
// as above.
extern bool BlockBlockNarrowPhaseRefactor;
// ScriptContext::getGlobalState VM segregation (0x18B4C2). BSS-zero
// shipped default (OFF -> shared globalStates[0]); server-value DELTA.
extern bool SegregatedLuaEnvironments;
// ScriptContext::getGlobalState(thread) debugger arm (0x190A9E).
// BSS-zero default (OFF -> identity delegate); server-value DELTA.
extern bool LuaDebugger;
// openState crash__ global (0x18BDEE, batch320; gated registration).
// BSS-zero default (OFF -> unregistered); server-value DELTA.
extern bool DebugCrashEnabled;
// ThreadRef function/thread registry routing (ThreadRef.cpp 0x1AEB2A/
// mac: 0x1AF68C/0x1AF4FA/0x1AED8E; batch448/451). BSS-zero default (OFF ->
// refs live in the calling state, not the global state); server-value
// DELTA as above.
extern bool AllowYieldInHooks;
// LuaInstanceBridge::on_index capitalized retry (0x1741F7; rodata
// "CamelCaseHackEnabled" 0xB2A6A5, batch533). BSS-zero default (OFF ->
// exact-name lookup only); the retry uppercases the first letter and
// re-runs on_index, firing camelCaseViolation. Server-value DELTA.
extern bool CamelCaseHackEnabled;
// setAndLockParent silent variant (Instance.cpp 0x5450AC TU-local;
// BSS-zero default preserves the throw path). Test-visible here.
extern bool NoThrowOnReparenting;
// ContentProvider::privateLoadContent 0x60B053 reads this (mangled
// __ZN5FFlag23ValidateAssetUrlEnabledE).  ON -> the http-asset-URL gate
// runs (isValidRobloxAssetUrl 0x60A38A rejects unknown-host /asset/ urls
// and path-traversal "asset/.." with a "blocking %s" warning); OFF -> the
// gate is skipped entirely.  Server-value DELTA as above; default ON (the
// validation shipped with this build -- the blocking log string exists at
// 0xB7730C).
extern bool ValidateAssetUrlEnabled;
// YieldFunctionStateObject::onReturnResult 0x1797F0 / onRaiseException
// mac: 0x179B84 read this (mangled namespace RBX::Lua::FFlag in the binary; the
// port keeps the single global FFlag namespace -- namespace-only DELTA).
// ON -> a synchronous result/exception on the executing thread is taken
// immediately (immediateResults / the throw-unwind back into execute());
// OFF -> the callbacks always schedule the deferred resume.  BSS-zero
// shipped default (OFF); server-value DELTA as above.
extern bool ImmediateYieldResultsEnabled;

// Lua profiler hook gate (include/script/LuaProfiler.h; the hook surfaces
// mac: 0x1979D0-0x1A8C3C queue behind the profiler era -- see that header).
// BSS-zero default (OFF -> hook dispatch routes straight through the
// LuaProfiler shell untouched); server-value DELTA as above.
extern bool LuaProfilerHooks;

// isHeardLocally 0x244B9A reads this (mangled __ZN5FFlag17AreSoundsFilteredE).
// ON  -> the sound audibility walk (SoundService/Workspace/CoreGuiService/
//        local-player gate) runs; OFF -> every sound is heard locally.
// BSS-zero default would be OFF, but the 2013 shipped client audibly filtered
// sounds, so the port defaults ON (server-value DELTA as above).
extern bool AreSoundsFiltered;

// DE4528 (SoundChannel flags / preloadSound gate).  Default OFF.
extern bool DE4528Fixed;

// ChatOutput::onPlayerChatMessage emote-filter gate.  Read at 0x5C2AA8
// (`cmp byte ptr [picbase+0x9E6AA4], 0` -> __ZN5FFlag15FilterEmoteChatE,
// 0xFA9548): ON -> chat text whose walk matches the "/e " 3-char prefix
// (literal at 0xB72841, decompile-confirmed) or the "/emote " 7-char
// prefix (literal at 0xB72845, raw byte window '...tify_bkg\0/e \0/emote
// \0ChatBubbleT' resolves both EAs; anchor = the pop-eax at 0x5C2AA4) is
// dropped from the player chat output.  The registration arm calls
// FLog::RegisterFlag("FilterEmoteChat", &flag, 0, 0); the zero initial
// is taken as the shipped default (registration machinery unported --
// server-value DELTA as above).
extern bool FilterEmoteChat;

// ProfanityFilter::ContainsProfanity gate (read at 0x5D2104 head and by
// ChatOutput::onGameChatMessage via off_EB8418 -> __ZN5FFlag18-
// LocalTextFilteringE, 0xF88527): ON -> the game chat text is passed to
// ContainsProfanityWorker for the blocklist walk; a hit drops the ChatLine
// construction before any allocation.  The raw flag storage byte at
// 0xF88527 is 0x01 -- the shipped default is UNLIKE FilterEmoteChat
// (which reads 0x00): LocalTextFiltering defaults TRUE (server-value
// DELTA as with the other flags: landed = the raw init byte).
extern bool LocalTextFiltering;

// ChatOutput::render2d_classicStyle gate (read at 0x5C427B-84: the
// off_EB5C98 pointer-slot deref -> the flag byte; the storage mangles as
// __ZN5FFlag20NativeChatRenderingE).  ON -> the classic "PlayerName; text"
// row list renders (the 0x5C426C painter cluster); OFF -> the renderer
// exits before its get2DExtents pass.  Default = the raw flag byte (see
// FastFlags.cpp).
extern bool NativeChatRendering;

} // namespace FFlag

namespace DFFlag {

extern bool PromoteAssemblyModifications;
// throwable_lua_tostring guard (0x165EEA): throw past 200000 chars.
// BSS-zero default (OFF -> no length check); server-value DELTA.
extern bool ShutdownScriptsThatGenerateSuperLongStrings;
// ScriptContext::reportError 0x1960D1: ON -> fire the DataModel void()
// lua-exception signal (DM+0xAEC).  Symbol evidence (batch718):
// __ZN6DFFlag25LuaExceptionSignalEnabledE 0xFA6418 (+ ...Initer 0xFA641C).
// BSS-zero default (OFF); server-value DELTA as above.
extern bool LuaExceptionSignalEnabled;

} // namespace DFFlag

namespace DFInt {

// ScriptContext::reportError reads both (batch716).  Symbol evidence
// (batch718): __ZN5DFInt23LuaExceptionPlaceFilterE 0xFA6408
// (+ ...Initer 0xFA640C) and __ZN5DFInt32LuaExceptionThrottlingPercentageE
// 0xFA6410 (+ ...Initer 0xFA6414).  Same server-value DELTA as
// FFlag/DFFlag (BSS-zero here; delivered at run time in the binary).
// LuaExceptionPlaceFilter: nonzero -> reportError captures and reports
// the call stack only when it equals the DataModel placeID (dm+0xD80).
extern int LuaExceptionPlaceFilter;
// LuaExceptionThrottlingPercentage: handed to
// StatsService::report("LuaExceptions", entry, throttle) 0x196093.
extern int LuaExceptionThrottlingPercentage;

} // namespace DFInt

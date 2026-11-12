/* App/util/FastFlags.cpp — October 2013 (static fast-flag storage).

   See include/Util/FastFlags.h for the symbol evidence and for the
   documented DELTA on the *values* (server-delivered at run time in the
   binary; fixed defaults here). */

#include "stdafx.h"

#include "Util/FastFlags.h"

namespace FFlag {

// mac: 0x5784C7 reads this.  Default true -> Joint::canBuildJoint uses the
// 0.1f small-part tolerance (0x3DCCCCCD) rather than 0.35f (0x3EB33333).
bool PhysicsAllowAutoJointsWithSmallParts_DE6056 = true;

// mac: 0x56539A reads this.  Default false -> onNewPair creates + inserts the
// contact directly (the 2013 shipped behavior).
bool DeferredContacts = false;

// mac: 0x6E4C4C reads this.  Default true -> pixel-rounded absolute layout.
bool RoundGuiAbsoluteSizeAndPosition = true;

// mac: 0x3AD14A reads this.  Default true -> hover honors clipping (the
// shipped behavior: clipped-away widgets don't hover).
bool CheckMouseOverWithClipping = true;

// mac: 0x52701A reads this.  Default false -> 8 world steps per UI step (the
// 2013 shipped behavior; 60Hz mode postdates this build).
bool Physics60HZ = false;

// KernelData::addBodyToNewList reads these.  Defaults false (BSS-zero
// in the binary image; server-delivered at run time there).
bool FreeFallOptimization = false;
bool ImpulseSolverForContact = false;
bool SkipBroadPhaseWhenThrottling = false;
bool BlockBlockNarrowPhaseRefactor = false;
bool SegregatedLuaEnvironments = false;
bool LuaDebugger = false;
bool DebugCrashEnabled = false;
bool AllowYieldInHooks = false;
bool CamelCaseHackEnabled = false;

// mac: 0x60B053 reads this.  Default true -> isValidRobloxAssetUrl gate runs
// (the 2013 shipped client carries the url-validation hardening).
bool ValidateAssetUrlEnabled = true;

// YieldFunctionStateObject callbacks read this.  Default false (BSS-zero)
// -> the callbacks always schedule the deferred resume (see FastFlags.h).
bool ImmediateYieldResultsEnabled = false;

// Lua profiler hook gate (include/script/LuaProfiler.h -- hook surfaces
// mac: 0x1979D0-0x1A8C3C queue behind the profiler era). BSS-zero default (OFF).
bool LuaProfilerHooks = false;

// isHeardLocally 0x244B9A: shipped client filtered sounds locally.
bool AreSoundsFiltered = true;

// DE4528Fixed: sound-channel preload gate.  Default OFF (BSS-zero).
bool DE4528Fixed = false;

// FilterEmoteChat: ChatOutput::onPlayerChatMessage 0x5C2AA8 gate (see
// FastFlags.h).  Default OFF (the RegisterFlag(..., 0, 0) initial) ->
// "/e ..." emote lines still reach the chat output.
bool FilterEmoteChat = false;

// LocalTextFiltering: ProfanityFilter::ContainsProfanity 0x5D2104 head
// gate + ChatOutput::onGameChatMessage 0x5C2DC6-area gate (the byte
// pointed to by off_EB8418 -= FFlag::LocalTextFiltering @0xF88527).
// Landed named from the mangled symbol __ZN5FFlag18LocalTextFilteringE.
// Default TRUE -- the raw flag data byte at 0xF88527 is 0x01 (unlike
// FilterEmoteChat 0xFA9548 which reads 0x00).
bool LocalTextFiltering = true;

// NativeChatRendering: ChatOutput::render2d_classicStyle 0x5C427B-84
// gate (the off_EB5C98 pointer slot -> 0xFA9A58; landed named from the
// rarc30 probe -- the __ZN5FFlag20NativeChatRenderingE symbol register
// is stripped, so the slot deref is the identity evidence).  Default
// TRUE -- the raw flag data byte at 0xFA9A58 is 0xFF: the shipped
// classic "PlayerName; text" list renders.
bool NativeChatRendering = true;

} // namespace FFlag

namespace DFFlag {

// (PromoteAssemblyModifications is defined in util/DFFlag.cpp -- the
// DFlag set's home TU -- so the two flag objects stay link-disjoint;
// the same extern in FastFlags.h resolves from DFFlag.obj.)
// throwable_lua_tostring guard (0x165EEA, section 258c).
bool ShutdownScriptsThatGenerateSuperLongStrings = false;
// ScriptContext::reportError 0x1960D1 reads this.  Default false
// (BSS-zero) -> the DataModel lua-exception signal fire is dead.
bool LuaExceptionSignalEnabled = false;

} // namespace DFFlag

namespace DFInt {

// ScriptContext::reportError 0x195D9A/0x196093 read these.  Defaults 0
// (BSS-zero in the binary image; server-delivered at run time there):
// with the filter off, reportError never captures/reports a call stack.
int LuaExceptionPlaceFilter = 0;
int LuaExceptionThrottlingPercentage = 0;

} // namespace DFInt

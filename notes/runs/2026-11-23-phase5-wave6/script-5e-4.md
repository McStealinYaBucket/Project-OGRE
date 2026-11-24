# script-5e-4 (Phase 5e, fourth pass: ScriptContext and the typed Lua extra space)

Builds in `build-scr/` (PLAYER) and `build-scr-studio/` (STUDIO). Figures come from a private copy of the
reccmp project (`build-scr/w6/root/`: symlinks plus a real `reccmp/`) whose CSVs were regenerated from two
reccmp runs of the scored link (`build-scr/w6/full.sh`). Scored exe/pdb/map copies: `build-scr/w6/snap/<name>/`;
runs: `build-scr/w6/runs/`. The shared `reccmp/` was never written. The tree carried other agents' wave-6 edits
while I built (settings/GlobalSettings.h, World/SleepStage, Kernel, serialization, Profiling), so whole-build
totals include their work. Two transient LNK2005s from those edits (GameSettings ctor, ~SpanningNode) were
linked through with `LINK=/FORCE:MULTIPLE` in my build dir only; they were gone by the final link.

## Row 5e

`tools/row_coverage.py` now has a `5e` row (all 17 `Client/App/script/*.cpp`) and FIXES ScriptContext.obj at
0x0066fa60-0x0068bd50 (its `__FILE__` strings run 0x0066fca0..0x0068bbd0; the 2013 template runs split the
markers into clusters more than 0x2000 apart, so `densest()` took only the 0x00682220 one).

| | functions at 100% | ScriptContext.obj | ThreadRef.obj |
|---|---|---|---|
| Before (sweep12 `build/sweep12/P-final.json`, corrected range) | **454 / 1335 = 34.0%** | 167 / 641 | 13 / 61 |
| After (`build-scr/w6/runs/s3p.json`) | **474 / 1335 = 35.5%** | 180 / 641 | 19 / 61 |

(The old 0x00682220 range gave 310/848 for sweep12.) CSVs: `row-5e-before.csv`, `row-5e-after.csv`.
The lapi.obj/lstate.obj extra-space functions below are Lua TUs, outside the row.

Whole build, private CSVs: PLAYER **9,879** entries at effective 100% (sweep12: 9,721; others' work included).

## Functions raised to 100% (PLAYER, s3p)

| Function | PLAYER | Before | Marker |
|---|---|---|---|
| `RobloxExtraSpace::RobloxExtraSpace(RobloxExtraSpace*)` | 0x00874f40 | 0 (stub) | FUNCTION (luaconf.h) |
| `RobloxExtraSpace::~RobloxExtraSpace` | 0x008766a0 | 2.1 (stub) | FUNCTION (luaconf.h) |
| `RobloxExtraSpace::RobloxExtraSpace()` | 0x00876970 | unpaired | FUNCTION (luaconf.h) |
| `RobloxExtraSpace::constructChild` | 0x00875050 | 100 (CSV) | FUNCTION (luaconf.h) |
| `Set<RobloxExtraSpace>::Hook::remove` | 0x00874cf0 | unpaired | CSV pair |
| `Set<RobloxExtraSpace>::insert` | 0x00874d80 | unpaired | CSV pair |
| `Set<RobloxExtraSpace>::~Set` | 0x00876580 | unpaired | CSV pair |
| `shared_ptr<RobloxExtraSpace::Shared>(Shared*)` | 0x00876910 | unpaired | CSV pair |
| `sp_counted_impl_p<Shared>::dispose` | 0x00876680 | unpaired | CSV pair |
| `Set<RobloxExtraSpace>::Hook::is_linked` | 0x0066ff00 | 94.4 | CSV pair |
| `Set<RobloxExtraSpace>::Iterator::Iterator` | 0x0066ff70 | 93.1 | CSV pair |
| `Set<RobloxExtraSpace>::Iterator::operator->` | 0x006714d0 | 92.0 | CSV pair |
| `Set<RobloxExtraSpace>::Iterator::operator++` | 0x00671570 | 91.8 | CSV pair |
| `ScriptContext::onCheckTimeout` | 0x0067a1e0 | 98.2 | FUNCTION (new) |
| `ScriptContext::ypcall` | 0x006878c0 | 58.2 | FUNCTION |
| `ScriptContext::onHeartbeat` | 0x00689f00 | 85.5 | FUNCTION |
| `ScriptContext::printCallStack` (others' DebugSettings layout) | 0x0067abc0 | 99.0 | FUNCTION* |
| `WeakThreadRef::addToNode` | 0x00871200 | 54.8 | FUNCTION |
| `WeakThreadRef::removeFromNode` | 0x00871260 | 66.7 | FUNCTION |
| `WeakThreadRef::WeakThreadRef(const WeakThreadRef&)` | 0x00871b90 | unpaired | FUNCTION (new) |
| `detail::LiveThreadRef::LiveThreadRef(lua_State*)` | 0x008713a0 | 61.4 | FUNCTION |
| `WeakFunctionRef::~WeakFunctionRef` | 0x00871e90 | 46.2 | FUNCTION |
| `WeakFunctionRef::removeRef` | 0x00871f30 | 55.6 | FUNCTION |
| `YieldingThreads::waiterCount` (paired once closeStates called it) | 0x008a8fc0 | - | CSV pair |

\* printCallStack reached 100% from another agent's DebugSettings layout (+0x64 byte); PLAYER marker flipped.

STUDIO markers were added (all `STUB`) but not measured: the STUDIO build was still compiling at report time.

Raised, not to 100%: closeStates (no 2013 body, now a 2013 reconstruction) 96.3 (s1 order of `std::swap`; the
other order measured 93.4 and is reverted), LibraryService ctor 98.1 (unchanged; see blockers).

The 514 stock Lua functions: lua_newthread, lua_newstate, lua_close and luaE_freethread dipped to 96-99% in
s1 only because the generated link stubs still owned 0x00874f40/0x008766a0; after `gen_link_stubs.py PLAYER`
dropped those two stubs they are 100% again (s2, s3). No FUNCTION marker in my files lost its 100% against
sweep12 (s3p vs `build/sweep12/P-final.json`, read one by one).

## What changed

1. **row_coverage.py**: row `5e`, ScriptContext.obj FIXED range, docstring.
2. **onCheckTimeout / onHeartbeat**: `timedoutCount` is a `volatile long` incremented/decremented through
   `RBX::RbxInterlockedIncrement/Decrement` (0x0067a308, 0x00689f89); `timedout` stays `rbx::atomic<int>`
   (swap through RbxInterlockedExchange). onHeartbeat's last log is "Script context heartbeat finish".
   The ctor, clampWaitTimeout and openState now call the settings agent's
   `GlobalAdvancedSettingsItem<LuaSettings>::singleton()` (0x00654600, in their GlobalSettings.h) through the
   unchanged `LuaSettings::singleton()` spelling; the singleton itself is at 100% in s2/s3 (their work).
3. **The typed RobloxExtraSpace in lapi.obj/lstate.obj**: no stand-ins were needed. luaconf.h's opaque view is
   now limited to the Lua TUs other than `lapi_c`/`lstate_c`; those two see the typed class and the inline
   bodies, which sit at luaconf.h 831-863 so the asserts land on **852** (`node`) and **862**
   (`shared->threadCount >= 0`). They include the real `RBX/Intrusive/Set.h` (spelled `RBX/`, as the 2013
   `__FILE__` records it; this also fixed the four Set helpers in ScriptContext.obj), `security/SecurityContext.h`,
   `script/ThreadRef.h` and `script/ExitHandlers.h` (Continuations must be complete for the inline dtor).
   CMakeLists.txt: lapi.c and lstate.c drop the `Lua-5.1.4/standin` include dir (the real `RBX/Debug.h` needs
   Log's `FastLog.h`, which the stand-in would shadow); neither TU includes a stand-in itself. Shared gets
   `Shared() : threadCount(0), context(0)`. `staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch` is
   regenerated; patches 01-15 on upstream reproduce the installed `Client/App/Lua-5.1.4/src` byte for byte.
4. **closeStates 0x00685060** (new 2013 body, STUB P/S 0x007ea9c0): try/catch(`RBX::base_exception`) around the
   logs, `scripts.clear()` and `yieldEvent.reset()`, "Exception thrown while cleaning up Lua: %s" +
   `RBXASSERT(false)` on line **891**; `removeBlocking` + reset for gcJob/waitingScriptsJob; `waitingThreads.clear()`;
   `!FFlag::LuaDebugger` swaps the states out with `std::swap` (VC9's self-check is the `cmp eax,esi` at
   0x0068541e) and closes them, else closes in place; heap stats tail. `globalStates` is now
   `boost::array<lua_State*, 2>` (typedef `GlobalStates`) so `std::swap` works; every other use is `[i]`.
5. **ypcall**: parameter named `thread` (assert text `top == lua_gettop(thread)`), child goes straight into a
   WeakThreadRef, `context` read once before `resume`, case 1 builds a local `Lua::Continuations` (the 0xb0
   frame) whose handlers bind the **caller's** thread (`WeakThreadRef(thread)`, `_1`), then
   `continuations.reset(new Continuations(local))`; `default:` sits before `case 1:` (EH states 6 then 7/8).
6. **ThreadRef.cpp**: addToNode/removeFromNode re-read `node` after the log; the copy ctor stores neither
   previous nor next; LiveThreadRef tests the flag first and works on the member `this->L`; WeakFunctionRef's
   dtor/removeRef test `functionId && !empty()` and read `functionId` after `getGlobalState` returns.
   Asserts stay on 112/419 (`tools/assert_lines.py --apply`). ScriptContext.cpp's 34 placed asserts stay on
   their 2013 lines, now including closeStates' 891.
7. **Generated stubs**: `tools/gen_link_stubs.py --build build-scr PLAYER` dropped the two now-defined
   RobloxExtraSpace stubs from `Client/_generated/player` (nothing else changed).

## What blocks the rest

1. **startScript's assert at 2550** (`scripts.find(script.get())`): 2013's `script` is a `shared_ptr`
   (`shared_ptr<BaseScript> script = start.script`); ours is a raw pointer used throughout the 3,000-byte body.
   Needs the body rewritten around the shared_ptr, not attempted.
2. **closeStates 96.3**: the zero for the swapped-out pair is one register copied twice in 2013
   (`xor eax,eax; mov ecx,eax; mov edx,eax`); neither `std::swap` order reproduces it. Also data pairing of
   `FFlag::LuaDebugger` (0x00e12ec8 assumed; FastFlags.cpp is not mine) and the unpaired
   `safe_queue<WaitingThread>::clear`/`LuaAllocator::getHeapStats`/`clearHeapMax` callees.
3. **LibraryService ctor 98.1**: the only diff is the end pointer of boost's prime list
   (`prime_list_template<unsigned>::value + 0x98` = 0x00bdb080), which reccmp reads as a string in the
   original and as an unrelated string in ours. Data-row issue, not code.
4. **delay 97.1**: the clamp's branch polarity (2013 falls through to the store when `floor > timeout`).
5. **ScriptContext ctor 97.6**: register choice in the `onCamelCaseViolation` bind setup.
6. Unnamed ScriptContext functions (202, `PLAYER-app-unnamed-by-class.csv`): the ones near 100% that I read
   (0x00674ec0, 0x0067f850, GcJob/WaitingScriptsJob::stepDataModelJob, deque `_Growmap`,
   `shared_from_dynamic_cast<DataModel>`) differ only by unpaired callees (`WeakThreadRef` copy ctor before
   this pass, the folded `weak_ptr::lock` twins), so they are pairing work for the CSV generator.

## Unresolved (for the integrator and other owners)

- **STUDIO**: not measured this pass. Run `tools/gen_link_stubs.py --build <studio build> STUDIO` after a STUDIO
  link: `Client/_generated/studio` still holds stubs with `STUB` markers on 0x00924c30 and 0x00926390, which
  luaconf.h now also marks (the symbols are defined, so the stubs only duplicate the markers). Then flip
  luaconf.h's STUDIO markers (0x00924d40, 0x00926660, 0x00924c30, 0x00926390) and the STUDIO twins of the
  ThreadRef/ScriptContext functions above if they measure 100%. STUDIO addresses for the new markers:
  closeStates 0x007ea9c0, WeakThreadRef copy ctor 0x00828010.
- `ScriptContext::printCallStack` PLAYER 0x0067abc0 is FUNCTION now and depends on the DebugSettings layout
  another agent changed this wave (byte at +0x64); keep that change.
- **Lost entries in s3p vs sweep12 that are not mine** (17): ten `isNullClassName`/`getClassName`/
  `static_getCreator` for Spawner/StarterGui/StarterPack/PersonalServer/Social/Joints/Physics/Script/Marketplace
  services and Lighting now carry `...\include\util/object.h` where 2013 has `Util/Object.h` (the kept COMDAT
  comes from a TU that spells it lowercase; lapi.obj/lstate.obj carry no Object.h code, checked);
  `Described<GlobalAdvancedSettings/LuaSettings>::classDescriptor`, `CodeProfiler` vftable,
  `array_constructor`, `_Allocate<ChatMode>` (settings/profiling owners).
- `FFlag::AllowYieldInHooks` is defined in ThreadRef.obj in 2013 (registration 0x00a81db0, variable
  0x00e12d44); ours is in util/FastFlags.cpp, so the data does not pair (it does not cost the functions above).
- Edited outside the listed directories: CMakeLists.txt (the lapi.c/lstate.c include override, 5 lines),
  `staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch`, `Client/_generated/player/` (tool output).
- decomplint: ScriptContext.cpp and ThreadRef.cpp keep their pinned descents (2013 assert-line order is not
  address order); the new copy-ctor marker adds one in ThreadRef.cpp.

## Files changed

- tools/row_coverage.py
- Client/App/Lua-5.1.4/src/luaconf.h, staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch, CMakeLists.txt
- Client/App/include/V8DataModel/ScriptContext.h (timedoutCount, GlobalStates, boost/array.hpp)
- Client/App/script/ScriptContext.cpp, Client/App/script/ThreadRef.cpp
- Client/_generated/player/stubs_00.cpp, unresolved.txt (gen_link_stubs output)
- notes/runs/2026-11-23-phase5-wave6/script-5e-4.md, row-5e-before.csv, row-5e-after.csv
- Private: build-scr/w6/ (root, snapshots s1-s3, runs, full/score/snap/v/cmp scripts), build-scr-studio/

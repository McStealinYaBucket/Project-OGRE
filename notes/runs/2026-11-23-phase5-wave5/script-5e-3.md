# script-5e-3 (Phase 5e, third pass: the 2013 Lua extra space and script)

Builds in `build-scr/` (PLAYER) and `build-scr-studio/` (STUDIO). Every figure comes from a private
copy of the reccmp project (`build-scr/w5/root/`: symlinks plus a real `reccmp/`, the shared
generator copied in) whose CSVs were regenerated from two reccmp runs of the scored link. Each
scored exe/pdb/map is kept under `build-scr/w5/snap/<name>/`. The shared `reccmp/` was never
written. The tree held other agents' wave-5 edits while I built (Base `atomic.h`/`threadsafe.h`,
the TBB 2.1 headers, LuaSettings/GlobalAdvancedSettings, Instance.h, World.h), so whole-build
totals include their work.

## Row 5e

`tools/row_coverage.py --files Client/App/script/*.cpp`, PLAYER:

| | functions at 100% |
|---|---|
| Before (sweep11, `build/sweep11/P-final.json`) | **297 / 848 = 35.0%** |
| After (`build-scr/w5/runs/s4p.json`) | **306 / 848 = 36.1%** |

ThreadRef.cpp 7 → 13, Script.cpp 8 → 9, CoreScript.cpp 11 → 12 (both from others' header work).
The row tool counts ScriptContext.obj as 0x00682220–0x0068bd50 only, so four of this pass's
ScriptContext gains (getGlobalState, closeState, setThreadIdentity, spawn, all below 0x00682220)
are real 100% matches that the row does not count. CSVs: `row-5e-before.csv`, `row-5e-after.csv`.

Whole build, private CSVs (others' work included): PLAYER 9,604 entries at effective 100%
(sweep11: 9,405). STUDIO: see below.

## 1. The 2013 RobloxExtraSpace (luaconf.h)

`Client/App/Lua-5.1.4/src/luaconf.h` now has two views of one 44-byte layout:

- The Lua core (`LUA_CORE`/`LUA_LIB`, every Lua `.c` defines one) keeps the opaque members, so its
  TUs still build against the stand-in headers. The Lua rows are unchanged: 513 of the 514
  `player-lua.csv` rows are at effective 100% in both sweep11 and s4p. The other one is the same in
  both: the FFlag initializer `??__ELuaEnableBytecodeIniter` 0x00a82e60 at 87.5%.
- Every other TU gets the typed 2013 class: `Intrusive::Set<RobloxExtraSpace>::Hook` (0),
  `const shared_ptr<Shared>` (0xc; Shared = threadCount 0, context 4, allThreads 8),
  `intrusive_ptr<WeakThreadRef::Node>` (0x14), `Security::Identities identity:5` (0x18; signed,
  the copy in 0x00874f40 is `shl/sar 0x1b`, so an enum bit-field), `bool yieldCaptured:1` (0x1c),
  `weak_ptr<BaseScript>` (0x20), `scoped_ptr<Lua::Continuations>` (0x28), plus `context()`,
  `getThreadCount()`, `setContext()`, `getNode()` inline and `createNewNode()` /
  `eraseRefsFromAllNodes()` declared.
- `createNewNode` and `eraseRefsFromAllNodes` are inline in 2013 but never inlined: Node::create
  and closeState call them out of line (0x00872340, 0x00672d80). They are defined beside their
  only user (ThreadRef.cpp, ScriptContext.cpp) with their markers, so a marker flip never
  rebuilds all of App.
- `staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch` is regenerated so that
  `tools/lib/install_libs.py lua` reproduces the installed luaconf.h byte for byte (checked by
  applying 01–15 to upstream in a scratch tree).
- CMakeLists.txt: the `INCLUDE_DIRECTORIES "${OGRE_PRIOR_LUA_FALLBACK}"` override on
  ScriptContext.cpp, ThreadRef.cpp and LuaInstanceBridge.cpp is gone. All three compile against
  the 2013 Lua headers: L-0x2c, `LUA_IDSIZE 60`, the 2013 lobject.h.

Code that read the extra space by raw offset now uses the members: the WeakThreadRef ctor
(`node(0)` in the init list, `node = RobloxExtraSpace::get(thread)->getNode()` under the lock,
0x008723d0), Node::create (returns the raw `Node*` after `createNewNode`, 0x00872480; header
changed), and the two `*(L-20) = NULL` stores in ThreadRef.cpp, which 2013 does not have.

The constructors and destructor themselves (0x00874f40 child ctor with the luaconf.h:852 assert,
0x008766a0 dtor with :862, the root ctor 0x00876970, constructChild 0x00875050, Hook::remove
0x00874cf0, Set::insert 0x00874d80) live in lstate.obj. They need the Lua TUs to see the typed
class, which needs Base/App headers in the Lua TUs; left for a later pass (see Blockers).

## 2. getSingleton<Lua::WeakFunctionRef>

Moved from `v8tree/EnumProperty.cpp` to `script/ThreadRef.cpp` with the 2013 name "Function"
(push 0x00b4ab90). PLAYER 0x00871a40 95.7 → 100. ThreadRef.cpp's two asserts stay on 112/419.

## 3. Assert-line layout

**LuaLibrary.cpp:** all 13 asserts on their 2013 lines (getApi 44/49/50, on_index 96/101/102,
saveLibraryResult 146/171, find 194, push 239, resumeThread 315, resumeAllThreadsWithException
365, contentReady 420). `RBXASSERT_LINE`, `kLuaLibraryPath` and the two Mac-path gates
(`libraryResultsGate`, `libraryResponseGate`, `_snprintf`) are gone; plain `RBXASSERT`. The
365 assert (`libraryExceptionMap.find(libraryName) != ...`, 0x0088e620) was missing and is added,
with the parameter renamed to the 2013 `libraryName`. The file is in 2013 line order, which is not
address order (VC9 emits getApi/on_index after find/resumeThread), so decomplint reports 7 pinned
descents (was 1).

**ScriptContext.cpp:** 33 of 35 2013 asserts on their lines, laid out with a unit reorder (my
scratch script) plus `tools/assert_lines.py --apply`. The 2013 order the asserts give is: LuaProfiler
(321/356), hook (506), getGlobalState (552/564), openState (613), closeStates (891), closeState
(960), setThreadIdentity (1231), Continuations handlers (1373/1381), executeInNewThread
(1476/1490/1517), spawn (1606), delay (1651), ypcall (1725/1747/1768), addScript (2256),
removeScript (2304), reportError (2437), startScript (2549–2634), RuntimeScriptService
(2776–2828). `RBXASSERT_LINE` is gone. The file shrank from 4,179 to about 3,400 lines (blank
padding only). Not placed: closeStates' 891 `false` (our closeStates has no 2013 try/catch yet)
and startScript's 2550 text (`scripts.find(script.get())`: 2013's `script` is a shared_ptr).

Body fixes that came with the layout:

- ypcall is the 2013 `switch` on the resume status (case 0 first: its assert is 1747, the yield
  case's 1768; default throws `std::runtime_error("")`, string 0x00bda1b8).
- getGlobalState(Identities): unsegregated arm first, `globalStates[0]` / `globalStates[vm_index]`
  texts, `vm_index = 0; if (isInRole(...)) vm_index = 1;`.
- closeState: `void`, the 2013 try/catch around the first two dumps and the erase, "Exception thrown
  while erasing Lua refs: %s" + `RBXASSERT(false)` at 960.
- The open-state counter is `static rbx::atomic<int> contextCount` (inline `lock xadd` ++/--:
  openState 0x006884b0, closeState 0x00672f54), not `InterlockedIncrement` imports.
- setThreadIdentity is static (0x006730a6 `ret`), stores through two `get(thread)` calls;
  getThreadIdentity returns 0 for a null thread.
- openState's assert is 2013's `i==1`.
- clampWaitTimeout re-reads `LuaSettings::singleton()` for the store (delay 0x0067a45a).

The WaitingScriptsJob ctor 0x00679f90 the task mentions reads `LuaSettings+0x84`, not
ScriptContext+0x84 (`call GlobalAdvancedSettingsItem<LuaSettings>::singleton; fld [eax+0x84]`). It
is in v8datamodel/DataModelJob.cpp, not this task's files.

## Functions raised to 100% (PLAYER, private CSVs, s4p)

| Function | PLAYER | Before |
|---|---|---|
| `ScriptContext::spawn` | 0x00674740 | 99.2 |
| `ScriptContext::getGlobalState(Identities)` | 0x00670d20 | 25.9 |
| `ScriptContext::closeState` | 0x00672e60 | 24.0 |
| `ScriptContext::setThreadIdentity` | 0x00672f70 | 56.5 |
| `RobloxExtraSpace::eraseRefsFromAllNodes` (new marker) | 0x00672d80 | 100 (unmarked) |
| `RobloxExtraSpace::createNewNode` (new marker) | 0x00872340 | unpaired |
| `WeakThreadRef::WeakThreadRef(lua_State*)` | 0x008723d0 | 50.9 |
| `WeakThreadRef::reset` | 0x00871920 | 53.3 |
| `WeakThreadRef::Node::eraseAllRefs` | 0x00871990 | 48.8 |
| `WeakThreadRef::Node::create` | 0x00872480 | 22.2 |
| `Type::getSingleton<Lua::WeakFunctionRef>` | 0x00871a40 | 95.7 |

The WeakThreadRef ctor/reset/eraseAllRefs also needed the TBB 2.1 spin_mutex another agent
vendored this wave. New GLOBAL markers: `WeakThreadRef::sync` (PLAYER 0x00e12d45, STUDIO
0x00f8d529). New STUB markers: `LibraryService::LibraryService` (PLAYER 0x0088ec90, STUDIO
0x009eb7b0).

Raised, not to 100% (some of these gains come from other agents' LuaSettings and TBB work in the
same build): delay 80.9 → 97.1, printCallStack 73.4 → 99.5, YieldFunctionStateObject::execute
95.5 → 98.5, ScriptContext ctor 90.5 → 97.6, LuaProfiler ctor 91.7 → 96.2, onHeartbeat 78.3 → 85.5,
ypcall 47.6 → 58.2, resumeThread 50.0 → 70.6, releaseScript 65.4 → 72.0, resumeAllThreadsWithException
22.0 → 39.4, contentReady 49.1 → 56.9.

No FUNCTION marker in the owned files lost its 100% (compared marker by marker against sweep11).

## STUDIO

`build-scr-studio/` built from the same sources (snapshot `build-scr/w5/snap/t1`), CSVs
regenerated privately from two runs (`t1a`, `t1b`), scored as `t1p`: **9,394** entries at
effective 100% (sweep11's shared-CSV figure: 9,163; others' work included). The same 11 functions
reach 100% in STUDIO (getGlobalState 0x007d8c80, eraseRefsFromAllNodes 0x007dac50, closeState
0x007dad30, setThreadIdentity 0x007dae40, spawn 0x007dc330, createNewNode 0x00828830,
WeakThreadRef ctor 0x008288c0, reset 0x00827da0, eraseAllRefs 0x00827e10, getSingleton
0x00827ec0, Node::create 0x00828970). One more is STUDIO-only: removeScript 0x007ea160 (89.7 →
100). Its PLAYER twin 0x00684700 is at 97.6 in s5 (89.7 in s4; the gain is from other agents' Instance.h work), and the only difference left is the data pairing of
`FLog::ScriptContextRemove` 0x00de3f7a. That variable has no data row, and LOGVARIABLE has no
GLOBAL-marker convention yet. All 12 are `FUNCTION: STUDIO` now. No STUDIO FUNCTION marker in the
owned files lost its 100% against sweep11 (`build-studio/sweep11/S-final.json`).

## What blocks the rest

1. **The RobloxExtraSpace ctor/dtor in lstate.obj** (0x00874f40, 0x008766a0, 0x00875050,
   0x00876970, 0x00874cf0, 0x00874d80): the Lua TUs would have to see the typed class, so
   Client/App/Lua-5.1.4/standin would need real enough `rbx/Intrusive/Set.h`, boost smart
   pointers, `security/SecurityContext.h` and `script/ThreadRef.h` without changing the 514
   Lua functions. The class would then sit at luaconf.h lines ~790–870 so its asserts land on 852
   and 862.
2. **`__TBB_LockByte` pairing** (3rdparty/tbb-2.1, not mine): the original calls a C++-mangled
   `unsigned char __cdecl __TBB_LockByte(unsigned char &)` (0x00402240); ours is a C name, so with
   the shared CSVs every spin_mutex user shows a red call. The private CSVs pair it.
3. **delay 97.1:** only the block order of the clamp differs (2013 falls through into the
   `timeout < floor` store and jumps to isNanInf otherwise). I tested writing the clamp inline in
   delay instead of calling the static helper (build s5): the score is the same, so I reverted it.
   The remaining guess is a different condition spelling.
4. **printCallStack 99.5:** DebugSettings' byte is at +0x64 in 2013, +0x8c in ours
   (GlobalAdvancedSettings layout, another agent's).
5. **ypcall 58.2:** 2013's frame is 0xb0 against our 0x78; case 1 builds the continuations with
   more named locals. Needs a proper decompile.
6. **closeStates** has 2013's try/catch with `RBXASSERT(false)` at 891 and the job removal blocks;
   ours is the older reconstruction without a marker.
7. **LibraryService ctor** (0x0088ec90): the 2013 ctor builds a list and four maps and a
   typeid-based member at +0x44 onward; ours is a two-member stand-in.
8. From script-5e-2, unchanged: `Instance::fw()` out of line (Instance.h), twin pairing of
   `weak_ptr::lock` 0x0049ccc0 for GcJob/WaitingScriptsJob/ContentReadyHelper.

## Unresolved (for the integrator and other owners)

- `rbx::atomic<int>` ++/-- must stay inline `lock xadd` (openState 0x006884b0, closeState
  0x00672f54); swap goes out of line. Base's current atomic.h already does this; keep it.
- The TBB 2.1 `__TBB_LockByte` declaration should have C++ linkage
  (`?__TBB_LockByte@@YAEAAE@Z`, PLAYER 0x00402240), so the shared CSVs pair it.
- `tools/row_coverage.py`: ScriptContext.obj starts at 0x0066fa60 in PLAYER, not 0x00682220.
- Edited outside the listed directories: `staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch`
  (kept in step with luaconf.h), the ScriptContext/ThreadRef/LuaInstanceBridge lines in
  CMakeLists.txt, and v8tree/EnumProperty.cpp (the singleton removal only).

## Files changed

- Client/App/Lua-5.1.4/src/luaconf.h, staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch
- CMakeLists.txt (the three-file include override only)
- Client/App/include/V8DataModel/ScriptContext.h (closeState void, setThreadIdentity static)
- Client/App/include/script/ThreadRef.h (Node::create returns Node*)
- Client/App/script/ScriptContext.cpp, ThreadRef.cpp, LuaLibrary.cpp
- Client/App/v8tree/EnumProperty.cpp (singleton removed)
- notes/runs/2026-11-23-phase5-wave5/script-5e-3.md, row-5e-before.csv, row-5e-after.csv
- Private: build-scr/w5/ (root with private CSVs, snapshots s1–s5 (PLAYER) and t1 (STUDIO), runs, the score/snap/v scripts),
  build-scr-studio/

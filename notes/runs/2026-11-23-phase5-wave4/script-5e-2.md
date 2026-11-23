# script-5e-2 (Phase 5e, second pass: script)

Builds in `build-scr/` (PLAYER) and `build-scr-studio/` (STUDIO). Each figure comes from a private copy of the reccmp project and its CSVs (`build-scr/w4/root/`, symlinks plus a real `reccmp/` and a patched `gen_reccmp_csv.py`). Before every final run the CSVs were regenerated from two runs of the current link. The shared `reccmp/` was never written. Totals include other wave-4 agents' work that was in the tree at build time (Lighting, RunService, the v8xml files, DataModel).

## Row 5e

`tools/row_coverage.py --files Client/App/script/*.cpp`, PLAYER:

| | functions at 100% |
|---|---|
| Before (sweep10, `build/sweep10/P-final.json`) | **255 / 848 = 30.1%** |
| After (`build-scr/w4/runs/P9.json`) | **296 / 848 = 34.9%** |

Per file: LuaAtomicClasses 145 → 163, LuaArguments 13 → 27, LuaLibrary 14 → 20, ScriptContext 20 → 22, LuaSignal 12 → 13.

About 14 of the 41 come from the `norm_sig` patch below (the `Bridge<T,true>::getValue<Variant>` instantiations in the LuaArguments range). They only count once the integrator applies that patch to `tools/ida/gen_reccmp_csv.py`. The CSVs are `row-5e-before.csv` and `row-5e-after.csv` in this directory.

Whole build, private CSVs (others' work included): PLAYER 9,211 entries at effective 100% (sweep10: 9,029), STUDIO 8,998 (sweep10: 8,826). `reccmp-vtable` PLAYER mismatches: 995 → 988.

## Layouts

**ScriptContext (0x288).** The member block in `V8DataModel/ScriptContext.h` (0x68–0x288, written in wave 2) is now **proven by the destructor**. `~ScriptContext` (PLAYER 0x0068b1a0, STUDIO 0x007f0900) is at 100% in both targets. It destroys every member with a destructor at its 2013 offset, in 2013 order: 0x284, 0x274, 0x26c, 0x264, 0x260, 0x25c, 0x258, 0x254, 0x250, the five RunningAverage blocks, 0x14c, 0x144, 0x13c, 0x118, 0x10c, 0xcc, 0xc8, 0xc0, 0xb0, 0xa0, 0x8c, 0x70. The constructor (0x0068ad20, 90.1%) has the same offsets store by store (`tools/class_layout.py` output: `build-scr/w4/sc-layout.txt`). It cannot reach 100% for two reasons, both outside this task's files (see Blockers): the inline `tbb::spin_mutex` safe_queue at 0xcc, and LuaSettings' layout.

Two base-class facts were wrong in our tree. Both are fixed in the header:

- **2013 ScriptContext is `DescribedCreatable`, not `DescribedNonCreatable`.** The RTTI names `DescribedCreatable<ScriptContext,Instance,sScriptContext,1,0>`, and the destructor stores FactoryProduct's vftables (0x00b3422c…) before `~Instance`. With this fix, the dtor reaches 100%, the AbstractFactoryProduct slot thunk 0x00674a90 goes from 0% to 100%, and PLAYER's `FactoryProduct<ScriptContext>::Creator` vftable 0x00b13858 now exists and matches.
- **The functionality value is 1 (`INTERNAL_LOCAL`), not 2016's `RUNTIME_LOCAL` (17).** The RTTI says so, and `classDescriptor()` 0x0067f453 stores 1 into the attributes. With this fix our symbols carry `$00@`, the same spelling as 2013.

**WeakThreadRef (0x1c).** The offsets match `tools/class_layout.py`: vptr 0, quick_intrusive count 4, previous 0xc, next 0x10, liveThreadRef 0x14, node 0x18, `new 0x1c` at three call sites. It is not proven yet: its ctor and dtor are blocked by `sync` (see Blockers).

## Functions raised to 100%

16 per target, in both PLAYER and STUDIO. Every `FUNCTION:` marker in the owned files (159 PLAYER, 157 STUDIO) was checked as effective in the final runs P9 and PS5, and was re-checked after the reorder (P10, PS6).

| Function | PLAYER | STUDIO | Before | Cause |
|---|---|---|---|---|
| `ScriptContext::~ScriptContext` | 0x0068b1a0 | 0x007f0900 | 99.4 | FactoryProduct base |
| `ScriptContext::scriptShouldRun` | 0x00682810 | 0x007e8660 | 83.6 | `fastDynamicCast<CoreScript>(script) != NULL` only; the null path belongs to the cast |
| `Lua::lua_tofloat` | 0x00881930 | 0x0092ba00 | 70.2 | `==` tests on ±inf, then upstream G3D's inline `isNaN(double)` (`x<0 \|\| x>=0`) passing NaN through |
| `Lua::isCloseVector3` | 0x00884380 | 0x0092e380 | 89.4 | `if (count > 2)` (cmp 2; jle) |
| `Bridge<Vector3>::on_eq`, `Bridge<Vector2>::on_eq`, `Bridge<Color3>::on_eq` | 0x008827e0, 0x00882950, 0x0087fcb0 | 0x0092c8b0, 0x0092ca20, 0x009e4330 | 61–63 | `getObject(L,1) == getObject(L,2)`: G3D `operator==` evaluates arg 2 first |
| `Bridge<Vector3>::on_index` | 0x00884400 | 0x0092e400 | 83.5 | `object.unit()` out of line (0x004908d0), `object.magnitude()` |
| `Bridge<Region3>::on_index` | 0x00883780 | 0x0092d780 | 83.4 | `Vector3Bridge::pushVector3` out of line |
| `Bridge<RbxRay>::on_index` | 0x00883480 | 0x0092d480 | 80.4 | `pushNewObject(L, object.unit())` by value (no EH temp) |
| `Bridge<BrickColor>::on_index` | 0x00885fa0 | 0x0092ffa0 | 83.7 | number/Number and name/Name as separate ifs, which the compiler tail-merges (backward jump at 0x00886027); `lua_pushstring(L, std::string)` out of line |
| `Bridge<CellID>::on_index` | 0x008892b0 | 0x00933180 | 74.9 | `pushVector3(L, getLocation())` |
| `LibraryBridge::push`, `LibraryBridge::find` | 0x00889630, 0x00889d60 | 0x009e5ed0, 0x009e6600 | 58–61 | the hand-written assert helpers (Mac path, `_snprintf`) replaced by `RBXASSERT_LINE` at the 2013 lines |
| `Bridge<Library>::on_index` | 0x0088c6a0 | 0x009e9240 | 62.2 | same; `getApi` is a free function |
| `Lua::getApi` | 0x0088c3c0 | 0x009e8f60 | unpaired | iOS `RBX::Lua::getApi(lua_State*)`: a free function, not a `LibraryBridge` member |

Raised but not to 100%:

| Function | Before → after | What still differs |
|---|---|---|
| spawn | 97.1 → 99.2 | RobloxExtraSpace offset only |
| LuaProfiler ctor | 85.5 → 91.7 | `== "(resume)"` fixed; lua_Debug size remains |
| delay | 79.2 → 80.9 | |
| Vector2int16 on_mul | 69.5 → 87.7 | float lanes; `Vector2int16(Vector2(b) * c)` |
| newCoordinateFrame | 54.8 → 89.5 | it calls lua_tofloat |
| paletteBrickColor | 54.4 → 79.6 | |
| saveLibraryResult | 28.1 → 79.4 | |
| LuaArguments::get | 62.8 → 65.3 | |
| Vector3int16 on_unm | 69.9 → 93.0 | |

Removed from spawn, delay and ypcall: the "defensive" `*(L-20)=NULL` stores, which 2013 does not have. The line count is kept so later asserts stay on their lines. The compares are now `< 1` and `< 2`, as in 2013.

`LuaAtomicClasses.cpp` and `LuaLibrary.cpp` markers were reordered with `tools/reorder_markers.py`. PLAYER descents went 5 → 0 and 2 → 1. Rebuilt and rescored: no loss, +6.

## Losses against sweep10 (all read with --verbose; none caused by these changes)

**PLAYER.** Each loss and its cause:

- `std::_Tree<string,shared_ptr<Script>>::erase` 0x004bc580: twin `_Max` callee pairing.
- 0x006fa600 and MemoryBinder D0 0x006fc970: v8xml, another agent.
- `sBoolValue::EventDescImpl::connectGeneric` 0x0079d8e0: Lighting twin callee.
- SendPhysics 0x008e5fe0 and 0x008e60b0: another agent.
- `Described<ScriptContext>` ctor 0x0067fcd0 dropped to 96% with the shared CSVs and is back at 100% with the patched `norm_sig`. The code is identical. Its callee `classDescriptor` 0x0067f410 pairs only through the signature fix, because `PLAYER-names.csv` carries the old NonFactoryProduct spelling as a `match:` name.

**STUDIO.** Each loss and its cause:

- Client-shell tokenizer 0x00403d80, 0x0048c540 and 0x0048c660.
- GImage::Error 0x00528680, gzip_decompressor 0x00687690, MemoryBinder 0x0074af00 and SendPhysics.
- `Lighting::replaceSky` 0x007d3500: Lighting.cpp was edited by another agent after the obj was compiled.

## Blockers for the rest (none in this task's files)

1. **ScriptContext.cpp, ThreadRef.cpp and LuaInstanceBridge.cpp compile against the 2016-fork Lua headers.** `CMakeLists.txt` sets `INCLUDE_DIRECTORIES "2013 55% App/App/Lua-5.1.4/src"` on these three files, so they get the fork's 40-byte RobloxExtraSpace, where 2013 has 44 bytes, and `LUA_IDSIZE 256`, where 2013 has 60. Tells:
   - Every `RobloxExtraSpace::get(L)` is `L-0x28` against 2013's `L-0x2c`: spawn 0x00674850, `YieldFunctionStateObject::execute` 0x0087c6c2, WeakThreadRef ctor 0x00872448.
   - `yieldCaptured` is `or dword [+0x18],0x20` against `or byte [+0x1c],1` (2013's own bool unit).
   - The LuaProfiler ctor frame is 0x15c against 0x98 (lua_Debug).

   **Fix:** give `Client/App/Lua-5.1.4/src/luaconf.h` the full 2013 class, with typed members at the offsets it already documents: Hook 0, `shared_ptr<Shared>` 0xc, `intrusive_ptr<Node>` 0x14, `identity:5` 0x18, `bool yieldCaptured:1` 0x1c, `weak_ptr<BaseScript>` 0x20, `scoped_ptr<Continuations>` 0x28, plus `context()` and `getThreadCount()`. Put it behind a guard that App TUs define, then drop the source property. This blocks about 40 ScriptContext/ThreadRef functions. Owners: the Lua agent's header, and CMakeLists.txt.
2. **`rbx::spin_mutex` must be `tbb::spin_mutex` (TBB 2.1 semantics).** Client/Base `rbx/threadsafe.h` still has 2016's `atomic<int>` version. Tells:
   - ScriptContext ctor 0x0068ae2e: 2013 inlines the safe_queue ctor, with the deque aux alloc and the lock byte at +0x20.
   - WeakThreadRef ctor 0x00872425: `__TBB_LockByte`, and the unlock stores the returned byte (TBB 2.1 `scoped_lock::my_unlock_value`). TBB 4.4 headers store 0.

   This needs a TBB 2.1 `spin_mutex.h` in 3rdparty. `Body::getPV_Spin_Lock` 0x007554e0 (74.6%) shows the same thing.
3. **`rbx::atomic<int>` exchange/increment go through `RbxInterlockedExchange`/`Increment` out of line** (`onCheckTimeout` 0x0067a2f5). Ours inlines `xchg`. Client/Base `rbx/atomic.h`.
4. **LuaSettings is 0x28 too large.** The ScriptContext ctor reads `+0x78`/`+0x7c` where we read `+0xa0`/`+0xa4`, and delay's `clampWaitTimeout` reads `+0x70` where we read `+0x98`. The cause is in GlobalAdvancedSettings / GlobalAdvancedSettingsItem (V8DataModel, not this task's).
5. **`Instance::fw()` is out of line in 2013** (removeScript 0x00684767). The call between the FLog test and its use forces the double FLog read. Instance.h.
6. **`Type::getSingleton<Lua::WeakFunctionRef>`** is defined in `v8tree/EnumProperty.cpp:331` with the name "WeakFunctionRef". 2013 has it in ThreadRef.obj (0x00871a40, 95.7%) with the name **"Function"** (push at 0x00871a6a). Move it to `script/ThreadRef.cpp`.
7. **Twin pairing** (pairing only, no code fix): `GcJob`/`WaitingScriptsJob::stepDataModelJob` and `LibraryService::ContentReadyHelper` (98.9%) call `weak_ptr::lock` 0x0049ccc0, an unpaired twin of our folded `weak_ptr<PartInstance>::lock`. The Region3/CoordinateFrame `on_eq` (98.4%) call the `GLight::operator==` fold of `Matrix3::operator==`. `distanceVector3` (96.3%) calls `RbxRay::distance`.

## Unresolved (for the integrator and other owners)

- **`tools/ida/gen_reccmp_csv.py` `norm_sig`: apply `script-5e-norm_sig.patch` (this directory), three regexes.**
  - (a) A bool template argument: Itanium `true`/`false`, MSVC `1`/`0`. On its own this adds 30 previously unpaired `Bridge<T,true>` rows, including the 15 `getValue<Variant>` instantiations that are now at 100% in both targets.
  - (b) An enum template argument: Itanium `(Enum)1`, MSVC `1`.
  - (c) A reference non-type template argument: llvm-undname prints `char const*const RBX::sX`.

  With (b) and (c), PLAYER signature pairs rise from 848 to 1,335, because every `Described<T, sT, …, N, M>` and `FactoryProduct` instantiation now pairs. `--selftest` passes.
- The `PLAYER-names.csv` `match:` names for 0x0067fcd0 (and its STUDIO twin) still spell `NonFactoryProduct<Instance,sScriptContext>`, `$0BB@`. They should become the `FactoryProduct<…>`, `$00@` spelling on the next IDA sync.
- Blockers 1–6 above: luaconf.h and CMakeLists.txt; Client/Base `threadsafe.h` and `atomic.h`; GlobalAdvancedSettings; Instance.h; EnumProperty.cpp.
- `LuaLibrary.cpp` and `ScriptContext.cpp` use `RBXASSERT_LINE` as a stopgap. LuaLibrary's 2013 asserts sit at lines 44, 49, 50, 96, 101, 102, 146, 171, 194, 239, 315, 365 and 420. The 315 and 420 helpers (`libraryResultsGate`, `libraryResponseGate`) still have Mac paths and `_snprintf` and need the same treatment.
- Edited outside the listed directories: `Client/App/include/V8DataModel/ScriptContext.h` (ScriptContext's own header; base class and functionality). No other agent's file was touched.

## Files changed

- Client/App/include/V8DataModel/ScriptContext.h
- Client/App/include/script/LuaLibrary.h
- Client/App/script/ScriptContext.cpp
- Client/App/script/LuaAtomicClasses.cpp
- Client/App/script/LuaLibrary.cpp
- notes/runs/2026-11-23-phase5-wave4/script-5e-2.md, row-5e-before.csv, row-5e-after.csv, script-5e-norm_sig.patch
- Private, not shared: build-scr/w4/ (runs, root/ with the patched generator and private CSVs, score scripts), build-scr-studio/

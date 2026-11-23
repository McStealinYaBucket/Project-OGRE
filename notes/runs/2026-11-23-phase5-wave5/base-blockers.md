# Wave 5: shared blockers in Base, Util and the object model

Label: base-blockers. Build dirs: `build-base/` (PLAYER), `build-base-studio/` (STUDIO). Private reccmp
root with regenerated CSVs: `build-base/root/` (symlinks plus its own `reccmp/`), scored from
`build-base/priv/` and `build-base-studio/priv/`. Runs and scored copies: `build-base/runs/`,
`build-base/scored10/` (PLAYER, final), `build-base-studio/scored3/` (STUDIO, final). The shared `reccmp/` was not
written.

## Result

| | sweep11 | now (private CSVs, current link) | lost 100% |
|---|---|---|---|
| PLAYER | 9,405 | **9,712** (P10) | 0 |
| STUDIO | 9,163 | **9,460** (S4) | 0 |

These totals include other agents' concurrent edits in the shared tree (Primitive/World, the Lua
RobloxExtraSpace work in luaconf.h/ThreadRef). The per-fix lists below name only functions whose
diffs I read. Between runs, six Lua functions (symbexec, luaV_execute, constructor, patchtestreg,
removevalues, luaK_setreturns) went in and out of 100%. Each time the cause was a no-.reloc
coincidence: `and eax, 0x7fc000`-style masks equal the address of one of our RTTI COLs. None of
these swings comes from a code change.

## The fixes

### 1. rbx::atomic (Base `rbx/atomic.h`)

Only `swap` goes through `RBX::RbxInterlockedExchange` (0x009b8f20). In 2013, `++`, `--` and
`compare_and_swap` are the `_Interlocked*` intrinsics:
- `intrusive_ptr_add_weak_ref` 0x0040d2d0 shows `mov edx,1; lock xadd`.
- `intrusive_ptr_release` 0x00670490 shows `or edx,-1; lock xadd; jne`.
- `intrusive_ptr_try_lock` 0x006cbce0 shows `lock cmpxchg`.
- `swap` calls the helper in `SimpleThrottlingArbiter::isThrottled` 0x0064dfc0 (`updatingThrottle.swap(1)`) and in `Time::now<Fast>` 0x009bb680.

My first attempt routed every operation through the helpers. That cost about 70 intrusive_ptr and
signal functions (disconnectAll and ~signal: 24.2% and 63.6%), so I narrowed it to swap. The
declarations moved above the class, and the ctor assert stays on line 224.

`ScriptContext::onCheckTimeout` 0x0067a1e0 is now at 98.2% (86.3% before). The swap matches. The
remaining miss is `++timedoutCount`, which 2013 compiles as a call to `RbxInterlockedIncrement`
0x009b8f10. A `rbx::atomic<int>` cannot do that, so the fix belongs to the ScriptContext owner (see Unresolved).

### 2. rbx::spin_mutex = TBB 2.1 tbb::spin_mutex

I vendored the TBB 2.1 headers in `3rdparty/tbb-2.1/`, unmodified, with COPYING, README and
`VENDORED.txt`:
- Release tbb21_20080605oss, interface 3011. This is the exact build in `bin/player/tbb.dll` ("BUILD_DATE Thu, 5 Jun 2008", FileVersion 2.1.2008.0605).
- I fetched it from the Internet Archive capture of threadingbuildingblocks.org.
- The tarball's sha256 is c326c9f7…6927 (`VENDORED.txt` has the full value).

In `CMakeLists.txt`, the tbb-4.4 include root is now `3rdparty/tbb-2.1/include`; it is a two-line
change. In 2.1, `scoped_lock::my_unlock_value` is a `uintptr` holding what `__TBB_LockByte` returned,
and release stores that byte back. `rbx::spin_mutex` is now `class spin_mutex : public tbb::spin_mutex {}`,
because PLAYER's symbols spell `class rbx::spin_mutex`.

Raised to 100%, PLAYER / STUDIO:

| Function | PLAYER | STUDIO |
|---|---|---|
| `__TBB_LockByte` (was 38.2%) | 0x00402240 | 0x00443510 |
| `Body::getPV_Spin_Lock` (was 74.6%) | 0x007554e0 | 0x00894800 |
| `DataModel::GenericJob::GenericJob` (was 50%) | 0x0063ee60 | 0x0075c930 |
| `safe_queue<ScriptContext::WaitingThread>::push` | 0x00678170 | 0x007e0090 |
| `safe_queue<ScriptContext::WaitingThread>::pop_if_present` | 0x006781e0 | 0x007e0100 |
| `safe_queue<ContentProviderTask>::push` | 0x008efd50 | 0x00a0db40 |
| `safe_queue<ContentProviderTask>::pop_if_present` | 0x008efa10 | 0x00a0d800 |
| `ContentProviderJob::ContentProviderJob` | 0x008efdc0 | 0x00a0dbb0 |
| `safe_heap<PriorityTask>::push_heap` | 0x0082c8c0 | 0x00923060 |
| `safe_heap<PriorityTask>::pop_heap_if_present` | 0x0082b5f0 | (not listed) |
| `PriorityThreadPoolData` ctor (was 17.4%) | 0x0082c2f0 | 0x00922b00 |
| `WeakThreadRef(lua_State*)` | 0x008723d0 | 0x008288c0 |

Also raised: the ThreadPool `shared_count<rbx::spin_mutex>` and heap templates, `RBX::join` and
`timed_join`. The WeakThreadRef ctor, `reset`, `Node::eraseAllRefs` and `Node::create` also depend
on the Lua agent's RobloxExtraSpace change.

### 3. TaskScheduler::Job::Stats without timeNow

The Stats ctor writes `timespanSinceLastStep` at +0 and `timespanOfLastStep` at +8 (ret 0xc). Lines
are kept, so the assert at Job.cpp:186 stays in place. Raised to 100%, PLAYER / STUDIO:
- `Job::Stats::Stats`: 0x009b98d0 / 0x00a70890
- `computeStandardError`: 0x009b98b0 / 0x00a70870
- `updateWakeTime`: 0x009b99b0 / 0x00a709d0
- `updateError`: 0x009b9cd0 / 0x00a70cf0
- `PhysicsJob::error`: 0x0072b740 / 0x00799140
- `ContentProviderJob::error`: 0x008ef6f0 / 0x00a0d4f0
- `computeStandardSleepTime`: STUDIO 0x00a70ab0 only; PLAYER 0x009b9a90 is at 99.2%.

`PhysicsJob::stepDataModelJob` 0x0072b880 stays at 97.7%. Its remaining diff is not in Stats.

### 4. Name::declare<sName>() inline (Util/Name.h, Util/Object.h)

- **Name.h** gains `callDoDeclare<sName>` (a call_once target, a `jmp` to doDeclare) and `declare<sName>()`. `declare` tests for NULL (returning `getNullName()`), then calls `boost::call_once(&callDoDeclare<sName>, flag)` (0x0040c2c0), then `doDeclare`.
- **Name::empty()** is `getNullName() == *this`, which compares addresses. The proof is `isNullClassName` 0x00632290, which calls `getNullName` a second time and compares pointers.
- **Object.h**: the Creator `getClassName`/`getClassNameUnconstructed` and `NonFactoryProduct::className` now call `declare<>` (0x004c8040 inlines the same sequence).

Raised to 100% (PLAYER counts; STUDIO is similar):
- about 78 `callDoDeclare<…>` (e.g. 0x0062fc40 `sContentFilter`)
- 34 `doDeclare<…>` rows, including the `Name.h` marker 0x00480e90, flipped to FUNCTION
- 44 `getClassName` (Creator and NonFactoryProduct)
- 15 `isNullClassName`
- 6 `ServiceProvider::create<T>` (0x0063ae40, 0x0063afc0, 0x0063e880, 0x0066de70, 0x00597bc0, …)
- the `std::map<const Name*, const ICreator*>` tree helpers

`ServiceProvider::find<ContentFilter>` 0x00638de0 went from 88.6% to 98.8%; its last diff is a
shared_ptr `operator=` fold. `isNullClassName<sContentFilter>` 0x00632290 went from 63.9% to 95.2%.
What is left there is the include spelling `include\util/object.h` in DataModel.obj (see Unresolved).

### 5. Instance::fw() out of line

- **Instance.h** declares `fw()`. **Instance.cpp** defines it. This was a minimal addition before the size asserts, so Instance.cpp still inlines it, as 2013 does.
- **getName()** now reads through `fw()`. In 2013, 152 of the 194 calls to 0x006beaa0 are followed by `add eax,N`, and removeScript 0x00684767 calls fw before the name.
- **Marker**: the 0x006beaa0 marker was removed. The kept copy is an ICF fold with GuiText getters, and our fw's line records do not survive the fold. The CSV pairing now pairs the address by folded name, and it is at 100%.

Results, PLAYER / STUDIO:

| Function | Before | After |
|---|---|---|
| `DataModel::DataModel` 0x0064ec50 | 97.3% | 99.1% |
| `ScriptContext::removeScript` 0x00684700 | 89.7% | 97.6% (STUDIO 0x007ea160 at 100%) |
| `DataModel::arbiterName` 0x0064de40 / 0x0076a910 | 81.5% | 100% |

(`GuiItem::getTitle` 0x006c8c10 also reached 100%, but it did so before the getName change, so I do not attribute it.)

The DataModel ctor's last diffs are data pairing: an assert string and a function-pointer pairing.

### 6. InstanceHandle, ContentId, ContentProvider

- `InstanceHandle(DescribedBase*)` 0x006fa560 was already out of line, at 100%.
- `InstanceHandle(shared_ptr)` is now out of line (Handle.cpp). Both links fold it with `boost::_bi::storage1<value<shared_ptr<T>>>`'s ctor (PLAYER 0x007a8a20, STUDIO 0x0091f4f0). It has no marker, because a marker took the address away from the storage1 pairing and cost two storage2/list1 callers.
- `ContentId::getName()` returns the `const Name*` at +0x1c. No 2013 symbol names this accessor.
- `ContentProvider::readContent(const char*, std::istream&, unsigned long)` returns `std::string`:
  - PLAYER 0x006676a0 ends `ret 0x10`, with a hidden return pointer.
  - SerializerV2::load 0x006fd2d9 calls `~basic_string` on the temp right after.
  - `unsigned long` is the iOS spelling.
  - The body is still a STUB.

Their callers (serializeNode 0x00628a60, load 0x006fd070) belong to the serialization owner and still use the old code, so neither score has moved yet.

### 7. LuaSettings layout and the 2013 settings class names

- **Base class**: LuaSettings derived from GlobalAdvancedSettings, a ServiceProvider, which is 0x28 bytes larger. It now derives `GlobalAdvancedSettingsItem<LuaSettings, sLuaSettings>` (in `GlobalAdvancedSettings.h`), which is `DescribedCreatable<Class, GlobalAdvancedSettings::Item, sClassName>, Service`. This matches the RTTI spelling `FactoryProduct@VLuaSettings@RBX@@VItem@GlobalAdvancedSettings@2@`.
- **Item ctor** (0x0085e220): sets the name, throws `RBX::runtime_error("singleton %s already exists")` if the slot is taken, then sets the slot. The dtor clears the slot (0x0085e310 stores 0 to 0x00de68a8).
- **Item** is `NonFactoryProduct<Instance, sSettingsItem>`, and `sSettingsItem = NULL` (no "SettingsItem" string exists in PLAYER).
- **Layout**: gcPause +0x64, gcStepMul +0x68, double +0x70, +0x78/+0x7c, byte +0x80, float +0x84, and `DECOMP_SIZE_ASSERT(LuaSettings, 0x88)`.
- **ScriptContext ctor** 0x0068ad20 now reads +0x78/+0x7c (90.5% → 97.6%). It is still not at 100% because 2013 calls `GlobalAdvancedSettingsItem<LuaSettings>::singleton()` 0x00654600 for each read, and ours is a static local.
- **Class names**: `sGlobalAdvancedSettings = "GlobalSettings"` and `sGlobalBasicSettings = "UserSettings"`. In PLAYER, 0x00b31084 points to 0x00b31070 "GlobalSettings" and 0x00b31088 points to 0x00b31060 "UserSettings".

Raised to 100%, PLAYER / STUDIO:
- the FactoryProduct/GlobalAdvancedSettingsItem vftables 0x00b7ced8–0x00b7cf8c (STUDIO 0x00c421f0–0x00c422a4)
- `Creatable<Instance>::create<LuaSettings>`: 0x00654050 / 0x007268f0
- `Described<LuaSettings>::classDescriptor`: 0x00788570 / 0x00735cc0
- `Described<GlobalBasicSettings>::classDescriptor`: 0x00657150

The three FactoryProduct<LuaSettings> functions whose IDA `match:` names still spell the old base
are paired by markers in `LuaSettings.h`, so they stay at 100%:
- `static_getCreator`: 0x0085db70 / 0x009481a0
- `getClassName`: 0x0085dc20 / 0x00948250
- the `??_E` thunk: 0x0085e300 / 0x00948920

`LuaSettings::scalar deleting destructor` (0x0085e310 / 0x00948930) now pairs. It is at 64.5%
because it calls `~Instance`. The item ctor 0x0085e220 is marked STUB: the 2013 copy pushes the
"LuaSettings" literal, and our kept COMDAT, from LuaLibrary.obj, reads `sLuaSettings` from memory.

## Markers flipped

- **Base** `TaskScheduler.Job.cpp`, PLAYER and STUDIO: Stats, computeStandardError, updateWakeTime and updateError. Also computeStandardSleepTime, STUDIO only.
- **Name.h**: doDeclare, PLAYER 0x00480e90.
- **Marker-line flips in other owners' files**, only where my fix took the function to 100%:
  - `RunService.cpp`: PhysicsJob::error, P+S
  - `ContentProviderJob.cpp`: error and the ctor, P+S
  - `Body.cpp`: getPV_Spin_Lock, P+S
  - `DataModel.cpp`: GenericJob ctor, PLAYER (it has no STUDIO marker)

## Checks

- Both targets build.
- `reccmp_addr_padding`: 0 badly padded.
- `check_2016_copies`: silent.
- decomplint: no new out-of-order errors in my files. The existing ones are in TaskScheduler.Job.cpp lines 27–108, Instance.cpp and ContentProvider.cpp.

## Unresolved (for other owners)

- **ScriptContext owner** (`V8DataModel/ScriptContext.h`/`.cpp`):
  - `onCheckTimeout` 0x0067a2f5 increments `timedoutCount` through `RBX::RbxInterlockedIncrement` (an explicit call, or a member type other than `rbx::atomic<int>`, whose `++` is inline in 2013). That is the last diff, at 98.2%.
  - The ctor and `clampWaitTimeout` read LuaSettings through `GlobalAdvancedSettingsItem<LuaSettings,sLuaSettings>::singleton()` 0x00654600, one call per read. This needs that template static, built on `GlobalAdvancedSettings::singleton()` and its mutex at +0xac; I did not model it.
- **Serialization owner**:
  - `TextXmlWriter::serializeNode` 0x00628a60 can now use `contentId.getName()` for the mime Name.
  - `SerializerV2::load(istream&)` 0x006fd070 can discard `readContent`'s std::string. `readContent` 0x006676a0 itself (513 B) is still a STUB in `util/ContentProvider.cpp`.
- **ThreadRef.cpp owner** (being edited this wave, so I left it alone): flip `WeakThreadRef(lua_State*)` PLAYER 0x008723d0 / STUDIO 0x008288c0. Also flip STUDIO 0x00827da0 `reset`, 0x00827e10 `eraseAllRefs`, 0x00828970 `Node::create` and 0x00828830 `createNewNode`; all are at 100% now.
- **DataModel.cpp**: `isNullClassName<sContentFilter>` 0x00632290 (95.2%) and `static_getCreator<Visit>` 0x00630f20 need the include spelling `include\util/object.h` first in DataModel.obj.
- **IDA owner** (next sync of `ida/exports/*-names.csv`):
  - The `match:` names of FactoryProduct<LuaSettings> (PLAYER 0x0085db70/0x0085dc20/0x0085e300, STUDIO 0x009481a0/0x00948250/0x00948920) should spell `VItem@GlobalAdvancedSettings@2@`. 0x0085e300 is `??_E?$GlobalAdvancedSettingsItem@VLuaSettings…@OBM@AEPAXI@Z`.
  - 0x0085e310 is `RBX::LuaSettings::scalar deleting destructor`.
  - 0x0085e220 is `GlobalAdvancedSettingsItem<LuaSettings,sLuaSettings>::GlobalAdvancedSettingsItem()`.
- **Other GlobalAdvancedSettings items** (Debug, Game, Network, TaskScheduler, Render, Physics, FastLog settings) still derive GlobalAdvancedSettings in this tree. RTTI shows each one on `GlobalAdvancedSettings::Item` plus `GlobalAdvancedSettingsItem<T>`, so each has the same 0x28 offset error. The template is now in `GlobalAdvancedSettings.h` for their owners.
- **Pairing churn**: in some regenerations, Constants::uiStepsPerSec 0x0084d380 pairs with `impulseSolverSymStateForceBound`. It has no marker (v8kernel owner, as in wave 4).

## Files changed

- Client/Base/include/rbx/atomic.h, threadsafe.h, TaskScheduler.Job.h; Client/Base/rbx/TaskScheduler.Job.cpp
- 3rdparty/tbb-2.1/ (new: include/tbb/*, COPYING, README, VENDORED.txt); CMakeLists.txt (TBB include root and its comment)
- Client/App/include/Util/Name.h, Object.h, Handle.h, ContentId.h; Client/App/util/Handle.cpp
- Client/App/include/V8Tree/Instance.h (fw, getName); Client/App/v8tree/Instance.cpp (the fw definition only)
- Client/App/include/V8DataModel/ContentProvider.h; Client/App/util/ContentProvider.cpp (readContent signature)
- Client/App/include/V8DataModel/GlobalAdvancedSettings.h, GlobalBasicSettings.h (comment), LuaSettings.h; Client/App/v8datamodel/GlobalAdvancedSettings.cpp, GlobalBasicSettings.cpp, LuaSettings.cpp
- Marker lines only: Client/App/v8datamodel/RunService.cpp, DataModel.cpp; Client/App/v8kernel/Body.cpp; Client/App/util/ContentProviderJob.cpp

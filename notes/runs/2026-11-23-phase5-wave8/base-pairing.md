# phase5-wave8 base-pairing: Stats, Base/object headers, shared pairing causes

Builds: `build-bp/` (PLAYER) and `build-bp-studio/` (STUDIO). Scoring used a private root
(`build-bp/w8/root`: symlinks, plus private `reccmp/` CSVs and the project yml). Each measurement
pools two runs of the current link (`gen_reccmp_csv.py --json`); `round2.sh` adds a second round.
- Scripts: `build-bp/w8/{snap,score,pscore,full,round2,v}.sh` and `cmp.py`.
- Scored copies: `build-bp/w8/snaps/<run>/`. JSONs: `build-bp/w8/runs/`.

The shared `reccmp/` was not written, nothing was committed, no IDA database was opened and nothing
under Client/Rendering was edited. Every heavy command ran through tools/capped.sh, one build at a
time; none hit the cap.

## Result

| | sweep14 | final | lost against sweep14 |
|---|---|---|---|
| PLAYER (c7r, two regeneration rounds) | 10,076 | **10,712** | 53, read below |
| PLAYER with reccmp patch 0002 (c7x) | | **10,718** | 0 more |
| STUDIO (s1p) | 9,803 | **10,011** (10,009 with patch 0002 on the same link) | 8, none mine |

- Both totals include the other wave-8 agents' uncommitted work: World, TimerService,
  PhysicsSettings, service headers and the kernel.
- My share is measured separately in each item below.

## 1. TypedStatsItem<T> (the sweep14 loss): fixed

The 2013 class, from the bytes:
- `TypedStatsItem<int>` is `new 0xa8` (creator PLAYER 0x0048e840).
  - `Stats::Item` is 0x88: double at 0x60 and string at 0x68 (Item ctor 0x0048e470).
  - VC9's `sizeof(boost::function0<int>)` is 0x20 (probe).
  - So the class holds **only** a `boost::function<T()>` at +0x88. There is no `bound` pointer.
- `update()` overrides the base (0x0048e7a0 int, 0x0048e7d0 bool): `formatValue(func())`.
- Bound children bind the static `deref(const T*)` into the function.
  - ios: `TypedStatsItem<T>::deref`, and `Creatable<Instance>::create<TypedStatsItem<bool>, bool const*>`.
  - PLAYER folds it with `mov eax,[esp+4]; ret` 0x004a20c0, which the creator passes to
    `function::assign_to`.
- `Item::update` (0x0073ec00) refreshes the parent `Item` through `fastDynamicCast` and a tail call.
  - The prior tree's `delegate` member does not exist; Item had no member at +0x34.
- `formatValueIi/Id/If/Iy/Ij/Ib/Im` are now the ios-named `Item::formatValue<T>(const T&)` explicit
  specializations.
  - `<unsigned long long>` uses "%llu" (PLAYER string 0x00b50b30), not the old VS2012 "%I64u" stand-in.
  - `<unsigned int>` is new.
  - AsyncHttpQueue's call is updated (same line count).

Raised (PLAYER / STUDIO):

| Entry | PLAYER | STUDIO |
|---|---|---|
| TypedStatsItem<int> vftable{Debugable} | 0x00b01ce4: 96.6% → **100** | 0x00bd614c: 96.6% → **100** |
| TypedStatsItem<bool> vftable{Debugable} | 0x00b01d84: 96.6% → **100** | |
| `TypedStatsItem<int>::update` (TEMPLATE) | 0x0048e7a0: **100** | 0x0043ef20: **100** |
| `TypedStatsItem<bool>::update` (TEMPLATE) | 0x0048e7d0: **100** | 0x0043ef50: **100** |
| `Item::update` (new FUNCTION) | 0x0073ec00: 25% → **100** | 0x007c6b50: 25% → **100** |
| `formatValue<unsigned long long>` (FUNCTION) | 0x0073b480: 36% → **100** | 0x007c3b20: 96% → **100** |
| `formatValue<unsigned int>` (new FUNCTION) | 0x0073b4e0: **100** | 0x007c3b80: **100** |
| `Item::Item` | 0x0048e470: 98% → **100** (FUNCTION) | 0x0043e560: 65%, STUB (STUDIO's older revision differs) |

## 2. Header fixes

| Fix | File | PLAYER | STUDIO |
|---|---|---|---|
| `ICreator::create` returns `shared_ptr<Object>` (Object subobject, +0x18) | Util/Object.h | **~80** `FactoryProduct<…>::Creator::create` copies 98.7% → 100 (c1p list: Teams 0x0053d1d0, Backpack 0x0053c830, Flag 0x007114c0, …) | **57** |
| `getRobloxLocked()` returns the FWValue<bool> unconverted | V8Tree/Instance.h | InsertService::insert 0x007d6a20: 95.0 → 98.9 (native boost_po) → **100** (with /GL boost_po, c5/c7). TextLabel::setText 0x006beeb0 stays 100 | |
| `Heartbeat::expirationTime` is a `Time` | Util/RunTransition.h | RunService::raiseHeartbeat 0x0072da30: 64.4 → 97.9 | |
| `ServiceProvider::create` calls `setAndLockParent` (call 0x00621b60) | v8tree/Service.cpp | 0x006a98f0: 97.8 → **100**, FUNCTION | 0x0076f930: 97.8 → **100**, FUNCTION |
| `Guid::copyDataFrom` is `assign(other.data)` | Util/Guid.h | Primitive::setGuid 0x00753c10: 98.4 → **100** | |

Notes on these fixes:
- **Creator::create.** Evidence: `AbstractFactoryProduct<Instance>::create` 0x0059a5e0 →
  0x00584b20 does `__RTDynamicCast(…, Object, Instance)`, so the type at +0x18 is RBX::Object.
  `Object.h` keeps its line count: the four asserts at lines 231–254 stay put, because a double blank
  line was dropped.
  - **Outside my files:** v8xml/SerializerBinary.cpp:1142 had to spell
    `dynamic_pointer_cast<Instance, Object>` to compile. That is a one-token change, and the line count
    is unchanged.
- **raiseHeartbeat.** Its last diff is pairing-only: the `signal_with_args<1,void(Heartbeat const&)>`
  call. The other remaining diff is `fld [esp+0x48]` (a local copy of wallStep) against our
  `[ebp+8]`. It stays STUB (RunService.cpp is not mine).
- **InsertService::insert.** Its marker in InsertService.cpp (not mine) can become FUNCTION once
  /GL boost_po is in.
- **Primitive::setGuid.** Its marker was already FUNCTION.
- **`Profiling::Mark::~Mark` 0x00745150 (69%): not a Base issue.**
  - Base's `Time::Interval` already has user-declared ctors, so VC9 already returns it through a
    hidden pointer, and the verbose shows that in both images (3-argument `operator-`).
  - The real diffs are in Profiling.cpp:
    1. The 2013 code calls `now<Benchmark>()` before it takes `&startTime`. That is evaluation
       order: ours pushes `&startTime` before the call.
    2. 2013 keeps `elapsed` in an x87 register for both the log value and the parent
       (`fld st(0)` … `fstp st(1)`).
  - This is for the Profiling owner.
- **`rbx::atomic` volatile: no.** Measured by a full PLAYER build (c2 against c1q).
  - Making `rbx::atomic::v` volatile lost **31**: every `signals::signal<…>::disconnectAll` (0.861),
    `intrusive_ptr_release`/`add_weak_ref`/`weak_release` (0.70–0.85) and `dumpThreadRefCounts`.
  - It gained nothing of its own.
  - Reverted.
  - The probe that motivated it is still right about one class:
    - TaskScheduler::TaskScheduler 0x009b55e0 is byte-identical in a probe compile only with a
      volatile member.
    - The reason is its member at +8, built by the **second** atomic-style class, whose ctor
      0x005ba6f0 asserts at atomic.h **line 182**.
    - That ctor stores `v = 0` before testing FLog::Asserts; the line-224 `rbx::atomic` ctor
      0x004022d0 tests first. That ordering is the volatile tell.
  - So 2013 atomic.h has a second class with a `volatile long v`, defined around line 182. Its name
    is unknown, and I did not invent one.

## 3. Shared pairing causes

### (a) program_options /GL: now the PLAYER default (+244 net)

`CMakeLists.txt` adds an option, `OGRE_BOOST_PO_GL`, default **ON**, which affects PLAYER only. With
it, boost_po builds with the shell's flags `/O2 /Ob2 /Oi /Oy /GS /fp:precise /GL /GR /EHsc`; `OFF`
gives the old native build.

The measurement used one source snapshot, built both ways back to back (c5 = /GL, c6 = native; only
boost_po and the link differ), with two regeneration rounds each:
- **c6r 10,451 → c5r 10,695: +292 / −48.**
- Gains:
  - 165 vftables: every `sp_counted_impl_pd<T*, Creatable<Instance>::Deleter>` (0.8 → 1) and the
    program_options exception vftables.
  - 93 shell-region bodies: error_info_injector/clone_impl, `value_semantic_codecvt_helper::parse`
    0x00478170 (0.43 → 0.99) and token_iterator.
- Losses: STL bodies that carry program_options names at engine addresses (`_Tree<…program_options…>`,
  `vector<function1<…basic_option…>>`, `_Uninit_fill_n<shared_ptr<option_description>>`), all
  pairing-only, plus the Base push_back twin chain (see c).
- The 2026-11-13 trial lost 154. That was before the shell stand-ins and the twin rules.

**What it does not fix:** the named near misses stay put in both builds. These are ~DataModel
0x0064d850 (99.76), ~TaskScheduler 0x009b4e00, ~Job 0x009ba1f0, computeStandardSleepTime 0x009b9a90,
RunService::start 0x0072d300, ServiceProvider::onChildAdded 0x006a9770, and the DataModel ctor.

`--verbose` shows the cause is the shell, not program_options:
- The original keeps two identical `~shared_ptr` bodies: the shell's LTCG copy 0x00405b50 (421
  callers in every region) and the engine copy 0x007e4e40 (233 callers).
- Which one a caller gets depends on its **symbol**. A `~shared_ptr<X>` whose X the 2013 /GL shell
  also instantiates resolves to the shell copy.
- Ours has no shell code instantiating them, so 281 `~shared_ptr<X>` names fold into one native body.
  - That body is 0x00463170, from Network's API.cpp.obj.
  - It pairs with 0x00405b50, at 100% and with the most votes.
- Engine callers that the original sends to 0x007e4e40 then show `<OFFSET>`.
- The fix is shell code, or stand-in instantiations in a /GL shell TU of the `shared_ptr<X>` dtors the
  2013 shell owns. Writing that list needs caller-by-symbol evidence; not attempted.

### (b) prime_list one-past-end pointer: reccmp patch 0002

The patch is `tools/reccmp-patches/0002-ogre-one-past-end-pointers.patch`; `apply.sh` documents it.
It is not applied to the shared venv: I measured with a patched copy under `build-bp/w8/pylib`
through PYTHONPATH. The integrator runs `sh tools/reccmp-patches/apply.sh`.

The rule is in ParseAsm.lookup:
- An immediate one past the end of a **matched DATA** entity (≥ 8 bytes) is named `<var>+end`,
  when both of these hold:
  1. the same function's code also mentions the variable's start;
  2. what starts at that address is nothing, or a string reccmp did not match.

Four looser versions failed:

| Version | Effect | Example of what broke |
|---|---|---|
| Any end pointer | −27 | strings after `luaT_eventname`, `co_funcs` |
| Strings only, no begin-reference test | −13 | |
| Without "unmatched" | | lost luaB_collectgarbage: `optsnum` + "collect" |
| Without "nothing else starts there" | | guard variables, floats, `prop_*` |

| Run | Gains | Losses |
|---|---|---|
| PLAYER sweep14 copy | +5 | 0 |
| PLAYER final (c7) | +6 | 0 |
| STUDIO (s1) | +6 | 0 |

The six gains are LegacyContentTable 0x008f08a0, ContactManager 0x007b63a0, LibraryService
0x0088ec90, StarterGuiService ctor 0x0070c150 (STUDIO 0x00798860), `LRUCache<…ContentProvider…>`
0x00663980 and the unordered `table<type_info…>` 0x005c4490.

### (c) Base STL copies per .obj: cause found, not fixed

`vector<shared_ptr<T>>::push_back` 0x009b7020 (Thread.obj) and 0x009ba960 (Job.obj) differ only in
rel32 targets.
- Their `_Insert_n` callees 0x009b6af0 and 0x009ba5c0 differ only in the `push __ehhandler` at +6
  and in relative branches.
- So 2013's linker kept apart EH-bearing instantiations that are otherwise identical.
- Ours folds `push_back<shared_ptr<Job const>/Job/Thread/Coordinator>` into one body
  (map: 0x0067f7a0), and one reccmp pairing serves one of the two originals.

Under /GL boost_po the shared body moved to the other twin:
- TaskScheduler::disableThreads 0x009b7140, getJobsInfo 0x009b75f0, getJobsByName 0x009b78f0,
  setThreadCount 0x009b7ec0 and push_back 0x009b7020 dropped to 97–99%.
- addCoordinator and SequenceBase::onAdded stay at 98%.

No build arrangement I tried reproduces the 2013 non-folding; it needs a probe link of two TUs with
an EH-bearing template. Not attempted.

## Losses against sweep14, read with --verbose

**PLAYER (53):**
- 44 are the /GL program_options and twin-pairing ones above. All are pairing-only: a call target
  flips to the other twin.
- Not mine:
  - `fastRemove<IndexedTree>` 0x00830340 and `setIndexedTreeParent` 0x00999850: the world agent's
    KernelData/IndexArray.
  - `_Incsize`/`push_front<FailedUrl>` 0x0086bc70 / 0x0086c5b0: twin churn; the callee flips.
  - `Bridge<Library>::on_tostring` 0x0087f770: callee pairing.
  - `~shared_ptr<CRenderSettingsItem>` 0x00542240: shell twin.

**STUDIO (8), none in my files:**
- Camera/TouchTransmitter `??_E`/`??_G` twins: TouchTransmitter.h and Camera.h are modified by
  another agent.
- `fastRemove<IndexedTree>` and setIndexedTreeParent.
- `describedClassDescriptor` atexit stubs for PhysicsService/ScriptService: their headers are being
  edited by another agent.

## Checks

- Both targets build (b7, bs2), with no FAILED steps.
- `check_2016_copies`: silent.
- `reccmp_addr_padding`: 1,669 files, 0 bad.
- `reccmp-decomplint` PLAYER: the only out-of-order lines in my files are AsyncHttpQueue.cpp 138/325/492
  and Service.cpp 129, all already in sweep14's report. The new markers are in ascending order:
  `formatValue<unsigned int>` was moved after `<unsigned long long>`, and `Item::update` to the end
  of StatsItem.cpp.

## Files changed

- Client/App:
  - `include/V8DataModel/StatsItem.h` and `TypedStatsItem.h`; `v8datamodel/StatsItem.cpp`
  - `util/AsyncHttpQueue.cpp` (one call plus comments; line count unchanged)
  - `include/Util/{Object.h, Guid.h, RunTransition.h}`
  - `include/V8Tree/Instance.h` (`getRobloxLocked` only)
  - `v8tree/Service.cpp`
  - `v8xml/SerializerBinary.cpp`: line 1142, forced by the Object.h change; not my file.
- Client/Base/include/rbx/atomic.h: volatile tried and reverted. It is byte-identical to HEAD.
- `CMakeLists.txt`: the `OGRE_BOOST_PO_GL` option and comment, PLAYER boost_po /GL by default.
- `tools/reccmp-patches/0002-ogre-one-past-end-pointers.patch` (new) and `apply.sh` (comment).
- tools/ida/gen_reccmp_csv.py: unchanged.

## Unresolved

- **Integrator:**
  - Run `sh tools/reccmp-patches/apply.sh` (patch 0002).
  - Reconfigure `build/` so that `OGRE_BOOST_PO_GL=ON` takes effect. A cache without the option gets
    the ON default; a cache from my trials does not exist outside build-bp.
  - Regenerate with at least two pooled rounds: the /GL program_options names need the second round.
- **InsertService owner:** the InsertService::insert 0x007d6a20 marker can become FUNCTION (100% with
  /GL boost_po).
- **Profiling owner:** `Mark::~Mark` 0x00745150. The cause is `now()` evaluated before `&startTime`
  and `elapsed` kept on the x87 stack; it is not the Interval return.
- **RunService owner:** raiseHeartbeat 0x0072da30 (97.9). It reloads wallStep from a stack copy, and
  the Heartbeat signal call is unpaired.
- **Base:** name and write the second atomic-style class (atomic.h line 182, volatile member, ctor
  0x005ba6f0, TaskScheduler +8). That is the rest of TaskScheduler::TaskScheduler 0x009b55e0, which
  is identical in a probe with a volatile member.
- **Client shells:** the remaining `~shared_ptr`/STL engine-copy pairing needs the 2013 shell's own
  instantiations. It covers ~DataModel, ~TaskScheduler, ~Job, computeStandardSleepTime,
  RunService::start, onChildAdded, the DataModel ctor and the 15 `find<T>` copies.
- **Base:** the per-.obj STL copies (c).
- **Stats:** `Item::formatMem` 0x0073b300 (53%), `formatValue(double, fmt, …)` 0x0073b380 (55%),
  `formatValue<bool>` 0x0073b510 (65%) and `formatRate` 0x0073c670 (59%) are still STUB. The STUDIO
  `Item::Item` 0x0043e560 is at 65%.

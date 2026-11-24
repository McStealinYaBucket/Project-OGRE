# phase5-wave9 shell-base: shell template twins, Base leftovers, FastFlags, Stats

Builds: `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO). Scoring used private roots `build-sh/w9/root`
(PLAYER) and `build-sh/w9s/root` (STUDIO): symlinks, private `reccmp/` CSVs and the project yml.
- Scripts: `build-sh/w9/{snap,score,ab,v,link}.sh`, `targets.py`, `cmp.py`; STUDIO copies in `build-sh/w9s/`.
- `ab.sh` is the fixed protocol for every figure below. It starts from the shared `reccmp/` CSVs, then:
  1. two runs, then a regeneration;
  2. two more runs, then a second regeneration;
  3. a final run (`<name>r`).
- Scored copies: `build-sh/w9*/snaps/<name>/`. JSONs: `build-sh/w9*/runs/`. Regenerated CSVs: `build-sh/w9/csv-<name>/`.
- The A/B links (`build-sh/w9/link{A,B,C}`, `build-sh-studio/w9/link{A,B}`) are relinks of one and the same
  set of objects, with only the stand-in object swapped (rsp from `build.ninja`, `link.sh`). So the A/B
  deltas are mine alone, although other agents were editing App headers throughout the session.

The shared `reccmp/` was not written. Nothing was committed, no IDA database was opened and nothing under
Client/Rendering was edited. Every heavy command ran through tools/capped.sh, one build at a time; none hit the cap.

## Results

| | without the stand-in (B) | final tree (C, PLAYER / A, STUDIO) | sweep15 |
|---|---|---|---|
| PLAYER entries at effective 100% | 10,788 (LBr) | **10,815** (LCr) | 10,729 |
| STUDIO entries at effective 100% | 10,241 (SBr) | **10,268** (SAr) | 10,192 |

- B already includes items 2–4 below and the other agents' uncommitted work, so B against sweep15 is not my share alone.
- My items' entries are listed below.

## 1. Shell template twins (stand-in): PLAYER +27 net, STUDIO +27 net

**New tool: `tools/shell_twin_callers.py`.** It reads the reccmp JSON, our map and our `.obj` relocations.
- For every paired engine function, it aligns the original's direct call targets with the calls in our `.obj`
  section for the same symbol.
- It then reports, per symbol of ours, how often the original calls the copy in the LTCG shell region and how
  often it calls a native copy.
- Shell region: PLAYER 0x00401000–0x00480430; STUDIO 0x004f6000 via `--shell-end`.

Findings on sweep15:
- **PLAYER:** 222 symbols are called at shell copies. 25 of them share one of our bodies with a native-owned
  symbol; those 25 are the twin pairs that lose calls.
- **STUDIO:** 338 symbols, 44 mixed. They are largely the same symbols, because both shells compile ClientBase.
- Almost no symbol is called at both kinds of copy, so the "shell-owned symbol" model holds at the symbol level.
- Callers whose call counts differ were skipped: 1,136 in PLAYER.

**The stand-in.** `Client/Win/placeholder.cpp` (both shells compile it) now holds one `/include`-kept
`extern "C"` function. It is not original code and has no marker. It instantiates these, the shell-owned
symbols with evidence:
- `~shared_ptr<Instance>` (73/0) and `shared_ptr<Instance>::operator=` (40/0)
- `vector<shared_ptr<Instance>>::resize` (34/0) and `erase`
- `~shared_ptr<StandardOut>` (7/0)
- `weak_ptr<DataModel>::lock` (6/0)
- `shared_ptr<DescribedBase>::operator=` and `shared_from_this`
- the `function0<void>` copy (`assign_to_own` 3/0)
- `shared_from<RunService>` and `shared_ptr<RunService>::operator=`
- `~shared_ptr<Script>`
- PLAYER only: the `shared_ptr<Tasks::Coordinator>` copy ctor (see item 2)

`Client/ClientShared/placeholder.cpp` (STUDIO only) adds `Stats::Item::Item`: STUDIO's copy 0x0043e560 is
the shell's /GS LTCG copy.

Measured on identical objects (C against B, `cmp.py LBr LCr`): **PLAYER +51 / −24**.
- Targets reached: RunService::start 0x0072d300, ServiceProvider::onChildAdded 0x006a9770,
  HeartbeatTask::stepDataModelJob 0x0072dc30 and Job::addCoordinator 0x009baa80.
- Also: Lighting::onChildAdded 0x00707510, GcJob/WaitingScriptsJob::stepDataModelJob, shared_from<DataModel>
  0x006d9090 and shared_from<GuiItem> 0x0091d170.
- Shell-region bodies:
  - resize 0x004226f0, _Ufill, _Umove, _Uninit_fill_n, _Uninit_copy
  - weak_ptr::lock 0x00439eb0
  - assign_to_own 0x00437a90
  - shared_from<RunService> 0x004618d0
  - DescribedBase op= 0x00469520
  - Creator<Script>::create 0x004074e0 and doDeclare<sScript> 0x00413740
- Vftables: bad_weak_ptr 0x00b032e8 and sp_counted_impl_pd<Script*> 0x00b13820.
- Plus about 20 template bodies.

STUDIO (A against B, `SBr` against `SAr`): **+59 / −32**.
- Item::Item 0x0043e560 went from 65% to **100%** (marker promoted to FUNCTION).
- The STUDIO A run still had the CoreGuiService entry, so it includes the 6 ScreenGui losses described below.
  C has not been scored in STUDIO.

**Losses, each read with `--verbose`: all pairing-only, one callee twin each.**
- 15 PLAYER callers of a native `~shared_ptr` now call the body our regeneration pairs with
  `exception_ptr::~exception_ptr`. The original calls the native twin, which is paired under the Players name.
  - ScriptContext closeState, printCallStack and setTimeout; ~ScriptStatInformation; ContentProvider::isValidRobloxAssetUrl
  - TimerService::onHeartbeat; the two TGenericSlotWrapper::execute; Bridge<EventInstance>::on_index
  - LibraryService::registerDevelopmentLibrary; ContentProviderJob processTask and addTask; httpGetPostWinInet
  - find/getPublicServiceByClassNameString; callable<signal<shared_ptr<Instance>>>
- 7 callers of `_Destroy_range`, `_Copy_opt` and `~vector` on `shared_ptr<Instance>` (shell-owned in ours now).
  Our call pairs with the `<boost::thread>` named twin:
  - checked_delete and sp_counted_impl_p of `vector<shared_ptr<Instance>>`
  - Instance::predelete, ~ServiceProvider, unchecked_copy, ArchiveBinder::processID, InsertService::insert
- Cause: the original's native twins (`~shared_ptr` 0x007e4e40, `_Destroy_range` 0x00829d90, op= 0x006eda50,
  assign_to_own 0x0095b340, copy ctor 0x00576dc0) have no CSV row of their own. The regeneration pairs our
  native body with them only by chance.

Two things I tried and dropped:
- **TEMPLATE markers that pin each twin by a symbol.** I gave each shell/native twin pair (PLAYER and
  STUDIO addresses) a TEMPLATE marker naming a symbol only that copy serves. They paired nothing: the shell
  rows stayed paired with the CSV's names. Removed.
- **`rbx::signal<void(bool)>::operator()` (9/0).** It cost the Lighting setters 7 entries. Its shell copy
  0x00439b20 has a second shell twin 0x00456d20 (PropertyDescriptor), and our link folds the two. Removed.
- **The CoreGuiService entry.** Its header made the shell instantiate the `create<ScreenGui>` chain, which 2013
  keeps native (−6: doDeclare<sScreenGui> 0x00694230, create<ScreenGui> 0x006997d0 and others). Removed in
  C (C against A: +6 / −5).

Still short, all from twins this stand-in does not reach:
- ~DataModel 0x0064d850 (99.76), the DataModel ctor (99.70), ~TaskScheduler 0x009b4e00 (99.52),
  ~Job 0x009ba1f0 (99.28) and computeStandardSleepTime 0x009b9a90 (99.21). In each, the only diff is the
  native `~shared_ptr`/`_Destroy_range` twin, which needs the twin rows above.
- raiseHeartbeat 0x0072da30 (97.87). The original calls a third copy of signal operator(), 0x0072d630,
  which is native and separate from 0x005ebad0. These are native per-.obj copies, as with push_back.

## 2. Base

- **The second atomic class (atomic.h:182).**
  - Written as `rbx::atomic_unk0x005ba6f0<T>` in the atomic.h padding: `volatile long v`, an out-of-line
    parameterless ctor whose assert lands on line 182, and the interlocked operators.
  - The name is a placeholder: no symbol names it.
  - `TaskScheduler::taskCount` (+8) uses it. Line 224 and every later line are unchanged.
  - It has a TEMPLATE marker:

    | | PLAYER | STUDIO |
    |---|---|---|
    | ctor | 0x005ba6f0 (none) → **100%** | 0x0065e110 (none) → **100%** |
    | TaskScheduler::TaskScheduler | 0x009b55e0: 97.77 → 98.88 | 0x00a6d3d0: 97.8 → 98.9 |

  - TaskScheduler::sampleRunningJobCount gets STUB markers (PLAYER 0x009b3ff0, STUDIO 0x00a6bea0), so the
    ctor's `mov eax, offset` pairs. TaskScheduler.cpp keeps its line count.
  - What remains in the ctor:
    - two `mov byte [esp+0x4e/0x4f], bl` empty temporaries in the intrusive list inits;
    - the unpaired `function0::assign_to<bind_t<…TaskScheduler…>>` at 0x009b4350 (STUDIO 0x00a6c140), which
      has no row.
- **The push_back twins (0x009b7020 / 0x009ba960).** The cause is shell ownership, not a per-.obj copy rule:
  - Job.obj's `_Insert_n` 0x009ba5c0 calls the shell's `shared_ptr` copy ctor 0x00422330.
  - Thread.obj's twin 0x009b6af0 calls the native one, 0x00576dc0.
  - So `shared_ptr<Tasks::Coordinator>(const&)` is shell-owned. With it in the PLAYER stand-in, the two chains
    stay apart in our link.
  - Job::addCoordinator 0x009baa80 is now at **100%**. 0x009ba960 and 0x009ba5c0 now pair (97.8 / 96.5).
  - disableThreads, getJobsInfo, getJobsByName and setThreadCount are unchanged (97–99). The Thread.obj chain
    0x009b7020 has no row. 0x009b6af0 pairs as `vector<shared_ptr<boost::thread>>` and needs the same twin rows.

## 3. FastFlags

- `FreeFallOptimization` and `ImpulseSolverForContact` are now `true` (.data bytes 1 at PLAYER 0x00d30efe/eff).
- `FFlag::SymmetricContact` is declared and defined `true`. It is the .data byte 1 at 0x00d30f08, registered
  at 0x00a65a80. world-5d.md said false; the initial byte is 1.
- KernelData.h readers: the code only loads the byte, so there is no change.
- SimBody::updateSymmetricContactState 0x0082e000 can now be written (SimBody.cpp, not mine).

## 4. Stats and raiseHeartbeat

- `Stats::Item::setValue(double, const string&)` is the inline 2013 helper: the +0x60 store, then `operator=`.
- formatMem and formatValue(double, fmt, …) use it; formatValue uses `G3D::vformat`.
- formatValue<bool> is `= value ? "true" : "false"; statValue = value ? 1 : 0`.
- formatRate is a single `formatValue(rate(), fmt, rate(), 100.0 * cv())`.

All four are byte-identical in probe_compare and at **100%** in reccmp in both targets. Markers promoted to
FUNCTION:

| Function | PLAYER | STUDIO |
|---|---|---|
| formatMem | 0x0073b300: 53 → 100 | 0x007c39a0: 53 → 100 |
| formatValue(double, fmt, …) | 0x0073b380: 55 → 100 | 0x007c3a20 → 100 |
| formatValue<bool> | 0x0073b510: 65 → 100 | 0x007c3bb0 → 100 |
| formatRate | 0x0073c670: 59 → 100 | 0x007c4ba0 → 100 |

raiseHeartbeat 0x0072da30 stays at 97.87: see item 1.

## Checks

- check_2016_copies: silent.
- reccmp_addr_padding: 1,669 files, 0 bad.
- decomplint PLAYER: TaskScheduler.cpp has the same out-of-order set as sweep15. No new descents in my files.
- Neither target was rebuilt after the last edits, which are comment-only:
  - Win/placeholder.cpp: header comment.
  - StatsItem.h: one comment line added; the header has no asserts.
  - TaskScheduler.cpp: one blank line removed so its line count equals HEAD. The scored binaries had it +1
    after line 371.
- The final source compiles: C's object is the current Win/placeholder.cpp.

## Files changed

- Client/Base/include/rbx/atomic.h, rbx/TaskScheduler.h, Client/Base/rbx/TaskScheduler.cpp
- Client/App/include/Util/FastFlags.h, Client/App/util/FastFlags.cpp
- Client/App/include/V8DataModel/StatsItem.h, Client/App/v8datamodel/StatsItem.cpp
- Client/Win/placeholder.cpp (stand-in), Client/ClientShared/placeholder.cpp (STUDIO stand-in)
- Client/WindowsClient/placeholder.cpp: unchanged at the end.
- tools/shell_twin_callers.py (new)

## Unresolved

- **Generator (owner, tools/ida/gen_reccmp_csv.py):**
  - Give the original's native twins rows of their own, with a native-owned symbol:
    - PLAYER: ~shared_ptr 0x007e4e40, _Destroy_range 0x00829d90, op= 0x006eda50, assign_to_own 0x0095b340,
      copy ctor 0x00576dc0.
    - STUDIO: 0x007b83e0, 0x007bc7a0, 0x00751110, 0x0085d930.
  - Pair the shell copies by a shell-owned symbol (`tools/shell_twin_callers.py` lists both kinds).
  - That would recover the 22 pairing-only losses above, plus ~DataModel, the DataModel ctor, ~TaskScheduler,
    ~Job, computeStandardSleepTime and the TaskScheduler push_back callers.
  - TEMPLATE markers do not override the generated shell rows.
- **SimBody.cpp owner:** write updateSymmetricContactState 0x0082e000 now that FFlag::SymmetricContact exists.
- **Shell:** the stand-ins go away as the real 2013 shell classes are written: ReflectionMetadata,
  FrameRateManagerStatsItem, the views. I did not write any; each needs its own bytes work.
- **Native per-.obj copies:** signal operator() at 0x0072d630 and 0x005ebad0 (raiseHeartbeat), and the
  Thread.obj push_back chain. Their bodies differ only in their EH handler. Not reproduced.
- **TaskScheduler ctor:** the two empty-temporary byte stores in the intrusive list inits are not reproduced.
- **Integrator:** regenerate with at least two pooled rounds. Without that, the stand-in scores −39 against
  the stale CSVs (LAa 10,703 against LBa 10,742).

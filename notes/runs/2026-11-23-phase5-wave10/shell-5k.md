# phase5-wave10 shell-5k: the first real 2013 client-shell class (ReflectionMetadata)

Builds: `build-sh/` (PLAYER, full build of the shared tree at 09:40, 400 steps) and `build-sh-studio/`
(STUDIO, `--target studio`, rebuilt at 10:30 after another agent's Type.h compile error cleared).
Every figure below is a relink of one and the same set of objects with only my shell objects swapped
(`build-sh/k/mkrsp.py` writes the rsp from build.ninja, `k/link.sh` relinks), scored with wave 9's
fixed protocol (`k/ab.sh`: shared CSVs, 2 runs, regenerate, 2 runs, regenerate, final run `<name>r`)
on private roots. The shared `reccmp/` was not written, no IDA database was opened, nothing under
Client/Rendering was edited, nothing was committed, every heavy command ran through tools/capped.sh
(none hit the cap).

## Results

| PLAYER (final run) | entries at 100% | against A |
|---|---|---|
| A: shell as in HEAD | 10,605 | |
| B: + ReflectionMetadata (first version) | 10,744 | +139 / −0 |
| D: + lookups on `name->str`, class header inlined into writeClass | **10,755** | +153 / −3 (+14 / −3 against B) |
| E: D + one stand-in entry deleted (final tree) | **10,755** | identical to D (0 / 0) |

| STUDIO (final run) | entries at 100% | |
|---|---|---|
| SA: final tree without ReflectionMetadata.obj | 10,698 | |
| SB: final tree | **10,837** | +144 / −5 |

The absolute numbers are below sweep16 (10,909 / 10,373) because the shared tree carried the other
wave-10 agents' unfinished edits when I built; the deltas are mine alone.

Every loss was read with `--verbose`; all are pairing-only:
- PLAYER `Player::getTeamColor` 0x005273c0 and `getNeutral` 0x005273d0: trivial identical-body twins,
  unpaired in D's regeneration (both still 100% wherever paired).
- PLAYER vftable 0x00b11c68 (error_info_injector<bad_placement_any_cast>{for boost::exception}): B had
  paired it under the wrong RTTI name; D pairs the right name and loses a twin slot.
- STUDIO `_Tree<InstanceHandle…>::erase` 0x007490e0 / 0x007493b0, `map<InstanceHandle…>` 0x0074a610,
  unordered `node_constructor` 0x009ad8c0 / `table_impl` 0x009adac0: each differs by one callee that
  pairs to another identical-body destructor twin (`~pair<…>` against `~StatCollection` /
  `GImage::Error::~Error`).

**Row 5k** (tools/row_coverage.py --row 5k, PLAYER): A 695/3,493 (19.9%) → D **731/3,493 (20.9%)**
(sweep16 P-final: 697). The ~110 other gains are vftables, data and engine-side templates outside the row:
- 40 Metadata vftables, 9 `sp_counted_impl_pd` vftables and 4 `BoundPropGetSet` vftables;
- the singleton's static destructor;
- MemoryBinder/ArchiveBinder template bodies, including ArchiveBinder::processID 0x006fb300, one of
  integrate.md's six twin losses.

## 1. ReflectionMetadata (new: Client/ClientBase/ReflectionMetadata.h/.cpp)

The 2013 path comes from its assert (`Client\ClientBase\ReflectionMetadata.cpp`, `bound`, line 118 in
both targets); the line is reproduced by layout.

Against 2016, 2013 has:
- no Enums/Enum/EnumItem classes, no Item::minimum/maximum and no Class::insertable;
- no Reflection::save, no conflict report and no ` [noyield]`;
- no Plugin security level (the writer's jump table at 0x00448034 has six cases).

Layouts come from the create<T>/ctor bytes:

| Class | Size | Members |
|---|---|---|
| Reflection | 0x64 | classes at +0x60 |
| Item | 0x80 | +0x60 browsable = 1, +0x61/+0x62 = 0, description at +0x64 |
| Class | 0xa4 | +0x80 = −1, +0x84 = 0, preferredParent("") at +0x88 |
| Classes, Members folders | 0x60 | |
| Member | 0x80 | |

Properties: Browsable, Deprecated, IsBackend, summary, ExplorerOrder, ExplorerImageIndex,
PreferredParent, all in category "Reflection", in the 2013 dynamic-initialiser order. The FLog channel
defaults to 1.

The PLAYER singleton loads `<exe dir>\ReflectionMetadata.xml` through GetModuleFileNameA, ATL::CPath and
RemoveFileSpec, with a counter-less `Limits::Counter::Activator` live around it. STUDIO's singleton
(0x004338a0) uses `RobloxSettings::getResourcesFolder()` + "/ReflectionMetadata.xml" and is not
written. The API writer exists in PLAYER only: STUDIO has none of its strings, so /OPT:REF dropped it there.

**Functions at 100% (FUNCTION markers), 15 PLAYER + 1 STUDIO:**

| Function | PLAYER | STUDIO |
|---|---|---|
| Reflection::load | 0x00447c10 | 0x00433aa0 |
| Reflection::singleton (header) | 0x00447920 | |
| safe_static_init_singleton (header) | 0x004478d0 | |
| Reflection::get(PropertyDescriptor) | 0x00447ec0 | |
| Reflection::get(EventDescriptor) | 0x00447f00 | |
| Writer::writeMetadata | 0x00447f40 | |
| Writer::writeMemberInfo | 0x00447fa0 | |
| Writer::writeArg | 0x00448210 | |
| Writer::writeArgs | 0x00448320 | |
| Writer::writeFunction | 0x00448370 | |
| Writer::writeYieldFunction | 0x00448560 | |
| Writer::writeCallback | 0x004488c0 | |
| Writer::writeClass | 0x00448ab0 | |
| Writer::writeEnum | 0x00448d10 | |
| Writer::writeEnumItem | 0x00448d80 | |

STUB:

| Function | Address | Score | Cause |
|---|---|---|---|
| safe_static_do_get_singleton | 0x00447970 | 87.0 | create<Reflection> convention (see Unresolved 1); ATL `operator+(CStringT, PCXSTR)` 0x00440db0 gets an LTCG register convention in ours because the original's native callers (0x0066431a, 0x00623b1b) do not exist in our tree |
| writeProperty | 0x00448050 | 91.1 | our LTCG inlines get(Property) into it; the original keeps get(Property) out of line |
| writeEvent | 0x00448750 | 89.1 | the same, with get(Event) |
| writeAPI | 0x00448df0 | 75.0 | the original has an `and esp,-8` aligned frame inherited from its caller, the inlined writeEverything in the shell's command-line handler (0x004054b8), which is not written |
| safe_static_do_get_singleton (STUDIO) | 0x004338a0 | 41.4 | the RobloxSettings path is not written |

Two source facts the bytes forced:
- **The lookups take the Name's string by reference** (`name->str`): no copy and no EH frame. App's
  `Name::toString()` returns a copy (the 2016 signature).
- **writeClass writes the class header in place**: 2016's writeClassHeader exists for its conflict
  report, which 2013 does not have.

## 2. Stand-in entries removed

- `Client/Win/placeholder.cpp`: deleted `shared_ptr<DescribedBase>::operator=` and
  `enable_shared_from_this<DescribedBase>::shared_from_this`. The PLAYER map shows both now come from
  ReflectionMetadata's object. E against D: 10,755 = 10,755, 0 / 0.
- Every other entry stays: no real shell code instantiates them yet.
- `Client/ClientShared/placeholder.cpp` (Stats::Item::Item) stays. STUDIO's FrameRateManagerStatsItem
  (0x0043e8a0) has no 2013 `__FILE__` string to place it, and PLAYER's copy is RbxOgre's (Rendering).

## 3. The six mixed-ownership twin callees: not stand-in problems

Read on sweep16, original call targets (`build-sh/k/snaps/s16`):

| Function | Original calls | Copy |
|---|---|---|
| ScriptContext::setTimeout 0x0067eee0 | 0x006d9090 | native |
| ArchiveBinder::processID 0x006fb300 | 0x006fb240 | native |
| InsertService::insert 0x007d6a20 | 0x00535280 | native ~vector |
| unchecked_copy 0x008b3170 | 0x004ba520 | native _Copy_opt |
| callable<…> 0x0093d4a0 | 0x006762f0 | native function1::operator() |
| ~vector<shared_ptr<Instance>> 0x00535280 | 0x00427f30 | shell _Destroy_range |

So five of the six call **native** copies; only ~vector's callee is shell-owned, and that is already in
the stand-in.
- **setTimeout:** the original has two native `shared_from<T>` bodies, 0x006d9090 and 0x0091d170. They
  differ only in their EH handler (per-.obj copies, like signal operator()). Ours folds
  `shared_from<ScriptContext>` with the GuiItem one.
- **~vector<shared_ptr<Instance>>:** ours is native at 0x004cb580 and calls our shell
  `_Destroy_range` 0x00414b00, as the original does. The CSV still pairs the original 0x00535280 with
  our `~vector<shared_ptr<thread>>` body. That is generator twin pairing.
- **unchecked_copy:** the original calls native `_Copy_opt` 0x004ba520, but also has a shell copy
  0x00429110 (called by the shell's erase and by the engine at 0x0055fbd5). The symbol is contradictory,
  so it is not an instantiation the shell can take over.

Adding `shared_from<DataModel>`, `map<string, shared_ptr<Script>>`, `_Copy_opt` or
`function1<…>::operator()` to the shell would move native-owned symbols into the LTCG set, so none was
added. processID came back at 100% in B/D/E through regeneration.

## 4. Row 5k near misses

The top shell near misses all differ only by an unpaired `__ehhandler$` push:
- call_once 0x0040c2c0 (99.4), `_Insert_n` 0x004261d0 (99.2), `_Uninit_fill_n<string>` 0x00413600,
  wstring `_Construct` 0x00474df0;
- `_Insert_n` also calls `_Allocate` at an unpaired copy.

These are generator rows (libdata `__ehhandler$` for LTCG functions), not source.

The shell's Creator::create family (≈60 functions at 77–90%) has one real cause. App's
`Util/Object.h` takes the address of `Creatable::create<T>`:

```cpp
shared_ptr<T> (*thisFunction)() = &create;
```

That stops LTCG giving create<T> its custom convention: the original passes the result in edi, ours
pushes it.

Experiment C: B plus the shell objects compiled with an Object.h copy where those five lines are blank,
so lines are kept (`build-sh/k/ovr`, never in the tree):

| Function | B | C |
|---|---|---|
| create<CRenderSettingsItem> 0x0040d4f0 | unpaired | **100** |
| create<Reflection> 0x0040d7b0 | unpaired | **100** |
| shared_ptr<Reflection>::operator= 0x00449790 | unpaired | **100** |
| Creator::create (×4 measured) | 88.6–89.9 | 94.9–97.4 |
| EnumDesc<…>::lookup ×7 | 34 | 75 |

- 34 shell entries rose and 22 fell (pairing churn of the writer functions).
- The remaining Creator::create diff is only the Object.h `__FILE__` string, because the experiment's
  copy lives on another path. In the real header that diff does not exist.

## Checks

- check_2016_copies: silent.
- reccmp_addr_padding: 1,671 files, 0 bad.
- reccmp-decomplint on Client/ClientBase and Client/Win: clean (PLAYER and STUDIO).
- ReflectionMetadata.cpp builds under both shells' flags (PLAYER obj via `k/cc.py`, STUDIO in the full
  build-sh-studio build).
- The edits after the last scored link are marker words only (STUB to FUNCTION), so line numbers are
  unchanged.

## Files changed

- Client/ClientBase/ReflectionMetadata.h, Client/ClientBase/ReflectionMetadata.cpp (new).
- Client/Win/placeholder.cpp: one stand-in entry deleted, unused parameter dropped, header comment.
- **CMakeLists.txt (shared, outside my ownership; one line):** `OGRE_CLIENT_COMMON_SOURCES` gains
  Client/ClientBase/ReflectionMetadata.cpp. Without it the class cannot be built. This is a small
  separate commit for the integrator.

Saved files:
- Runs and JSONs: `build-sh/k/runs/` (A, B, C, D, E; `*-cmp.txt`) and `build-sh/ks/runs/` (SA, SB).
- Scored exe/pdb/map: `build-sh/k/snaps/*` and `build-sh/ks/snaps/*`.
- Scripts: `build-sh/k/{mkrsp.py,link.sh,pipe.sh,cc.py,cc_ovr.py}`.

## Unresolved

1. **App owner, Util/Object.h:** delete the five `shared_ptr<T> (*thisFunction)(…) = &create;` lines in
   Creatable::create. Replace each with a blank line, so Object.h's asserts keep lines 231 and 360.
   - Evidence: create<Reflection> 0x0040d7b0 passes the result in edi. That LTCG convention is
     impossible while the address is taken. Experiment C above.
   - Native code is unaffected: the local is dead under /O2.
   - Expected effect: the shell's create<T> copies and most of its ~60 Creator::create functions reach
     100%, plus the Reflection singleton.
2. **App owner, security/SecurityContext.h:** the Permissions enum is 2016's. 2013 has None 0,
   RobloxPlace 1, LocalUser 2, WritePlayer 3, RobloxScript 4, Roblox 5, with no Plugin (writer jump
   table 0x00448034). ReflectionMetadata.cpp switches on 2013 numbers until the header is fixed.
3. **App owner, Util/Name.h:** 2013's `Name::toString()` most likely returns `const std::string&`. Every
   2013 lookup passes `&name->str` without a copy. ReflectionMetadata reads `name->str` meanwhile.
4. **App owner:** `std::ostream& RBX::operator<<(std::ostream&, const Name&)` (PLAYER 0x006131f0) is
   not declared or defined in the tree. ReflectionMetadata.cpp declares it locally, so it is a new
   unresolved symbol. The integrator should rerun tools/gen_link_stubs.py (delete the exe first).
5. **Integrator:** the CMakeLists.txt line above; regenerate CSVs with two pooled rounds (the gains need
   the regeneration: B scored 10,556 against the stale CSVs before it).
6. **Generator (owner):**
   - The six "mixed-ownership" twin losses are native-twin pairing, not shell ownership (item 3);
   - the shell's LTCG `__ehhandler$` stubs need libdata rows (row 5k near misses, item 4).
7. **Shell, still to write:**
   - the WindowsClient command-line handler around 0x004054b8. It inlines writeEverything; writeAPI's
     aligned frame comes from it.
   - STUDIO's singleton (needs RobloxStudio's RobloxSettings).
   - LogManager.cpp (Client\Win, assert line 354; MainLogManager/ThreadLogManager/RobloxCrashReporter).
   - DSVideoCaptureEngine.cpp (Client\Win, about 30 asserts).
   - The views and verbs (LeaveGameVerb, ScreenshotVerb, ToggleFullscreenVerb, RecordToggleVerb,
     UserInput, ViewUpdateJob, RenderJob, VideoControl, RbxWebView).
   - Each will retire more stand-in entries.
8. **FrameRateManagerStatsItem:** STUDIO's copy is shell code but has no 2013 `__FILE__`; PLAYER's
   is RbxOgre's (Client/Rendering, the other contributor).

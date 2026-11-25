# Wave 12: App follow-ups (agent "ap")

Build dirs: `build-ap/` (PLAYER) and `build-ap-studio/` (STUDIO).

Scoring used a private root, `build-ap/w/root` (symlinks plus its own `reccmp/` and `reccmp-project.yml`), and build-ah's ab protocol:
- two runs on the shared CSVs;
- a regeneration (`gen_reccmp_csv.py`, pooling both runs);
- two more runs, then a second regeneration;
- the final run.

| Item | Location |
|---|---|
| Scripts | `build-ap/w/{snap,score,ab,cmp.py,probelink.sh}` |
| Runs | `build-ap/w/runs/` |
| Scored exe/pdb/map | `build-ap/w/snaps/{P4,S3}` (final links before the Name.h change), `{P5,S4}` (with it) |
| Regenerated CSVs | `build-ap/w/csv-P4-PLAYER`, `csv-S3-STUDIO` |

## Result

| | sweep18 | final (P4r / S3r) | gained | lost |
|---|---|---|---|---|
| PLAYER | 11,673 | **11,789** | 119 | 3 |
| STUDIO | 11,071 | **11,194** | 124 | 1 |

The links also contain the other agents' uncommitted work from this wave:
- Network: CrashReporter, Players/Server/Peer.
- Shell: VideoControl, CVersionInfo, DumpErrorUploader, GameVerbs, LogManager.

I read every loss with `--verbose`. None is from my changes:
- **PLAYER 0x004069d0 / STUDIO 0x004256d0, `~intrusive_ptr<thread_data_base>`.**
  - The `library` row names a body that is really a HeapFree helper.
  - The new `Client/Network/CrashReporter.cpp` now emits that COMDAT first (recomp 0x0046aed0, `CrashReporter.cpp.obj`). In sweep18 it came from ScriptContext.obj, at 0x004d8ef0.
  - This is a pairing loss caused by the network agent's new file.
- **PLAYER 0x005c8930 `IsOlderOrderedPacket` and 0x00826990 `httpGetPostWinHttp`.** Both read `0x7fffff` as `jpeg_natural_order[5]+3`: no-`.reloc` data coincidences.

These moved between links and came back by the final link:
- the RakNet SlidingWindow, Lua and `Math::isDenormal` coincidences;
- the sp_counted vftables, which stayed at 0.8 until the CSVs were regenerated.

My own entries are listed under each change below. The shell entries listed are the ones whose shell-5k-2 diagnosis was "ATL helper conventions". All of them rose in the P2 link (`runs/P2r.json` vs `build/sweep18/P-final.json`).

## Changes

### 1. FLog::Network default is 6 (`script/ScriptContext.cpp:81`)

`LOGVARIABLE(Network, 6)`. datacmp P2: the `FLog::Network (0xd30f1b) DIFF 6 : 1` line is gone. No function moved.

### 2. GlobalAdvancedSettingsItem<CRenderSettingsItem> ctor 0x00456ee0 (84.3): isolated, not fixable in App

Method: I relinked PLAYER with only `RenderSettingsItem.cpp.obj` recompiled against one 60edf76 header at a time. The header came from an overlay `-I` placed first; `/showIncludes` confirms the overlay copy is used. The link added `/FORCE:MULTIPLE`. The probe objects and links are `build-ap/w/rsi-*.obj` and `probe-*.{exe,map}`.

| Header reverted | ctor frame | CRenderSettingsItem ctor frame | `create<CRenderSettingsItem>` |
|---|---|---|---|
| none (control) | 0x28 | 0x1c | custom convention, as in 2013 |
| Name.h | 0x28 | | |
| SecurityContext.h | 0x28 | | |
| **Object.h** | **0x2c** (2013 value) | 0x20 (2013 value) | `and esp,-8` aligned frame, standard convention: **wrong** |

Descriptor.h was not tried. Its old form (`const Name*`) does not compile against the wave-11 users, and Object.h already accounts for the difference.

**Cause.** VC9 LTCG pads frames to keep the stack 8-byte aligned down a call chain whose root is aligned.
- 2013 roots the chain at `GlobalAdvancedSettingsItem<CRenderSettingsItem>::singleton()` (0x00407230, shell). The chain runs:
  1. `createInstance` 0x00409020, whose `sub esp,8` is only padding;
  2. `create` 0x0040d4f0;
  3. the CRenderSettingsItem ctor 0x004567d0, frame 0x20;
  4. the template ctor 0x00456ee0, frame 0x2c.
- Our shell has no caller of `singleton()`. 2013 has 17 shell call sites, e.g. 0x00403fa0, 0x004396c0, 0x004660c0, 0x004678e0, all unwritten.
- The old Object.h made `create` address-taken, which re-rooted the alignment there. It produced 0x2c, but broke `create` itself and the +30 wave-11 entries.

**Decision.** Object.h stays as it is. The fix is to write the shell's `singleton()` callers (shell owner). I also probed a stand-in TU calling `CRenderSettingsItem::singleton()` (`w/probe_singleton.cpp`). It did not restore 0x2c: our `singleton()` inlines `createInstance`, so the 2013 `createInstance` padding frame never appears. The real shell callers, or a matching `createInstance`, are needed.

`throw_exception<bad_placement_any_cast>` 0x0044d400 (frame 4 bytes larger) is very likely the same LTCG alignment effect. I did not probe it.

### 3. Util/FileSystem.h and the native callers of the shell's ATL copies

**`Util/FileSystem.h`** now has:
- `enum FileSystemDir { DirAppData, DirPicture, DirVideo, DirExe }`. The values come from 0x006ea4c0: `cmp edi,3` for the exe branch, and switches over 0/1/2.
- `std::string getUserDirectory(bool, FileSystemDir, const char*)`.

The shell agent already switched `LogManagerExterns.h` to include it.

**`util/FileSystem.cpp`** was rewritten from the 2013 bytes as the Windows FileSystem.obj. The asserts are laid out on 2013 lines 105 and 151. The Mac-carry `filepathExists`, `clearCacheDirectory` and one-argument `getUserDirectory` are kept unmarked.

| Function | PLAYER | STUDIO | Result |
|---|---|---|---|
| `VistaAPIs::~VistaAPIs` | 0x006ea0e0 | 0x007723c0 | **100 / 100**, FUNCTION |
| `getBaseCacheDirectory(bool)` | 0x006ea260 | 0x007725a0 | **100 / 100**, FUNCTION |
| `getUserDirectory(bool, FileSystemDir, const char*)` | 0x006ea4c0 | 0x00772800 | **100 / 100**, FUNCTION (after the ~CPathT nameref below) |
| `getCacheDirectory(bool, const char*)` | 0x006eab90 | 0x00772ed0 | **100 / 100**, FUNCTION |

Notes on FileSystem.cpp:
- `VistaAPIs` is defined at the top of the file: 2016's Win/VistaTools.h class, laid out per the 2013 dtor (`getKnownFolderPath` +0, `gShell32DLLInst` +4).
- Also gained: `CStringT::Delete` 0x006ea0f0 and the `cached` atexit dtor 0x00a96370.

Lessons:
- `if (create) if (!CreateDirectoryA(...) && ...)` and `if (create && !CreateDirectoryA(...) && ...)` schedule the `GetLastError`/`CreateDirectoryA` import loads in a different order.
- `!CPath(dir.c_str()).FileExists()` is the 2013 form; `PathFileExistsA(CString(...))` is not.

**`util/Http.cpp`** has the 2013 Windows bodies. The ATL headers are included after line 100, so the line-100 assert stays.

| Function | PLAYER | STUDIO | Result |
|---|---|---|---|
| `FUN_00622e00` (call_once target: clears `g_unk0x00de3e50`) | 0x00622e00 | 0x006cb500 | **100 / 100**, FUNCTION |
| `Http::trustCheck` (CUrl scheme switch, `res://` checks, `CString::Format("res://%s")`) | 0x00624210 | 0x006cd090 | **100 / 100**, FUNCTION |
| `Http::isMoneySite` (paypal.com / rixty.com host suffix) | 0x00623bb0 | 0x006cca30 | 85.1, STUB (2013 frame is 4 bytes larger; the two `Right()` temporaries get separate slots) |
| `Http::trustCheckBrowser` | 0x00624620 | 0x006cd530 | 94.1, STUB (2013 tail is `test al,al; setne al` after `isMoneySite`; `\|\|` and `!= 0` forms are both wrong) |
| `FUN_006238a0`, `FUN_00623a90`, `FUN_00623da0` (the three url tests trustCheck calls) | as named | 0x006cc5f0 / 0x006cc910 / 0x006ccc20 | STUB stand-ins, `__declspec(noinline)` |

Also gained: `CStringT::Right` 0x00623230, `Replace` 0x00623640, `MakeLower` 0x00623840, `CSimpleStringT(PCXSTR,int,mgr)` 0x00622ee0.

**`util/ContentProvider.cpp`**, appended at the end so no assert moves. This is the cache cleanup thread body: the ctor binds `(&fn, 604800, &member+0x84)`.

| Function | PLAYER | Result |
|---|---|---|
| `FUN_0065ff50` (seconds between two SYSTEMTIMEs) | 0x0065ff50 | byte-identical in probe_compare, 96.7 in reccmp: the 2013 `push 10000000` (0x00989680) lies inside its `.text`, so without `.reloc` it reads as an address. Kept STUB |
| `FUN_00664260` (FindFirstFile loop, FLog::ContentProviderCleanup "Deleting old file number %u") | 0x00664260 | 61.9, STUB (frame 0x28 larger; the two duplicated branches are probably one inline helper in 2013) |
| `FUN_00661a30` (cache path) | 0x00661a30 | STUB stand-in |

**Shell entries that rose from these native callers.** These are the ones shell-5k-2 listed as blocked by ATL conventions.

PLAYER:
- **Format and its helpers:** `CStringT::Format` 0x00441660, `FormatV` 0x004419d0, `CloneData` 0x004414f0, `operator+` 0x00440db0, `CPathT::Append` 0x00436b10, `AddBackslash` 0x00440f00, `GetBuffer` 0x0041b960, `Concatenate` 0x00441110, `CPathT()` 0x00406b10.
- **MainLogManager:** `getCrashEventName` 0x0043ee20, `getLogFileName` 0x0043efb0, `getFastLogFileName` 0x0043f0f0, `MakeLogFileName` 0x0043f230, `WriteCrashDump` 0x0043f730, `handleDebugAssert` 0x00440850, `handleFailure` 0x00440920.
- **Other shell:** `ThreadLogManager::getLogFileName` 0x0043f380, `purecallHandler` 0x0043f9b0, and the ReflectionMetadata singleton `safe_static_do_get_singleton` 0x00447970 (was 91.9).

STUDIO: the same set, at 0x0041cb00–0x0041e2c0 and 0x0041f0f0/0x0041f430. It includes `fastLogMessage` 0x0041c790 and `getSessionId` 0x0041c8e0.

**`include/Util/Http.h`:** the TEMPLATE nameref for 0x00436af0 / STUDIO 0x0041b750 is now the `~CPathT` name.
- Once FileSystem.obj references `~CPathT`, our shell copy carries three folded names (`~CSimpleStringT`, `~CStringT`, `~CPathT`), and reccmp reports it as `~CPathT`. The `~CStringT` nameref then stopped pairing: 0x00436af0 "gone", and every caller at 99.4/97.9.
- With the `~CPathT` name: +4 per target (the dtor, `getUserDirectory`, `trustCheck`, and `httpGetPostWinInet` back to 100), 0 lost.
- The comment line explains the fold; the line count is unchanged.

### 4. Type<Region3int16> / getSingleton<Region3int16>: the new `voxel/Grid.cpp`

- The 2013 `__FILE__` is `Client\App\voxel\Grid.cpp`; the wine path lists already had it.
- In both targets `getSingleton<Region3int16>` is the last function of Grid.obj (PLAYER 0x00793b60, after the Grid code from 0x00792cb0; BasicPartInstance.obj follows). Wave 11's "next to BasicPartInstance" was that same spot.
- The new TU holds only that specialization, including `Reflection/Type.h`.
- EnumProperty.cpp keeps a declaration and the `TypeRegistrar`. Four comment lines keep its later lines in place.

Results:

| Function | PLAYER | STUDIO |
|---|---|---|
| `getSingleton<Region3int16>` | 0x00793b60 **100** | 0x00847020 **100** (FUNCTION both) |
| `Type<Region3int16>` ctor | 0x00793a90 96.8 → **100** | 0x00846f50 → **100** |

Removing the function-local static renumbers EnumProperty.obj's `$S` guard variables. On the stale shared CSVs this cost 11 getSingleton<T> entries (0.913); they all came back with the regenerated CSVs.

### 5. Name::operator< compares sortIndex (Util/Name.h)

- The 2013 `MemberDescriptorContainer<T>::compare` (one folded copy, PLAYER 0x00614cd0, passed to `_Lower_bound` from 0x00617b70) is `mov ecx,[a+0xc]; ...; cmp [ecx],[eax]; setl`: a signed compare of `Name+0`, which is `sortIndex`.
- `operator<` is now `sortIndex < other.sortIndex`, and our compare is byte-identical to 0x00614cd0 (probe_compare).
- PLAYER P5 vs P4 on the same CSVs: 0 gained, 0 lost (`runs/P5a.json`, `runs/P4z.json`).
- STUDIO S4 vs S3 on the same CSVs: 0 gained, 0 lost (`runs/S4a.json`, `runs/S3z.json`). Kept: the change is neutral now and matches the 2013 bytes.
- compare has no marker yet. It needs a TEMPLATE marker in reflection/member.h, but adding lines there moves member.h's asserts. Lay the header out for it first.

### 6. Not done (time)

- The Variant converts in EnumProperty.cpp, still at wave 11's numbers.
  - `convert<int>` 80.6: the remaining diffs are register choice in the `isType` loads (`mov eax,[esi]; mov eax,[eax+0x10]` in 2013 vs ours through ecx) and the iRound temp store order.
- ObjectBridge::newInstance (67.5).
- The large unnamed groups (ScriptContext, DataModel, Humanoid, EnumDesc<>).

## Unresolved

1. **Shell owner: the 0x00456ee0 frame.** It needs the shell's `GlobalAdvancedSettingsItem<CRenderSettingsItem>::singleton()` callers, and a `createInstance` that stays out of line (2013 0x00409020, 26 bytes, `sub esp,8` padding). Do not revert Object.h.
2. **Shell / generator:** `CStringT(const wchar_t*)` 0x00440d20 (59.2). Our only copy is FileSystem.obj's native one, because no shell TU of ours instantiates it. In 2013 a shell function owns it.
3. **ContentProvider STUDIO markers** for FUN_0065ff50, FUN_00661a30 and FUN_00664260 are not added; the STUDIO addresses were not located.
4. **reccmp-decomplint:**
   - Http.cpp: STUDIO `urlEncode` 0x006ca1b0 now follows `trustCheckBrowser` 0x006cd530. STUDIO puts urlEncode before the trust checks, PLAYER after them (0x00629c80), so one target keeps a descent.
   - ContentProvider.cpp: 0x0065ff50 at the end of the file joins the file's six existing descents.
5. **reccmp/no-.reloc:** FUN_0065ff50's `push 10000000` reads as an address in the original.
6. **IDA owner (names):**
   - 0x006ea260 `RBX::FileSystem::getBaseCacheDirectory(bool)`, 0x006eab90 `getCacheDirectory(bool, const char*)`, 0x006ea0e0 `VistaAPIs::~VistaAPIs`, 0x006ea0f0 `CStringT::Delete`.
   - 0x00623bb0 `Http::isMoneySite`, 0x00624210 `Http::trustCheck`, 0x00624620 `Http::trustCheckBrowser`, 0x00623230 `CStringT::Right`, 0x00623840 `CStringT::MakeLower`, 0x00623640 `CStringT::Replace`, 0x00614cd0 `MemberDescriptorContainer<T>::compare`.
   - 0x00407230 `GlobalAdvancedSettingsItem<CRenderSettingsItem>::singleton`, 0x00409020 its `createInstance`.
   - STUDIO: 0x007723c0 / 0x007725a0 / 0x00772800 / 0x00772ed0, 0x006cca30, 0x006cd530.
7. **Integrator:**
   - Regenerate the shared CSVs: the new markers and the EnumProperty `$S` guard renumbering need it.
   - The intrusive_ptr<thread_data_base> loss comes from Network's new CrashReporter.cpp (COMDAT order), not from App.

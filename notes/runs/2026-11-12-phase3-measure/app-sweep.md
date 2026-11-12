# app-sweep (workflow phase3-measure, wf_271afd88-e3a)

**The measured App figure is 90 of 1,036 PLAYER markers at 100% (86 of 1,016 for STUDIO), about 0.4% of the App code, not the claimed ~55%.** The kept set held after a rebuild and rescore: every `// FUNCTION:` marker is still effective 100%, and none sits below it.

`tools/app_sweep.py` is the new script. `set FUNCTION|STUB` flips every in-scope marker one way. `apply P.json S.json` makes a marker FUNCTION only if it scored 100% for that target. It never touches Client/_generated, and it only changes the marker word, so line numbers don't move. Its other commands are `report`, `xrefs`, `order` and `selftest` (which passes). The docstring gives each exact command.

I ran three full sweeps (flip everything, build, score `--json`, apply), then one confirming rebuild and rescore. Outputs are in build/sweep/.

**1. Measured figures**

| | PLAYER | STUDIO |
|---|---|---|
| Original markers | 1,034 | 1,014 |
| Kept as FUNCTION (original markers) | 89 | 85 |
| Plus ReleaseAssert (new marker, 100%) | 1 | 1 |
| Total FUNCTION | 90 | 86 |
| Unique addresses at 100% | 92 of 992 | 88 of 980 |
| reccmp `--nolib` implemented (was 12 / 14) | 101 / 38,580 | 99 / 40,441 |
| reccmp with libraries | 1,930 / 40,468 (97.37%) | 1,817 / 42,329 |

- **Unique PLAYER marker addresses by score:** 92 at 100%, 2 at 99–100%, 41 at 95–99%, 48 at 90–95%, 165 at 75–90%, 241 at 50–75%, 348 below 50%, and 55 unpaired. 937 were paired.
- **The four 100% addresses with no FUNCTION** (92 vs 89+1 kept): six addresses in each target score 100% but stay STUB, because the linker folded several trivial bodies onto them and the marker can't be pinned to one definition. In PLAYER these are 0x00575d00, 0x006be550, 0x006c8740, 0x006ceaa0, 0x00962a50 and 0x00992a80. Four of them have no FUNCTION marker at all, and two keep FUNCTION only on the marker whose name matches the recompiled symbol.
- **28 of the 90 kept markers** sit on explicit specializations (`template<>`, for example the Bridge on_index functions), so FUNCTION is the right marker for them.
- **How I defined "the App portion of PLAYER":** I placed each function's `C:\BuildAgent\...\Client\<lib>\` assert strings along `.text`. App is one contiguous block, 0x00606a00–0x009ad3a0, between the last Network-string function and DeviceCaps. That block holds 20,304 function-size rows, 19,665 of them not library (Lua sits inside it and is excluded as library).
  - 85 FUNCTION markers fall inside the block: 85 / 19,665 = **0.43%**.
  - Every placed marker inside it, regardless of score, covers only 924 / 19,665 = 4.7%.
  - App code that the linker placed elsewhere (inline and template copies at 0x407000–0x5bc000) is not counted.
- **Against the claimed ~55%:** even if every imported definition matched, the tree would reach about 5% of App by function count.

**What I changed to get there**
- **Pairing fixes.** Each one is backed by the function that calls it at the matching spot, and checked against the original bytes:
  - **ReleaseAssert:** `// STUB:` marker on its definition in Client/Base/util/Debug.cpp, at PLAYER 0x009b19c0 / STUDIO 0x00a68ab0. The original body is exactly `channel==255 ? RBXCRASH(msg) : FLog::FastLog(channel,msg,0)`, and it now scores 100%.
  - **RBXCRASH(const char*):** STUB marker in Client/Base/util/RbxCrash.cpp at 0x009b9090 / 0x00a68e10. Both originals are a 5-byte tail jump to `Debugable::doCrash`. It scores 0% only because doCrash is not paired.
  - **`RBX::_internal::_debugHook`:** GLOBAL marker in Client/Base/RbxAssert.cpp at 0x00e1aac8 / 0x00f9dcb8.
  - **29 `Bridge<T>::className` definitions:** GLOBAL markers with a name line in LuaAtomicClasses, LuaEnum, LuaSignal, LuaLibrary, ThreadRef and LuaInstanceBridge. Each original slot points at exactly the source's string ("CFrame", "EnumItem", and so on). 22 are also confirmed by the functions that load them; 7 rest only on sitting in the same table with the matching string.
  - datacmp: PLAYER variables went 173 → 203 and STUDIO 160 → 190; issues stayed at 9 in each, so none of the new globals introduced an issue.
- **Result of the fixes:** PLAYER addresses at 100% went 70 → 93 (STUDIO 66 → 89).

**2. Near misses (90–99.99%)**
- notes/app-sweep.csv has one row per marker (2,052 rows): score, effective flag, diff shape, every tag, and a `pairing_only` flag. A row with `pairing_only=1` differs only in references that are not yet paired.
- **PLAYER, 91 near misses:**

| Shape | Count |
|---|---|
| unpaired callee | 39 |
| other | 22 |
| assert file/line | 20 |
| data reference | 8 |
| x87 vs SSE | 1 |
| inlining (size) | 1 |
| stack cookie | 0 |
| ebp frame | 0 |
| register allocation / scheduling (as main shape) | 0 |

  - 47 of the 91 are `pairing_only`.
  - By directory: App/script 48, App/util 20, App/v8datamodel 9, App/include 5, App/v8world 5, Base 3, Network 1. STUDIO is the same within one function.
- **No near miss looks like a flag problem.** None has a stack cookie, an ebp frame or a real x87/SSE split. Across all paired functions, the 199 ebp-frame diffs go both ways (87 only in the original, 56 only in ours, 40 in both). They and the 30 x87/SSE diffs are all in functions below 90%, so they come from code or pairing differences, not flags.
- **Root causes I found but could not fix in what I own:**
  - **(a) A throw-info table is not paired.** `__TI2?AVruntime_error@std@@` (PLAYER 0x00cd66cc, STUDIO 0x00e292d4) is referenced from 15 functions. It is named in names.csv with source `ida:`, but no reccmp CSV row carries it.
  - **(b) symbols.csv name rows don't pair with PDB names.** They use Itanium-style signatures like `RBX::Face::minMax(G3D::Vector3 const&, ...) const`, which never equal the recompiled names. Examples: Face::minMax, Instance::setRobloxLocked, VerbContainer::getVerb, LegacyLock::Implementation::~Implementation and Debugable::doCrash. The `std::logic_error` string constructor (0x0041bbe0 / 0x00403b60) is in no CSV.
  - **(c) Assert file and line strings.** The original's `__FILE__` is `C:\BuildAgent\work\407e17641b466b38\Client\App\Util\Math.cpp`; ours is `O:\Client\App\util\Math.cpp`, which also differs in directory case. STUDIO's checkout is fa0a2141fe3d2f4d, so one build cannot match both targets' strings. `__LINE__` reflects the source file's layout, and the original assert strings record the 2013 line numbers (for example Math.cpp lines 1308, 1343 and 1420).
  - **(d) Class layouts from 2016 are wrong for 2013.** Examples: a DataModel member at +0xbe5 in 2013 vs +0x1395 in ours, ScriptContext +0xf1 vs +0x260, GenericJob size 0x248 vs 0x268, an AssemblyStage allocation 0x28 vs 0x2c, and MovingAssemblyStage onEdgeRemoving reading +0x10 vs +0xc.
  - **(e) G3D headers seen by App code differ from 2013.** The original calls `G3D::Matrix3::operator=` (23 calls in 17 functions), `Vector2::zero` (8 calls in 5) and `Vector3::zero` out of line, and copies floats with `fld/fstp`; our headers inline these. This belongs to the G3D owner.
- notes/app-sweep-xrefs.csv (366 rows) lists every remaining unpaired reference with its original address and how many functions agree on it.

**3. Out-of-order files (report only; nothing reordered)**
- decomplint still reports 336 PLAYER and 341 STUDIO `function_out_of_order`.
- 117 files carry markers; 100 have descents; 35 have both descents and 100% functions.
- **A 100% score confirms that marker's pairing** for every function body over 16 bytes. Bodies of 16 bytes or less (the folded getters and empty virtuals) match their bytes but don't prove which definition they belong to.
- **Reordering to ascending PLAYER order is consistent with STUDIO order in 85 of the 100 files** with descents: after sorting by PLAYER address, STUDIO addresses ascend too. These include ScriptContext.cpp, ThreadRef.cpp, LuaEnum.cpp, GUI.cpp, LuaSignal.cpp, Math.cpp, DataModel.cpp and SimulateStage.cpp.
- **15 files are inconsistent** (STUDIO inversions after sorting by PLAYER): LuaAtomicClasses.cpp 408, PartInstance.cpp 147, LuaArguments.cpp 36, IAdornable.h 26, IPipelined.cpp 15, Body.cpp 9, Sound.cpp 7, Item.h 2, and NormalId.cpp, UnifiedWidget.h and SpanningTree.h 1 each. These are template or inline code the linker places differently, or STUDIO's different source line. For those files ascending PLAYER order is not consistent with STUDIO.

**4. Checks (run on the final build)**

| Check | PLAYER | STUDIO |
|---|---|---|
| reccmp-decomplint (exit 1) | 336 function_out_of_order, 42 duplicate_offset, 3 unicode_decode_error (unchanged) | 341 / 34 / 3 |
| reccmp-vtable (exit 1) | 148 found, 66 not matching (unchanged) | 105 found, 44 not matching (unchanged) |
| reccmp-datacmp (exit 1) | 203 variables, 9 issues | 190 variables, 9 issues |
| check_folded (exit 0) | 981 addresses, 0 problems | 969 addresses, 0 problems |
| reccmp_addr_padding (exit 0) | 1,649 files, 0 badly padded markers | |

reccmp_addr_padding needs the toolchain venv's python; with system python3 it fails on `import reccmp`.

## Structured fields

```json
{
 "files": [
  "tools/app_sweep.py",
  "notes/app-sweep.csv",
  "notes/app-sweep-xrefs.csv",
  "Client/Base/util/Debug.cpp",
  "Client/Base/util/RbxCrash.cpp",
  "Client/Base/RbxAssert.cpp",
  "Client/App/script/LuaAtomicClasses.cpp",
  "Client/App/script/LuaEnum.cpp",
  "Client/App/script/LuaSignal.cpp",
  "Client/App/script/LuaLibrary.cpp",
  "Client/App/script/ThreadRef.cpp",
  "Client/App/script/LuaInstanceBridge.cpp",
  "build/sweep/PLAYER-final.json",
  "build/sweep/STUDIO-final.json",
  "build/sweep/PLAYER-final-nolib.json",
  "build/sweep/STUDIO-final-nolib.json",
  "build/sweep/order-final.csv",
  "build/sweep/lint-PLAYER.txt",
  "build/sweep/lint-STUDIO.txt",
  "build/sweep/vtable-PLAYER.txt",
  "build/sweep/vtable-STUDIO.txt",
  "build/sweep/datacmp-PLAYER.txt",
  "build/sweep/datacmp-STUDIO.txt"
 ],
 "unresolved": [
  "Folded duplicate markers: 6 addresses per target score 100% but carry several markers, so they stay STUB and need FOLDED handling. PLAYER: 0x00575d00, 0x006be550, 0x006c8740, 0x006ceaa0, 0x00962a50, 0x00992a80. STUDIO: 0x0078ad10, 0x007a06a0, 0x007a6470, 0x007aa330, 0x00812790, 0x00892cf0.",
  "tools/ida/gen_reccmp_csv.py (not mine) should emit a data row for __TI2?AVruntime_error@std@@ (PLAYER 0x00cd66cc, STUDIO 0x00e292d4; 15 near-miss references) and a row for the std::logic_error string constructor (0x0041bbe0, 0x00403b60).",
  "symbols.csv name rows with Itanium-style parameter lists never pair with the recompiled PDB names, which leaves many callees unpaired. Examples: Face::minMax 0x008fc9f0, Instance::setRobloxLocked 0x0061d780, VerbContainer::getVerb 0x006f90d0, Debugable::doCrash 0x009b1a00 / 0x00a68af0. Pairing doCrash would bring RBXCRASH (0x009b9090) to 100%.",
  "Assert __FILE__ strings: the original uses C:\\BuildAgent\\work\\407e17641b466b38\\Client\\... (PLAYER) and fa0a2141fe3d2f4d (STUDIO), with directory case like App\\Util and V8DataModel; ours is O:\\Client\\... Matching it needs a build-path change in winewrap (not mine), and one build cannot match both targets. __LINE__ needs the 2013 source layout, which the original assert strings record.",
  "G3D headers used by App code: the 2013 build calls Matrix3::operator=, Vector2::zero, Vector3::zero and Color4::one out of line and copies floats with fld/fstp; ours inline them. This is for the G3D owner's patches.",
  "Wrong 2016 layouts block several near misses: DataModel (+0xbe5 vs +0x1395, GenericJob size 0x248 vs 0x268), ScriptContext (+0xf1 vs +0x260), LibraryService, GcJob, AssemblyStage (0x28 vs 0x2c), MovingAssemblyStage. ScriptContext::sandboxThread returns with ret where ours uses ret 4.",
  "The App portion of PLAYER (0x00606a00\u20130x009ad3a0, 19,665 non-library functions) comes from bracketing assert __FILE__ strings, so it is approximate; App code the linker placed elsewhere (0x407000\u20130x5bc000) is not counted.",
  "Seven of the 29 className GLOBAL markers (Color3, Vector2, Region3int16, Vector3int16, Vector2int16, UserInputState, KeyCode) rest only on sitting in the same table with the matching string, not on a referencing function. datacmp reports no issue for any of the new globals."
 ]
}
```


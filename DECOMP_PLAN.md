# Project OGRE: Decompilation Plan

Project OGRE's plan for a matching decompilation of the October 2013 Windows Roblox clients, following the method [OpenRBX](https://github.com/Asphaltian/OpenRBX) used for the December 2007 client. Rules live in `CLAUDE.md`. This file says what to do in what order, and records status. Update the **Status** line of each phase as it moves.

## How 2013 differs from 2007

| | OpenRBX (Dec 2007) | This project (Oct 2013) |
|---|---|---|
| Disassembler | Ghidra + PDB loader | IDA Pro 9.5, `.i64` per binary |
| Name source | `WebService.pdb`, unstripped, from RBXGS | Mac Studio build symbols (+ iOS), no Windows PDB |
| Layout source | PDB TYPES records (exact) | Windows constructors (exact), Mac DWARF if present (order only), 2016 source (hypothesis) |
| First target | WEBSERVICE (has PDB), then transfer to ROBLOX | PLAYER, then transfer to STUDIO |
| Compiler | VC++ 8.0 SP1, known | Unknown until the Rich header is read |
| Flags per library | Read from PDB command blocks | Inferred from 2016 `.vcxproj`, proven from the bytes |
| Third party | G3D 6.09, boost 1.34.1, Lua 5.1.1, RakNet 3.0, Codejock, Ogre, SDL, FMOD | **RakNet 4.0** (not 3.0: a different API and a different codebase, so none of OpenRBX's reconstructed `raknet30` transfers). The rest is to be inventoried; expect Qt (Studio), libcurl, OpenSSL and others |
| Starting point | Empty tree | **`App/` already ~55% decompiled for 2013** (the user's prior work) |

**The existing App decomp changes the order of work.** App is the biggest engine library and the one everything else depends on. With over half of it already written, the core object model (Phase 4) and the App-side subsystems (Phase 5) become *verify and complete* jobs instead of *start from nothing*. The two new steps are importing the tree into the build (Phase 2) and measuring it honestly (Phase 3).

The biggest risk is the one 2007 did not have: **no PDB means nothing records the flags, the source paths or the struct layouts directly.** The second biggest is **LTCG**: if most of the engine was built `/GL`, instruction-exact matching is only reachable late. Phase 0 settles both before any C++ gets written.

---

## Phase 0: Inventory and feasibility

**Status:** done 2026-11-11. Facts are in `CLAUDE.md` → *Facts table*; raw dumps in `notes/pe-facts.txt` (`tools/pe_facts.py`), paths in `notes/paths-2013.txt` (538 from PLAYER/STUDIO) and `notes/paths-2013-mac-ios.txt` (1,579).

**Findings**

- **Toolchain: VS2008 SP1** (`cl` 15.00.30729, linker 9.0, `/MD` against `msvcr90`/`msvcp90`). Not VS2012 as the existing App decomp assumed.
- **LTCG: go.** 40 of ~860 objects in PLAYER (4.7%) and 127 of ~940 in STUDIO (13.5%) are `/GL`. The engine is overwhelmingly plain `/O2` objects. Which objects are LTCG is a Phase 1 question (the Studio delta of 87 suggests the Qt shell).
- **Protection: both Players are VMProtect-wrapped, STUDIO is not, and the damage is small.** 26 SDK-marker regions in 26 intact functions plus one stripped function, 11,351 bytes (0.16% of `.text`), the same 27 functions in both Players (`notes/vmprotect-player.csv`). They are anti-cheat paths (memory checkers, debugger checks, `executeInNewThread`, replicator ping/send jobs); they come out of the denominator. `.text` calls through the original IAT at `0x00aa1000` in `.rdata`, not the `.vmp1` loader table.
- **No `.reloc`** in either target (fixed base 0x400000).
- **Own DLLs** (each has a PDB path under the build agent's `Client\`): `Log.dll` (`Client\Log`), `boost.dll` (`Client\boostlibs`), `OgreMain.dll` + `RenderSystem_Direct3D9/GL` + `Plugin_CgProgramManager/ParticleFX` (`Client\Rendering\ogre-v1-6-4`), `RobloxProxy.dll` and `NPRobloxProxy.dll` (`Client\Installer`, PLAYER folder only), `CrispRMF.dll` (STUDIO only). No launcher exe ships in either folder.
- **Third party:** boost 1.53.0 (partly in `boost.dll`), **Ogre 1.6.4** as DLLs (the renderer is still Ogre), FMOD Ex 4.32.05 (`fmodex.dll`), Intel TBB 2.1 (`tbb.dll`, 2008-06-05), NVIDIA Cg 2.1.0017, **Lua 5.1.4** (confirmed by the user; the binaries record the `lua-5.1.4` source path), zlib **1.2.3 and 1.1.4** both linked into the exe, RakNet 4.x (`Client\Network\raknet\Source`; the exact 4.0xx is not in a string. The 2016 tree has 4.036 dated 2012-02-01, a hypothesis to confirm by body matching), Qt 4.8.5 + QtWebKit 4.9.4 + phonon 4.4 (STUDIO), OpenSSL 1.0.0c DLLs (STUDIO only), VMProtect SDK, MFC 9/OpenMP 9 redistributables. HTTP is WinHTTP/WinInet, **no libcurl**. Bullet is not linked in any October 2013 build. Full table in `CLAUDE.md` (Third party, pinned).
- **Mac oracle is weaker than planned, iOS is stronger.** Mac Studio: i386, externals-only symtab (10.6k RBX names), no DWARF. iOS: armv7, unencrypted, full symtab with locals (20.5k RBX names each). Phase 1's name transfer should lead with iOS and use Mac (same x86 architecture) for body similarity.
- **2016 flags hypothesis for App Release:** `/O2 /Ob2 /Oi /Oy- /GS- /arch:SSE2 /fp:fast /GR /MD /D_SECURE_SCL=0`, precompiled `stdafx.h`, no `/GL`.
- **Source tree** matches the 2016 layout: `Client\App\{gui,humanoid,reflection,script,security,tool,util,v8datamodel,v8kernel,v8tree,v8world,v8xml,voxel,include}`, `Base`, `Network`, `Rendering\{AppDraw,g3d,GfxAdapters,GfxBase,RbxG3D,RbxOgre}`, `ClientBase`, `ClientShared`, `Win`, `WindowsClient`, `RobloxStudio`. Directory case is inconsistent in the originals (`V8DataModel` vs `v8datamodel`).

**Target decision (2026-11-11).** After the inventory the user supplied a later Player, 0.126.0.39503 linked 2013-10-23 (`bin/player_1023/`), labelled as the build matching the symbolled Mac Studio (10-23) and a symbolled iOS 2.126.39527 (10-29, `ref/ios_1029/`), and approved IDA 9.4. The assert-line measurement in `CLAUDE.md` confirms that family and shows Windows STUDIO 10-17 is on a different source line. So: **PLAYER = the Oct 23 build** (assistant's reading of the user's labels; the user has not said it in words), the Oct 15 build is `PLAYER_1015`, and the VMProtect-redirected functions are excluded from the denominator. Still open: which Player the existing App decomp's `// 0xADDRESS` comments refer to (probing two addresses from `HANDOFF.md` was inconclusive; pair by body in Phase 2).

**Goal:** fill in the facts table in `CLAUDE.md`, and decide whether exact matching is feasible.

1. **Lay out the workspace.** Originals go in `bin/` (Windows) and `ref/` (Mac, iOS, 2016 source), all gitignored. Record each file's SHA256 in `CLAUDE.md`. Extract the `.ipa` files (they are zips: `Payload/*.app/`) and the 2016 `.7z` into their own directories under `ref/`. Do not build or run anything from inside them.
2. **List every folder's contents.** List each Windows folder in full: the main exe, any launcher (`RobloxPlayerLauncher.exe`?), DLLs shipped beside it (Qt `QtCore4.dll`/`QtGui4.dll`? `msvcr110.dll`? `fmodex.dll`?), content folders. Note which DLLs are Roblox's own and which are third party. Every Roblox-built DLL is a potential extra target.
3. **Identify the toolchain.** Parse the Rich header (`pefile`, or any Rich header dumper) of each Roblox PE. It lists `prodid:build` counts per tool: the C++ compiler build (`Utc1600_CPP` = VS2010, `Utc1700_CPP` = VS2012, …), any `*_LTCG_*` entries, MASM, the linker and the resource compiler. **The build number, not just the version, picks the toolchain** (RTM vs SP1 vs Update N). Record it.
4. **Check feasibility.**
   - LTCG: how many objects in the Rich header are LTCG versus plain C++? If the engine is mostly `/GL`, write that up and **stop for a decision** (see *Fallback* at the end).
   - Protection: section names, entropy and the entry point. A packed or virtualized `.text` is a stop-and-ask.
   - CRT: imported (`/MD`, `msvcr1x0.dll`) or static (`/MT`).
   - `/DYNAMICBASE` and `.reloc` (reccmp needs relocations to mask addresses).
5. **Characterize the Mac build.** Run `file` and `otool -hv` (i386, x86_64 or fat), `nm -m | wc -l` (how many symbols), and check for embedded DWARF (`dwarfdump --debug-info | head`) or a separate `.dSYM`. Note the clang version from `__TEXT,__cstring` or the build-version load command. **The answer decides how strong the name oracle is.**
6. **Characterize the iOS builds.** Same checks. Note armv7, armv7s or arm64, and whether they are encrypted (`cryptid` in `LC_ENCRYPTION_INFO`; an App Store `.ipa` is encrypted and only its plist and resources are usable).
7. **Pin engine revisions.** Find version strings in every binary (e.g. `0.1xx.0.xxxx`). Note the Player-Studio gap and the iOS bracket.
8. **Inventory third-party code** from strings: boost (`boost::` RTTI, version macros), G3D, Lua (`$Lua: Lua 5.1...`), RakNet 4.0 (pin the exact 4.0xx release from its strings), FMOD, Qt (`Qt 4.x.x`), libcurl (`libcurl/x.y.z`), OpenSSL (`OpenSSL 1.0.xx`), zlib (`deflate 1.2.x`), libpng, IJG, Ogre (is it still there, or replaced by the newer renderer?), Direct3D 9/11 and OpenGL imports, SDL, Bullet, Boost.Asio, TinyXML/RapidXML, Poco. Exact versions decide which source to vendor.
9. **Recover the source tree** from `__FILE__` strings in asserts and logs (`grep -a -o '[A-Za-z]:\\\\[^"]*\.\(cpp\|h\)'` style sweeps). Compare it with the 2016 tree. The 2013 paths are the directory layout to reproduce.

**Exit criteria:** the facts table is complete; compiler and toolchain are named; there is an LTCG/protection go or no-go; the third-party list has versions; the 2013 source path list is saved to `notes/paths-2013.txt`.

---

## Phase 1: IDA databases (and the first i64s)

**Status:** steps 1, 2, 4 and 6 done for PLAYER and STUDIO (2026-11-12); step 3 done by FLIRT for the VC9 CRT/STL/ATL and by `.obj` body matching for the third-party libraries (`tools/lib/`, snapshot `-04-pre-libs`, PLAYER 1,833 library names); step 5 not started. Reports: `notes/runs/2026-11-12-phase1-resume/`, sessions in `ida/LOG.md`.
- Function discovery repaired the import table and EH funclets (PLAYER 48,868 functions, 41,977 in `.text`). RTTI and vftables named in all three Windows databases (snapshot `-02-pre-rtti`).
- VC9 SP1 FLIRT sigs (`toolchain/sig/`, `tools/ida/apply_flirt.py`): 97 PLAYER, 26 STUDIO functions.
- Name transfer (`tools/pair_names.py`, verified by four adversarial reviews, then rule fixes and an ABI gate): **PLAYER 6,712 named** (6,505 `ios:`, 207 `mac:`) + 318 `guess:` comments; **STUDIO 6,820** + 188, helped by 20,115 masked body matches against PLAYER (`tools/body_match.py`). Snapshots `-03-pre-names`.
- **1,328 of the 2,733 App-decomp functions now have a PLAYER address** (`ida/exports/PLAYER-app-decomp-map.csv`).
- Coverage of `.text` functions over 32 bytes is about 20% named. The rest needs third-party library matching (boost, G3D, Lua, RakNet, zlib/IJG/libzip) and more evidence classes, not more oracle work.

**Goal:** one clean, analyzed, saved `.i64` per binary, with library code labeled and the names transferred from the Mac build.

1. **Create the PLAYER database.** Load `bin/RobloxPlayerBeta.exe` (or the real name) in IDA 9.5 as a PE, with manual load off, let auto-analysis finish, and apply the FLIRT signatures for the exact MSVC toolset from Phase 0 (CRT, STL, EH). Make sure RTTI is parsed (IDA's MSVC RTTI handling, or a Class Informer-style script if it misses any).
   → **Save, pack, snapshot `ida/snapshots/PLAYER-00-autoanalysis.i64`.**
2. **Repeat for STUDIO, MACSTUDIO and each IOS build**, each with its own `.i64` and `-00` snapshot. For the Mac database, confirm the symbols loaded (function names, not `sub_`) and DWARF if present.
3. **Build signature libraries for third-party code.** Compile each identified third-party library (exact version, exact MSVC toolset, best-guess flags from the 2016 projects) and run FLAIR (`pcf` → `sigmake`) to produce `.sig` files, then apply them. Iterate the flags until the hit rate stops climbing. The winning flags are evidence for `CMakeLists.txt`. Snapshot as `-01-flirt`.
4. **Transfer names from the Mac build into PLAYER** (snapshot `-02-mac-names` first). Anchor in this order, logging the source of each name:
   1. **RTTI classes:** map each Itanium `_ZTS`/`_ZTV` class to its MSVC `.?AV` descriptor and vftable. This names every vftable and, with the destructor-slot offset and overload-order caveats in `CLAUDE.md`, most virtual functions.
   2. **Unique strings:** a function that is the sole referrer to a unique string in both builds is the same function.
   3. **Call graph propagation:** from paired functions, pair their unique callees and callers.
   4. **Similarity:** Diaphora (it supports IDA 9) or BinDiff across MACSTUDIO→PLAYER, accepting only high-confidence matches, as `guess:` until confirmed by 1–3.
   Use iOS as a third vote where Mac and Windows disagree.
5. **Apply first-pass types.** Make the 2016 headers parseable (strip what IDAClang chokes on), import the class declarations as local types, and apply them **only** to classes whose 2013 vftable slot count agrees. Everything else gets a stub struct sized from its constructor's allocation (`operator new(0x...)`).
6. **Export** to `ida/exports/`: names, functions, types. Commit the exports and `tools/ida/*.py`.
   → **Save, pack, snapshot `-03-phase1-done`, log it.**

**Exit criteria:** every database exists and opens cleanly. PLAYER's names CSV carries a name source for every named function. The coverage numbers (% named by Mac, by RTTI, by FLIRT, unnamed) are recorded here.

---

## Phase 2: Repository and build skeleton

**Status:** steps 1–7 done 2026-11-12 except CI (no git repo yet) and green checks. reccmp `a0f494c0` plus one local patch. The App tree is imported, 397 of 400 non-vendored TUs compile (2 out of the build as 2016 leftovers: `v8xml/XmlSerializer.cpp`, `util/StreamRegion.cpp`), and it links into both exes by default (`OGRE_PRIOR_APP=ON`, `/FORCE:UNRESOLVED` for ~40 symbols, generated stubs in `Client/_generated/`). Prior names are in both databases (`prior:`, snapshot `-05-prior-app`). Checks: `check_folded` and padding pass; `decomplint` (336 out-of-order, 42 duplicate offsets), `reccmp-vtable` (66 of 148 not matching) and `datacmp` (9 issues) fail. Reports: `notes/runs/2026-11-12-phase2-skeleton/`, `notes/runs/2026-11-12-phase3-measure/`.

**Goal:** a CMake project that builds an empty-but-linking PLAYER exe with the original toolchain, and a reccmp setup that scores it.

1. **Get the toolchain.** Get the exact MSVC build from Phase 0 (portable copy, as OpenRBX did with MSVC800-SP1), plus the matching Windows SDK and DirectX SDK of the era. Pin it in the README. Gate decomp-specific flags behind a `MSVC_FOR_DECOMP` check on the compiler version, as OpenRBX does.
2. **Copy the scaffolding from OpenRBX:** `util/decomp.h` (`STUB`, `DECOMP_SIZE_ASSERT`, `undefined*`), `.clang-format`, `.clang-tidy`, `.editorconfig`, `tools/reccmp_addr_padding.py`, `tools/check_folded.py`, `tools/ncc/`, `cmake/reccmp.cmake`, and the CI workflow shape. Retarget them from WEBSERVICE/ROBLOX to PLAYER/STUDIO.
3. **Lay out directories** from `notes/paths-2013.txt`, using the 2016 tree to fill gaps. One OBJECT library per original static library. Start every library's flags from the 2016 `.vcxproj`, each with a `# TODO prove` comment.
4. **Set up reccmp.** Write `reccmp-project.yml` with PLAYER and STUDIO targets, their hashes and data sources, and `reccmp-user.yml` (gitignored) pointing at `bin/`.
5. **Generate the CSVs:** `tools/ida/gen_reccmp_csv.py` reads `ida/exports/` and writes `reccmp/player-function-size.csv` (with OpenRBX's filter: drop unwind funclets, catch handlers, adjustor and import thunks, and record the count), `player-<lib>.csv` per third-party library from the FLIRT and body hits, and `player-symbols.csv` from the Mac transfer.
6. **Smoke-test the build.** The exe builds and links with `/OPT:NOREF` (an `OPENRBX_KEEP_UNREFERENCED` equivalent), and `reccmp-reccmp --target PLAYER` runs and reports 0% against the full denominator.
7. **Import the existing App decomp into `Client/App/`.** Bring it in as a single commit with no edits, so its history and this repo's conventions stay separable. Then make one dedicated conversion pass:
   - Convert its annotations to this repo's marker format (`// FUNCTION: PLAYER 0x...`), with eight lowercase hex digits and ascending order per file. If it has no address markers, add them from the `.i64` names.
   - Downgrade **every** marker to `// STUB:` at first. Phase 3 promotes them only on measurement.
   - Match its directory layout to the 2013 `__FILE__` paths from Phase 0. Move files, but do not edit their bodies in this pass.
   - Fix whatever stops it building under the original toolchain, and write down every fix in the commit message (a construct that needed changing is codegen evidence).
   - Push its function and class names into `ida/PLAYER.i64` (snapshot first, `-04-prior-app`) with the `prior:` name source.

**Exit criteria:** `cmake --build` and all eight checks run green in CI, with the existing App tree building as part of the PLAYER target.

---

## Phase 3: Prove the compiler and flags

**Status:** steps 0–3 done 2026-11-12. **Step 0:** 90 of 1,036 PLAYER App markers at 100% (86 of 1,016 STUDIO), 0.43% of App's 19,665 non-library functions, against the claimed ~55%; 91 PLAYER near misses (90–99.99%), none a flag problem (`notes/app-sweep.csv`). **Step 1:** the libraries are in the reccmp build; PLAYER library rows at 100%: lua 503/514, raknet 346/463, g3d 122/131, zlib 44/45, libzip 65/65, boost 373/400 paired. **Step 2:** every engine library's flags proven by 77 probes (`CLAUDE.md` → *Codegen Patterns*); GfxBase is `/Od`, the client shells and Log.dll are `/GL /GS` without `/arch`. **Step 3:** `_SECURE_SCL=0`, vector 0x10, string 0x1c. Exit criteria not fully met: the probes are `.obj` matches, not yet 100% rows in the build for Base, AppDraw, GfxBase, GfxAdapters, RbxOgre and the shells.

**Goal:** before decompiling Roblox code at scale, prove the toolchain and each library's flags on small, certain functions. This is the phase that replaces having the PDB's command blocks.

0. **Measure the existing App decomp.** It is the best calibration set available: hundreds of functions with a known intended source. Flip every App `STUB` to `FUNCTION`, score once with `--json`, keep the ones that are effective 100%, and revert the rest (OpenRBX's one-build sweep). Record the **measured** App figure (implemented / every App function in PLAYER) next to the claimed 55% in the status board. Sort the near misses (90–99%) by score. They are the cheapest wins in the project and the best evidence for which flags are wrong. A whole class of near misses that share one diff shape (a stack cookie, an `ebp` frame, x87 vs SSE) is a flag problem, not a code problem.
1. **Third-party bodies first.** Vendor the exact third-party versions and build them with the candidate flags. Every relocation-masked byte match on a library function is a proof of the toolset plus flags for that library. Record the match rate per library.
2. **One probe per engine library.** Pick three to five small, leaf, non-virtual functions per Roblox library (a getter, a setter, a math helper, a small constructor). Write them, then flip `/GS`, `/Oy`, `/arch`, `/fp`, `/Ob` and `/GL` one at a time until all of them hit 100%. Write the winning set into `CMakeLists.txt`, with the deciding function's address next to each flag.
3. **Settle the STL configuration.** Find a constructor of a class holding a `std::vector` and a `std::string`. Its stores reveal `_ITERATOR_DEBUG_LEVEL`/`_SECURE_SCL` and the container sizes. Prove them with `DECOMP_SIZE_ASSERT`.
4. **Start the codegen notes.** Each surprise goes into `CLAUDE.md` → *Codegen Patterns*.

**Exit criteria:** every engine library has at least one 100% function under its final flags. The STL configuration is proven.

---

## Phase 4: Core object model

**Status:** started 2026-11-12 (`notes/runs/2026-11-12-phase4-pairing/`, scout data `notes/class-layouts-2013.csv`, `tools/class_layout.py`). 2013 layouts written and size-asserted: Debugable (4), Object/AbstractFactoryProduct (4), EventSource (4, 3 slots), DescribedBase (0x10), GuidItem<Instance> (0x14), Limits::Countable (8, non-virtual), **Instance (0x60; primary vftable 0x00b2a6ec lines up slot for slot)**, ServiceProvider (0x8c, ctor 100%), MemberDescriptorContainer (0x60, `rbx::trie`), ClassDescriptor (0x20c, ctor 100%), VerbContainer (0x14), DataModelArbiter (0xa20), DataModelJob (0x218), GenericJob (0x248), GcJob (0x228), LibraryService (0x68), MovingAssemblyStage (0x28), Name (0x20, ctor 100%). **Second pass, same day** (`notes/runs/2026-11-12-phase4-core2/`): DataModel members 0xac8–0xe0c mapped from ctor 0x0064ec50 and dtor 0x0064d850 (STUDIO 4 bytes lower from 0xc64, a source difference); ScriptContext 0x288 fully laid out and size-asserted; `Diagnostics::Countable` calls the out-of-line interlocked helpers; FWBase 0x18 / FWInstance 0x38 / IFWHolder 5 slots. Vftables: PartInstance 0.27 → 0.73, BaseThreadPool::PoolData and EventSource's secondaries to 100%, MultiJoint 0.08 → 0.47, `PartInstance::resize` back at 100%. 32 functions moved to the files their 2013 asserts name. Proven layouts imported into both `.i64`s (48 types each). Open: DataModel size assert (waits on GuiBuilder: ours 0x24, PLAYER 0x20, STUDIO 0x1c); DataModel ctor 48.8% / dtor 78.6% and ScriptContext ctor 59.3% bodies not written; 38 PLAYER / 16 STUDIO vftables structurally wrong (PartInstance's secondary bases unmodelled); unpaired `RbxInterlockedIncrement` 0x009b8f10 (Base `atomic.cpp`) and `propRobloxLocked`/`propArchivable` globals hold three functions at 94–97%; STUDIO 0x00415400 (a second `getTarget`-shaped body) unidentified; TaskScheduler, Security::Context, EnumDesc<T>.

**Third pass, 2026-11-12/13** (`notes/runs/2026-11-12-phase4-core3/`): GuiBuilder 0x20 (STUDIO 0x1c) and **`DECOMP_SIZE_ASSERT(DataModel, 0xe10)` enabled**; DataModel's vftable has all 34 slots (processAccelerators … processEvent); `RbxInterlocked*` 40% → 100%; DataModel ctor 62.5% / dtor 92.8%, ScriptContext ctor 88.1%; FWInstance owns its three bytes. PartInstance's primary vftable has all 64 slots and its secondary bases (IHasPrimaryPart, IHasLocation, IAdornable, CameraSubject, Selectable) are modelled; `Edge` is 0x1c. **7 `EnumDesc<T>` vftables at 100%** (CreatorType, Genre, GearGenreSetting, GearType, SoundType, Material, ChatColor), which meets the reflection exit criterion for EnumDesc. 28 files reordered (decomplint out-of-order 328 → 254 PLAYER). Open: the near-miss pass (stopped by the usage limit), `Instance`'s full vftable for the exit criterion, the DataModel ctor's PhysicsInstructions member and `Workspace(IDataState*)`, LuaSettings 8 bytes short, IScriptFilter `novtable`, 40 files whose order is pinned by assert lines, `convertToItem` include spelling.

**Fourth pass, 2026-11-13** (`notes/runs/2026-11-13-phase4-core4/`): FWInstance ctor and FWBase ctor/dtor at 100% (Allocator base, `FWFinal<T>`); Instance ctors 83% / dtor 90.5%; `Time` mostly at 100%; Debugable proven `novtable`. **Exit criterion not yet met:** all four Instance vftables are structurally complete in both targets, but some slots stay red on twin and thunk pairing (the original keeps two copies of trivial bodies that our link folds), and `Util/Object.h` must drop `RBXBaseClass` from `Object` and `AbstractFactoryProduct` (a private compile makes `~Instance` byte-identical). Also open: `BOOST_THREAD_DYN_LINK`, the `GlobalAdvancedSettingsItem<T>` base for PhysicsSettings/LuaSettings, the second atomic class at 0x005ba6f0, the FWInstance/FWFinal D0 symbol rows.

**Fifth pass, 2026-11-13 morning** (`notes/runs/2026-11-13-phase5-wave1/phase4-exit.md`): `Object` and `AbstractFactoryProduct` lost `novtable`; both Instance ctors and `~Instance` are at 100% in both targets; `BOOST_THREAD_DYN_LINK`/`BOOST_SYSTEM_DYN_LINK` are on; the 12 Base log channels are defined; vftables now pair by RTTI name (PLAYER 1,282 of 2,886 at 100%). Instance's `{for Described}` and `{for AbstractFactoryProduct}` vftables are at 100%; **`{for Debugable}` (0.714) and `{for GuidItem}` (0) are held by the twin trivial bodies** (CLAUDE.md Gotchas), so the exit criterion is still open. Most reflection descriptor templates have no instantiation at 100% yet (`tools/template_vtables.py`).

**Goal:** the classes everything else inherits from, matched constructor, destructor and scalar deleting destructor first, so every later class can be wired onto a proven base.

Most of these classes live in App, so the existing decomp most likely covers much of this phase already. For each class below, start from the existing tree: if its ctor, dtor and scalar deleting dtor already score 100% after Phase 3, the class is proven, so move on. If not, fix that trio first, before any of its methods, because a wrong layout there is what holds the rest of the existing App code below 100%.

Suggested order (adjust to what the 2013 RTTI actually shows):

1. Base utilities: `RBX::Debugable`, assert/log helpers, `RBX::Name`, `RBX::Guid`, `RBX::GuidItem`.
2. Signals and events. The 2007 `Notifier`/`Listener` and the 2016 `rbx::signal` differ; **the 2013 binary decides which one, or which hybrid, existed.**
3. Reflection: `Reflection::Descriptor`, `ClassDescriptor`, `PropertyDescriptor`, `TypedPropertyDescriptor<T>`, `EnumDesc<T>`, `Described<>`/`DescribedCreatable<>`/`FactoryProduct<>`. Apply OpenRBX's *Reflection and Registration* lessons, re-proven for this compiler.
4. `RBX::Instance` and its ancestors, then `ServiceProvider`, `DataModel`.
5. `RBX::Security`/identity context, the `TaskScheduler` core.

For each class: diff the 2016 header against the 2013 vftable, write the header with offset comments, match the ctor, dtor and scalar deleting dtor, then the remaining methods in address order. **Re-import the proven header into the `.i64`** so every later decompile reads the right layout.

**Exit criteria:** `Instance`'s full vftable is claimed and `reccmp-vtable` is green on it. Each reflection template has at least one instantiation at 100%.

---

## Phase 5: Engine subsystems (breadth)

**Status:** started 2026-11-13. After the first wave (`notes/runs/2026-11-13-phase5-wave1/`), PLAYER functions at 100% per row: 5b (DataModel and core services) 200 / 1,372 (14.6%), 5e (script) 246 / 848 (29.0%), 5g (content, Http) 177 / 475 (37.3%). Our tree instantiates 9 of 2013's ~162 `EnumDesc<T>`; WinHttp.cpp and WinInet.cpp do not exist yet. Row 5a (math) at 133 / 154 functions (86.4%, `tools/row_coverage.py`): Math 81/81, Extents 10/10, Face 10/10, NormalId 11/11, IndexedMesh 11/15, Region3 4/5, InterpolatedCFrame 2/18, Frustum 1/1. Quaternion, Units, PV, Velocity, Region3int16 and Vector3int16 have no out-of-line Windows body. Next by unnamed-function count (`ida/exports/PLAYER-app-unnamed-by-class.csv`): DataModel 255, ScriptContext 224, EnumDesc 143, Lua bridges 135, GuiService 109, ContentProvider 108.

Rows 5a–5e and 5h are mostly App code, so they start from the existing ~55%. Add each subsystem's measured existing coverage from Phase 3 to its Notes cell, and finish partially covered App subsystems before opening non-App ones (network, rendering, the client shell), so the App number reaches 100% as one milestone.

Take one subsystem at a time, smallest dependency surface first. Each one is a sequence of small PRs (one class per PR, as OpenRBX's CONTRIBUTING asks).

| Order | Subsystem | Notes |
|---|---|---|
| 5a | Math and G3D (Roblox's G3D fork) | Leaf-heavy; good for proving float codegen early |
| 5b | v8tree / v8datamodel basics (`Workspace`, `Lighting`, `Players`, `Teams`, …) | Builds on Phase 4 |
| 5c | Serialization: XML (`.rbxl`/`.rbxm`), and binary format code if present in 2013 | Check strings for `<roblox!` |
| 5d | Physics / v8world / kernel (`World`, `Primitive`, `Assembly`, `Joint`s, the contact solver) | Large and float-heavy, so do it after 5a proves the patterns |
| 5e | Script: Lua VM (forked 5.1.4), `ScriptContext`, the bridge (`LuaBridge`/reflection glue), sandboxing | Lua's own translation units are near-vendor, so match them early as a toolset check |
| 5f | Network: RakNet 4.0, `Replicator`, `ClientReplicator`/`ServerReplicator`, filtering | Vendor the exact RakNet 4.0xx release from Phase 0, build FLIRT sigs for it, and record any Roblox patches as diffs against upstream. OpenRBX's 3.0 reconstruction does not apply |
| 5g | Http, content providers, asset fetch (libcurl wrapper) | |
| 5h | GUI (`ScreenGui`, `Frame`, `TextLabel`, …) and CoreGui hooks | |
| 5i | Rendering: whatever 2013 actually used (Ogre-based, or the newer Gfx renderer; Phase 0 decides), D3D9/OpenGL backends | Largest uncertainty against 2016, so put it late |
| 5j | Sound (FMOD wrapper) | |
| 5k | Client shell: `WindowsClient`/player app, input, window, launcher handoff | PLAYER-only code |

Bookkeeping per subsystem: a status row added to the table below, the score delta recorded, the `.i64` updated and its exports refreshed, and an `ida/LOG.md` entry.

---

## Phase 6: STUDIO transfer

**Status:** not started

**Goal:** reuse PLAYER's work for STUDIO exactly as OpenRBX transferred WEBSERVICE to ROBLOX.

1. **Body matching.** For each PLAYER function at 100%, find STUDIO bodies that match with relocations masked, and propagate names through calls and data references reached via matched bodies. **Generate** `studio-*.csv` from their `player-` twins, and add the `// FUNCTION: STUDIO` marker under the PLAYER one only where the STUDIO comparison actually reaches 100%.
2. **Allow for the revision gap.** Studio was built two days after Player. A function whose body differs only slightly may be a genuine source change, not a codegen difference. Check the iOS builds on either side of the two dates before "fixing" it.
3. **Studio-only code.** The Qt UI (vendor the exact Qt version Phase 0 found and build FLIRT sigs for it), the legacy MFC/Codejock tools if the folder ships them, and the Studio-specific services. Hand rows go only in `studio-client.csv`.
4. **Pair vftables** by RTTI class and offset across PLAYER and STUDIO, as OpenRBX did.

Quote STUDIO progress as implemented functions over **every** STUDIO function, never accuracy over the transferred subset.

---

## Phase 7: Data, completeness, and the end state

**Status:** not started

- Globals, statics, string literals and float constants (`player-globals.csv`, `-statics.csv`, `-strings.csv`, `-floats.csv`), with `reccmp-datacmp` green.
- Turn off `/OPT:NOREF` once enough is reachable, and reproduce link order.
- Smoke test: the rebuilt exe starts, loads a local place file offline, and renders. Never connect it to live services.
- Only after accuracy is the goal of record: a non-matching "modern" build configuration, as OpenRBX intends.

---

## Fallback: if Phase 0 finds LTCG across most of the engine

Under `/GL`, a function's codegen depends on the whole program, so byte-exact matching only becomes reachable once nearly everything is present. Options to put to the user:

1. **Functional decomp first, matching later.** Keep reccmp for scoring, but treat a function as done at "semantically equivalent, high similarity" with a separate marker (e.g. `// EQUIVALENT: PLAYER 0x...`). Revisit for exact matches once coverage is high. This keeps the annotation discipline and the `.i64` workflow intact.
2. **Match non-LTCG islands exactly.** Usually third-party libraries and some engine libraries were not `/GL`. Do those to 100% under the normal rules, and the LTCG parts as in option 1.
3. **Compile each LTCG translation unit without `/GL` and compare against an `/FA` listing,** as OpenRBX did for its six LTCG objects. Accept that the linked-image score will lag.

Do not pick one without the user's decision.

---

## Status board

| Area | PLAYER impl/total | STUDIO impl/total | Notes |
|---|---|---|---|
| Phase 0 facts | done | done | 2026-11-11. VS2008 SP1, LTCG minority, PLAYER is VMProtect-wrapped: decision pending |
| App (existing decomp) | claimed ~55% / measured 90 FUNCTION of 1,036 markers at first sweep; **510 FUNCTION + 24 FOLDED, 686 STUB** after Phase 5's first wave | 495 + 24, 678 STUB | |
| **Whole binary (reccmp)** | **4,621 functions at 100% (2,506 non-library)**; paired ("Implemented") 6,133 / 40,527 (15.1%), `--nolib` 3,739 / 38,073 | **4,474 at 100% (2,417)**; paired 5,923 / 42,371 (14.0%), `--nolib` 3,656 / 39,950 | 2026-11-13 07:20. "Implemented" counts functions reccmp pairs, at any score; the 100% count is the matching figure. `--nolib` counts CSV-paired functions too (oracle signatures, position data); "Implemented" skips STUB markers; entries count functions plus vftables |
| Vftables (reccmp-vtable) | 1,282 / 2,886 at 100% (RTTI pairing since 2026-11-13) | 1,325 / 2,815 | `tools/vtable_structural.py` |
| Phase 5 rows (functions at 100%) | 5a 133 / 154, 5b 200 / 1,372, 5e 246 / 848, 5g 177 / 475 | | `tools/row_coverage.py` |
| Third party (library) | 1,848 library entries at 100% (G3D CSV rows 205 identical after patches 0006–0010) | 1,780 | `__FILE__` paths fixed; misses now per-TU statics, link-order inline copies |
| Core object model | | | |
| … | | | |

**IDA databases**

| DB | Last snapshot | Named % | Notes |
|---|---|---|---|
| `ida/PLAYER.i64` (Oct 23) | `PLAYER-10-wave1-sync` | 24% (11,715 / 49,091; 3,897 `match:`) | 2026-11-12; 221 adjustor-thunk targets split; 48 proven types, `this` typed on 34 functions |
| `ida/PLAYER_1015.i64` | `PLAYER_1015-00-autoanalysis` | 3% (1,391 / 44,866) | |
| `ida/STUDIO.i64` | `STUDIO-10-wave1-sync` | 28% (12,493 / 44,815; 3,746 `match:`) | Different source line from PLAYER; 33 false FLIRT names removed; 48 proven types |
| `ida/MACSTUDIO.i64` | `MACSTUDIO-00-autoanalysis` | 90% (64,643 / 71,887) | |
| `ida/IOS_1014.i64` | `IOS_1014-00-autoanalysis` | 99% | |
| `ida/IOS_1021.i64` | `IOS_1021-00-autoanalysis` | 99% | Closest to STUDIO |
| `ida/IOS_1029.i64` | `IOS_1029-00-autoanalysis` | 99% (84,574 / 85,425) | Closest to PLAYER |

# Project OGRE

Project OGRE is a decompilation of the Roblox Windows client as of October 2013. The goal is the same as [OpenRBX](https://github.com/Asphaltian/OpenRBX), which did the December 2007 client: write the source back until the original compiler turns it into the same binary, instruction for instruction. That project is the template for everything here: its conventions, its reccmp workflow and its rule that evidence beats guessing. There are three differences from 2007, and each one changes how the work is done:

1. **The disassembler is IDA Pro 9.x (9.4 is installed at `~/ida-pro-9.4` and approved by the user), not Ghidra.** Every database is an `.i64`, and Claude leaves one behind after every analysis session (see [IDA Databases](#ida-databases-the-i64-rule)).
2. **No Windows binary ships a PDB.** In 2007, `WebService.pdb` was ground truth. Here, names come from the **unstripped Mac Studio build**, struct and file shapes come from the **2016 Roblox source**, and the Windows binaries themselves are the final judge.
3. **The compiler is a later MSVC, not VC++ 8.0 SP1.** Phase 0 of `DECOMP_PLAN.md` identifies the exact one from the Rich header. Until then, none of OpenRBX's MSVC 8 codegen notes count as true for this binary. Re-prove each one.

`DECOMP_PLAN.md` lays out the phases. This file holds the rules that apply in every phase. When you learn something that cost a round trip, add it to the matching section here, in the same style OpenRBX uses: the rule, then the evidence, then the function that proved it.

## Inputs

| Input | Role | Matched? |
|---|---|---|
| `bin/player/` (was `Windows Player Client October 15th/`) | **Target PLAYER_1015 (secondary).** The Oct 15 Windows player, 0.126.0.39225, and the only source of the shipped DLLs, `RobloxPlayerBeta.exe`. **VMProtect-wrapped, see the facts table** | Yes, primary |
| `bin/studio/` (was `Windows Studio Client October 17th/`) | **Target STUDIO.** Windows Studio, `RobloxStudioBeta.exe` (Qt 4.8.5 only; no legacy MFC `Roblox.exe` ships). Not protected | Yes, second |
| `ref/macstudio/` (was `Mac Studio Client (CONTAINS SYMBOLS)/`) | **The name oracle.** Unstripped Mach-O, playing the role `WebService.pdb` played for 2007 | No, reference only |
| `ref/ios_1014/`, `ref/ios_1021/` (the two `.ipa` files, extracted beside them) | iOS armv7 builds, unencrypted, with local symbols. Twice the Mac build's RBX symbol count | No, reference only |
| `ref/src2016/extracted/robloxsrc/Trunk2012/Build/` (the engine tree; `Contribs/` beside it) | Source three years newer: layout, class names, members, project flags | No, reference only |
| `bin/player_1023/RobloxPlayerBeta.exe` | **Target PLAYER (primary since 2026-11-11).** Player 0.126.0.39503, linked 2013-10-23. Exe only, no DLLs. The user supplied it as the build that matches the symbolled Mac and iOS binaries. VMProtect-wrapped like the Oct 15 build | Yes, primary |
| `ref/ios_1029/robloxmobile` | iOS 2.126.39527 (Oct 29), armv7, unencrypted, full symtab (118,286 symbols, 20,600 RBX). **The closest symbolled build to PLAYER** | No, reference only |
| **Existing 2013 `App/` decomp (~55%)**, in `2013 55% App/` | The user's own prior work on the 2013 `App` library. **The starting point for `Client/App/`, not a reference** | Yes, this *is* the source tree for App |

### The existing App decomp

About 55% of the 2013 `App` library (the engine core: v8tree, v8datamodel, v8world, reflection, script, gui and so on) is already decompiled. Rules:

- **Build on it, never redo it.** Before writing any App function, check whether the existing tree already has it. Re-decompiling a finished function from scratch throws away work and causes merge conflicts.
- **Measured 2026-11-12 (first sweep): 90 of 1,036 PLAYER markers at 100% (86 of 1,016 STUDIO), 0.43% of App's 19,665 non-library functions** (App block 0x00606a00–0x009ad3a0; `tools/app_sweep.py`, `notes/app-sweep.csv`). Even every placed marker at 100% would cover 4.7% of App by function count; the tree's 2,733 Mac-addressed functions are at most ~14%. Main causes of the misses: 2016 class layouts (DataModel, ScriptContext, GenericJob, AssemblyStage), G3D header inlining, unpaired callees, `__FILE__`/`__LINE__` in asserts. **Its invented names lose to oracle names:** 21 of its names conflict with iOS/Mac (e.g. the ChatOutput `createChatBubble*` functions, `StatsItem::formatValueIf…` for `Stats::Item::formatValue<T>`); `prior:` never outranks `ios:`/`mac:`.
- **55% is a claim; reccmp makes it a measurement.** OpenRBX's rule holds: *a claimed match is not a verified match.* When the tree is imported, score every function. Mark each one `// FUNCTION:` only at 100% and `// STUB:` otherwise, and record the measured number in `DECOMP_PLAN.md`. Expect the claimed and measured figures to differ, in both directions. OpenRBX found byte-exact bodies hiding behind `STUB` markers, and inflated claims behind `library` rows.
- **Its proven layouts outrank the 2016 source.** A class whose constructor and destructor already match at 100% in this tree has a proven layout. Use it (and import it into the `.i64`) instead of the 2016 header.
- **Its unproven layouts do not.** A header that no 100% function uses is still a hypothesis, the same as a 2016 header. Check it against the 2013 constructors before building on it.
- **Keep its conventions unless they break a check.** If it uses different markers, placeholder names or file paths from this file, convert it in one dedicated pass (see `DECOMP_PLAN.md`, Phase 2), not piecemeal inside feature PRs.
- **Its `// 0xADDRESS` comments are MACSTUDIO addresses, not Windows ones** (measured 2026-11-11, `tools/app_decomp_addr_census.py`). 6,939 comments over 2,733 Mac functions: 84% land on a MACSTUDIO function start, 1,145 of 1,192 `0xADDR (NNNB` notes match that function's exact size, and no base or offset maps them onto PLAYER, PLAYER_1015 or STUDIO. `HANDOFF.md`'s "IDA database for the Player PE" is a mislabel; the tree's own headers say "MAC dbq24" / `october2013symbolledrobloxstudio.i64`. A few addresses are from a 2016 PDB-named binary and are labelled so. So a PLAYER address for any of its functions comes from the Mac→PLAYER pairing, never from the comment.
- **Push its names into the IDA databases.** Every function it names goes into `ida/PLAYER.i64`, at the PLAYER address the pairing found, with the `match:` source (100%) or `prior:` source (not yet verified), so the Mac name transfer does not overwrite them with something worse.

The Player and Studio builds are **two days apart**, and the iOS builds bracket them. Never assume two binaries were built from the same engine revision. Pair functions by body, the way OpenRBX transferred WEBSERVICE to ROBLOX, and record each binary's version string in the facts table below.

### Facts table (fill in during Phase 0, then never guess these again)

| Fact | PLAYER | STUDIO | Mac Studio | Evidence |
|---|---|---|---|---|
| Filename, file version | `bin/player/RobloxPlayerBeta.exe` (internal name `RobloxApp.exe`), 0.126.0.39225 | `bin/studio/RobloxStudioBeta.exe` (internal `RobloxStudio.exe`), 2013.10.16.1 | `ref/macstudio/.../MacOS/RobloxStudio`, 2013.10.23.2 | VERSIONINFO / Info.plist |
| SHA256 | `6fa8abbc35ad37db1134b234bbbe17f3713c42f7df18dad296f65590222c8691` | `68ba1c5fd39ccc535b6ed77a171e3c94fd6d258fd8a5777af428dbbf9b623ba6` | `92f41e7c33708e1038cd0f8debf79f43911bfbd76dcc82760d9b472e32ddbb7a` | `sha256sum` |
| Link timestamp (UTC) | 2013-10-15 23:07:29 | 2013-10-17 02:17:09 | n/a | PE header |
| Image base, `/DYNAMICBASE`, `.reloc` present | 0x400000, no, **no `.reloc`** | 0x400000, no, **no `.reloc`** | n/a | PE header |
| Linker version | 9.0 (VS2008) | 9.0 (VS2008) | n/a | Optional header |
| Compiler build (`cl` major.minor.build) | **15.00.30729 (VS2008 SP1)**: 689 C++ + 130 C objects. Also 5 C++ at 15.00.21022 (RTM) and 146 C++ + 15 C at 13.10.4035 (a prebuilt SDK static lib; `RenderSystem_Direct3D9.dll` carries the same build) | 15.00.30729: 687 C++ + 127 C. Same 13.10.4035 block (145 + 15) | Not recorded in the binary. Links `libstdc++.6`, SDK 10.8, min OS 10.6 | Rich header / load commands |
| LTCG (`/GL`) objects present? How many? | **40** `Utc1500_LTCG_CPP` (0x8a) of ~860 VS2008 objects (4.7%): the client shell (WindowsClient's own files + boost program_options). Log.dll: 2 of 2 | **127** of ~940 (13.5%): RobloxStudio's own `.cpp` and moc files. **No engine library is `/GL`** | n/a | Rich header; `tools/probe_libs.py` (`regarg`, `ltcg8` stats) |
| CRT: static (`/MT`) or `msvcr1x0.dll` | Dynamic (`/MD`): imports `MSVCR90.dll`, `MSVCP90.dll` | Same | n/a | Import table |
| Packed or protected sections? | **Yes, VMProtect, but only 26 SDK-marker regions** inside 26 otherwise intact functions plus one function with its markers stripped: 11,351 bytes of `.text` (0.16%), same 27 functions in both Players. List: `notes/vmprotect-player.csv` (`tools/vmprotect_scope.py`). Entry point, TLS callback and the *loader's* import directory are in `.vmp1`, but **`.text` calls only through the linker's original IAT at `0x00aa1000`–`0x00aa2eac` in `.rdata`** (1,928 named slots, intact on disk). Auto-analysis named the wrong table | **No.** Plain sections, entry in `.text`, imports in `.rdata`. Imports `VMProtectSDK32.dll` only as SDK markers (`VMProtectBeginMutation`, `VMProtectEnd`, …). PDB path recorded: `C:\BuildAgent\work\fa0a2141fe3d2f4d\Client\RobloxStudio\bin\Release\RobloxStudioBeta.pdb` | No | Section names, entropy, `notes/pe-facts.txt` |
| Function count after IDA auto-analysis (not final, see `ida/LOG.md`) | 50,537 (Oct 23); 44,866 (PLAYER_1015) | 39,271 | 71,887 | `ida/exports/*-functions.csv` |
| Architecture | x86 | x86 | **i386** (thin, not fat) | Mach-O header |
| Symbols: symtab only, or DWARF/dSYM too? | n/a | n/a | **Symtab only, externals only.** 56,013 symbols, 35,745 defined in text, 10,621 `_ZN3RBX…`. No local symbols, no `__DWARF`, no `.dSYM` | `llvm-nm`, `llvm-otool -l` |
| RTTI type descriptors (`.?AV`) | 5,616 (5,090 in `RBX`) | 5,873 | n/a | strings |
| Build agent checkout | `C:\BuildAgent\work\838e6b7f780c1f82\Client` | `C:\BuildAgent\work\fa0a2141fe3d2f4d\Client` | relative `../App/...` | `__FILE__` strings |

**Which build pairs with which (measured, 2026-11-11).** Count of assert `file:`/`line:` records that differ between two builds, over the source files both builds mention (0 would mean the same revision of those files; platform-only asserts keep it above 0):

| | PLAYER 10-23 | PLAYER_1015 | STUDIO 10-17 | Mac 10-23 | iOS 10-14 | iOS 10-21 | iOS 10-29 |
|---|---|---|---|---|---|---|---|
| **PLAYER 10-23 (39503)** | 0 | 230 | 825 | **140** | 1057 | 810 | **75** |
| PLAYER_1015 (39225) | 230 | 0 | 705 | 370 | 1033 | 692 | 303 |
| STUDIO 10-17 | 825 | 705 | 0 | 829 | 699 | **148** | 883 |

So there are two source lines. PLAYER 10-23, Mac Studio 10-23 and iOS 10-29 are one family (the 0.126 release). **Windows STUDIO 10-17 is not in it:** it sits with iOS 10-21, about as far from either Player as the Oct 14 iOS build is. Use Mac 10-23 and iOS 10-29 as the name oracles for PLAYER, iOS 10-21 for STUDIO, and expect real source differences (not codegen) between PLAYER and STUDIO. Example: STUDIO's `DataModel` lacks `networkStatsWindowsOn`, so its members 0xc64–0xcac sit 4 bytes lower (ctor 0x0076b720 vs PLAYER 0x0064ec50), and its `GuiBuilder` is 0x1c against PLAYER's 0x20. PLAYER 10-23 has the same Rich header as PLAYER_1015 (689 + 130 objects at 15.00.30729, 40 LTCG) and the same VMProtect shape (67 `jmp` + 25 `call` sites into `.vmp0/.vmp1`). SHA256 `326d9c3e3baa18e9a6bdd6e3187ee1a1d2eddbaecf87c57b9e35da8c6857b10d`, build agent checkout `C:\BuildAgent\work\407e17641b466b38`. In the facts table below, the PLAYER column still describes the Oct 15 build (PLAYER_1015) except where this paragraph says otherwise. `ref/macstudio_dup/` is a byte-identical second copy of the Mac Studio binary.

**iOS builds (both usable, and a stronger name oracle than the Mac build).** `ref/ios_1014` is 2.124.39095, `ref/ios_1021` is 2.125.39335. Both are thin armv7, **`cryptid 0` (not encrypted)**, built with Xcode 5.0 clang against the iOS 7.0 SDK, and carry a full symtab including locals: 117,597 and 118,367 symbols, about 20,500 defined `_ZN3RBX…` each (twice the Mac count). SHA256 `2a976c1874911384d7ee323872ef978f979917941fd0676b2aed49900f6da0cd` and `00e764c7c99995b05048ea8840d5d2155278ee571f668386d2b3d6a253d7bebe`. Everything is statically linked there, so third-party names are present too.

**Revision order:** iOS 39095 (Oct 14) < PLAYER 39225 (Oct 15) < iOS 39335 (Oct 21); STUDIO (Oct 17) and Mac Studio (Oct 23) carry date versions only. PLAYER and STUDIO came from different build-agent checkouts.

**Third party, pinned 2026-11-11, byte-matched 2026-11-12** (details in `DECOMP_PLAN.md` Phase 0; matchers in `tools/lib/*_match.py`, sources in `staging/`, reports in `notes/runs/2026-11-12-phase2-skeleton/`): **RakNet 4.036**, compiled inside the Network project: the 4.034 PC release plus the upstream 4.035/4.036 changes and Roblox's edits, 566 of 567 located PLAYER functions identical (`DS_Multilist.h` removal moves a `__LINE__` in `BPSTracker::Push1` 0x005ca3e0; 25 TUs linked, no plugins). **Lua 5.1.4** compiled as C++, stock lua.org source plus 15 byte-justified patches (`Table.readonly`, `Udata.may_gc`, `lua_exception`, 44-byte `RobloxExtraSpace`, `FFlag::LuaEnableBytecode`, `luaL_loadprotected`, compat switches off; ldblib/liolib/loslib/loadlib/linit absent): 514/514. **boost 1.53.0**, pristine (thread+system in `boost.dll`, 337/337; filesystem/program_options/iostreams static). **G3D 8.00** with its bundled **zlib 1.2.3** (45/45, built `ZLIB_WINAPI`) and **libzip 0.9** (65/65); **IJG 6b contributes no linked code**, only jerror's message table at 0x00bc2250. Pristine G3D 8.00 is no longer downloadable, so G3D is built from the prior tree's fork plus 5 patches: 161 identical, 22 not. Static D3DX9 (above); static ATL (33 RTTI) and DirectShow base classes. **Bullet is not linked in any October 2013 build**; the `App.BulletPhysics` folder in the existing tree has no counterpart in the binaries.

**The compiler is VS2008 SP1, not VS2012.** The existing App decomp's `HANDOFF.md` says it was compile-checked with `v110_xp` (VS2012), and the 2016 tree is `v140`. Neither is the 2013 toolchain. The Rich header and the `msvcr90`/`Microsoft.VC90.CRT` dependency settle it. Consequences: `_SECURE_SCL` (not `_ITERATOR_DEBUG_LEVEL`) is the STL switch, the STL is Dinkumware 9.0, and C++11 does not exist.

**No `.reloc` in either target.** reccmp cannot use relocations to mask addresses here. It has to rely on the recompiled PDB plus its own operand heuristics. Check this in Phase 2 before trusting any score.

**Remaining Rich prodids** (from richprint `comp_id.txt` and kirschju/richheader): `0x0096` build 20413 ×11 = AliasObj900 (build not in any table; present in every VC9 binary here, probably CRT alias objects), `0x0004` build 8447 = VC6 SP5 import/export, `0x009c` build 30319 = VS2010 Implib, PLAYER only `0x0019` build 9210 = VS2002 Implib and `0x001c` build 9178 = Utc13 C (XP SP1 DDK). **The 13.10.4035 objects** ("Server 2003 SP1 DDK" compiler) are most likely a static `d3dx9.lib` from a late-2004 DirectX 9.0c SDK: the D3DX block sits at the tail of `.text` (about `0x009ef000`–`0x00a5b000`) and carries its own zlib 1.1.4, libpng 1.0.5 and IJG 6b. The other identified ones: `0x83/0x84` Utc1500 C/C++, `0x8a` Utc1500 LTCG C++, `0x91` linker, `0x92` export, `0x93` implib, `0x94` cvtres, `0x95` MASM 9, `0x5f/0x60` Utc1310 C/C++, `0x0f` MASM 7.10, `0x01` imports.

## Ground Truth, In Order of Authority

Never guess a name, an offset, a path or an enum value when one of these records it. Check them in this order:

0. **A function in this tree at 100% under reccmp,** including the existing App decomp once it is scored. A match is proof, and it outranks everything below.
1. **The Windows binary itself.** Bytes, RTTI (`.?AV` type descriptors and Complete Object Locators), vftables, string literals, `__FILE__` paths in asserts, jump tables, import tables. It is the only thing that is *exactly* the 2013 Windows build. Everything below is evidence about a *different* build.
2. **The Mac Studio symbols.** Itanium-mangled names carry the namespace, the class, and **the full parameter list** (more than an MSVC public name gives without a PDB). If a dSYM or embedded DWARF is present, it also gives struct members, locals and line tables. This is the closest analogue to the 2007 PDB. The catch is that it was built by a different compiler for a different ABI (see [Mac Symbol Transfer](#mac-symbol-transfer)).
3. **The iOS builds.** Same engine, same month, ARM. Use them to break ties when the Mac build and the Windows build disagree on something version dependent, and to confirm that a name existed in October 2013.
4. **The 2016 source.** Correct about *shape* far more often than about *detail*. File layout, class hierarchy, member order, helper names and project flags are good hypotheses. Members, virtuals, parameters and whole subsystems were added or removed between 2013 and 2016. **A 2016 line is a hypothesis until the 2013 bytes confirm it.**
5. **The 2007 OpenRBX tree.** (Not for networking: 2013 uses RakNet 4.0, where OpenRBX reconstructed RakNet 3.0, so none of `Client/Network/raknet30` or its notes apply.) Useful for classes that barely changed (`RBX::Instance`'s ancestry, `Described`/`FactoryProduct` reflection, G3D math helpers) and for the method. Never treat it as authority on a 2013 layout.

When two sources disagree, the higher one wins. Write the disagreement down in the class's header comment so the next agent does not re-litigate it.

### Rules for the 2016 source

- **Reference, not paste.** Read it to learn names and structure, then write the 2013 function from the 2013 disassembly. A pasted 2016 body almost never matches. Worse, it carries features and fixes that did not exist yet, and a partial match hides that.
- **Never commit it, or any part of it, to this repository.** The same goes for the original binaries and the `.i64` files. `.gitignore` covers `ida/*.i64`, `ida/*.id?`, `ida/*.nam`, `ida/*.til`, `bin/` and `ref/`.
- **Diff the 2016 class against the 2013 vftable before writing a header.** The vftable's slot count and the RTTI hierarchy are 2013 facts. A 2016 virtual with no slot did not exist yet. A 2013 slot with no 2016 counterpart was removed later, and it needs a placeholder name (`VTable0xNN`) until Mac symbols name it.
- **Project files are the best lead on compiler flags.** The 2016 `.vcxproj`/`.props` per-configuration settings (`/O2`, `/Oy`, `/GS`, `/fp`, `/GL`, `/arch`, runtime library, `_SECURE_SCL`/`_ITERATOR_DEBUG_LEVEL`) are the starting hypothesis for each library's flags. The 2013 bytes decide (see [Build Layout](#build-layout)).

## IDA Databases: The i64 Rule

IDA 9.x writes `.i64` for 32-bit and 64-bit inputs alike, so every database here is an `.i64`. **Re-running auto-analysis and redoing naming by hand is the most wasteful thing available.** IDA keeps nothing between sessions except what is saved in the database. So:

- **Every target has one canonical database:** `ida/PLAYER.i64` (Oct 23), `ida/PLAYER_1015.i64`, `ida/STUDIO.i64`, `ida/MACSTUDIO.i64`, `ida/IOS_1014.i64`, `ida/IOS_1021.i64`, `ida/IOS_1029.i64`. Create one with `tools/ida/run_first_analysis.sh TARGET binary`. Open the existing one. Never start a fresh database for a binary that already has one, unless the binary's SHA256 changed.
- **After the first auto-analysis finishes** (the queue is empty, FLIRT has been applied and RTTI parsed), save and pack the database before doing anything else. That is the baseline. Copy it once to `ida/snapshots/<TARGET>-00-autoanalysis.i64` and never overwrite it.
- **Leave an `.i64` behind at the end of every session that changed anything:** names, types, structs, comments, function bounds. Save with packing (deflate) so the database closes cleanly and does not leave a stale `.id0/.id1/.nam/.til` set behind. A crashed or unpacked database is a lost session.
- **Snapshot before any bulk script** (mass rename, type application, Diaphora import, a cross-binary name push) to `ida/snapshots/<TARGET>-NN-<what>.i64`. A bad bulk rename spreads through every later step, and the snapshot is the only undo that works across sessions.
- **Write a session note.** Append to `ida/LOG.md`: date, target, what changed, which snapshot came before it, and any script you ran (keep the script under `tools/ida/`). The `.i64` holds the state. The log says how it got there.
- **The `.i64` is not the only record.** At the end of a session, also export what you learned as text into the repo so it can be reviewed and diffed: `tools/ida/export_names.py` writes `ida/exports/<TARGET>-names.csv` (address, name, type, size, source of the name) and `ida/exports/<TARGET>-functions.csv`, and *File → Produce file → Create C header file* writes the local types to `ida/exports/<TARGET>-types.h`. The exports are what the `reccmp/*.csv` generators read. They also mean a corrupted `.i64` costs hours, not weeks.
- **Record where every name came from.** Use a repeatable comment prefix or the export's `source` column: `mac:`, `ios:`, `rtti:`, `str:`, `src2016:`, `flirt:`, `match:` (a body matched our build), `prior:` (named in the existing App decomp, not yet scored at 100%), `guess:`. A `guess:` name never goes into source.
- **Only one IDA can hold a database.** A GUI session locks it. Headless work (`idat -A -S"tools/ida/script.py" -L ida/logs/run.log ida/PLAYER.i64`, or idalib's `idapro.open_database()` / `close_database(save=True)`) has to wait until the GUI closes it, just as `reccmp-ghidra-import` needed Ghidra to release its project lock.
- **If an IDA MCP server is configured in this session, prefer it** for interactive reads (decompile, xrefs, rename). Use headless scripts for anything that touches more than a few dozen functions. Either way, the i64 rule still holds.
- **Lumina:** pulling metadata is the user's call. Never push a database to a public Lumina server. It sends function metadata from a proprietary binary off the machine.

### What to set up in each database once

- The FLIRT signatures for the exact MSVC toolset the Rich header names (the CRT and STL), plus signatures we build ourselves from our own compiles of each third-party library with the original compiler and flags (FLAIR: `pcf` → `sigmake`). This is the IDA counterpart of OpenRBX's `roblox-xtp.csv`, where library code was matched by compiling the library and comparing.
- Local types imported from our own headers as they mature (*File → Load file → Parse C header file*, or IDAClang for C++). Re-import after a class lands at 100% so the decompiler reads the proven layout, not the guessed one.
- Calling conventions fixed at the definition. `__thiscall` means a member function. A `__thiscall` on a global means that global is an instance. A bare `ret` on a member means `static`.

## Mac Symbol Transfer

The Mac build is the name oracle, but it is not the same program. Rules:

- **Pair by evidence, never by address.** In order of strength: a unique string literal referenced by both; the RTTI class plus vftable slot index; an xref chain from an already-paired function; Diaphora or BinDiff similarity. Similarity alone is weak across clang/x86-64 vs MSVC/x86, so a similarity-only pairing goes in as `guess:`. Confirm it with a string or slot before it is used.
- **Itanium vtables have two destructor slots** (complete D1 and deleting D0). MSVC has one (the scalar deleting destructor). Every slot after the destructor is off by one. Itanium also prefixes offset-to-top and typeinfo before the address point, and MSVC puts the Complete Object Locator at `[-4]`.
- **MSVC groups overloaded virtuals and can order them differently from declaration order.** Itanium follows declaration order. When a slot pairing across the two looks swapped inside an overload set, suspect this before suspecting a wrong pairing, and prove it with the 2016 header and the call sites.
- **Layouts do not transfer.** Even with DWARF, clang/libc++ (or libstdc++) differ from MSVC/Dinkumware on `std::string`, `std::vector` (check for `_Myproxy` and the `_ITERATOR_DEBUG_LEVEL` the binary was built with), `std::map`, `bool` packing, empty-base optimization under multiple inheritance, pointer size on x86-64, and `long double`. DWARF gives *member names and order*. Offsets come from the Windows constructors.
- **Inlining differs.** A function the Mac build kept out of line may be inlined everywhere on Windows, and the reverse. A Mac symbol with no Windows body is not missing code.
- **Demangle with `c++filt` or `llvm-cxxfilt`** and keep the demangled signature in the names export. That is where parameter types come from.

## Annotations

The same reccmp markers as OpenRBX, with this project's target names:

```cpp
// FUNCTION: PLAYER 0x00a1b2c0          complete, 100% match
// FUNCTION: PLAYER 0x00a1b2c0 FOLDED   identical code the linker merged
// STUB: PLAYER 0x00a1b300              incomplete, skipped by reccmp
// TEMPLATE: PLAYER 0x00c0ffe0          template body, nameref on the next line
// SYNTHETIC: PLAYER 0x00401040         compiler-generated (scalar deleting destructor, thunk)
// LIBRARY: PLAYER 0x00f01000           CRT or third party, in library_msvc.h inside #if 0
// GLOBAL: PLAYER 0x01234560            global variable
// VTABLE: PLAYER 0x01100440            virtual function table
// SIZE 0xf8                            class size assertion
```

- **A function in both binaries gets two markers,** PLAYER first, then STUDIO. Each binary is scored separately, so a function can be `FUNCTION:` in one and `STUB:` in the other.
- **Eight lowercase hex digits** for every address. CI checks this with `tools/reccmp_addr_padding.py`.
- **Ascending address order** within a translation unit. Under `/Gy` the linker keeps `.obj` order, so a descent usually means the source order or the pairing is wrong, and `reccmp-decomplint` fails on it. **Exception:** VC9 does not always emit an `.obj`'s functions in source order (IndexedMesh.obj puts `isUpperRoot`, line 219, at 0x008f90b0 before `~IndexedMesh`, lines 19–23, at 0x008f9160; Math.obj puts `matrixTiltQuadrant`, line 1272, before `rotationToByteBase`, lines 368–375). A descent that the 2013 assert lines pin stays; list it, never break the asserts to clear it. (OpenRBX says a descent silently drops later annotations; in reccmp `a0f494c0` it does not: `compare/ingest.py` pairs each marker through the PDB line records, `lines_db.find_function`, independent of order.)
- **The colon is required** on everything except `// SIZE`. reccmp skips a marker without one, silently.
- **`FUNCTION:` asserts 100%.** Anything less is `STUB:`. `library` in a CSV means third-party code and nothing else. Never use it to hide a Roblox function that will not match.
- Everything else under OpenRBX's *Annotations* section (namerefs in headers only, access labels above the marker, `// clang-format off` around long namerefs, braces split onto two lines, overloads paired through a synthetic row, `FOLDED` aliases) applies here unchanged. Read it there.

## Class Pattern

```cpp
// VTABLE: PLAYER 0x01100440
// VTABLE: STUDIO 0x01380a10
// SIZE 0xf8
class Instance : public GuidItem<Instance> {
public:
	virtual ~Instance();         // vtable+0x00
	virtual void VTable0x04();   // vtable+0x04, unnamed: no Mac symbol, no 2016 match

	// SYNTHETIC: PLAYER 0x004078f0
	// RBX::Instance::`scalar deleting destructor'

private:
	int worldIndex;                       // 0x28  mac: + ctor store at 0x004077a3
	undefined m_unk0x2c[0x40 - 0x2c];     // 0x2c
};
```

`DECOMP_SIZE_ASSERT(Instance, 0xf8)` goes in the `.cpp`. Member offset comments and `vtable+0xNN` comments are required. Gaps use a subtraction so the length documents itself. When starting a class, match the constructor, the destructor and the scalar deleting destructor first. Together they prove size, base chain, vftable layout and member order.

## Types and Names

- `undefined`, `undefined2` and `undefined4` from `util/decomp.h` stand in for unproven types. Do not write `int` or `float` until a 100% match proves it.
- **Name sources, best first:** Mac symbol (demangled), iOS symbol, RTTI, the 2016 source *confirmed by the 2013 binary*, string evidence (an assert or log message naming the function). Otherwise use the placeholders: `FUN_xxxxxxxx`, `g_unk0xXXXXXXXX`, `m_unk0xXX`, `p_unk0xXX`, `VTable0xXX`, `c_` on enum constants.
- Rename a placeholder only on unambiguous evidence. Competing plausible names mean the placeholder stays. A misleading name is worse than a neutral one.
- **File paths:** put each file at the path the 2013 binary's own `__FILE__` strings record (assert paths, log calls). Fall back to the 2016 tree's path only when the 2013 binary records nothing for that class, and note that in the file's header comment.

## Matching a Function

1. Read IDA's pseudocode and the disassembly side by side. Check the byte budget: the next function's address bounds the body. A body that cannot fit is a wrapper, and the logic is in a callee.
2. Look up the Mac symbol and its demangled signature. Look up the 2016 function of the same name, and read it for *intent* only.
3. Diff intent against the 2013 bytes. Every call, string, branch and store in the 2013 body has to be in the source, and nothing from 2016 that the bytes do not show.
4. Search the headers for a helper before writing arithmetic out. Hand-inlining a helper the original called is the most expensive mistake there is.
5. STUB every unknown callee, in ascending address order, with `STUB(0xADDRESS)` in the body so `/OPT:ICF` cannot fold them together.
6. Write clean C++, not decompiler pseudocode. No `*(int*)(this + 4)`, no gotos (OpenRBX's one sanctioned switch-dispatch goto excepted), no raw float bit patterns.
7. Build, then `reccmp-reccmp --target PLAYER --nolib --verbose 0xADDRESS`, and iterate to 100%.
8. Compare the vftable address too, not only the functions. That is what exposes a missing or misordered virtual.
9. Re-verify every function that touches a class you changed, run the [Checks](#checks), then **update the `.i64`** with any name, type or bound you learned, and log the session.

## Decompilation Principles

These carry over from OpenRBX unchanged:

- A type is proven when a `// FUNCTION:` that uses it reaches 100%, and not before.
- A diff means the code is wrong. Find the cause: layout, types, a missing base, an inlining decision. Register allocation is never the explanation.
- Fix the callee before sweeping the caller.
- A claimed match is not a verified match. Re-measure anything inherited, generated, or carried across from 2007 or 2016.
- Every annotation carries a real address. No placeholders.
- Casts plus pointer arithmetic mean the types are wrong. Find the real class.
- An unexplained address gap means a function sits in the wrong class, or one file mixes classes. Move whole classes.
- Measure with reccmp on the linked image, not by eye on an `/FA` listing.

## Codegen Patterns (this compiler)

**Proven 2026-11-12 by the Lua probe** (`tools/msvc9/lua_probe.py`, `notes/runs/2026-11-12-phase1-resume/codegen-probe.md`): VC9 SP1 15.00.30729.01 through `tools/msvc9/cl` reproduces **52 of 52** probed Lua functions in PLAYER byte for byte (masked), and 298 of 376 in a wider 17-TU scan. That settles the compiler: any argument from folder names (`Trunk2012`, `Client_2012.sln`) is outweighed by identical bytes.

- **App flags are `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0`. Two 2016 values are wrong for 2013: `/Oy` (no frame pointer), not `/Oy-`, and `/arch:SSE`, not `/arch:SSE2`.** Tells: `/Oy-` adds `push ebp; mov ebp,esp` (3 of 52 survive it); `/arch:SSE2` turns double compares into `movsd/ucomisd` where the target has `fld/fucomip`; no `/arch` gives `fcomp/fnstsw`. Engine-wide census (`tools/arch_census.py`): PLAYER `.text` has 0 scalar-double SSE2 instructions and 24,013 scalar SSE1. Proving functions: `luaO_rawequalObj` 0x008a7940, `luaH_getnum` 0x0098c480.
- **`/fp:fast`:** `math_deg` 0x008ad760 is an `fmul` by 57.29577951 only under fast. **`/GS-`:** 8 llex/lparser functions (e.g. `luaX_next`) match only without the cookie.
- **VC9 gives static functions custom register conventions without `/GL`:** `pushstr` 0x008a7a50 takes `L` in `esi` and the string in `edx`; `index2adr` 0x00873950 takes `ecx`/`eax`. Whether `<xmmintrin.h>` (or an `__m128` type) is visible in the TU changes this: without it, `newlstr`/`luaS_newlstr`/`fixjump`/`patchlistaux` pass the first argument in `ebx`. So each TU needs its real include set.
- Loop-alignment padding (`lea ecx,[ecx]`, `mov edi,edi`) and in-function jump tables reproduce. IDA's function size stops before an in-function jump table; compare up to the `.obj` length.
- **The Lua in the existing tree is the 2016 hardened fork, not 2013's.** PLAYER has stock struct offsets and order (no `LUAVM_SECURE`, no shuffled fields, no `LuaVMValue<T>`, no `ckey`: `panic` at `G+0x58`).
- **The real include set for that effect is `<intrin.h>` through `rbx/Debug.h` → `RbxPlatform.h`** (on WIN32 it includes `<windows.h>` and `<intrin.h>`, which pulls in `xmmintrin.h`). 2013's `luaconf.h` includes `rbx/Debug.h` for `RBXASSERT` (asserts at `luaconf.h:852`/`:862` in `RobloxExtraSpace` 0x00874f40/0x008766a0). So every Roblox TU that sees `RbxPlatform.h` gets the "intrinsics visible" register conventions.
- **Flags per library, proven 2026-11-12** (`notes/runs/2026-11-12-phase2-skeleton/lib-*.md`):
  - **Network = the App set.** 566 RakNet functions are identical under it; `/Oy-` drops that to 108, `/arch:SSE2` to 496, `/GS` to 424, `/Ob1` to 330. `/fp:fast`: `ReadFloat16` 0x005c3910. `/arch:SSE`: `SystemAddress::operator=` 0x005710d0.
  - **The static boost libraries (filesystem, iostreams, program_options) are built `/GS`** (cookie on), unlike App: `boost::iostreams::zlib_error::check` 0x006130e0 has the `__security_cookie` prologue. `/EHsc`, not boost.build's `/EHs`: `codecvt_error_category` 0x009c0f20 has no EH frame around its static's `atexit`. **`boost.dll` is `/GS-`** (47/47 identical; 35/44 with `/GS`) with `BOOST_SYSTEM_SOURCE` and `BOOST_CHRONO_HEADER_ONLY`.
  - **program_options is LTCG (`/GL`) in PLAYER.** `untyped_value::xparse` 0x00478330 passes `this` on the stack to the `multiple_occurrences` ctor at 0x00478060 and keeps an `and esp,-8` aligned frame. Only its vftable slots and cookies match an `.obj` compile; its internal calls cannot.
  - **zlib is stdcall (`ZLIB_WINAPI`):** `deflateInit2_` 0x009e1e80 ends `ret 0x20`; boost iostreams calls into that one copy.
- **`Vector3& __fastcall operator=` is user-defined in 2013 G3D** (upstream 8.00 has it; the 2016 fork and the existing App tree's `Vector3.h` do not). Tell: `Plane::getEquation` 0x009e3b40 copies with `fld/fstp` instead of a `mov` block. Every App function that assigns a `Vector3` depends on this.
- **`/OPT:ICF` folds library bodies with Roblox bodies.** 13 PLAYER addresses hold a boost/RakNet body that is also a Roblox function (e.g. 0x00402730 `RBX::Instance::canClientCreate`, 0x0062aa40 `reloadMT` vs `ClientAppSettings::Initialize`). Such an address gets one name; the other is `FOLDED`.

- **Every engine library's flags, proven 2026-11-12 by 77 byte-identical probes** (`tools/probe_flags.py`, census `tools/probe_libs.py`, `notes/runs/2026-11-12-phase3-measure/probe-libs.md`). STUDIO uses the same set as PLAYER for every library.
  - **Base, AppDraw, RbxG3D, RbxOgre, GfxAdapters = the App set** (`/Oy`, not 2016's `/Oy-`).
  - **GfxBase is `/Od`** (unoptimized) with `/Ob2 /GS- /arch:SSE /fp:fast`: 7 of 7 probes match per target and all 7 are lost under `/O2`. The 2016 project leaves Optimization unset, which means Disabled.
  - **The client shells are `/O2 /Oy /GS /fp:precise /GL` with no `/arch`** (x87 only, `fnstsw` compares, zero SSE): PLAYER's WindowsClient (also compiling Win, ClientBase, ClientShared and boost program_options) and STUDIO's RobloxStudio (+ ClientShared, Win, ClientBase, Qt defines without `QT_NO_DEBUG`). The exes link `/LTCG`. Tells: 12.9–19.9% of shell functions take arguments in non-standard registers against 0–1.9% in every engine library. The 2016 `/GS-` and `/arch:SSE2` are wrong here too.
  - **Log.dll is `/O2 /Oy /GS /GL`, no `/arch`.** **DeviceCaps (DXUT) has `NDEBUG` undefined**: its `assert()`s survive as `_wassert`.
- **Float and math patterns, proven 2026-11-13 by Phase 5a** (`notes/runs/2026-11-13-phase4-core4/math-5a.md`):
  - `G3D::iRound` is upstream's inline-asm `lrintf`, not the fork's ±0.5 plus `cvttss2si`. Tell: `movss [esp+N]; fld dword; fistp dword`. `Math::iRoundVector3` 0x00757ab0, 24.6% → 100% (G3D patch 0011).
  - NaN and denormal tests are `boost::math::fpclassify<float>`/`isfinite<float>` (integer tests on 0x7F800000/0x7FFFFF): `Math::isDenormal` 0x00759a40, `isNanInf` 0x00759aa0.
  - G3D 2013 has upstream reference counting: `MemoryManager` is a `ReferenceCountedObject`, so `Array`'s constructor is out of line (0x008e6db0) (G3D patch 0012).
  - `/O2` inlines a function defined later in the same TU, so TU membership matters: with `orthonormalizeIfNecessary` back in Math.cpp, `fromRotationAxisAndAngle` 0x00759ca0 reaches 100%. A recursive inline helper is inlined one level and calls its out-of-line copy (`IndexedMesh::setUpper` 0x008f9590).
  - `/Oi` inlines `memcmp` as a dword loop plus a byte tail (`png_sig_cmp` 0x009e96c0). A `double` literal compare stays x87 `fld qword` under `/arch:SSE` (`closestPointOnRay` 0x00758ff0).
- **Boost.Thread is called through `dllimport`** (`call [imp]` to boost.dll's `get_tss_data`), not through a thunk, so App needs `BOOST_THREAD_DYN_LINK`/`BOOST_ALL_DYN_LINK` (`Security::Context::current` is held at 89.7% by it).
- **STL: `_SECURE_SCL=0`**, proven by `Poly::getNumSurfaces` (PLAYER 0x00754610, STUDIO 0x008939c0), which stops matching under `_SECURE_SCL=1`; `_invalid_parameter_noinfo` is imported nowhere. `sizeof(std::vector<T>)` = 0x10, `sizeof(std::string)` = 0x1c (StatsService ctor).
- **Asserts carry the build machine's path and the 2013 line layout.** `__FILE__` is `C:\BuildAgent\work\407e17641b466b38\Client\App\Util\Math.cpp` in PLAYER (STUDIO: `fa0a2141fe3d2f4d`), with the original directory case, so one object can match only one target's strings. `__LINE__` is the 2013 line, which the assert strings record (Math.cpp 1308, 1343, 1420). 75 library functions (60 RakNet) differ from PLAYER by this string alone.
- **App code calls G3D helpers out of line that the 2016 fork inlines** (`tools/lib/g3d_calls.py`, G3D patch 0006): the `Matrix3` copy constructor 0x009d7f30 (315 calls in 237 functions; folded with `Matrix3(const float[3][3])`), `Matrix3::zero`/`identity`, the `Vector3` statics `zero`/`one`/`unitX`/`unitY`/`unitZ`/`inf`/`nan`/`minFinite`/`maxFinite` (0x009da920–0x009dab20), `Vector2::zero`/`one`, and `CoordinateFrame()` 0x009d9550 (160 calls). `Matrix3::operator=` stays inline (`rep movsd` at 0x007254de). Upstream G3D 8.x headers are the reference, not the fork.
- **How VC9 forms `__FILE__`** (probe under wine, matches the 2013 strings): the main file is its command-line spelling; a header found on an `/I` path is that `/I` spelling + `\` + the `#include` spelling (`C:\BuildAgent\work\<hash>\Client\App\include\Util/Extents.h`); a header found next to its includer is the includer's directory **lowercased** + `\` + the `#include` spelling (`c:\buildagent\…\client\network\raknet\source\DS_List.h`). `tools/msvc9/winewrap.py` reproduces this with `OGRE_TARGET=PLAYER|STUDIO`: the 2013 checkout paths are symlinked into the wine prefix's `drive_c`, sources are passed in their 2013 spelling (`tools/msvc9/paths-<T>.txt`, `gen_paths.py`), App's include root is spelled per project (RbxOgre `Client/App/Include`, GfxBase `Client/app/include`, the rest `include`), and boost comes from `C:\Users\build\build_SM-UTIL14\…\Contrib2\boost_1_53_0\include` (STUDIO: `SM-UTIL13`). That took the "file string only" misses from 59 per target to 0.
- **Class layout:** MSVC places polymorphic bases first, in declaration order, then empty non-polymorphic ones (Instance RTTI 0x00b2a6ec). App classes derive `Debugable`, which has `virtual void dump(std::ostream&)` at slot 0 and no virtual destructor (4 bytes) and is `novtable` (`RBXBaseClass`): PLAYER has no Debugable vftable and no constructor stores one. `Verb` derives it too. `Object` and `AbstractFactoryProduct` are **not** `novtable`: the Instance ctor stores AbstractFactoryProduct's vftable 0x00b2a2f0 at 0x00620df9 and the dtor stores Object's 0x00b29f08 at 0x0061d684. The 2013 Instance constructors are `Instance(FWInstance*)` 0x00620da0 and `Instance(const char*, FWInstance*)` 0x00620ed0. `FWInstance : FWBase, Allocator<FWInstance>` (ctor 0x0061c800), objects are created as `FWFinal<T>` through `FWBase::init<T>` 0x0061fe70 and held by a plain `shared_ptr<FWBase>`. `rbx::spin_mutex` is `tbb::spin_mutex` on Windows (`__TBB_LockByte` 0x00402240 → tbb.dll). `rbx::atomic<int>`'s parameterless ctor is out of line (0x004022d0). `Diagnostics::Countable<T>`'s ctor and dtor call `RbxInterlockedIncrement`/`Decrement` (0x009b8f10/0x009b8f00) out of line: Instance ctor 0x00620f05, ThreadRef(lua_State*) 0x00673934, Job::Job 0x009bac28; hand-written counts are wrong. `FWBase` is 0x18 and `FWInstance` 0x38, with its three `FWValue<bool>` at 0x18/0x19/0x1a (FWInstance ctor 0x0061d7e0); `IFWHolder` has 5 slots and no virtual destructor (vftable 0x00b79100). **`Edge` is 0x1c, not 2016's 0x20:** `Sim::EdgeState` and `ThrottleType` are `enum : unsigned char` (VC9 extension), proven by SleepStage 0x008e7e00 reading bytes `[edge+8]`/`[edge+9]` and the Joint SpanningEdge thunk 0x008eaa40 (`sub ecx,0x1c`). A user-declared destructor makes the deleting destructor store the vptr, so it stops folding with a twin (`~AssemblyStage` 0x008ea3c0 vs EdgeBuffer 0x008ea600).

Still to re-prove from OpenRBX's *MSVC 8.0 Codegen Patterns* list, with a probe compile each:

- Argument evaluation order, NRVO, the inlining budget (`/Ob2` heuristics changed between MSVC 8 and 11), `std::min`/`max` returning references, and the switch lowering thresholds all need re-checking.

Write each confirmed pattern here as OpenRBX does: the rule, the instruction-level tell, and the function that proved it.

## Checks

Run all of them before committing, not just the accuracy number:

```bash
reccmp-reccmp   --target PLAYER --nolib                       # score
reccmp-reccmp   --target PLAYER --nolib --verbose 0xADDRESS   # one function
reccmp-reccmp   --target PLAYER --nolib --json out.json       # what is left (filter on "effective")
reccmp-decomplint Client --module PLAYER --warnfail
reccmp-vtable   --target PLAYER
reccmp-datacmp  --target PLAYER
python tools/check_folded.py --target PLAYER
python tools/reccmp_addr_padding.py
```

**STUDIO builds in its own tree**, because its asserts carry a different checkout path: `cmake -G Ninja -S . -B build-studio -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake -DOGRE_TARGET=STUDIO && cmake --build build-studio --target studio`, `python3 -P tools/ida/gen_reccmp_csv.py STUDIO --build build-studio`, then run reccmp from `build-studio/`. PLAYER uses `build/` as above. `gen_reccmp_csv.py` takes `--json A.json,B.json` and **must pool 2–3 fresh reccmp runs of the current link**: its position passes are not monotone from a single run (one run lost 83 entries), and a run from an earlier link pairs stale recomp addresses (the VerbContainer D0 0x006f9250 once got `Assembly::onLowersChanged`). function-size rows extend over in-function jump tables and over tails IDA cut at a call it took for noreturn (`Bridge<T>::on_newindex` 0x00670170: 40 → 48 bytes). `tools/app_sweep.py set FUNCTION|STUB`, `apply P.json S.json`, `report`, `xrefs` does the one-build sweep and writes `FOLDED`. `--json` reports an effective match with its raw ratio and a separate `effective` flag, so filter on the flag. Twin pairing in the generator is deterministic (a reccmp 100% pair is 1,000 votes; ties go to a `match:` name, then the lowest address). `tools/vtable_structural.py` splits `reccmp-vtable` mismatches into structural (a slot missing, extra or misplaced) and pairing-only (identical twins, unpaired scalar deleting destructors). `tools/reccmp_addr_padding.py` needs the venv python. Set `PYTHONIOENCODING=utf-8` or a successful `--verbose` dies on an emoji in a cp1252 console.

Upstream [reccmp](https://github.com/isledecomp/reccmp) at `a0f494c0` (`tools/requirements.txt`) **plus one local patch**, `tools/reccmp-patches/0001-ogre-imports-and-vc9-funcinfo.patch`, applied to `toolchain/venv` by `sh tools/reccmp-patches/apply.sh` (safe to rerun). It does two things upstream cannot: (1) a CSV type `import` with lowest-address-wins, so calls pair with the linker IAT at 0x00aa1000 and not VMProtect's copy in `.vmp1` (without it a call through `[DeleteCriticalSection]` scores 66.67%); (2) FuncInfo magic 0x19930521/0x19930522, which VC9 writes and upstream ignores (PLAYER 1 record → 6,339). Other limits that stay: without `.reloc`, reccmp's pointer scan cannot see pointers into `.text` (vtable slots, function tables), so vtables and globals need RTTI-derived CSV rows; and the import-thunk scan counts every `jmp [IAT]` as a thunk, tail jumps included.

**How reccmp pairs without markers** (read from `a0f494c0`): a CSV `symbol` pairs only with a PDB PUBLICS name, and a CSV `name` only with a PDB proc name. So an oracle name in Itanium spelling (`RBX::Face::minMax(G3D::Vector3 const&, …) const`) never pairs; `tools/ida/gen_reccmp_csv.py` resolves library rows against our own map and PDB (`tools/lib/pdb_view.py`), and `reccmp/<t>-libdata.csv` adds what paired functions reference (globals, vtables, strings, floats, throw info, `__ehhandler$` stubs reccmp's own finder misses). Oracle names pair through a normalised signature (`norm_sig`/`oracle_pairs`: qualified name, parameter types and const, std typedefs folded, Itanium D0 → MSVC deleting dtor) and only when unique on both sides; the row then carries our exact PUBLICS name in the `symbol` column (673 PLAYER pairs). Statics have no PUBLICS entry; anonymous-namespace hashes (`?A0x…`) depend on the source path. Without `.reloc`, immediates such as Lua's 0x7fc000 masks are read as addresses.

Build and score: `cmake -G Ninja -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake && cmake --build build`, then run reccmp from `build/` (CMake 3.25+, or `/Z7` is dropped and the PDB has no line records). The imported App tree is in the default build (`OGRE_PRIOR_APP=ON`); the exes link with `/FORCE:UNRESOLVED` (`OGRE_FORCE_UNRESOLVED`, about 40 symbols, listed in `Client/_generated/<t>/unresolved.txt`), and `tools/gen_link_stubs.py` binds unresolved callees that have an original address to generated `STUB`s through `/alternatename`. The exe link lines exceed 32,767 characters, so `cmake/link/` forces response files (wine's link.exe otherwise exits 1 silently).

## reccmp Data Files

Same scheme as OpenRBX: one CSV per library under `reccmp/`, generated rows (plus `#` lines reccmp skips), and per-target prefixes (`player-*.csv`, `studio-*.csv`).

- **`player-function-size.csv`** is the baseline of every function extent, generated from `ida/exports/PLAYER-functions.csv`, **not** hand-edited. Filter out unwind funclets, catch handlers, adjustor thunks and import thunks the same way OpenRBX did, and note the filter's function count in this file.
- **Library files** (`-crt`, `-stl`, `-boost`, `-g3d`, `-lua`, `-raknet`, `-qt`, `-curl`, `-zlib`, …, one per library actually found in Phase 0) come from FLIRT hits on our own compiled libraries and from relocation-masked body matching, typed `library`.
- **`player-symbols.csv`** names Roblox functions the tree does not annotate yet (from Mac symbol transfer), with no type, so they stay in the denominator. It is regenerated, never edited, because a `symbol` or `name` row is a *pairing*, not a label.
- **`studio-*.csv`** files are generated from their `player-` twins by body matching, except `studio-client.csv`, which holds hand rows for what only Studio has (Qt UI, legacy MFC tools). A hand row anywhere else is lost on the next transfer.
- **Addresses are absolute** (8 hex digits), which is what reccmp `a0f494c0`'s CSV loader expects, and filter counts sit in `#` lines reccmp skips; both differ from OpenRBX. `tools/ida/gen_reccmp_csv.py` writes every file, library rows from `ida/exports/<T>-lib-resolved.csv` (`tools/lib/lib_consistency.py`).
- Every other rule under OpenRBX's *Rules that bite* applies: quoted labels containing commas, row order for folded addresses, `stub` for vftable slots not yet byte-exact, a blank type is skipped rather than applied.

## Build Layout

Each original static library gets its own CMake OBJECT library, because each one was compiled with different flags. Without a PDB there are no command blocks to read, so the flags are inferred and then **proven** with the evidence below:

- `/GS`: a `__security_cookie` load in the prologue.
- `/Oy-` or `/Oy`: whether `ebp` frames are kept.
- `/arch:SSE2`: scalar SSE in float code.
- `/fp:fast` vs `/fp:precise`: compare reassociation and the handling of compare-with-NaN.
- `/GL`: Rich header LTCG prodids, cross-module inlining, and calling conventions MSVC rewrote to register passing on internal functions.
- `/Ob`: whether small helpers are ever out of line.
- `/EHsc` vs `/EHa`: FuncInfo flags.
- Runtime library: whether the CRT is imported or linked in.

Start from the 2016 `.vcxproj` value, then flip it if the bytes disagree, and write the deciding function next to the flag in `CMakeLists.txt`.

**If the Rich header shows most of the engine was built `/GL`, stop and tell the user before Phase 2.** Under whole-program optimization, a function's codegen depends on the entire program, so byte-exact matching only becomes possible near the end. `DECOMP_PLAN.md` has the fallback.

The `.text` section records link order, as it did in 2007: client objects, then each engine library, then third party. Recover it from the string and RTTI bracketing, and write it down here once known.

Known so far (PLAYER, from `ida/exports/PLAYER-lib-resolved.csv`): RakNet's own `.obj`s sit at 0x005a6600–0x006091b0 inside Network (scattered template and header copies elsewhere); Lua at 0x00873950–0x008b0750 and 0x00988f70–0x009a8150 (two runs, so two link groups); boost filesystem's `.obj`s at 0x009bd680–0x009c0fb0; the G3D library (G3D + zlib + libzip interleaved in resolution order, so one static lib as in 2016's graphics3D project) runs 0x009d5b70–0x009edf00, right after `RBX::AdornBillboarder` and before `RBX::Frustum` (RbxG3D, 0x009ee010); D3DX9 is the tail (about 0x009ef000–0x00a5b000). `/Gy` keeps `.obj` order inside each library.

## Gotchas

- **Body matchers that mask float constants call different functions identical.** `G3D::Color4::one` at 0x007681b0 (loads 0.6f) and `Color3::one` at 0x009d7c30 (0.7f) were "identical" rows until reccmp compared them. Compare constant pools, not just masked code.
- **Link order decides which inline copy survives.** With App linked before g3d, the linker keeps ChatOutput.cpp's inline `Matrix3::identity`/`zero` (EH frame, `fld1`) and the g3d copies drop from 100% to 8% and 23%. The exes now link in the 2013 `.text` order: PLAYER client, RbxOgre, Network, GfxAdapters, App, Base, AppDraw, GfxBase, g3d, RbxG3D, boost_static; STUDIO client, RbxOgre, Network, GfxAdapters, RbxG3D, g3d, GfxBase, App, Base, AppDraw, boost_static. (program_options really belongs inside WindowsClient.)
- **Without `.reloc`, scores churn on coincidences.** An immediate in the original (RakNet's `0x7fffff`) can equal one of our data addresses after an unrelated change and drop a function from 100%; identical-body twins (folded by the original's linker) pair with a different twin on each CSV regeneration. Compare `--json` runs before and after, and read every lost 100% with `--verbose` before blaming a change.
- **`reccmp-datacmp` crashes (KeyError in `cvdump/types.py`) on ClassDescriptor-typed statics** (`ClassDescriptor::rootDescriptor`'s `root` 0x00e24868, the `describedClassDescriptor` statics). Do not drop them from the CSVs: that costs `Described<PVInstance>::classDescriptor()` 0x004b6a30 (and three more) their 100%. `reccmp-project.yml` lists `describedClassDescriptor` and `root` under `report: ignore-variables` instead, and datacmp then runs to the end (exit 1 only because issues remain).
- **The prior tree has wrong markers and misplaced code.** It had `Extents::vv` at 0x009dab20, which is `Vector3::maxFinite` (fixed); 29 assert paths in it are hard-coded Mac literals (`/Applications/buildAgent/work/fa0a2141fe3d2f4d/…/ScriptContext.cpp`; ScriptContext's 11 are fixed, ThreadRef.cpp's 17 cite other files and wait on moves). The assert's file is where the 2013 function lived: 37 PLAYER functions sat in the wrong file, 32 were moved on 2026-11-12 (`SimJobTracker` into `SimJob.cpp`, `KernelData` inline in `KernelData.h`, `EdgeList` into `Primitive.h`, the text XML reader/writer into `XmlSerializer.cpp`; `notes/runs/2026-11-12-phase4-core2/placement.md`), and 5 remain (`getComponent` 0x0071b080, `MergeBinder::announceIDREF` 0x00447460, `World::insertJoint`/`removeJoint`, `GuiBuilder::updateSummaryStats`).
- **`__LINE__` is matched by layout, not by `#line`.** Lay the header or `.cpp` out so each assert lands on its 2013 line (`Joint.h`: `getJointType` 106, `align` 111, `resetLink` 183 took all three to 100%). `#line` would break reccmp's marker pairing through line records. `RBXASSERT_LINE(expr, line)` in `ScriptContext.cpp` is a stopgap for files that cannot be laid out yet; replace it when the file is.
- **Include spelling is part of the match.** A template defined in a header gets its PDB line records under the `#include` spelling, and reccmp reports "Debug data out of sync" for its marker when that differs from the spelling the original's `__FILE__` records: `EnumDesc<T>::convertToItem` 0x007761a0 stays unpaired while `InputObject.cpp`, `LuaArguments.h` and `LuaEnum.h` include `Reflection/EnumConverter.h` where 2013 wrote `reflection/enumconverter.h`.
- **Reorder functions with `tools/reorder_markers.py` (plan / apply / `--strict`), never by hand.** It moves whole marked definitions within blank- or comment-separated runs, refuses any move that shifts an assert's line, and refuses a run that adds STUDIO descents. Its loose mode can break declaration order (C2908 in `reflection/type.cpp`); rebuild after every apply.
- **A `--json` run is valid only with the PDB and map it scored.** An agent that relinks `build/` after a run makes every join of that JSON with `build/` (`gen_reccmp_csv.py`, `match_names.py`) wrong. Rescore, or keep copies of the scored PDB and map. reccmp's "Implemented" count skips `STUB` markers, so flipping a marker to `STUB` lowers it without losing any match.

- **Run heavy commands through `tools/capped.sh`** (builds, reccmp, idat/idalib, PDB or evidence parsers). Claude Code runs inside VS Code's systemd scope, and systemd stops that whole scope, VS Code included, when the kernel OOM-kills anything in it. On 2026-11-12 a 13–22 GB python (`tools/ida/import_types.py dump`, which then loaded the whole PDB type stream through reccmp's Cvdump parser; it now streams `cvdump -t` and peaks at about 50 MB) crashed VS Code twice. `capped.sh` gives the command its own scope with `MemoryMax` (default 8G, `OGRE_MEMMAX` to change; exit 137 means it hit the cap). Builds use a 6-job CMake pool (`OGRE_COMPILE_JOBS`). reccmp-reccmp peaks at about 0.7 GB.
- **Roblox clients of this era checked their own code at runtime** (memory/hash checks against `.text`). Never patch a target binary in place to test a hypothesis. Use a debugger on a copy, or reason statically. A patched binary that "works" proves nothing.
- **A comment-only edit that adds lines needs a rebuild before scoring.** Markers resolve through the recompiled PDB's line records.
- **`.clang-format` regroups includes.** Use `// clang-format off` to pin an order that matters.
- **Do not sweep timestamps across the working tree** while vendored trees are marked `-text`.
- **A database opened in two IDA instances, or killed mid-save, can corrupt.** Snapshot first.

## Project Structure

```
Client/            # Engine libraries and the client projects, layout from the 2013 __FILE__ strings
3rdparty/          # SDKs the original built against from outside its tree
util/              # decomp.h, decomp.cpp, compat.h (from OpenRBX)
reccmp/            # player-*.csv, studio-*.csv
tools/             # check_folded.py, reccmp_addr_padding.py, ncc
tools/ida/         # IDAPython: exports, name transfer, CSV generation
ida/               # .i64 databases (gitignored), snapshots/, logs/, LOG.md
ida/exports/       # text exports from the .i64s (committed)
ref/               # 2016 source, Mac and iOS builds (gitignored, never committed)
bin/               # original Windows binaries (gitignored, never committed)
DECOMP_PLAN.md     # phases and status
```

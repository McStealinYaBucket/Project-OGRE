# app-decomp-addresses (workflow phase1-names, 2026-11-11)

ANSWER: The address comments in "2013 55% App/" do not address any Windows binary. They address the Mac Studio build (ida/MACSTUDIO.i64, i386 Mach-O, 2013.10.23.2 / 0.126.0.39503, SHA256 92f41e7c...ddbb7a) at its native load addresses (__TEXT at 0x1000, __text 0x8430-0x9588a6), with constant offset 0. HANDOFF.md's label "IDA database for the Player PE ... dbq24.i64" is wrong; the tree's own file headers say "MAC dbq24", "Mach-O studio 0.126.0.39503" and "october2013symbolledrobloxstudio.i64" (the filename of ref/macstudio_dup). That is why addresses like 0x2581B0 sit below 0x400000.

COUNTS (tools/app_decomp_addr_census.py; an address comment = a 0x token of 5-8 hex digits inside a // or /* */ comment):
- 6,939 address comments, 3,522 distinct addresses, in 569 files.
- Distinct annotated functions: 2,733 MACSTUDIO functions (2,708 annotated at their start, 25 only through an interior address). 2,667 of them carry a Mac symbol name, 66 are sub_*.

FUNCTION START vs INTERIOR, per target (all 6,939 comments):
- MACSTUDIO: 5,831 start (84.0%), 742 interior (10.7%), 366 miss (5.3%).
- PLAYER: 50 start (0.7%); PLAYER_1015: 25 (0.4%); STUDIO: 42 (0.6%); IOS_1029/1021/1014: 70/52/29. These are chance level.
- The 366 MACSTUDIO misses by Mach-O section: 109 are outside every section (float bit patterns and other constants, not addresses); 155 in __DATA (__common 65, __const 48, __data 21, __bss 21) and 3 in __nl_symbol_ptr (globals, descriptors, vtables); 60 __cstring, 19 __TEXT,__const, 12 __gcc_except_tab; 8 in __text that are range end addresses (e.g. "0x18A30C-0x18A54B").
- By distinct address: 2,708 start, 567 interior, 247 miss.

SIZE CHECK (independent of strings): the tree has 1,192 notes of the form "0xADDR (NNNB". In MACSTUDIO, 1,145 (96.1%) are a function start whose IDA size is exactly NNN. In PLAYER, PLAYER_1015, STUDIO and all three iOS builds: 0.

STRING-REFERENCE TEST (tools/app_decomp_string_test.py + tools/ida/string_refs.py, run on scratch copies of MACSTUDIO, PLAYER, PLAYER_1015, STUDIO): 305 (address, string literal) pairs over 162 distinct commented addresses, each literal tied to exactly one address in the tree.
- MACSTUDIO: 136 addresses testable (string present and referenced from a function); 111 (81.6%) land inside a function that references the string at offset 0, 103 of those exactly at its start. Offset vote: +0x0 gets 111, the next best 64K-aligned offset gets 1.
- The 25 MACSTUDIO failures are pairing artefacts, not a different base: 10 are data addresses (reflection descriptor globals such as 0xFA65FC, whose strings are referenced from static initialisers like InitFunc_123 at 0x1AA4F0), 15 are comments naming a callee or neighbouring function inside my 40-line window (e.g. LuaLibrary.cpp:510 0x189CD1 vs registerDevelopmentLibrary at 0x63056C).
- PLAYER: 137 testable, 0 at offset 0; no 64K-aligned offset collects more than 1 address and no 4K-aligned offset more than 2. PLAYER_1015: 0 of 137, same vote ceiling. STUDIO: 0 of 137, same. No image base or constant rebase maps the comments onto any Windows binary.
- 257 of the 305 strings are present in each Windows binary, so the code is there, at unrelated addresses.

CONSISTENCY: one binary, one base, across every directory. Exceptions, all small:
- At least 7 addresses belong to the 2016 PDB-named Windows binary and are labelled so ("BinCodecs_2016.txt, PDB-named"): 0xA45D40, 0xA42670, 0xA41A00, 0xA47260, 0xA41140, 0xA44370 in App/v8xml/SerializerBinary.cpp/.h and 0x467C10 in App/include/reflection/property.h.
- Several comments say "in the 2013 PE" for addresses that are Mach-O __DATA (0xFA9B20, 0xFA9B44, 0xFA9AFC, 0xF88531 in ScriptContext.cpp/.h); treat "PE" in this tree as a mislabel.
- The 13 comments in vendored trees (Bullet, g3d, raknet) are constants, apart from 1 interior hit that is likely coincidental.

PER-DIRECTORY (comments / distinct addresses / MACSTUDIO start, interior, miss / distinct Mac functions):
- App/include: 3367 / 2377 / 3038, 184, 145 / 2104
- App/v8datamodel: 939 / 668 / 719, 157, 63 / 498
- App/util: 646 / 571 / 538, 36, 72 / 502
- App/script: 541 / 478 / 494, 42, 5 / 433
- App/v8world: 474 / 335 / 425, 43, 6 / 292
- App/gui: 365 / 304 / 127, 207, 31 / 99
- App/v8xml: 101 / 56 / 94, 0, 7 / 50
- App/v8tree: 78 / 48 / 77, 0, 1 / 47
- App/v8kernel: 69 / 55 / 69, 0, 0 / 55
- App/reflection: 39 / 26 / 39, 0, 0 / 26
- App/Lua-5.1.4: 5 / 5 / 5, 0, 0 / 5
- App/tool: 2 / 1 / 2, 0, 0 / 1
- App/security: 1 / 1 / 1, 0, 0 / 1
- Base/rbx: 50 / 33 / 47, 1, 2 / 30
- Base/include: 26 / 19 / 26, 0, 0 / 19
- Base (root): 1 / 1 / 1, 0, 0 / 1
- Network/include: 48 / 34 / 38, 6, 4 / 28
- Network (root): 18 / 15 / 13, 3, 2 / 11
- Network/raknet: 1 / 1 / 0, 0, 1 / 0
- Rendering/RbxOgre: 74 / 61 / 21, 51, 2 / 16
- Rendering/GfxBase: 69 / 52 / 49, 11, 9 / 41
- Rendering/g3d: 9 / 7 / 3, 0, 6 / 3
- G3D/include: 8 / 6 / 5, 0, 3 / 3
- App.BulletPhysics (4 subdirs): 8 / 8 / 0, 1, 7 / 1
- Top level: App 6627 comments / 2642 functions; Rendering 152 / 50; Base 77 / 47; Network 67 / 30; G3D 8 / 3; App.BulletPhysics 8 / 1.

CONSEQUENCE FOR PHASE 2: the prior decomp carries no Windows addresses at all. Every "// 0xADDR" is a MACSTUDIO address, so each annotated function already has its Mac symbol (2,667 named), and getting a PLAYER address for it is the same Mac-to-PLAYER pairing problem as the general name transfer. The body-pairing pass is not saved; it is replaced by a direct lookup into MACSTUDIO.

Canonical databases were not opened; scratch copies were deleted. Intermediate JSON (pairs, per-target string refs, per-comment records) is in /tmp/claude-1000/ogre-XgEG/ and is regenerable from the three scripts.

## Files

- `tools/app_decomp_addr_census.py`
- `tools/app_decomp_string_test.py`
- `tools/ida/string_refs.py`

## Unresolved

- Whether dbq24.i64 is byte-for-byte the same Mach-O as ref/macstudio (SHA256 92f41e7c...): not checkable without the prior operator's database. The evidence is indirect but strong: 1,145 of 1,192 size notes match function start and exact size.
- The 47 size notes (3.9%) that do not match a MACSTUDIO function start of that exact size were not examined individually; likely chunked functions or range notes, unverified.
- No Windows address exists for any annotated function. Mapping the 2,733 Mac functions to PLAYER addresses still has to be done by the Mac-to-PLAYER pairing pass.
- The string test pairs a comment with string literals by a 40-line window, not a C++ parse; it covered 162 of 3,522 distinct addresses. The census and size check cover all of them.
- The 742 interior comments were only checked as lying inside some MACSTUDIO function, not as lying inside the function the source text says they annotate.
- CLAUDE.md, DECOMP_PLAN.md and ida/LOG.md were not updated with this finding (out of scope for this step); the 'Push its names into ida/PLAYER.i64' rule in CLAUDE.md assumes the tree has PLAYER addresses, which it does not.

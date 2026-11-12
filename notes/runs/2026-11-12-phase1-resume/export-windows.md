# export-windows (workflow phase1-resume, 2026-11-12)

All six steps are done for PLAYER, PLAYER_1015 and STUDIO. The three canonical databases now have their RTTI named, and the evidence exports are written. No function was renamed and no function bounds changed. The partial tools/ida/msvc_rtti.py from the interrupted run had two bugs. (a) It required CHD attributes < 4, so it missed every CHD with the ambiguous bit set (attributes 5, e.g. CSourceStream, CHD 0x00c70da0). (b) Its rule for which base names a vftable was wrong: 23 of the 442 vftables OgreMain.dll exports came out wrong (Font's primary is 6BResource@1@@, not StringInterface; Camera's Renderable vftable is plain 6B@). I replaced the rule with clang's MicrosoftVTableContext path algorithm, run over the RTTI base array, plus an MSVC name-fragment parser that does back-references. It now reproduces all 550 vftable names exported by OgreMain.dll, RenderSystem_Direct3D9.dll, RenderSystem_GL.dll and Plugin_ParticleFX.dll exactly. That check is built in: `python3 -P tools/ida/msvc_rtti.py <dlls>`.

**Step 1, the scanner.** Counts are PLAYER / PLAYER_1015 / STUDIO.
- Type descriptors: 5,839 / 5,839 / 6,077. In PLAYER, 5,739 of them are class or struct descriptors.
- COLs, which equal vftables: 7,334 / 7,334 / 7,707.
- Classes with a vftable: 4,037 / 4,037 / 4,275.
- Slots: 50,346 / 50,346 / 57,240. Of these, 563 / 563 / 586 are _purecall and 1,902 / 1,902 / 1,949 are adjustor thunks.
- PLAYER has 5,839 distinct `.?A` strings in its raw data, so every one is a type descriptor; the "about 5,843" in the task is 4 more than that. The 1,702 class descriptors with no vftable belong to classes that have no vftable of their own.
- Vftables end at the next COL pointer in 5,773 cases. The rest end at data, a zero word, or a word something else references, such as the "png" bytes at 0xb02568 that look like a .text address.

**Step 2, the PE string path.** It had two gaps, both fixed:
- UTF-16 text that IDA had typed as a dword offset was missed. Example: `push offset off_BD7C1C` in 0x004034c0 is L"\RbxLogs\", and IDA read 0x0052005c as a .text address.
- A raw `push imm32` pointing at a string was missed. Example: 0x005e1530, where the address points 4 bytes into xmmword_B247DC.

Checked with tools/verify_evidence_strings_pe.py, a capstone-based verifier that does not use IDA: the string sets agree for 99.58% / 99.58% / 99.56% of functions that have a string. In PLAYER, 39 functions still differ. 20 are float constants the verifier reads as text ("333?", "fff?") and 1 is a GUID dword ("auds"). The rest are short literals the compiler copies with dword loads, such as "Level 00" in 0x456050, '[string "' in 0x8a7db0 and "<roblox!" in 0x6fd070; the export does not count those. I read 30 sampled functions against their instruction listings and all matched; I also checked 0x4034c0, 0x433f40, 0x456050 and 0x5e1530 against IDA's own state.

**Step 3, naming.** Snapshots are ida/snapshots/{PLAYER,PLAYER_1015,STUDIO}-02-pre-rtti.i64.
- TDs, COLs, CHDs, base class arrays, BCDs and vftables now carry MSVC's own decorated names (??_R0, ??_R4, ??_R3, ??_R2, ??_R1, ??_7). Secondary vftables carry their real base qualifier, e.g. ??_7Instance@RBX@@6B?$GuidItem@VInstance@RBX@@@1@@, instead of IDA's _0/_1 suffixes.
- Each one has a comment starting "rtti:": 34,286 / 34,286 / 35,966 comments.
- Vftable words are dword offsets; 86 / 150 / 73 words had to be re-made.
- Afterwards I checked every vftable word (57,680 / 57,680 / 64,947): all are dword offsets with an xref to the slot target, and all names and comments are in place.
- One name could not be applied: ??_7type_info@@6B@ at 0x00bc35bc (PLAYER) and 0x00bc163c (PLAYER_1015). The VMProtect loader's import slot at 0xe3a1d8 / 0xe381d8 already holds that name.
- The databases were saved packed. A second run renames nothing.

**Step 4, the exports.** ida/exports/<TARGET>-evidence holds functions.jsonl, strings.jsonl, vtables.jsonl and summary.json for all three targets.
- Same schema as the Mach-O exports. A vtables.jsonl record is one class; each of its groups is one vftable, with offset_to_top set to minus the COL offset. Each group also has col, col_offset, cd_offset, `for` (the base path in the name), and slots marked purecall or thunk {adjust, target}. Each record has a hierarchy field with the base class array.
- Functions are 48,858 / 49,235 / 44,599; 9,360 / 9,357 / 10,087 of them have a string.
- The `imports` field of functions.jsonl lists the __imp_ slots in .iat (or STUDIO's .idata) that each function calls or reads.

**Step 5, slot counts against IOS_1029** (tools/compare_vtable_slots.py). 672 of 681 comparable classes match what the Itanium ABI predicts. The prediction allows for Itanium's two destructor slots, its extra primary slots for functions that override a non-primary base, and a destructor that sits only in a secondary vftable on MSVC (RBX::Instance, because RBX::Debugable has no virtual destructor). Examples in the 20 spot checks:

| Class | MSVC | IOS_1029 |
|---|---|---|
| RBX::Instance family | [28,1,2,3] | [32,1,3,4] |
| RenderNode | [108,13] | [110,13] |

The 9 that do not match are the BodyMover family: one of its secondary bases has secondary bases of its own, which the predictor does not model. Another 20 classes have a different number of vftables on each side.

**Step 6.** I appended an entry to ida/LOG.md. functions.csv was not regenerated because no function name changed. Scratch copies are deleted.

## Files

- `tools/ida/msvc_rtti.py`
- `tools/ida/name_rtti.py`
- `tools/ida/export_evidence.py`
- `tools/verify_evidence_strings_pe.py`
- `tools/compare_vtable_slots.py`
- `ida/LOG.md`
- `ida/PLAYER.i64`
- `ida/PLAYER_1015.i64`
- `ida/STUDIO.i64`
- `ida/snapshots/PLAYER-02-pre-rtti.i64`
- `ida/snapshots/PLAYER_1015-02-pre-rtti.i64`
- `ida/snapshots/STUDIO-02-pre-rtti.i64`
- `ida/logs/PLAYER-02-rtti.log`
- `ida/logs/PLAYER_1015-02-rtti.log`
- `ida/logs/STUDIO-02-rtti.log`
- `ida/logs/PLAYER-02-evidence.log`
- `ida/logs/PLAYER_1015-02-evidence.log`
- `ida/logs/STUDIO-02-evidence.log`
- `ida/exports/PLAYER-evidence/functions.jsonl`
- `ida/exports/PLAYER-evidence/strings.jsonl`
- `ida/exports/PLAYER-evidence/vtables.jsonl`
- `ida/exports/PLAYER-evidence/summary.json`
- `ida/exports/PLAYER_1015-evidence/functions.jsonl`
- `ida/exports/PLAYER_1015-evidence/strings.jsonl`
- `ida/exports/PLAYER_1015-evidence/vtables.jsonl`
- `ida/exports/PLAYER_1015-evidence/summary.json`
- `ida/exports/STUDIO-evidence/functions.jsonl`
- `ida/exports/STUDIO-evidence/strings.jsonl`
- `ida/exports/STUDIO-evidence/vtables.jsonl`
- `ida/exports/STUDIO-evidence/summary.json`

## Unresolved

- Vftable names in classes with virtual bases (96 in PLAYER) depend on a heuristic that places the virtual bases on the remaining COL offsets in base-array order, without reading the vbtable. No exported name exists to check them against.
- 221 distinct adjustor-thunk targets in PLAYER (223 in PLAYER_1015, 213 in STUDIO) are not function starts: 163 sit inside another function's body (e.g. 0x007ff010 inside sub_7FF000) and 58 in a tail chunk (e.g. 0x006a9000 in sub_6A8E40). The function-discovery pass should split them; I left bounds alone because this task forbade changing them.
- Short string literals that the compiler copies with dword loads (e.g. "Level 00" at 0x00456050, "<roblox!" at 0x006fd070) are not counted as string references. Within the 0.4% that disagree, the one IDA string error found: it defined a string that starts with float bytes, '333?Unknown error.' (sub_6EAE00).
- The slot predictor does not model a secondary base that has secondary bases of its own (the BodyMover family, 9 classes). 20 classes have a different number of vftables in MSVC and Itanium (e.g. RBX::Seat: 10 vs 9). I did not trace which base causes the difference.
- A second run of name_rtti.py rewrites 113 comments with the same text. I believe this comes from IDA's comment length limit on very long template class names, but I did not confirm it.
- ??_7type_info@@6B@ stays unapplied in both Players: the name is held by the VMProtect loader's import slot.
- The 5,843 descriptor figure in the task does not match my 5,839 (there are exactly 5,839 distinct .?A strings in PLAYER's data); I could not find where 5,843 came from. The facts table in CLAUDE.md gives 5,616 '.?AV' descriptors for PLAYER_1015; I measured 5,839 TDs, 5,739 of them classes or structs, and did not reconcile it. I did not edit CLAUDE.md.
- The evidence exports add about 94 MB of JSONL under ida/exports. Whether to commit or gitignore them is still for the user to decide.

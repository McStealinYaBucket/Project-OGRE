# export-oracles (workflow phase1-names, 2026-11-11)

Oracle evidence export is written for MACSTUDIO, IOS_1029 and IOS_1021, and its string lists check out against an independent disassembly of the binaries.

**What was run**
- `tools/ida/export_evidence.py` (IDAPython, exits without saving) ran on byte-identical scratch copies of the three databases; the canonical `.i64` files were never opened and their SHA256s are unchanged (e272145d…, d09ff1f5…, 46a82348…).
- Output is in `ida/exports/<TARGET>-evidence/` as `functions.jsonl`, `strings.jsonl`, `vtables.jsonl` and `summary.json` (58 MB Mac, 72 MB each iOS).
- `tools/verify_evidence_strings.py` is the independent check: capstone on the raw Mach-O with its own movw/movt, literal-pool and PIC-base resolution, no IDA xrefs.

**Counts**

| | MACSTUDIO | IOS_1029 | IOS_1021 |
|---|---|---|---|
| Functions | 71,887 | 85,425 | 85,545 |
| With at least one string | 8,319 (11.6%) | 11,530 (13.5%) | 11,581 (13.5%) |
| Same, of `__text` functions only (stubs excluded) | 15.7% of 52,989 | 17.4% of 66,317 | 17.4% of 66,385 |
| With a type-name (`_ZTS`) reference | 914 | 931 | 931 |
| With a callee | 56,903 | 63,957 | 64,058 |
| With an immediate constant | 3,075 | 5,841 | 5,870 |
| With a named global or import | 34,346 | 40,848 | 40,908 |
| String literals (referenced by a function) | 20,373 (14,029) | 31,542 (20,056) | 31,622 (20,120) |
| Vtables with a `_ZTV` symbol | 3,567 | 4,623 | 4,625 |
| Construction vtables (`_ZTC`) | 711 | 740 | 740 |
| Vtables with no symbol | 392 | 0 | 0 |
| Vtable groups / slots | 8,683 / 110,329 | 9,432 / 116,342 | 9,433 / 116,327 |

**String verification**
- Whole population: export and verifier string sets are identical for 8,217 of 8,223 Mac functions (99.93%), 11,505 of 11,514 on IOS_1029 (99.92%) and 11,557 of 11,565 on IOS_1021 (99.93%).
- Of the leftovers I traced, the Mac ones (Qt `qt_metacast` strings) are copies in `__const`, outside the verifier's reach. On iOS, zlib `_deflate` (0x001bb90c) is consistent with a table lookup IDA follows further than the verifier, but I did not disassemble it, and `FlurryNetworkUtil` is a real disagreement about which NSString global is read. None of them was shown to be an export error.
- 24 named engine functions per database (about 1,000 strings each) all matched, e.g. `RBX::Instance::setParentInternal` (0x00542d16 Mac, 0x00701804 iOS), `RBX::LegacyContentTable` ctor (592 strings), `luaopen_math`. I also read `setParentInternal`, `jointsStepWorld`, `doDeclare<sAccoutrement>` and `luaopen_math` against IDA's listing by hand.
- For 43,169 functions with the same mangled name in Mac and IOS_1029, string sets are identical in 88.9% of the 6,717 that have strings, after normalising the build path. The rest are real differences such as `include/Util/Object.h` vs `include/util/object.h` and "OSX" vs "iOS".

**Things IDA got wrong that the script works around**
- **i386 PIC:** IDA loses the PIC base once it is copied out of the popped register. It lacked 12,202 of 136,040 PIC data references and read 59 displacements as absolute addresses (proof: `RBX::StepJointsStage::jointsStepWorld`, 0x005a44e8). The script resolves `[reg + disp]` against the function's PIC base itself. On armv7 the verifier found nothing IDA missed.
- **Mac data pointers:** the Mac binary is not PIE, so IDA does not mark pointers in data as offsets. The one-hop rule therefore tests "word holds the address of a string's first byte" rather than IDA's offset flag.
- **Guessed vtable labels:** IDA's own `_ZTV…` labels (single underscore) are unreliable; one sits on a typeinfo tail at 0x00f16e88. They are ignored, and symbol-less vtables are found by shape instead.
- **Demangling:** IDA's demangler fails on 5,508 Mac names (the `LZN…E` template arguments of `Described<>` / `NonFactoryProduct<>`), so Itanium names go through `llvm-cxxfilt`. No `__Z` function name is left undemangled.
- **armv7 `$shim` branch islands:** 2,276 of these are not flagged as thunks; 5,254 of 155,300 call edges (3.4%) went through one. They are now resolved to the real function.

**Vtable checks**
- Every `_ZTV` symbol `llvm-nm` lists is exported with the same class name, on Mac and IOS_1029 (IOS_1021 was not cross-checked against nm).
- On Mac and IOS_1029, every D1 destructor slot is followed by its D0 (5,659 and 6,750 cases, none broken) and no extents overlap; no class has two non-construction tables on Mac, one does on IOS_1029 (`Ogre::CompositionTechnique`, from an intermediate run).
- All 392 symbol-less Mac vtables are referenced from code, which is how they are told apart from construction vtables.
- Records carry `kind`, `code_refs`, and per group `offset_to_top`, `vbase_vcall_offsets`, `address_point` and slots. The MSVC side plugs into `VTABLE_SCANNERS`.

**Beyond the spec**
- Added fields: `typenames`, `func_refs`, `thumb`, `ranges`, `data_refs`.
- `imms` also includes 4- and 8-byte constants read from read-only data and ARM literal pools, so float constants appear on every target.

**For the next stage**
- `ida/exports/MACSTUDIO-symbols.tsv` is incomplete: IDA has 11,007 `__Z` function names it lacks (e.g. 0x0013e524 `boost::filesystem::detail::symlink_status`) and 462 `__ZTV` names (e.g. `__ZTVN3RBX9DebugableE` at 0x00ed1858). Use `functions.jsonl` names instead. IOS_1029 lacks only 169.
- `imms` agree across compilers in only 36.8% of same-named functions: x86 stores doubles as two 32-bit halves and division magic numbers differ.

I did not append to `ida/LOG.md`, since no database changed.

## Files

- `tools/ida/export_evidence.py`
- `tools/verify_evidence_strings.py`
- `ida/exports/MACSTUDIO-evidence/functions.jsonl`
- `ida/exports/MACSTUDIO-evidence/strings.jsonl`
- `ida/exports/MACSTUDIO-evidence/vtables.jsonl`
- `ida/exports/MACSTUDIO-evidence/summary.json`
- `ida/exports/IOS_1029-evidence/functions.jsonl`
- `ida/exports/IOS_1029-evidence/strings.jsonl`
- `ida/exports/IOS_1029-evidence/vtables.jsonl`
- `ida/exports/IOS_1029-evidence/summary.json`
- `ida/exports/IOS_1021-evidence/functions.jsonl`
- `ida/exports/IOS_1021-evidence/strings.jsonl`
- `ida/exports/IOS_1021-evidence/vtables.jsonl`
- `ida/exports/IOS_1021-evidence/summary.json`

## Unresolved

- MSVC vtables are not implemented: on a PE database vtables.jsonl comes out empty until a scanner is added to VTABLE_SCANNERS. The script has not been run on any PE database, so the PE string path (IDA-defined strings plus a printable-ASCII fallback) is untested.
- Why IDA has 11,007 Mac function names that llvm-nm does not was not determined. The dyld export trie is my guess; I did not check it.
- The 392 symbol-less Mac vtables start at the offset-to-top word; any vbase/vcall offsets before it are not recovered. VTTs (_ZTT) are not exported.
- Strings outside Mach-O C-string sections (__const, __data) are whatever IDA defined as a string, 4+ characters. Some are data misread as text (e.g. 'X94<', 'THEZOCQS'); strings.jsonl carries `seg` so they can be down-weighted.
- The string verifier only covers C-string sections of thin 32-bit Mach-O files. `imms`, `globals`, `imports`, `func_refs` and `blocks` were sanity-checked on samples and by Mac/iOS cross-comparison, not independently verified.
- The i386 PIC rule only adds data targets. A function-pointer reference IDA got wrong because it mistracked the PIC base is not corrected, so `func_refs` on Mac can contain rare false entries; I did not measure how many.
- The 14 to 15 residual string mismatches per database were not all traced individually; none of the ones I traced was an export error.
- The evidence directories total about 200 MB of JSONL under ida/exports/, which CLAUDE.md treats as committed. Whether to commit or gitignore them is the user's call.
- IOS_1014 was not in scope and has no evidence export.

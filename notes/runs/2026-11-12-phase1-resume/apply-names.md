# apply-names (workflow phase1-resume, 2026-11-12)

I fixed `tools/pair_names.py` by changing the rules behind each failure, not by special-casing addresses. I then snapshotted `ida/PLAYER.i64`, applied FLIRT and the names, saved it packed, exported it, and logged the session in `ida/LOG.md`. All of the reviewers' concrete failures are gone or now `guess:`. The ABI checker finds 0 impossible signatures in the final CSV (it found 34 before).

**Rule fixes:**
- **x86 calling-convention gate** for every class except rtti-slot. It checks each body's `ret N` and whether it reads ecx, using new facts for all functions in `ida/exports/PLAYER-evidence/abi.jsonl`. `tools/check_names_abi.py` (its checks now live in a reusable `classifier()`) also learned three things:
  - a two-parameter binary operator is a free function;
  - a method the 2016 headers declare `static` (same arity, same first parameter type) is cdecl;
  - one extra stack word with ecx unused is the LTCG `this`-on-stack convention, so it counts as uncertain, not wrong.
- **Names that cannot exist on Windows are rejected:** names PLAYER imports from a DLL (matched with template arguments and arity), Apple-only names (NSObject, Objective-C methods and blocks, `Mac*::`), and libstdc++ internals (`_M_*`, `std::__*`, `__gnu_cxx`). Names that only carry libstdc++ types in template arguments are downgraded to `guess:` (14 rows).
- **Destructors:** an oracle constructor or destructor that sets a vftable now needs a PLAYER body that sets one or calls a constructor/destructor. This removed lua_settop (0x00873b10) as `~lua_exception`.
- **Inlining level shift:** a pair that is one call level apart is detected, re-checked after propagation, and propagation reruns after drops. All 44 `Name::declare<X>` rows are gone.
- **import-set:** a key of only two generic runtime imports now needs agreeing paired callees (fixes doHttpPost).
- **Folded addresses:** the most-voted name comes first, and each row carries only its own evidence.
- **Control test:** applied to rtti-slot pairs as if they were another class, the new gates reject 5 of 4,707 (0.1%).

**Expected precision per class** (wrong-rate upper bounds use the rule of three):

| Class | Measured | Expected after fixes |
|---|---|---|
| rtti-slot | 0 of 60 sampled; 0 of 7,489 in the population arity test | At most about 5% wrong, unchanged by the fixes |
| string-unique / string-set | 0 of 60 | At most about 5% wrong |
| callgraph (strong) | 3 of 30 wrong before, plus 7 more found by the full-CSV ABI scan, all removed by the fixes | Fresh 30-row sample afterwards: 0 wrong (cards compared on callees, strings, ret N and size), so at most about 10% |
| callgraph-guess | 4 of 20 wrong, 1 uncertain | Still entirely `guess:` (comment only) |
| import-set | 1 of 10 wrong | Rule fixed; the 7 rows left all check out by hand, too few to quote a rate |

**FLIRT:** 97 functions accepted, 3 rejected and 26 globals named, matching the scratch measurement. It also created 10 functions at unaligned CRT starts, so the count went from 48,858 to 48,868.

**Names:** `tools/ida/apply_names.py` renamed 6,712 functions (6,505 `ios:`, 207 `mac:`), only where the name was `sub_`, `nullsub_` or IDA's own `j_<target>`. It wrote 7,035 repeatable comments, 323 of them comment-only, plus `prior:` lines for the 1,321 App-decomp map rows. It overwrote no FLIRT or other existing name. A second run changes nothing.

**Exports:** `tools/ida/export_names.py` wrote `ida/exports/PLAYER-names.csv` (48,314 rows) and regenerated `ida/exports/PLAYER-functions.csv`.

**Coverage, all functions (48,868):** `ios:` 6,505, `mac:` 207, `flirt:` 97, `rtti:` 0 (RTTI names are on data only), other IDA names (thunks and PE exports) 1,106, guess-only 318, unnamed 40,635.

**Coverage, `.text` functions over 32 bytes (29,279):** `ios:` 5,708, `mac:` 200, `flirt:` 53, other IDA names 7, guess-only 291, unnamed 23,020.

The final pairing run names 6,712 functions strongly plus 322 guess-only (it named 6,806 before the fixes). Its output is the same on every run. I also fixed the kill-flag bug in `tools/ida/decomp_dump.py` and `tools/ida/dump_pseudocode.py`.

## Files

- `tools/pair_names.py`
- `tools/check_names_abi.py`
- `tools/ida/apply_names.py`
- `tools/ida/export_names.py`
- `tools/ida/decomp_dump.py`
- `tools/ida/dump_pseudocode.py`
- `ida/exports/PLAYER-evidence/abi.jsonl`
- `ida/exports/PLAYER-names-proposed.csv`
- `ida/exports/PLAYER-names-conflicts.csv`
- `ida/exports/PLAYER-app-decomp-map.csv`
- `ida/exports/PLAYER-rtti-mismatch-ios1029.csv`
- `ida/exports/PLAYER-rtti-mismatch-mac.csv`
- `ida/exports/PLAYER-names.csv`
- `ida/exports/PLAYER-functions.csv`
- `ida/LOG.md`
- `ida/PLAYER.i64 (canonical, saved packed; gitignored)`
- `ida/snapshots/PLAYER-03-pre-names.i64`
- `ida/logs/PLAYER-03-flirt.log`
- `ida/logs/PLAYER-flirt.tsv`
- `ida/logs/PLAYER-03-names.log`
- `ida/logs/PLAYER-names.tsv`
- `ida/logs/PLAYER-03-export-names.log`

## Unresolved

- Precision after the fixes rests on small samples (one fresh 30-row callgraph sample, the earlier 60-row rtti and string samples, 7 import-set rows read by hand). No independent ground truth exists, so the figures are upper bounds on the wrong rate, not measured precision.
- About 45 doDeclare<X> bodies that lost their wrong Name::declare<X> name are now unnamed. The 5-byte callDoDeclare<X> jmp thunks that target them could name them, but no rule does that yet.
- The 2016-static rule depends on 2016 headers, and the inlining-level-shift test is a heuristic. Both can reject correct pairs; the control measured 5 of 4,707 rtti-slot pairs.
- LTCG register-argument bodies (for example 0x0081fb50 RBX::Joint::isHeavierThan, 0x00935a30 PlatformImpl::isChildPlatformMotor6D) are now rejected by the ABI gate even where the name may be right.
- 332 folded addresses carry only the most-voted name; for a trivial ICF body, which source function the linker kept cannot be determined.
- 14 rows whose names carry libstdc++ template types are comment-only `guess:`. They still need their MSVC spelling rewritten from PLAYER's RTTI.
- The 15 rtti-slot addresses with several oracle names that do not look folded are still unnamed, and why was not traced.
- IDA flipped flag 0x40 (FUNC_HIDDEN?) on some 5-byte thunk functions between the old and new functions.csv. The cause was not investigated.
- PLAYER_1015 and STUDIO were not touched. FLIRT for the third-party libraries (boost, G3D, Lua, RakNet, D3DX) is still not built.

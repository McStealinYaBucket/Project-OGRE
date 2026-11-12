# apply-libs (workflow phase2-skeleton, wf_e3afb0d0-6b4)

I applied the library matches to both canonical databases and took the first real score.

**Score.** With `--nolib`, no Roblox function is at 100% in either target yet.

| | PLAYER | STUDIO |
|---|---|---|
| Functions in the denominator (all) | 40,249 | 42,117 |
| Library functions | 1,781 (1,701 third-party + 80 CRT/STL/ATL from FLIRT) | 1,834 (1,621 + 213) |
| Roblox and unidentified (`--nolib` denominator) | 38,468 | 40,283 |
| `--nolib` result | 0 / 38,468, 0.00% | 0 / 40,283, 0.00% |
| Without `--nolib`: compared | 15 | 16 |
| Without `--nolib`: at 100% | 8 | 8 |
| Without `--nolib`: accuracy over compared | 92.41% | 89.76% |
| Implemented | 0.04% | 0.04% |

- **The 8 at 100%** in each target are `luaO_int2fb` and 7 static CRT functions: atexit, `__SEH_epilog4`, `__CxxUnhandledExceptionFilter`, `__CxxSetUnhandledExceptionFilter`, `_ValidateImageBase`, `_FindPESection` and `_setdefaultprecision`.
- **Why `--nolib` is now 0:** `luaO_int2fb` at 0x008a78c0 is typed library now.
- **Library rows are not compared by reccmp:** the 1,701 third-party rows are left out of the denominator, but reccmp compares none of them. Their sources are in `staging/`, not in the CMake build. Their "identical" status comes from the `.obj` match only.

**1. Re-verification and conflicts**
- I reran all four matchers (lua, raknet, g3d, boost) from scratch. Each one reproduced its CSV byte for byte (`cmp`).
- A new tool, `tools/lib/lib_consistency.py`, resolves the library CSVs to one row per address and writes `ida/exports/<T>-lib-resolved.csv`.

| Status | PLAYER (2,001 addresses) | STUDIO (1,894) |
|---|---|---|
| identical | 1,749 | 1,654 |
| located, not identical | 87 | 59 |
| span, no compiled counterpart | 149 | 173 |
| conflict | 13 | 7 |
| inside another function | 3 | 1 |

- **Oracle names that agree with the library name:** PLAYER 294, STUDIO 317.
- **Conflicts, withheld (no name, no reccmp row).** These are identical library bodies that the linker folded (`/OPT:ICF`) with Roblox functions; the oracle names them in RBX::. Examples: 0x00402730 (RBX::Instance::canClientCreate), 0x0040b950, 0x00418f60, 0x004805c0, 0x00528720, 0x005727f0, 0x005b8900, 0x0062aa40 (reloadMT vs ClientAppSettings::Initialize), 0x006be550, 0x0079f710 and 0x0096bde0. Also the span row on RBX::Extents::vv (PLAYER 0x009dab20, STUDIO 0x006aea20). Independent evidence: the App decomp's own STUB markers land on 0x00528720, 0x006be550, 0x0096bde0 and 0x0079f710.
- **Library-against-library disagreements (PLAYER 16, STUDIO 21):** resolved in favour of the byte-identical compiled body. The old name is kept as a `superseded ios: ...` comment line. These are the 10 off-by-one Lua names (plus luaH_next and luaH_getnum in STUDIO), boost clone_impl/error_info_injector destructors the linker folded together, and RakNet `IsPortInUse_Old`.
- **Two libraries at one address:** 0x008dcc30, 0x0096bde0 and 0x009b1a20 (STUDIO 0x00917010, 0x00917030, 0x00a150d0) are trivial boost and RakNet bodies folded together. All are identical, so they count as library either way.
- **Inside another function, dropped:**
  - Two boost program_options `rethrow` rows sit at +6 (0x00418566 and 0x004726d6). They match only after a `/GL` aligned-frame prologue.
  - boost `basic_timed_mutex::unlock` sits at 0x0082a952, 2 bytes into the identical `~lock_guard` at 0x0082a950 (STUDIO 0x00920de2).

**2. IDA databases**
- Snapshots were taken before any change: `ida/snapshots/PLAYER-04-pre-libs.i64` and `STUDIO-04-pre-libs.i64`, checked byte-identical to the canonical files with `cmp`.
- The new script `tools/ida/apply_lib_names.py` renames only `sub_`/`nullsub_`/`unknown_libname_`/IDA `j_` names, or oracle names as resolved above. It adds a comment `match: <lib> <version> identical | ...` or `lib: ... located, not identical`, and span rows get a comment only.
- **PLAYER:** 1,747 + 86 renamed (310 of them from an oracle name) and 149 span comments.
- **STUDIO:** 1,646 + 59 renamed (338 from an oracle name).
- **Kept as they are:**
  - Import-jump thunks: `malloc_0` and `free_0` at 0x009d6dc0 and 0x009d6dd0, and `realloc_0` at 0x005c6cc0 (RakNet's likely mislocation).
  - 5 STUDIO FUNC_LIB names (listed under open issues).
- **Function bounds changed.** IDA had made Lua `correctstack` and `luaK_goiffalse` tail chunks of their only callers. I split them into their own functions: PLAYER 0x008aaea0 and 0x009a7a40, STUDIO 0x009334b0 and 0x00a5ea40. Function counts went from 48,868 to 48,870 (PLAYER) and 44,599 to 44,601 (STUDIO).
- Both databases were saved packed; a second run changes nothing. The exports were regenerated; `export_names.py` now tags `match:` and `lib:`. The session is appended to `ida/LOG.md`.

**3. reccmp CSVs and checks**
- `gen_reccmp_csv.py` used to glob `<T>-lib-*.csv`. That fed STUDIO rows into PLAYER, gave STUDIO nothing, and had no symbol column or de-duplication. It now reads the resolved table and writes one row per address, only on functions that are kept, with IDA's size. Those addresses are removed from the symbols/crt/stl/atl files.
- **Rows written:**
  - PLAYER: lua 514, raknet 463, boost 483, g3d 131, libzip 65, zlib 45.
  - STUDIO: lua 514, raknet 464, boost 402, g3d 131, libzip 65, zlib 45.
  - Skipped as thunks: 48 PLAYER and 33 STUDIO identical rows sit on import or adjustor thunks.
- The 12 new files are listed in `reccmp-project.yml`.
- **Check results:**
  - `reccmp-vtable` (PLAYER and STUDIO): 0 found, exit 0.
  - `reccmp-datacmp` (both): "Variables: 0. Issues: 0", exit 0.
  - `check_folded` (run from `build/` with the venv python): PLAYER 977 annotated addresses, 0 folding problems; STUDIO 964, 0 problems.
  - `reccmp_addr_padding`: 1,395 files, 0 badly padded.
  - **`reccmp-decomplint` fails (exit 1)** on errors already in the imported App tree: PLAYER 336 `function_out_of_order` errors and 42 `duplicate_offset` warnings; STUDIO 341 and 34.

**4. Coverage after this step**

| | All functions | `.text` over 32 bytes |
|---|---|---|
| **PLAYER** total | 48,870 | 29,281 |
| library (match 1,749, lib 87, flirt 97) | 1,933 | 1,511 |
| named (ios/mac) | 6,402 | 5,647 |
| other IDA names | 1,102 | 7 |
| unnamed | 39,433 (was 40,635) | 22,116 (was 23,020) |
| **STUDIO** total | 44,601 | 30,974 |
| library | 1,931 | 1,530 |
| named (ios/mac) | 6,482 | 5,724 |
| other IDA names | 1,872 | 7 |
| unnamed | 34,316 | 23,713 |

## Structured fields

```json
{
 "files": [
  "tools/lib/lib_consistency.py",
  "tools/ida/apply_lib_names.py",
  "tools/ida/export_names.py",
  "tools/ida/gen_reccmp_csv.py",
  "reccmp-project.yml",
  "ida/exports/PLAYER-lib-resolved.csv",
  "ida/exports/STUDIO-lib-resolved.csv",
  "ida/exports/PLAYER-names.csv",
  "ida/exports/PLAYER-functions.csv",
  "ida/exports/STUDIO-names.csv",
  "ida/exports/STUDIO-functions.csv",
  "reccmp/player-lua.csv",
  "reccmp/player-raknet.csv",
  "reccmp/player-g3d.csv",
  "reccmp/player-zlib.csv",
  "reccmp/player-libzip.csv",
  "reccmp/player-boost.csv",
  "reccmp/studio-lua.csv",
  "reccmp/studio-raknet.csv",
  "reccmp/studio-g3d.csv",
  "reccmp/studio-zlib.csv",
  "reccmp/studio-libzip.csv",
  "reccmp/studio-boost.csv",
  "reccmp/player-symbols.csv",
  "reccmp/studio-symbols.csv",
  "ida/PLAYER.i64",
  "ida/STUDIO.i64",
  "ida/snapshots/PLAYER-04-pre-libs.i64",
  "ida/snapshots/STUDIO-04-pre-libs.i64",
  "ida/LOG.md",
  "ida/logs/PLAYER-libnames.tsv",
  "ida/logs/STUDIO-libnames.tsv",
  "ida/logs/04-lib-consistency.log"
 ],
 "unresolved": [
  "reccmp does not compare the 1,701 PLAYER / 1,621 STUDIO third-party library rows. Their sources (staging/lua-5.1.4-2013, staging/raknet, staging/g3d-8.00, staging/boost_1_53_0) are not CMake targets, so reccmp only removes them from the denominator. Their 'identical' status rests on the .obj byte match, not on a linked reccmp comparison. Adding those libraries as OBJECT libraries would let reccmp measure them.",
  "luaO_int2fb at 0x008a78c0 is now typed library, so the Roblox (--nolib) implemented count is 0 in both targets. Phase 3 (flipping the App STUBs) is what moves that number.",
  "STUDIO has 5 FUNC_LIB names from IDA's first-pass vc32rtf FLIRT that contradict identical library bodies: 0x009f0bc0 __mkgmtime_2 is Lua str_match, 0x0061dac0 _NonReentrantLock::_Release is RakNet::BitStream::ResetWritePointer, 0x0068e660 is RakPeer::GetMaximumNumberOfPeers, and 0x0069eea0 / 0x0069efa0 unknown_libname_4/5 are RakNet SimpleMutex / HuffmanEncodingTree constructors. They were not renamed (FUNC_LIB is never overwritten), but in the reccmp CSVs the library row wins.",
  "reccmp-decomplint fails, exit 1, on errors already in the imported App tree: PLAYER 336 function_out_of_order errors and 42 duplicate_offset warnings, STUDIO 341 and 34. reccmp also prints 976 'Failed to find function symbol' errors (STUB markers in files that are not compiled while OGRE_PRIOR_APP is OFF) and cannot decode 5 non-UTF-8 files: base64.hpp, CookiesService.h, and the vendored g3d CollisionDetection.cpp, Matrix3.h and Ray.h.",
  "The CLAUDE.md Checks block says 'python tools/check_folded.py' and 'python tools/reccmp_addr_padding.py'. Both need the venv python (toolchain/venv/bin/python), and check_folded must run from build/ where reccmp-build.yml is. With system python3 they fail on 'No module named reccmp'.",
  "The 12 conflict addresses in PLAYER (6 in STUDIO) are trivial library bodies the linker folded together with Roblox functions. They should get FOLDED handling once the Roblox side is annotated. They are listed in ida/exports/<T>-lib-resolved.csv (status conflict).",
  "The boost program_options rethrow rows at 0x00418566 and 0x004726d6 match only 6 bytes into their PLAYER functions (0x00418560, 0x004726d0, which have a /GL aligned-frame prologue). They are dropped, and the real functions stay unnamed.",
  "lib_consistency.py reads the current names CSV. Run after the apply, it reports no 'oracle agrees' or library-name disagreement counts (those names are now match:). The statuses are unchanged, and the pre-apply figures are recorded only in the summary and ida/LOG.md.",
  "The library agents' own open items are unchanged:\n- 22 G3D located but not identical; 133 unclaimed G3D-block functions.\n- RakNet 0x005c6cc0 NumberOfLeadingZeroes is a probable mislocation.\n- 5 program_options rows are not identical because of /GL.\n- 65 boost header bodies in PLAYER and 38 in STUDIO are linked from Roblox TUs."
 ]
}
```


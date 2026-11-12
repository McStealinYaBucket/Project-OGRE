# vmprotect-scope (workflow phase1-names, 2026-11-11)

VMProtect in PLAYER is confined to 26 SDK-marker regions inside 26 otherwise intact functions, plus one function whose markers were stripped without protection: 11,351 bytes of `.text` (0.1635% of 6,944,188). No function is replaced at its start, and no import goes through a `.vmp` stub. Nothing canonical was touched: IDA ran on scratch copies (since deleted), no database saved, no binary patched.

**1. What is redirected (PLAYER, Oct 23)**
- **Byte-scan hits resolved:** 92 rel32 jmp/call patterns into `.vmp*` = 26 real region entries + 34 inside holes + 32 operand bytes of real instructions (mostly `sub ecx,1Ch` adjustor thunks). All six `.vmp1` hits are false. The 137 `.rdata`/`.data` dwords that look like `.vmp` pointers are unreferenced table data.
- **Mechanism:** `push offset markerName` stays; the 6-byte `call ds:VMProtectBeginMutation` becomes a 5-byte `jmp` into `.vmp0`. The hole runs to the first `VMProtectEnd` call + 6. VMProtect refills holes with its own fragments, which jump between holes and `.vmp0`.
- **IDA misses one entry:** `0x005ae320` in function `0x005ae170` is not defined as code in `PLAYER.i64`.
- **Removed marker calls (18 bytes):**
  - `0x0052086b`: a second `VMProtectEnd` replaced by 6 NOPs.
  - `0x0081d7c8` / `0x0081d809` in function `0x0081d7c0`: Begin became `lea esp,[esp+4]; nop; nop`, the tail `jmp ds:VMProtectEnd` became `ret` + 5 NOPs, body intact. Its marker name "31" duplicates the one used by `0x005265f0`; that is the only difference I measured, not a proven cause.
- **Whole-function cost:** the 27 touched functions span 15,742 bytes.
- **`.rdata` side effect:** 11 marker-name literals are overwritten with random bytes (addresses in the CSV).

**2. Identities**
STUDIO imports exactly `VMProtectEnd`, `VMProtectBeginMutation`, `VMProtectIsDebuggerPresent`, `VMProtectIsValidImageCRC` (no Virtualization import) and has 25 marker functions. All 25 pair with PLAYER; two more regions are Player-only. For the 21 regions (20 in PLAYER, 21 in PLAYER_1015) whose code before the Begin call is identical to the twin's (addresses masked), the hole start and end offsets equal the twin's marker region exactly; `--selftest` asserts this.

| Hole | Size | Function | Identity | Basis |
|---|---|---|---|---|
| `0x0041e0d9` | 1007 | `0x0041dfd0` | `Document::executeScript` | string; Player-only |
| `0x0044ec85` | 282 | `0x0044ec20` | `RBX::RenderJob` slot 6 (`stepDataModelJob`) | RTTI; Player-only; name from 2016 |
| `0x0052082c` | 51 | `0x00520820` | unnamed `GetThreadContext`/Dr7 check | STUDIO `0x00605220` |
| `0x00526617` | 418 | `0x005265f0` | `isDebuggedDirectThreadFunc` | 2016 `Network/API.cpp`; calls `IsDebuggerPresent` |
| `0x00526c6a` | 137 | `0x00526c50` | `RBX::Network::initWithoutSecurity` | 2016 marker placement; iOS name exists |
| `0x0059b145` | 1046 | `0x0059b110` | `Replicator::SendDataJob` slot 6 | RTTI |
| `0x0059ea3e` | 91 | `0x0059ea20` | `Replicator::PingJob` slot 6 | RTTI |
| `0x005ad8b5`, `0x005ade07`, `0x005adf87` | 326, 274, 422 | `0x005ad890`, `0x005adde0`, `0x005adf60` | three unnamed statics in `Network\Client.cpp` | first one calls `IsValidImageCRC` |
| `0x005ae320` | 250 | `0x005ae170` | `RBX::Network::Client` slot 4 (`OnReceive`) | RTTI; `Client.cpp` assert string |
| `0x005beb7a` | 279 | `0x005bea50` | `ClientReplicator` slot 8 (`onServiceProvider`) | RTTI |
| `0x005bf650` | 434 | `0x005bf630` | `ClientReplicator::MemoryCheckerJob` slot 6 | RTTI |
| `0x005bf842` | 888 | `0x005bf820` | `ClientReplicator::MemoryCheckerCheckerJob` slot 6 | RTTI |
| `0x005e9daa` | 140 | `0x005e9da0` | `Replicator::PingBackItem::write` | RTTI slot 1 |
| `0x005eca8a` | 140 | `0x005eca80` | `Replicator::PingItem::write` | RTTI slot 1 |
| `0x0063a83c` | 120 | `0x0063a830` | `RBX::DataModel::allHackFlagsOredTogether` | iOS name; 2016 call pattern |
| `0x0063fbd3` | 1257 | `0x0063fbb0` | unnamed DataModel core-script loader | "Load CoreScripts" string |
| `0x00689025` | 2835 | `0x00688ff0` | `ScriptContext::executeInNewThread` (7 arguments) | `App\script\ScriptContext.cpp` asserts |
| `0x0068a680` | 147 | `0x0068a660` | `ScriptContext::executeInNewThread` (3 arguments) | callers |
| `0x0068a7f7` | 357 | `0x0068a7d0` | unnamed wrapper used by `LuaLibrary.cpp` | callers |
| `0x0073eaa5` | 126 | `0x0073ea50` | `runScript` in `Stats.cpp` | "StatsReporting" strings |
| `0x0081d737`, `0x0081d818` | 97, 87 | `0x0081d730`, `0x0081d810` | unnamed `ProgramMemoryChecker` helpers | STUDIO twins |
| `0x0081d89a` | 94 | `0x0081d890` | `ProgramMemoryCheckerImpl::step` | RTTI slot 1; runs to function end |
| `0x0081d938` | 28 | `0x0081d900` | `ProgramMemoryCheckerImpl` constructor | stores the vftable |

The braces in the CSV identity column tag each name's source; slot-to-method names are hypotheses.

**3. Imports**
- **Directory table:** the import directory (`0x00ed8f60`) and its IAT (`0x00ef1430`, size 0x2044) are in `.vmp1`. It is a normal unbound table on disk, names present, 2031 imports in 34 descriptors: the original 30 DLLs plus KERNEL32 ×3 and USER32 for VMProtect's runtime.
- **`.text` never uses it:** 0 instructions with an operand on it (IDA-decoded) and 0 raw FF15/FF25 patterns.
- **What `.text` uses:** the linker's original IAT at `0x00aa1000`–`0x00aa2eac` in `.rdata`, with 30 descriptors at `0x00ce3eb0` and 1928 named slots, all intact on disk. Raw counts: 21,725 `call [slot]` and 5,441 `jmp [slot]` over 1830 distinct slots.
- **`PLAYER.i64` has the names on the wrong table:** the real slots are `off_AA1000`-style, and IDA reads their hint/name RVAs as offsets into `.text`. `python3 -P tools/vmprotect_scope.py --iat PLAYER` prints the slot,dll,name map.
- **Removed SDK group:** the VMProtectSDK32.dll slots are orphaned at `0x00aa2c04`–`0x00aa2c10` (descriptor gone, names scrubbed, no references left).

**4. The two Players**
PLAYER_1015 protects the same 27 functions: 26 holes totalling 11,263 bytes, plus the same 18 patched bytes (11,281). Hole sizes are identical except `PingBackItem::write` and `PingItem::write` (140 vs 105 bytes), whose code changed between the builds.

The CSVs are regenerated by `python3 -P tools/vmprotect_scope.py`; I also wrote the PLAYER_1015 one, which was not asked for.

## Files

- `notes/vmprotect-player.csv`
- `notes/vmprotect-player_1015.csv`
- `tools/vmprotect_scope.py`
- `tools/ida/vmp_scope.py`
- `tools/ida/vmp_identity.py`

## Unresolved

- Mutation or virtualization: STUDIO's source markers are all VMProtectBeginMutation and PLAYER's orphaned SDK group has the same four slots, but the hole fragments show VM-handler idioms. I did not determine which mode the VMProtect project applied.
- How the original IAT at 0x00aa1000 is filled at run time was not traced. The Windows loader only fills the .vmp1 table, so the VMProtect loader must do it (entry 0x00ee050a, TLS callback 0x00eebaa4, both in .vmp1).
- PLAYER.i64 still needs the 1928 import names applied to 0x00aa1000-0x00aa2eac, the off_ offsets into .text removed, and 0x005ae170 defined as a function. I did not check whether those bogus offsets damaged function discovery.
- Hole ends for the regions with no identical-prologue STUDIO twin rest on the clean-tail rule plus a manual read: Document::executeScript 0x0041e4c8, RenderJob 0x0044ed9f, ClientReplicator slot 8 0x005bec91, PingBackItem::write 0x005e9e36, PingItem::write 0x005ecb16.
- Method names for vftable slots are hypotheses from iOS/Mac symbol names and 2016 marker placement (slot 6 = stepDataModelJob, Client slot 4 = OnReceive, ClientReplicator slot 8 = onServiceProvider). None was checked against an Itanium vtable.
- Ten functions have no symbol name: the Dr7 check, three Client.cpp statics, the core-script loader (loadCoreScripts or startCoreScripts), the LuaLibrary wrapper, three ProgramMemoryChecker helpers, and the 0x0081d7c0 helper.
- Why the markers in 0x0081d7c0 were stripped is not proven; the duplicate marker name "31" is the only difference measured.
- DECOMP_PLAN.md line 32 still says about 80 functions are redirected; the measured figure is 26 regions plus one stripped function. I did not edit the plan or CLAUDE.md.

# ida-prior (workflow phase3-measure, wf_271afd88-e3a)

I finished all five steps on ida/PLAYER.i64 and ida/STUDIO.i64, running headless idat one process per database.

**1. Snapshots.** I copied both databases to ida/snapshots/PLAYER-05-prior-app.i64 and STUDIO-05-prior-app.i64. `cmp` showed both copies identical to the originals, and I checked again just before the first write.

**2. Prior names** (new script tools/ida/apply_prior_names.py; counts are marker rows).
- **PLAYER, 1,034 rows at 992 addresses:**
  - 32 auto names renamed with the `prior:` tag. 31 use the Itanium spelling from the row's Mac name. `RBX::openScriptLibrary` has no mangled spelling and keeps the plain name. `RBX::Lua::LuaArguments::pushArray` also has the plain name, because its Mac spelling contains `__gnu_cxx`.
  - 925 rows agree with the existing ios:/mac: name (comment only).
  - 55 rows agree with another oracle name folded at the same address (comment only).
  - 22 conflicts: 21 disagreeing names were kept, plus one auto address (0x0073b300) where the prior name `RBX::StatsItem::formatMem` competes with the Mac symbol it cites, `RBX::Stats::Item::formatMem`, so it got no name.
- **STUDIO, 1,014 rows at 980 addresses:** 11 renamed, 938 agree, 44 folded, 21 conflicts (20 + 1).
- **Weak spot:** 31 of the 32 PLAYER renames sit on map rows of class callgraph-guess. The App tree supplies the name, but the PLAYER address comes from our own guess pairing.
- No match:/lib: name was touched. Conflict lists are in ida/exports/PLAYER-prior-conflicts.csv and STUDIO-prior-conflicts.csv.

**The 10 most interesting conflicts (PLAYER):**
1. 0x0096e420, 0x0096e5b0, 0x0096e7a0, 0x0096eb20 (ChatOutput.cpp): the prior tree calls these `createChatBubbleFrame`, `createChatBubbleTail`, `createChatBubbleTailFrame` and `createChatBubbleNotifyFrame`. The iOS local symbols name them `createChatBubbleMain`, `createChatBubbleWithTail`, `createScaledChatBubbleWithTail` and `createChatImposter`. The prior names were invented for functions the Mac build leaves unnamed.
2. 0x0096d500: `renderClassicLineList` vs iOS `RBX::renderClassicChatBox`.
3. 0x0073b380–0x0073b510 and 0x0073c670: `RBX::StatsItem::formatValue`, `formatValueId/If/Ii/Iy/Ib` and `formatRate` are `RBX::Stats::Item::formatValue<T>` in iOS. The prior tree misnamed the class and turned mangled template suffixes into function names.
4. 0x0062ee30: `DataModel::setUiMessageBrickCount` vs `DataModel::renderGuiRoot`, which is the Mac symbol at the address the prior comment cites.
5. 0x0071d640: `PartInstance::calcRenderingCoordinateFrame` vs `computeRenderingCoordinateFrame(PartInstance*)`. iOS has both as separate functions.
6. 0x0051e890 and 0x0051ee70: `TypesetterBitmap::layout` and `breakLine` vs `draw` and `getCursorPositionInText` (vftable-slot pairing).
7. 0x0066fa60: `RBX::libraryWriteGuard` vs iOS `illegal(lua_State*)`.
8. 0x0087ab20: `Lua::resolveLegacyMemberName` vs `PropertyNameCorrection`.
9. 0x00665810: `RBX::parseContent` vs `RBX::parseXml`.
10. 0x009b94d0: `RBX::Diagnostics::Countable` (2016 namespace) vs iOS `RBX::Limits::Countable`.

STUDIO additionally has vftable-slot conflicts on tiny bodies, for example 0x007a0640 `getStageType` vs `BuoyancyBoxContact::getType`. These look like linker-folded duplicates the oracle pairing did not list.

**3. Adjustor-thunk targets** (new script tools/ida/split_thunk_targets.py). A target is split only when the instruction before it (past int3/nop padding) is ret, jmp or a noreturn call, its own code does not reach the owner's earlier code, and nothing outside jumps into it except at the target itself.
- **PLAYER:** 161 body splits (the owner was the 8-byte `sub ecx,N; jmp target` thunk itself) and 58 tail-chunk splits, some chunks shared by up to 8 parents. Two more targets became functions as leftover code after a split. All 221 are now function starts and none was refused.
- **STUDIO:** 150 + 59 + 4 leftovers, so all 213 targets are now function starts.
- **Function counts:** PLAYER 48,870 → 49,091; STUDIO 44,601 → 44,814.
- **Names moved:** three deleting-destructor names in each database had been paired onto what is now the thunk. I moved them to the real body: PhysicsService, Selection and Rocket `D0`.
- My first version of the rule refused 3 targets wrongly (two shared-tail shapes and a call to `_CxxThrowException`). Checking the bytes with capstone showed all three are ordinary separate functions, so I fixed the rule.

**4. False FLIRT names in STUDIO** (new script tools/ida/revert_false_flirt.py). I removed the library flag and the name from 33 functions where an oracle or library match contradicts the FLIRT name:
- the 21 destructors (19 RBX classes plus 2 `G3D::Table` instances);
- `_NonReentrantLock::_Release` (really RakNet `BitStream::ResetWritePointer`), `__mkgmtime_2` (really Lua `str_match`) and `_Ctl3dAutoSubclass@4_0` (really `TextXmlWriter::serialize`);
- 9 `unknown_libname` functions.

All 20 RBX constructors and destructors among them reference their own class's vftable. Rerunning the existing apply_names.py then apply_lib_names.py named 28 from iOS/Mac and 5 as library code.

**5. Saves and exports.** Every step saved the databases packed, and no stale .id0/.id1/.nam/.til files are left. I regenerated both names and functions CSVs and appended the session to ida/LOG.md.

Coverage, before → after (`.text` functions over 32 bytes in brackets):

| | Functions | Library | Oracle | Prior | Other IDA | Unnamed |
|---|---|---|---|---|---|---|
| PLAYER before | 48,870 [29,281] | 1,933 [1,511] | 6,402 [5,647] | 0 | 1,102 [7] | 39,433 [22,116] |
| PLAYER after | 49,091 [29,247] | 1,933 [1,511] | 6,402 [5,644] | 32 [28] | 1,101 [7] | 39,623 [22,057] |
| STUDIO before | 44,601 [30,974] | 1,931 [1,530] | 6,482 [5,724] | 0 | 1,872 [7] | 34,316 [23,713] |
| STUDIO after | 44,814 [30,944] | 1,912 [1,509] | 6,510 [5,743] | 11 [10] | 1,872 [7] | 34,509 [23,675] |

Unnamed grows because the split-off bodies have no names yet. STUDIO's library count drops by 19 because 24 named FLIRT functions were removed and 5 real library matches came back.

**Edits to existing tools.** apply_names.py now strips only its own `prior: in 2013 App decomp:` comment lines, so a rerun no longer deletes the new prior lines. export_names.py now tags these names `prior:`.

**Rerun check.** On scratch copies, rerunning apply_prior_names, apply_lib_names, split_thunk_targets and revert_false_flirt changes nothing. apply_names rewrites the comments at the 3 moved-name thunk addresses, because ida/exports/<T>-names-proposed.csv still lists the old addresses.

## Structured fields

```json
{
 "files": [
  "tools/ida/apply_prior_names.py",
  "tools/ida/split_thunk_targets.py",
  "tools/ida/revert_false_flirt.py",
  "tools/ida/apply_names.py",
  "tools/ida/export_names.py",
  "ida/exports/PLAYER-prior-conflicts.csv",
  "ida/exports/STUDIO-prior-conflicts.csv",
  "ida/exports/PLAYER-names.csv",
  "ida/exports/PLAYER-functions.csv",
  "ida/exports/STUDIO-names.csv",
  "ida/exports/STUDIO-functions.csv",
  "ida/LOG.md",
  "ida/logs/STUDIO-flirt-revert.tsv",
  "ida/logs/PLAYER-thunk-split.tsv",
  "ida/logs/STUDIO-thunk-split.tsv",
  "ida/logs/PLAYER-thunk-split-pass2.tsv",
  "ida/logs/STUDIO-thunk-split-pass2.tsv",
  "ida/logs/PLAYER-prior.tsv",
  "ida/logs/STUDIO-prior.tsv",
  "ida/PLAYER.i64",
  "ida/STUDIO.i64",
  "ida/snapshots/PLAYER-05-prior-app.i64",
  "ida/snapshots/STUDIO-05-prior-app.i64"
 ],
 "unresolved": [
  "tools/ida/gen_reccmp_csv.py turns only ios:/mac: sources into symbol rows, so the 32 PLAYER and 11 STUDIO prior: names will not reach reccmp/*-symbols.csv unless the agent that owns the generator adds a rule for them.",
  "31 of the 32 PLAYER prior: renames rest on map rows of class callgraph-guess (about 4 in 20 wrong in the earlier sample). The App tree supplies the name; the PLAYER address is our own guess pairing, so these names are only as good as that pairing until reccmp scores them.",
  "ida/exports/<T>-names-proposed.csv predates the new function bounds. tools/pair_names.py should be rerun: 3 thunk addresses per database still carry stale proposed rows (apply_names.py rewrites their comments on every rerun), and the 221/213 newly split bodies may now be pairable.",
  "About 80 more STUDIO functions carry the same vc32rtf names (??1strstreambuf@std@@UAE@XZ_0.._71, ??1ios_base..., ??1_Locimp..., __mkgmtime_N) with no oracle or library row to contradict them. They are very likely RBX destructors as well (most are 84-byte bodies) but were not touched; checking which class's vftable each one stores would settle them.",
  "Several STUDIO conflicts on tiny bodies that pair by vftable slot (getStageType vs getType/getFontSize, SpanningTree::onSpanningEdgeRemoved vs IAdornable::render3dSelect, Kernel::getKernel vs TaskScheduler::Arbiter::getSyncronizationArbiter) look like linker-folded duplicates the proposed CSV does not list as folded. I did not establish whether they are folds or wrong pairings."
 ]
}
```


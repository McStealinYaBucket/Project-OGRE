# ida-sync-6 (workflow phase5-wave3)

The wave 3 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G. None hit the cap.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-12-wave3-sync.i64 and STUDIO-12-wave3-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep9/P-final.json and build-studio/sweep9/S-final.json, joined only with build/sweep9/scored/ and build-studio/sweep9/scored/ (the exe, pdb, map and pdbview they scored).
- Functions at effective 100%: PLAYER 6,704, STUDIO 6,500. Library-owned rows (1,733 / 1,641) are left to apply_lib_names.py.
- PLAYER: 675 renamed `match:` (658 unnamed, 10 IDA names, 5 stale match: names, 2 fact rows) and 12 vftable slots `rtti:`.
- STUDIO: 614 renamed `match:` (589 unnamed, 4 IDA names, 5 stale match: names, 16 carried over from PLAYER through the body match) and 12 `rtti:`. One FLIRT `FUNC_LIB` flag was cleared.
- Conflicts: PLAYER 160 (100 folded-original, 55 same-function, 3 oracle names kept by hand facts, 2 body-wins). STUDIO 154 (103 / 45 / 3 / 2, plus 1 entry point).
  - Body-wins, both targets. Our 100% `RunningAverageTimeInterval<1>::value` replaces the callgraph-only ios: `rate()` (PLAYER 0x005b9c70 / STUDIO 0x0064b8c0; rate() is inlined on Windows). Our static `biggerJointSize2` replaces the callgraph-only ios: `getJointSize2` (0x00820e70 / 0x00914520).
- Skipped, as in every earlier sync: STUDIO 0x0054b3b0, because its name already sits on the ICF twin 0x004f3770.
- Facts from the reports, each checked against both exes with capstone (new FACTS rows in tools/ida/match_names.py):

| Name | PLAYER | STUDIO | Tag | Evidence |
|---|---|---|---|---|
| `??0CRenderSettings@RBX@@QAE@XZ` | 0x009c5ce0 | 0x006c20b0 | rtti: | Stores vftable 0x00bbb474 / 0x00be712c, whose COL names `.?AVCRenderSettings@RBX@@`. CRenderSettingsItem() calls it on this+0x64. iOS 1021/1029 have the C2 symbol |
| `?getVideoMemory@SystemUtil@RBX@@YAIXZ` | 0x009b59b0 | 0x00a68be0 | src2016: | A tail `jmp` into the DXVRAM probe 0x009c0c30 / 0x00a77ac0 (FastLog "DXVRAM: Already initialized, returning %d"). CRenderSettingsItem() compares only eax with 16000000 |
| `boost::posix_time::to_simple_string(time_duration)` | 0x00706150 | 0x007d1cf0 | lib: | A free function (no `this`) that calls to_simple_string_type<char>. Its old ios: name `Lighting::getTimeStr` was callgraph-only |
| `RBX::Lighting::getTimeStr() const` (ios:) | 0x00706190 | 0x007d1d30 | ios: | Reads this+0x1a0/+0x1a4 and calls to_simple_string_type<char> (datamodel-5b-3.md) |

  The two link-stub names use exactly the decorated spelling of our tree's unresolved externals (Client/_generated/player/unresolved.txt). gen_link_stubs.py's exact lookup in ida/exports/<T>-names.csv will now find both addresses, which is the names.csv row integrate.md asked for.

**3. Types.** New classes, each with a constructor or destructor at effective 100% in sweep9 and a PDB size equal to the tree's DECOMP_SIZE_ASSERT or `// SIZE` (one exception, Client, below):

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| CRenderSettingsItem | 0xe4 | ~ 0x00456940 / 0x00437850 |
| BasePlayerGui | 0x74 | ~ 0x00708720 / 0x00795500 |
| PlayerGui | 0x74 | ctor 0x0070c050 / 0x00798750 |
| CoreGuiService | 0x90 | ctor 0x0070c350 / 0x00798a60 |
| GuiButton | 0x33c | ctor 0x007cb2e0 / 0x008e7d90 |
| GuiDrawImage | 0x84 | ~ 0x006bc620 / 0x007ab4c0 |
| GuiObject | 0x21c | ctor 0x007c77c0 / 0x008e4260 (it was imported only as a base before) |
| GuiService | 0xc0 | ctor 0x0069ed90 / ~ 0x00860c80 |
| KernelData | 0xb0 | ~ 0x0082f240 / 0x009a1780 |
| Limits::Counter::Activator | 0x8 | ~ 0x009b97f0 / 0x00a6b740 |
| Network::Client | 0xb10 | ~ 0x005ad760 / 0x0060c1a0 |
| Network::Peer | 0xac0 | ctor 0x005b9ac0 / 0x0064b710 |
| SendPhysics | 0x3c | ~ 0x008e60f0 / 0x00a043f0 |
| SimJob | 0x24 | ctor 0x00822390 / 0x00916360 |

- **Network::Client is 0xb10, not the 0xb0c that Client.h's `// SIZE` says.** The original allocates `push 0xb10; call operator new` right before Client::Client (PLAYER 0x0052443f). The members end at 0xb0c and Peer's 8-byte alignment pads the class. So the PDB layout is the 2013 one and the header comment is wrong.
- Local types: PLAYER 188 (69 listed), STUDIO 179 (67); they were 146 and 137. `this` is typed on 226 PLAYER / 212 STUDIO functions (143 / 129 of them ctor or dtor); it was 127 / 123.
- Still not imported:
  - ScreenGui and RunService in STUDIO: ctor/dtor at 0.982 and 0.937 / 0.899. They stay PLAYER-only.
  - Lighting, Workspace (0.19 / 0.45) and TaskScheduler (0.978 / 0.995) in both targets.

**4. Unnamed functions.** I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.
- PLAYER App block (0x00606a00-0x009ad3a0, 21,631 functions): those with an ios:/mac:/match: name went from 7,295 to 7,705, and 13,926 (64.4%) still have none.
- Network block (0x0051f6b0-0x00606a00: from the first function after RbxOgre's last Ogre name up to the App block, 5,336 functions): 1,766 to 1,812, and 3,524 (66.0%) still have none.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,194 (608 KB) |
| Ogre::RbxSceneManager | 306 |
| ScriptContext | 215 |
| FactoryProduct<>::Creator | 213 |
| DataModel | 192 |
| EnumDesc<> | 181 |
| RBX free functions | 155 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| InsertService | 94 |

Largest Network-block groups still unnamed: Replicator 385, no known class 949, Players 161, Player 120, ServerReplicator 119, FactoryProduct<>::Creator 63, DataModel 57, Server 56, Replicator::StreamJob 51.

**5. Name coverage, before → after.** I regenerated the names, functions and types exports (local types: PLAYER 575 → 617, STUDIO 474 → 516), and ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 12,470 → 13,151 | 13,279 → 13,906 |
| match: | 4,602 → 5,277 | 4,508 → 5,121 |
| rtti: | 167 → 175 | 163 → 172 |
| ios: | 6,210 → 6,208 | 6,270 → 6,268 |
| lib: | 73 → 74 | 46 → 47 |
| src2016: | 2 → 3 | 2 → 3 |
| ida: | 1,103 → 1,102 | 1,849 → 1,854 |
| prior: | 17 → 16 | 6 → 6 |

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-12-wave3-sync.i64, ida/snapshots/STUDIO-12-wave3-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (4 wave3 FACTS rows), tools/ida/import_types.py (14 wave3 CLASSES)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-12-db-updates.log, -12-db-updates.tsv, -12-types.log, -12-export-names.log

## Unresolved

- Network owner: Client/Network/include/Network/Client.h says `// SIZE 0xb0c`, but 2013's sizeof(Client) is 0xb10 (`push 0xb10` before Client::Client at PLAYER 0x0052443f, and our PDB agrees). Correct the comment, or add DECOMP_SIZE_ASSERT(Client, 0xb10).
- Integrate: rerun tools/gen_link_stubs.py (with each exe deleted first). ida/exports/<T>-names.csv now carries `??0CRenderSettings@RBX@@QAE@XZ` (PLAYER 0x009c5ce0 / STUDIO 0x006c20b0) and `?getVideoMemory@SystemUtil@RBX@@YAIXZ` (0x009b59b0 / 0x00a68be0). Two stubs then replace the shared /FORCE:UNRESOLVED address, and CRenderSettingsItem() 0x004567d0 (0.925) should improve.
- Integrate (CSV): the oracle row for Lighting::getTimeStr must move from 0x00706150 / 0x007d1cf0 to 0x00706190 / 0x007d1d30. The database and names.csv now have it there; to_simple_string takes the old address.
- The DXVRAM probe 0x009c0c30 / 0x00a77ac0 has no name. 2016's static GetDirecXVideoMemorySize has the same log strings, but no oracle symbol exists for it.
- pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170 (no function start changed this session either).
- The CRenderSettings layout inside the imported CRenderSettingsItem is the shell header's stand-in. Only what ~CRenderSettingsItem touches is proven. The real class belongs to the Rendering contributor (GfxBase/RenderSettings.h).
- The STUDIO skip at 0x0054b3b0 (name on the ICF twin 0x004f3770) remains.

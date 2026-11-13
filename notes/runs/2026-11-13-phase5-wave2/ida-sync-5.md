# ida-sync-5 (workflow phase5-wave2, wf_cba1073c-3f1)

WAVE2 IDA SYNC is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G. None hit the cap.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-11-wave2-sync.i64 and STUDIO-11-wave2-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** The sources were build/sweep8/P-final.json and S-final.json, joined only with the scored-i8/{P,S} copies of the exe, pdb and map.
- Functions at effective 100%: PLAYER 5,733, STUDIO 5,593.
- PLAYER renamed 704 functions `match:` (672 unnamed, 31 IDA j_ thunk names, 1 stale match:) and 79 vftable slots `rtti:`.
- STUDIO renamed 763 `match:` (32 of them carried over from PLAYER through the body match) and 72 `rtti:`.
  - In STUDIO, two false FLIRT Concurrency::VirtualProcessor names gave way to our 100% bodies: GuiBase2d::getGuiQueue 0x007b6090 and getZIndex 0x00800410.
- Conflicts: PLAYER 115 (76 folded-original, 36 same-function, 3 oracle names kept by hand facts). STUDIO 112 (77 folded-original, 29 same-function, 5 where our body replaced the old name, 1 entry point).
- Facts from the Integrate report, each checked against both exes with capstone:
  - **DataModel::getNumPlayers** is 0x0063e220 / STUDIO 0x0075c0d0. The ??_EDataModel adjustor{168} thunk (`sub ecx,0xa8; jmp` to 0x0064f1f0) is 0x0064de30 / 0x0076a900. They are slots 7 and 6 of DataModel's DataModelArbiter vftable 0x00b2ff80. The databases had the two names swapped.
  - **GuiMain's adjustor{28} thunk** 0x008b2ea0 / 0x00961180 is now ??_EGuiMain@RBX@@WBM@AEPAXI@Z. The stale ScreenGui name is gone. Without a hand fact the bulk rule would have picked the DescribedCreatable<GuiMain> name, because our linker folds three names at our 0x00596af0.
  - **FLog::GuiTargetLifetime** is the byte at 0x00de3f47 / 0x00f84e1f (iOS 1029 symbol; ~GuiLayerCollector loads it at 0x008b439b / 0x00988b9b).
- The rename script now handles a swap: when another address holds the wanted name and that address is itself due for a new name, the script frees the name first. Without this, both DataModel renames were skipped.

**3. Types.**
- New classes, each with a constructor or destructor at effective 100% in sweep8 and a size that matches the tree's DECOMP_SIZE_ASSERT or `// SIZE` comment:
  - DebrisService 0x94
  - Frame 0x2a4
  - GuiBase 0x7c, GuiBase2d 0x94, GuiBase3d 0x88
  - GuiImageMixin 0xb8, GuiTextMixin 0x54
  - GuiLabel 0x21c, GuiLayerCollector 0xb4
  - ImageLabel 0x2d4, TextLabel 0x270
  - PhysicsInstructions 0x60 (destructor 0x00635600)
  - PLAYER only: ScreenGui 0xec and RunService 0xb0. Their STUDIO constructors are at 0.982 and 0.937.
- **Bug found and fixed in the type dump.** For PLAYER's ImageLabel the newest PDB record was a stale 0x23c layout from an old header (a ContentId member, no GuiImageMixin base). Where the tree states a size, the dump now picks the newest record of that size.
- Local types: PLAYER 146, STUDIO 137 (both were 92). `this` is typed on 127 PLAYER / 123 STUDIO functions (was 81 / 82). Every imported size equals the tree's, except STUDIO GuiBuilder at 0x1c, which is the known deliberate STUDIO correction.

**4. Unnamed App functions.** I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv. The PLAYER App block (21,631 functions) with an ios:/mac:/match: name went from 6,869 to 7,295; 14,336 (66.3%) still have none. Largest groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,325 (617 KB) |
| Ogre::RbxSceneManager | 306 |
| ScriptContext | 215 |
| FactoryProduct<>::Creator | 213 |
| DataModel | 205 |
| EnumDesc<> | 186 |
| RBX free functions | 163 |
| GuiService | 109 |
| Ogre::RbxSceneNode | 104 |
| Creatable<> | 101 |

**5. Name coverage, before → after.** The names, functions and types exports are regenerated (PLAYER 575 local types, STUDIO 474), and ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 11,715 → 12,470 | 12,493 → 13,279 |
| match: | 3,897 → 4,602 | 3,746 → 4,508 |
| rtti: | 89 → 167 | 93 → 163 |
| ida: | 1,130 → 1,103 | 1,893 → 1,849 |
| ios: globals | 5 → 6 | 5 → 6 |

## Files changed

- ida/PLAYER.i64
- ida/STUDIO.i64
- ida/snapshots/PLAYER-11-wave2-sync.i64
- ida/snapshots/STUDIO-11-wave2-sync.i64
- ida/LOG.md
- tools/ida/match_names.py
- tools/ida/apply_db_updates.py
- tools/ida/import_types.py
- ida/exports/PLAYER-names.csv
- ida/exports/STUDIO-names.csv
- ida/exports/PLAYER-functions.csv
- ida/exports/STUDIO-functions.csv
- ida/exports/PLAYER-types.h
- ida/exports/STUDIO-types.h
- ida/exports/PLAYER-proven-types.json
- ida/exports/STUDIO-proven-types.json
- ida/exports/PLAYER-db-updates.csv
- ida/exports/STUDIO-db-updates.csv
- ida/exports/PLAYER-match-conflicts.csv
- ida/exports/STUDIO-match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/PLAYER-11-db-updates.log
- ida/logs/PLAYER-11-db-updates-pass2.log
- ida/logs/PLAYER-11-db-updates-pass2.tsv
- ida/logs/STUDIO-11-db-updates.log
- ida/logs/STUDIO-11-db-updates.tsv
- ida/logs/PLAYER-11-types.log
- ida/logs/STUDIO-11-types.log
- ida/logs/PLAYER-11-export-names.log
- ida/logs/STUDIO-11-export-names.log

## Unresolved

- The PLAYER PDB scored in sweep8 carries a stale RBX::ImageLabel type record (0x23c, an old ImageLabel.h layout with a ContentId member and no GuiImageMixin base) beside the correct 0x2d4 one. import_types now picks the record whose size matches the tree, but the stale record could still affect other tools that read PDB types (datacmp, class_layout.py).
- The match: rename table from the first PLAYER apply pass is not saved as its own tsv: the second pass overwrote ida/logs/PLAYER-db-updates.tsv. The planned rows are in ida/exports/PLAYER-db-updates.csv, and the counts are in ida/logs/PLAYER-11-db-updates.log.
- Three STUDIO renames were skipped because the recompiled name already sits on an identical twin: 0x0054b3b0 (twin 0x004f3770), 0x0091c920 (twin 0x006880a0) and 0x0091c990 (twin 0x00688a50).
- pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv were not regenerated. They still predate STUDIO isUpperRoot 0x00a0e170.
- rbx::placement_any<Region3>::operator=<Enum> at 0x00633680 (which Integrate needs as a symbol row) is left unnamed. ICF folds every Enum instantiation, so which iOS instance it is cannot be decided.
- Class layouts not imported yet: Lighting (size assert 0x1f0, but no ctor or dtor at 100%), and the STUDIO layouts of ScreenGui (ctors at 0.982) and RunService (ctor 0.937, dtor 0.899).
- Integrate should know the database names for 0x0063e220/0x0064de30 (PLAYER) and 0x0075c0d0/0x0076a900 (STUDIO) are now the corrected getNumPlayers/??_E pair (rtti:). GuiMain's {28} thunk now carries ??_EGuiMain as a match: name, so gen_reccmp_csv's MATCH_FIX no longer has a stale name to correct.

# studio-names (workflow phase1-resume, 2026-11-12)

Yes, still going. The STUDIO name transfer is done. I snapshotted `ida/STUDIO.i64` to `ida/snapshots/STUDIO-03-pre-names.i64`, applied FLIRT, applied names in two passes, saved it packed, exported it and logged the session in `ida/LOG.md`. PLAYER's outputs are byte-identical to before.

**Body match against PLAYER** (new `tools/body_match.py`, no IDA): it compares function bodies with addresses masked, using capstone on both PE files.
- 40,902 of 44,599 STUDIO bodies occur in PLAYER at all. Most of these sit in large groups of tiny thunks and template copies.
- 20,115 are accepted one-to-one pairs (4,011,975 bytes; 18,476 are `.text` functions over 32 bytes): 14,718 unique on both sides, plus 5,397 resolved because an accepted caller calls them at the same position.
- Rejected: 110 where a call target disagrees, 111 where a referenced string differs (build path and assert line numbers masked first), and 463 bodies under 16 bytes with no support from a neighbouring pair.
- 20,103 bodies have several identical twins and stay unpaired.
- 4,413 of the pairs land on a PLAYER function that `PLAYER-names-proposed.csv` names.

**Pairing** (`tools/pair_names.py --target STUDIO --oracle ios1021=IOS_1021 --oracle mac=MACSTUDIO --body-match PLAYER`):
- New class `body-match` does two things:
  - It seeds each oracle with the function carrying the PLAYER name, through the usual tests (1,179 iOS, 1,172 Mac).
  - It adds PLAYER as an extra source. Its rows carry the PLAYER row's own tag and the evidence `player: body-identical to PLAYER 0x...`. A disagreeing oracle makes a conflict, and nothing is named.
- Result: 6,849 functions with a strong row (+195 guess-only) and 76 conflicts.
- Strong rows by source: 6,157 oracle only, 5,690 oracle and PLAYER agreeing, 40 PLAYER only.
- Net gain over a run without body-match: +116 functions (184 gained, 68 lost to different callgraph propagation).
- `tools/check_names_abi.py` now reads per-target thunk sizes and skips the PLAYER VMProtect list for other targets. All 44,599 STUDIO functions got ABI facts in `ida/exports/STUDIO-evidence/abi.jsonl`.

**Adversarial check:** I decompiled each STUDIO row next to its iOS 10-21 or Mac function on scratch copies, and read string pointers from the PE.

| Sample | Rows | Wrong |
|---|---|---|
| Main, seed 20261112: body-match | 15 | 0 |
| Main: callgraph | 12 | 0 |
| Main: string-unique | 10 | 0 |
| Main: rtti-slot | 10 | 0 |
| Main: string-set | 8 | 0 |
| Main: body-match-guess | 5 | 0 |
| Targeted, seed 20261113: PLAYER-only body-match | 10 | 0 |
| Targeted: bodies under 40 bytes | 10 | 0 |
| Targeted: call-position pairs | 6 | 0 |

So the measured wrong rate is 0% in every class, and no class crossed the 3% threshold. The body-match class was 31 of 31 correct across both samples, which bounds its wrong rate at about 10% by the rule of three. Every other class is bounded at 20–30% by sample size.

- `body-match-guess` stays `guess:` because it inherits PLAYER's own `callgraph-guess` rows, which were 4 of 20 wrong in the PLAYER review.
- Bug the review found and fixed by changing the rule: with body-match counted as a strong witness, the validation step dropped 25 correct string-unique and string-set pairs (`GuiBuilder::build*`, `ContentProvider`). Body-match now ranks last as a witness.
- Whole-population cross-check: about 3,100 functions carry both a body-match row and an independent oracle row, and 2 disagree. Neither got a name:
  - `0x007b2520`: PLAYER says `BackpackItem::isEnabled`, the oracles say `inBackpack`.
  - `0x005b8c50`: iOS says `callDoDeclare<sTeams>`, the others say `doDeclare<sTeams>`.

**Database changes:**
- FLIRT accepted 26 functions (atls 23, msvcrt 2, msvcprt 1); `0x00a7cf4b` was rejected as before. It named 5 globals and created no functions.
- `tools/ida/apply_names.py` renamed 6,820 functions and wrote 7,027 comments, plus 45 more in pass 2. A second run on a copy changes nothing.
- 29 functions kept an existing name. 21 of them are IDA's own first-pass `vc32rtf` library names on RBX destructors (for example `??1strstreambuf@std@@UAE@XZ_32` on `RBX::Soundscape::SoundChannel::~SoundChannel` at `0x008036e0`).
- Exports: `STUDIO-names.csv` (50,684 rows), `STUDIO-functions.csv` (44,599), and the app-decomp map now places 1,318 of 2,733 Mac functions.

**Coverage, all 44,599 functions:** `ios:` 6,557, `mac:` 263, `flirt:` 223 (197 from IDA's first pass plus 26 today), `rtti:` 0, other IDA names 1,876, guess-only 188, unnamed 35,492.

**Coverage, 30,972 `.text` functions over 32 bytes:** `ios:` 5,768, `mac:` 246, `flirt:` 139, other IDA names 7, guess-only 170, unnamed 24,642.

Files are in `~/Documents/Project OGRE`; `ida/LOG.md` has the full session note.

## Files

- `tools/body_match.py`
- `tools/pair_names.py`
- `tools/check_names_abi.py`
- `tools/sample_proposed_names.py`
- `ida/exports/STUDIO-evidence/abi.jsonl`
- `ida/exports/STUDIO-PLAYER-bodymatch.csv`
- `ida/exports/STUDIO-names-proposed.csv`
- `ida/exports/STUDIO-names-conflicts.csv`
- `ida/exports/STUDIO-app-decomp-map.csv`
- `ida/exports/STUDIO-rtti-mismatch-ios1021.csv`
- `ida/exports/STUDIO-rtti-mismatch-mac.csv`
- `ida/exports/STUDIO-names.csv`
- `ida/exports/STUDIO-functions.csv`
- `ida/LOG.md`
- `ida/STUDIO.i64 (canonical, saved packed; gitignored)`
- `ida/snapshots/STUDIO-03-pre-names.i64`
- `ida/logs/STUDIO-03-flirt.log`
- `ida/logs/STUDIO-flirt.tsv`
- `ida/logs/STUDIO-03-pair-names.log`
- `ida/logs/STUDIO-03-names.log`
- `ida/logs/STUDIO-names-pass1.tsv`
- `ida/logs/STUDIO-03-names-pass2.log`
- `ida/logs/STUDIO-names.tsv`
- `ida/logs/STUDIO-03-export-names.log`

## Unresolved

- The per-class samples are small (5 to 31 rows), so every '0% wrong' is a bound, not a precise rate: about 10% for body-match (31 rows) and 20-30% for the other classes. No independent ground truth exists.
- 21 RBX destructors and 5 unknown_libname_N functions carry IDA first-pass vc32rtf library names that the oracles contradict, for example ??1strstreambuf@std@@UAE@XZ_32 on Soundscape::SoundChannel::~SoundChannel at 0x008036e0. apply_names never overwrites a library name, so they stay wrong until someone decides to strip vc32rtf hits that have Roblox-class evidence.
- 20,103 STUDIO bodies have several identical PLAYER twins and stay unpaired unless a caller's call position resolves them.
- Body-match costs 68 names the plain oracle run gave, because callgraph propagation takes different routes; the net gain is +116 functions. The lost names were not reviewed one by one.
- One row (RBX::FrameRateManagerStatsItem::update, STUDIO 0x0043e8a0) had no iOS 10-21 symbol to compare against; it was judged only on the body's consistency with the name.
- PLAYER_1015 was not touched. Third-party FLIRT (boost, G3D, Lua, RakNet, D3DX) is still not built.

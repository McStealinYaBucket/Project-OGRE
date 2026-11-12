# pairing (workflow phase4-pairing, wf_4baec7d9-68d)

## Name and data pairing, FOLDED markers and re-sweep: final figures

All five steps are done. No function that was at 100% at the start lost it, in either target, and every `// FUNCTION:` marker measures effective 100%. App markers at 100% went from 92 to 154 FUNCTION plus 23 FOLDED in PLAYER (159 addresses), and from 87 to 150 plus 23 FOLDED in STUDIO (155 addresses).

**1. Starting figures.** I rebuilt `build/` (PLAYER) and `build-studio/` (STUDIO, as the buildpath report asked), flipped every marker to FUNCTION and generated the CSVs the old way (`OGRE_NO_SIGPAIR=1`).

| Starting point | PLAYER | STUDIO |
|---|---|---|
| Functions at 100% | 1,704 | 1,645 |
| of which library | 1,602 | 1,547 |
| of which non-library | 102 | 98 |
| App markers that would be FUNCTION | 92 / 1,036 | 87 / 1,016 |
| reccmp summary (all markers FUNCTION) | 2,819 / 40,468 | |

No CMakeLists.txt change was needed: neither report asked for one.

**2. Name pairing** (`tools/ida/gen_reccmp_csv.py`, `norm_sig` and `oracle_pairs`)
- Our PUBLICS names are read with `llvm-undname` and the ios:/mac:/prior: names with their demangled form. Both are reduced to one form: qualified name, parameter types and const, with no return type.
- The reduction folds std typedefs, treats long as int, removes top-level const, maps the Itanium D0 destructor to MSVC's scalar/vector deleting destructor, and handles C names and the anonymous namespace.
- A pair is made only when the signature is unique on both sides. The symbols CSV row then carries our exact PUBLICS name in a new `symbol` column. A static function has no PUBLICS name, so its row carries our proc name instead.

| Pairing result | PLAYER | STUDIO |
|---|---|---|
| Paired by signature | 673 | 696 |
| Paired by static name | 7 | 8 |
| Ambiguous (e.g. `Type<int>` and `Type<long>` collapse) | 4 | 3 |
| Name present, signature differs | 16 | 23 |
| Dropped because a position pair contradicts it | 7 | 7 |
| Not defined in our build | 4,720 | 4,796 |

- The dropped pairs are where the original linker folded identical bodies (`unique_lock<mutex>::lock` and `<recursive_mutex>::lock`, `??_E` destructors of Clump, EdgeBuffer and RotateJoint, `assertNotInPipeline`). In those cases the position pair is kept.
- Of the rows written, 662 of 666 (PLAYER) and 685 of 689 (STUDIO) pair in reccmp as written; 252 and 259 of them are at 100%.
- **Verification.** All 20 sampled pairs were run with `--verbose` (`build/sweep2/verbose/`). Each paired with the intended function of ours. 7 are at 100%; the rest are real code differences (e.g. `setIsArchivable` uses `FWValue` in 2013).
- **Named examples:** `Face::minMax` 0x008fc9f0, `VerbContainer::getVerb` 0x006f90d0, `Debugable::doCrash` 0x009b1a00 / STUDIO 0x00a68af0 and `RBXCRASH` 0x009b9090 / 0x00a68e10 are now all at 100%. `setRobloxLocked` 0x0061d780 pairs but scores 10% because the code differs.

**3. Data rows**
- The position method now also reads the functions a reccmp `--json` run pairs, but only where the whole instruction stream lines up with ours and our body ends where the original's does. The end check is skipped for functions already at 100%.
- It also reads changed reference lines from the diff, but only where both instructions sit at the same offset with the same mnemonic and length.
- The library pass decides first. When the original's linker folded several of our bodies into one, the body with the most references wins.
- The CSVs pool three reccmp runs (`--json build/sweep2/P3.json,P4.json,P6.json`; S3/S4/S6 for STUDIO). Re-generating from a single run is not monotone: one run lost 83 entries. Results:
  - `__TI2?AVruntime_error@std@@` is now paired (0x00cd66cc / 0x00e292d4).
  - The `logic_error(string)` constructor is now paired (0x0041bbe0 / 0x00403b60).
  - libdata grew from 931 to 3,380 rows (PLAYER) and from 834 to 3,311 (STUDIO).
- **vftable sizes.** Vtable rows now carry the original's slot count, capped at our gap to the next symbol. This restores G3D::Plane 0x00b032e0, RakPeerInterface 0x00b27d54 / 0x00bdf23c and bad_lexical_cast 0x00b2cfe0 / 0x00bf3408 to 100%.

**4. FOLDED.** `app_sweep.py apply` now writes `FUNCTION: … FOLDED` on every marker at an address when all of them are among the symbols our link map folded onto that body.
- This covers 5 addresses per target, 23 markers each: PLAYER 0x00575d00, 0x006be550, 0x006c8740, 0x00962a50, 0x00992a80 and STUDIO 0x0078ad10, 0x007a06a0, 0x007a6470, 0x007aa330, 0x00892cf0.
- The 6th (PLAYER 0x006ceaa0 / STUDIO 0x00812790): only Sound.cpp's `askSetParent` is emitted there, because our build never emits `IAdornable::isVisible(Rect2D)` out of line. Sound.cpp gets FUNCTION and IAdornable.h stays STUB.
- `app_sweep.py` now reads STUDIO's PDB from `build-studio/`.

**5. Final figures** (after apply, rebuild and rescore; the rebuilt PDB view is identical to the one the CSVs were generated against)

| | PLAYER | STUDIO |
|---|---|---|
| App FUNCTION markers | 154 | 150 |
| App FOLDED markers | 23 | 23 |
| Functions at 100% (JSON) | 2,585 (1,765 library, 820 non-library) | 2,527 (1,711 library, 816 non-library) |
| reccmp with libraries | 3,548 / 40,468 (8.77%) | 3,454 / 42,329 (8.16%) |
| reccmp `--nolib` implemented | 1,501 / 38,363 | 1,519 / 40,225 |
| Same, earlier sweep (with libraries / `--nolib`) | 1,930 / 101 | 1,817 / 99 |

- Compared with the start, no entry lost 100% and 959 / 964 entries gained it.
- One entry I gained mid-run was given back: `??__Froot` (the atexit destructor of `root`) at 0x00a8f380 / 0x00b3c340, now at 50%, after I excluded the variable that crashes datacmp (see the last item).

**Remaining near misses (90–99.99%):** 86 PLAYER, 87 STUDIO.

| Shape | PLAYER | STUDIO |
|---|---|---|
| other | 66 | 66 |
| assert file/line | 8 | 8 |
| unpaired callee | 6 | 6 |
| inlining (size) | 5 | 6 |
| x87 vs SSE | 1 | 1 |
| data reference | 0 (was 8) | |

Unpaired callees went from 39 to 6 in PLAYER.

**Checks**

| Check | PLAYER | STUDIO |
|---|---|---|
| decomplint | 334 out of order, 24 duplicate_offset (was 42), 3 unicode errors | 340, 16 (was 34), 3 |
| vtable | 254 found, 99 not matching: 155 match (was 82) | 209, 70: 139 match (was 61) |
| datacmp | 565 variables, 17 issues (was 203 / 9) | 560, 19 (was 190 / 9) |
| check_folded | 0 problems | 0 problems |
| addr_padding | 1,653 files, 0 bad markers | |

decomplint, vtable and datacmp exit with 1, as before. The new datacmp issues are content differences such as boost `stored_vtable` pointers, `optsnum`/`opts` and FFlag values, not crashes.

**Excluded variable.** reccmp-datacmp crashed with a KeyError on `root___RBX::Reflection::ClassDescriptor::rootDescriptor` (0x00e24868). That variable is now left out of the generated data rows, which is why the `??__Froot` function above dropped to 50%.

## Structured fields

```json
{
 "files": [
  "tools/ida/gen_reccmp_csv.py",
  "tools/app_sweep.py",
  "reccmp/player-symbols.csv",
  "reccmp/player-libdata.csv",
  "reccmp/studio-symbols.csv",
  "reccmp/studio-libdata.csv",
  "notes/app-sweep.csv",
  "notes/app-sweep-xrefs.csv",
  "Client/App/util/Sound.cpp",
  "Client/Rendering/GfxBase/include/GfxBase/IAdornable.h",
  "Client/App/include/Util/IndexedTree.h",
  "Client/App/include/V8DataModel/Item.h",
  "build/sweep2/PF.json",
  "build/sweep2/SF.json",
  "build/sweep2/PF-nolib.json",
  "build/sweep2/SF-nolib.json",
  "build/sweep2/P0.json",
  "build/sweep2/S0.json",
  "build/sweep2/P3.json",
  "build/sweep2/P4.json",
  "build/sweep2/P6.json",
  "build/sweep2/S3.json",
  "build/sweep2/S4.json",
  "build/sweep2/S6.json",
  "build/sweep2/verbose",
  "build/sweep2/lint-PLAYER.txt",
  "build/sweep2/lint-STUDIO.txt",
  "build/sweep2/vtable-PLAYER.txt",
  "build/sweep2/vtable-STUDIO.txt",
  "build/sweep2/datacmp-PLAYER.txt",
  "build/sweep2/datacmp-STUDIO.txt",
  "build/sweep2/folded-PLAYER.txt",
  "build/sweep2/folded-STUDIO.txt"
 ],
 "unresolved": [
  "Wrong prior marker, outside my FOLDED-only marker scope: Client/App/include/Util/Extents.h:60 puts Extents::vv at PLAYER 0x009dab20, but that body is G3D::Vector3::maxFinite (4 aligned votes). It blocks the maxFinite pairing, which holds Extents::express 0x00796920 (99.17%) and Extents::toWorldSpace 0x00796d00 (99.06%) below 100%. The marker needs removing or re-addressing by the App-tree owner; the IDA ios: name at that address is wrong too.",
  "The original's linker folded unique_lock<mutex>::lock and unique_lock<recursive_mutex>::lock onto 0x004366c0. reccmp pairs one-to-one, so AsyncHttpQueue::isUrlBad 0x0086e540 (99.23%) and LibraryService::registerDevelopmentLibrary 0x0088cce0 (98.92%, which calls exception_ptr::~exception_ptr) cannot both pair. Both reached 100% in an intermediate run and dropped again; neither was at 100% at the start.",
  "The CSVs depend on which reccmp --json runs feed the position passes, and re-running from a single run is not monotone (one run lost 83 entries). The current CSVs pool build/sweep2/P3, P4, P6 (S3, S4, S6 for STUDIO); their headers record this. After any code change those JSONs' addresses for our build go stale: rescore and regenerate, pooling 2-3 fresh runs.",
  "reccmp bug: reccmp-datacmp raises KeyError 49565 in reccmp/cvdump/types.py on the static `root` in ClassDescriptor::rootDescriptor (PLAYER 0x00e24868), which kills the whole datacmp run. It is excluded (DATACMP_CRASH in gen_reccmp_csv.py), so its atexit destructor (PLAYER 0x00a8f380, STUDIO 0x00b3c340) scores 50%.",
  "G3D::CoordinateFrame::inverse (PLAYER 0x004a2dc0, STUDIO 0x00532180) is still unpaired. It has no oracle name, and the match: name at 0x004a2e90 (toObjectSpace(Box)) is wrong per the g3d report. The generated CSVs take no hand rows, so the fix is in the IDA names or the lib-resolved export.",
  "The vftable size cap (the original's code-pointer run, limited to our gap) can hide a trailing virtual ours lacks. RBX::AssemblyStage vftable 0x00b94b28: the original has 6 code slots (6th 0x008e4760), ours has 5. It still reports 100%, as it did before this change, so AssemblyStage probably lacks a virtual.",
  "Remaining unpaired-callee near misses: MovingStage::onMechanismRemoving 0x0099a5b0 (original calls getTypedChild<IndexedMesh>, ours IndexArray::operator[]; a code difference). Type::getSingleton 0x0079ff00: the Type<void> constructor marker in Client/App/include/reflection/Type.h:80 (0x0079fc70) pairs differently from what the call site needs.",
  "21 references per notes/app-sweep-xrefs.csv still have 2 or more agreeing votes and no conflict. They come from instructions whose offsets differ between the two sides, so the alignment rule does not pair them. Examples: Adorn::preSubmitPass to 0x00528720 (17 votes but 1 conflict), Name::getNullName 0x00613a90.",
  "CLAUDE.md needs updating (I may not edit it):\n- STUDIO is scored from build-studio/.\n- gen_reccmp_csv.py takes --json A.json,B.json to pool reccmp runs.\n- Oracle names now pair through the symbols CSV `symbol` column (signature match).\n- FOLDED is written by app_sweep.py apply."
 ]
}
```


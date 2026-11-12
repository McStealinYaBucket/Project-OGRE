# pair-names (workflow phase1-resume, 2026-11-12)

counts: [{"evidence_class": "rtti-slot", "count": 2690}, {"evidence_class": "string-unique", "count": 1970}, {"evidence_class": "string-set", "count": 226}, {"evidence_class": "import-set", "count": 12}, {"evidence_class": "callgraph", "count": 1908}, {"evidence_class": "callgraph-guess (source_tag guess:)", "count": 331}]
total_player_functions 48858 total_named 6806 app_decomp_mapped 1328

I wrote `tools/pair_names.py` and ran it for PLAYER against IOS_1029 (the primary oracle) and MACSTUDIO. It names **6,806 of 48,858 PLAYER functions** with strong evidence, plus another 331 functions that are only guesses. It reads only the evidence exports and opened no database, so no `.i64` changed and nothing was added to `ida/LOG.md`. The result is the same on every rerun; the full run takes about 5 seconds.

**Run:** `python3 -P tools/pair_names.py --target PLAYER --oracle ios1029=IOS_1029 --oracle mac=MACSTUDIO --pe bin/player_1023/RobloxPlayerBeta.exe --app-decomp "2013 55% App"`. For STUDIO later, swap in `--target STUDIO --oracle ios1021=IOS_1021`. `--selftest` checks the helper functions.

**How each class works**
- **rtti-slot:** classes are matched by a normalized class name (MSVC `class X`, `char const * const sName` and Itanium enum casts are written the same way).
  - Itanium D1+D0 are merged into one slot, paired with MSVC's scalar deleting destructor; those rows carry the D0 name and a note in `demangled`.
  - Itanium keeps a copy of an override of a non-primary base in its primary vtable (the one with a `_ZThn`/`_ZTv` thunk). MSVC does not, so those copies are removed, and the destructor too when the count calls for it (the RBX::Instance case).
  - MSVC gives a virtual base its own vftable while Itanium shares it, so those are set aside on both sides. That recovered the PartInstance and Accoutrement families, about 95 functions, with no new disagreements.
  - Slots inside the span of an overload set are skipped, and any class with a pure-virtual mismatch or a remaining count mismatch pairs nothing.
  - On Mac, where a vtable slot only has an IDA label, the slot layout comes from the same iOS class (87 classes); the Mac address is kept.
  - One MSVC address with several oracle names is kept as ICF folding only if the oracle bodies look alike: 332 addresses. The largest is 0x00927cc0 with 814 trivial `return false` names.
- **string-unique:** assert strings compare as "expr file: <basename> line: N". A bare `__FILE__` path is never used.
- **string-set, import-set:** both sides are demangled with llvm-undname or llvm-cxxfilt and reduced to qualified names. Only imports both sides have count, intrinsics are dropped, and the shape must agree.
- **callgraph:** oracles run together. A pair needs two independent seeds, or both oracles agreeing, to be strong; with one seed it is tagged `guess:`.
- **Checks on every candidate:**
  - **Strings:** each string one side has that the other side also has somewhere must sit in the partner or its callees to depth 2. This allows for inlining.
  - **Callees:** callees already paired on either side must match, to depth 2.
  - **Wrappers:** clang-only wrappers are removed on the oracle side so nothing pairs with them. These are a C1/D1 that only calls its C2/D2, IDA `j_` thunks and `$shim` islands. This fixed a systematic mistake where the Windows singleton wrapper was being named `EnumDesc<T>::EnumDesc()` C1.
  - **Fake names:** names IDA made up on Mach-O, such as `InitFunc_96`, are not accepted as names.

**Results (functions, by best class):** rtti-slot 2,690; string-unique 1,970; string-set 226; import-set 12; callgraph 1,908; callgraph-guess 331. That makes 12,443 rows (folded addresses carry several rows) and 6,806 strong functions.
- **.text functions over 32 bytes not already named by IDA:** 29,265; 5,997 are named (20.5%), plus 299 guesses.

**How often the two oracles agree, for functions both of them name:**

| Class | Agree | Disagree |
|---|---|---|
| rtti-slot | 2,432 | 2 |
| string-unique | 1,624 | 3 |
| callgraph | 1,817 | 32 |

The disagreements show up as real inlining or platform differences: `Name::declare`/`doDeclare`/`callDoDeclare`, `onLocalPlayerNotIdle` vs `registerLocalPlayerNotIdle`, RakNet `_Old`/`_PC` variants, `tbb::spin_mutex` vs `RBX::mutex`, and `va_list` as `char*` vs `void*`. All 87 conflicts are in `PLAYER-names-conflicts.csv` and none of them gets a name.

**iOS RBX coverage:** of 16,286 iOS functions named `_ZN3RBX…`, 5,259 got a PLAYER home (32.3%). Counting `_ZNK3RBX` as well, 9,305 of 24,360 (38.2%). Mac: 5,134 of 15,838 (32.4%).

**App decomp:** 1,328 of the 2,733 annotated Mac functions got a PLAYER address; 35 of those are guesses. By class: string-unique 511, callgraph 376, rtti-slot 343, string-set 61, import-set 2. Of the Mac functions that have no symbol, 27 of 93 got a PLAYER address.

**Classes with slot-count mismatches** (full lists, with reasons, in the two `PLAYER-rtti-mismatch-*.csv` files):
- **iOS (28):**
  - the BodyMover family (BodyAngularVelocity, BodyForce, BodyGyro, BodyMover, BodyPosition, BodyThrust, BodyVelocity, Rocket);
  - Humanoid, SkateboardPlatform, VehicleSeat;
  - boost `thread_data<…>` ×3 and `boost::exception` (2 vs 4 and 1 vs 2, both destructor slots pure or odd);
  - boost iostreams/io classes, some with equal counts but pure-virtual slots that differ.
- **Mac (50):** the same set, plus 10 `RBX::Reflection::Metadata::*` classes [28 1 2 3 | 32 1 3 4], 10 `sp_counted_impl_p*` classes, and `Ogre::RbxEntity`.
- **Group count still differs:** 4 classes on each side.

## Files

- `tools/pair_names.py`
- `ida/exports/PLAYER-names-proposed.csv`
- `ida/exports/PLAYER-names-conflicts.csv`
- `ida/exports/PLAYER-app-decomp-map.csv`
- `ida/exports/PLAYER-rtti-mismatch-ios1029.csv`
- `ida/exports/PLAYER-rtti-mismatch-mac.csv`

## Unresolved

- Precision was not measured against an independent ground truth. The only checks are oracle agreement (2,432:2 rtti-slot, 1,624:3 string-unique, 1,817:32 callgraph), a hand review of about 20 sampled rows, and a test against 'Class::method' text inside strings, where every mismatch traced to a string naming a different function. iOS and Mac are both clang builds and inline alike, so agreeing with each other does not prove MSVC kept the same function.
- Family-wide ambiguities remain wherever only one oracle names the function: the Name::declare/doDeclare/callDoDeclare and Singleton<EnumDesc>::doGetSingleton wrapper chains, and outer-versus-inlined-inner pairs such as Player::onLocalPlayerNotIdle vs registerLocalPlayerNotIdle. Where both oracles name the function these were caught as conflicts.
- Coverage is 20.5% of .text functions over 32 bytes. Precision was preferred: callgraph only pairs a function when exactly one unpaired neighbour is left, or when the string sets agree, and 3,182 / 2,486 callgraph candidates (iOS / Mac) failed the checks. Statically linked CRT, D3DX and third-party code is mostly not attempted here; that is FLIRT's job.
- Slot-count mismatches still unpaired: the BodyMover family, Humanoid, SkateboardPlatform and VehicleSeat (a secondary base that has secondary bases of its own), boost thread_data and boost::exception (pure or odd destructor slots), boost iostreams classes (pure-virtual slots differ), and on Mac only RBX::Reflection::Metadata::* and sp_counted_impl_p* (internal-linkage destructor slots that iOS could not supply a layout for). 4 classes still differ in vftable count.
- Inside an overload span, slots are skipped rather than resolved. MSVC's overload ordering is not modelled, so some named virtuals stay unpaired.
- 15 rtti-slot addresses had several oracle names that do not look like ICF folding (e.g. 0x00851c00, 0x0083edd0, 0x00401210) and are left unnamed. Why was not traced.
- import-set is nearly useless for iOS (Ogre and boost are statically linked there) and contributes only 12 functions. Arity cannot be checked because the PE export has no prototypes, so block and callee counts stand in for it.
- D0 rows name the MSVC scalar deleting destructor with the Itanium deleting-destructor symbol. That is the nearest Itanium name, not an exact equivalent (MSVC's is ??_G); it is flagged in the demangled column.
- No IDA database was updated. Pushing these names into ida/PLAYER.i64 (as ios:/mac:/guess: names, plus prior: names for the App map) still has to be done, with a snapshot first.

# phase5-wave7 util-target: shared Util headers and per-target build fixes

Builds: `build-utl/` (PLAYER) and `build-utl-studio/` (STUDIO). Scoring used a private root
(`build-utl/w7/root`, symlinks plus private `reccmp/` CSVs and project yml), regenerated with
`gen_reccmp_csv.py` from two pooled runs of each link (scripts `build-utl/w7/{snap,score,full,v}.sh`, `cmp.py`).
Scored copies are in `build-utl/w7/snaps/<run>/`, JSONs in `build-utl/w7/runs/`.

## Result

| | sweep13 | final | lost (all read with --verbose) |
|---|---|---|---|
| PLAYER (`q5p`) | 9,893 | **10,064** | 10 |
| STUDIO (`s3p`) | 9,637 | **9,796** | 4 |

These totals also include the other wave-7 agents' uncommitted work in the shared tree. The entries below are mine.
Only one loss is mine: the `TypedStatsItem<int>` vftable, in each target (item 3).

The interrupted run's edits to my files were kept, because they compile and lose no 100%. Two details were corrected:
a duplicate TEMPLATE marker, and the `initWithoutSecurity` comment, which claimed the function was virtualized.

## 1. Per-target define and Object.h spelling

- `CMakeLists.txt`: `add_compile_definitions(OGRE_TARGET_${OGRE_TARGET})`, with PLAYER as the default. This gives every
  TU either `OGRE_TARGET_PLAYER` or `OGRE_TARGET_STUDIO`. The define changes every compile line, so each build dir
  rebuilds everything once.
- `DataModel.cpp`: under `#ifdef OGRE_TARGET_PLAYER`, `#include "util/object.h"` comes first. It replaces a blank
  line, so the line count is unchanged.
  - PLAYER gains: isNullClassName<sContentFilter> 0x00632290, static_getCreator<Visit> 0x00630f20,
    Creator<Visit>::getClassName 0x00631dc0, isNullClassName<Visit>, and 11 more isNullClassName<> in
    0x00632330–0x006329f0 (15 in all).
  - STUDIO is unchanged.
- PLAYER's DataModel.obj supplied the other copies, which 2013 takes from Network and shell TUs. Those copies
  would then carry the lowercase spelling.
  - `Client/Network/Player.cpp` ends with a stand-in block of explicit instantiations. It is not 2013 code, has no
    marker, and its comment lists the 2013 callers:
    - `ServiceProvider::find<Spawner/StarterGui/StarterPack/PersonalServer/Social/Joints/Physics/Script>`
    - `create<MarketplaceService>`
    - `create<Lighting>`
  - Network links before App, so these copies keep `include\Util/Object.h`.
  - That won back all 12 PLAYER entries the define alone cost: 0x0052ed00, 0x0052f430/4d0/570/610, 0x00583dc0,
    0x00586b10, 0x00588ca0, 0x00588da0, 0x005b9e40, 0x0061a3e0 and Creator<Lighting>::getClassName 0x00449980.
  - Delete each line once its real caller is written.

## 2. Util headers

- **ExtentsInt32() is inline** in `Util/ExtentsInt32.h`, with its FUNCTION markers moved there.
  - The Primitive ctor no longer stores an EH state before the call.
  - ExtentsInt32() is at 100% in both targets: P 0x004e5df0, S 0x0056dac0.
- **ConcurrencyValidator counters are `volatile long`**, not `rbx::atomic<int>`.
  - Every 2013 ctor stores `writing` ahead of the EH state/this-save for `writeLocation`.
  - A probe compile (`cl` on a 3-member stand-in) shows VC9 sinks a plain store below those stores and keeps a
    volatile one in place.
  - The pre/post helpers use `BOOST_INTERLOCKED_*`.
  - The header comment was rewritten in place, so the dtor asserts stay on lines 30/31.
  - Results, all PLAYER:

    | Function | Address | Before | After |
    |---|---|---|---|
    | Primitive ctor | 0x00756a40 | 99.3% | **100%** |
    | SpatialHash ctor | 0x008eef80 | effective 98.4% | **100%** |
    | ContactManager ctor | 0x007b63a0 | 80.4% | 95.9% |
    | ~ConcurrencyValidator | 0x00501660 | 80.0% | 93.1% |

  - ContactManager's remaining diff is the prime_list end pointer 0x00bdb080, which pairs as a string (a CSV
    issue). ~ConcurrencyValidator's is the `Include\util/ConcurrencyValidator.h` spelling: 2013's kept copy comes
    from an RbxOgre TU.
- **ComputeProp.h**: `ComputeProp<Type,O>` is {val, dirty, object, getFunc}, with a ctor that takes the object and
  getFunc (sizeof 0x18 for <float, Primitive>). Primitive.h already includes it. Swapping in its `JointKProp`
  stand-in is for world-5d-4.
- `Util/SpanningEdge.h`: the interrupted run's edit is kept. It drops the four list-link members, because 2013
  Joint's hooks are its own `boost::intrusive` bases (Joint ctor 0x008215c0). It compiles, and Joint/StepJoints
  gained.

## 3. Stats::Item

- `RBX::StatsItem` becomes `RBX::Stats::Item`, `sStatsItem` moves to `Stats::sStatsItem`, and a typedef
  `StatsItem` stays for outside users. StatsService uses `sStats` ("Stats") with INTERNAL_LOCAL and RobloxPlace.
- `DataModel.h`'s TEMPLATE nameref for `fastDynamicCast` (P 0x00641940, S 0x0075f1f0) now spells
  `VItem@Stats@RBX@@`. It is at 100% again in both targets.
- The ~53 renamed template entries pair again after the second CSV regeneration round. Gains include:
  - classDescriptor<Stats::StatsService> 0x0048b550
  - NonFactoryProduct<…sStats> vftables
  - Stats::Item vftables (P 0x00b01ba4, S 0x00bd51a4/0x00bd51ac)
  - doDeclare<sStatsItem>
  - Described<Item> ctor
- **Loss (one per target): `TypedStatsItem<int>` vftable** (P 0x00b01ce4, S 0x00bd614c), slot 0x70.
  - 2013's `TypedStatsItem<T>` overrides `update()` (P 0x0048e7a0 for int, 0x0048e7d0 for bool). Each override
    calls the boost::function at +0x88 and passes the result to `formatValue` (0x0073b450 / 0x0073b510).
  - Ours does not override `update()`, so the slot holds `Item::update`. The vftable used to score 100% only
    because the old name paired `Item::update` with 0x0048e7a0 by position. The rename makes the signature pair it
    with its real address, 0x0073ec00.
  - The fix is a 2013 `TypedStatsItem` (function-only, no `bound` pointer) with its `update()`. That is a Stats
    rewrite, not attempted here.

## 4. FFlags

- `FFlag::AllowYieldInHooks` is defined in `script/ThreadRef.cpp` (FASTFLAGVARIABLE, on a blank line, so lines
  are unchanged).
  - It pairs: libdata row 0x00e12d44 to ThreadRef.obj.
- `FFlag::LuaDebugger` has GLOBAL markers P 0x00e12ec8 / S 0x00f67698 in `util/FastFlags.cpp`.
  - Both are verified against the 2013 registrations: P 0x00a82050 `push 0xe12ec8`, S 0x00b14760 `push 0xf67698`.
  - closeStates reads it at 0x00685403.
- `reccmp-datacmp` PLAYER reports no issue for either flag (55 issues overall, none of them these).

## 5. classDescriptor COMDATs

- **`Described<GlobalAdvancedSettings>::classDescriptor`**: P 0x005268b0 and S 0x00607940 went from 86.2% to
  **100%**.
  - Network's `API.cpp` registers Client, Server, Player, Players, NetworkSettings and Peer, and instantiates the
    GAS copy through `initWithoutSecurity`, which sees the class name only as an extern. Network links before App
    in both targets.
  - Also gained:
    - classDescriptor<NetworkSettings> P 0x00526130, S 0x00607440
    - classDescriptor<Client> P 0x005261b0, S 0x005cc7b0
    - Creator<Client> ctor and getClassName
    - doDeclare/callDoDeclare<sClient>
  - `initWithoutSecurity` now has STUB markers, P 0x00526c50 / S 0x006079c0. It holds a VMProtect marker region
    (P 0x00526c6a, 137 bytes), so it cannot match.
- **`Described<LuaSettings>::classDescriptor`**: P 0x00788570 and S 0x00735cc0 went from 86.2% to **100%**.
  - The new file is `Client/App/v8datamodel/FactoryRegistration.cpp`, the 2013 registration TU.
    - Its dynamic initializers start at P 0x00a75a20 and call GAS's classDescriptor first and LuaSettings' later.
    - Its classDescriptor block is 0x00786d70–0x0078a800.
    - The 2013 binary records no `__FILE__` for it, so the name comes from 2016, with an uppercase F so the glob
      links it ahead of LuaSettings.cpp.
  - It holds only `RBX_REGISTER_CLASS(GlobalAdvancedSettings)` and `RBX_REGISTER_CLASS(LuaSettings)`. Both were
    removed from their own .cpp files, with one-line comments in their place so line counts are unchanged.

## 6. NetworkOwner, NetworkSettings, GlobalAdvancedSettings::singleton

All markers below are flipped to FUNCTION.

- **NetworkOwner::Unassigned**: P 0x00559100 and S 0x005b7bc0 went from 58.4% to **100%**.
  - The 2013 header has two asserts, on lines 28/29 (`s == SystemAddress()`, `s != Server()`).
  - Server() is the only other member. ServerUnassigned, AssignedOther, isClient and isServer were dropped: no 2013
    code uses them.
  - Also gained: `SystemAddress::operator!=` 0x007cfa80.
- **NetworkSettings ctor**: P 0x005b7cf0 went from 47.4% to **100%**, S 0x00609e80 from 98.3% to **100%**.
  - Each placeholder bool is its own member with its own init.
  - STUDIO's older revision stores only 0x64–0x6c, so the 0x6d init is `#ifdef OGRE_TARGET_PLAYER`.
  - Also gained: the item-template ctor P 0x005b7790 and Described<NetworkSettings> ctor P 0x005b7720 / S 0x006098b0.
- **GlobalAdvancedSettings::singleton**: P 0x00658980 went from 38.1% to **100%**, and S 0x00741110 too.
  - The chain is modelled as in 2013:
    - The static getter `globaladvancedsettingsStatic` (P 0x00658850, S 0x00740fe0) takes its return slot in esi
      and publishes the raw pointer.
    - `initAdvancedSingleton` (P 0x006588e0, S 0x00741070, was 26.1%) runs the getter under a
      `Limits::Counter::Activator` for an empty counter.
  - All three are at 100% in both targets.

## Losses read with --verbose (final vs sweep13)

- Mine: `TypedStatsItem<int>` vftable (item 3).
- Not mine:
  - `Primitive::getGeometryType`/`getCollideType`, P 0x00753cb0/0x00753d20 and S 0x00893060/0x008930d0: the assert
    line moved 588 → 590 in world-5d-4's in-progress Primitive.cpp.
  - `~ContactManager` P 0x007bb5e0 / S 0x008d7f80: `~SpatialHash` no longer pairs (world-5d-4).
  - Lua symbexec/luaV_execute/constructor/patchtestreg/removevalues/luaK_setreturns (PLAYER only): the known
    no-.reloc coincidences. patchtestreg's diff is an immediate read as an RTTI address.

## Checks

- Both targets build.
- `check_2016_copies`: silent.
- `reccmp_addr_padding`: 0 badly padded.
- `reccmp-decomplint` on every file I changed: clean for both modules.

## Unresolved

- **world-5d-4**:
  - The Primitive ctor P 0x00756a40 is now at 100%, so its marker in Primitive.cpp can be FUNCTION.
  - `Primitive::JointKProp` can become `ComputeProp<float, Primitive>` (`Util/ComputeProp.h`).
  - Primitive.cpp line shift: getGeometryType/getCollideType asserts.
  - `~SpatialHash` pairing.
- **Rendering contributor**: 2013's kept `~ConcurrencyValidator` (P 0x00501660) is an RbxOgre copy, spelled
  `Client\App\Include\util/ConcurrencyValidator.h`. Ours comes from an App TU. It is 93.1%, and the spelling is the
  only diff.
- **Base owner**: if other 2013 constructors also store an `rbx::atomic<int>` ahead of an EH state, `atomic::v` may
  be `volatile` in 2013. Only ConcurrencyValidator was tested, and it is fixed locally.
- **Stats**: the 2013 `TypedStatsItem<T>` has only a boost::function at +0x88 and overrides `update()`
  (P 0x0048e7a0 int, 0x0048e7d0 bool; RTTI lists int/unsigned/uint64/float/bool/double). This is needed to win back
  the vftable. Bound children (LuaStatsItem) would bind through the function.
- **Integrator**:
  - Regenerate the shared CSVs: two or more pooled rounds, because the renamed Stats statics pair only on the second round.
  - ContactManager ctor and LibraryService ctor both trip on the prime_list end pointer 0x00bdb080, which pairs as a string.
- **Network**: the stand-in instantiations at the end of `Network/Player.cpp` go away as the real 2013 callers are
  written: `find<SpawnerService>` 0x0053be80 etc., `create<MarketplaceService>` 0x00598400, and the shell's
  `Creator<Lighting>`.
- **datamodel-5b-5**: STUDIO `Described<StarterGuiService>::classDescriptor` 0x0065dab0 now comes from
  StarterGuiService.obj, where the literal is defined. It was 86.2% in my intermediate STUDIO run and back at 100% in
  the final one. Watch it when StarterGuiService.cpp lands.
- **`GlobalBasicSettings`' getter** (P 0x006589b0, no raw-pointer publish) is not modelled.

## Files changed (mine)

- `CMakeLists.txt` (define).
- App:
  - `Client/App/v8datamodel/DataModel.cpp` (include) and `Client/App/include/V8DataModel/DataModel.h` (one nameref).
  - `Client/App/include/Util/{ExtentsInt32,ConcurrencyValidator,ComputeProp,SpanningEdge}.h`,
    `Client/App/util/{ExtentsInt32,FastFlags}.cpp`, `Client/App/script/ThreadRef.cpp` (FFlag line).
  - `Client/App/include/V8DataModel/{StatsItem,StatsService,NetworkSettings}.h`.
  - `Client/App/v8datamodel/{StatsItem,StatsService,NetworkSettings,GlobalAdvancedSettings,LuaSettings}.cpp` and
    the new `FactoryRegistration.cpp`.
- Network: `Client/Network/{API,Client,Peer,Player,Players,Server}.cpp`, `include/Network/{API,NetworkOwner}.h`.

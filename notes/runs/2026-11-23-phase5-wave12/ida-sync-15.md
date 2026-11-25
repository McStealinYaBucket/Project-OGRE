# ida-sync-15 (workflow phase5-wave12)

The wave 12 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed (no .id0/.nam/.til left). Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G; none hit the cap. ida/Archive.zip, Client/ and the shared reccmp/ CSVs were not touched; nothing was committed; the 2016-copy check is silent.

**1. Snapshots.** ida/snapshots/PLAYER-21-wave12-sync.i64 and STUDIO-21-wave12-sync.i64, `cmp`-identical to the canonical databases before any change.

**2. Names from the scored runs** (build/sweep19/P-final.json and build-studio/sweep19/S-final.json, joined only with build*/sweep19/scored/).
- Functions at effective 100%: PLAYER 8,963, STUDIO 8,548 (sweep18: 8,852 / 8,432). Library-owned rows (1,744 / 1,652) left to apply_lib_names.py.
- PLAYER renamed 99: 86 `match:` (83 dummies, `Handler` -> `purecallHandler`, two `resume_adapter<...>` spellings) and 13 facts.
- STUDIO renamed 94: 81 `match:` (incl. 3 via PLAYER body match; 4 IDA names, two of them `j_safe_static_*` thunks that reccmp pairs with `safe_static_init_mutex<...>` twins) and 13 facts. Skipped as before: STUDIO 0x0054b3b0.
- The new match: names are this wave's shell and Network code: LogManager / MainLogManager / RobloxCrashReporter, CrashReporter (Start thread, ProcessExceptionInThead, the StringCch helpers), CVersionInfo, VideoControl / SoundState, LeaveGameVerb / RecordToggleVerb, the ATL CStringT/CPathT helpers, FileSystem getBaseCacheDirectory / getCacheDirectory, ~VistaAPIs, Http::trustCheck, ~Network::Player, the Players signal dtors and disconnectAll.
- **Conflicts:** PLAYER 118 (98 folded-original, 20 same-function), STUDIO 119; no body-wins. Gone since wave 11: 0x0044db50 (the generator fix pairs the right twin now), 0x004595b0, and 0x0070a240 / STUDIO 0x00796960 (Players::askAddChild is FOLDED now).

**Facts for gen_link_stubs** (tools/ida/match_names.py). Decorated names as our tree declares them; each checked from the exe bytes. 9 of the 21 "unknown" callees per target in `Client/_generated/<t>/unresolved.txt` now have an exact names.csv row.

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x009b8060 | 0x00a72540 | RBX::utf8_encode(const std::wstring&) | src2016: | WideCharToMultiByte into std::string |
| 0x009b8100 | 0x00a725e0 | RBX::utf8_decode(const std::string&) | src2016: | MultiByteToWideChar into std::wstring, 16 callers |
| 0x009d5da0 | 0x006aaa70 | G3D::Log::println(const std::string&) | str: | printHeader, fprintf '%s\n', fflush (iOS symbol) |
| 0x009d5fa0 | 0x006aac70 | G3D::Log::print(const std::string&) | str: | same body with '%s' (iOS symbol) |
| 0x009b0f40 | 0x00a68030 | RBX::Log::setLogProvider | src2016: | stores to 0x00e1aa80 / 0x00f9dc70; the MainLogManager ctor passes `this` (0x0043fbb7) |
| 0x00e1aa80 | 0x00f9dc70 | data RBX::Log::provider | ios: | iOS symbol; written only by setLogProvider |
| 0x009b19a0 | 0x00a68a90 | RBX::setAssertionHook | src2016: | stores to `_debugHook` 0x00e1aac8 / 0x00f9dcb8 (our GLOBAL markers) |
| 0x009b19b0 | 0x00a68aa0 | RBX::setFailureHook | src2016: | stores to `_failureHook` (+4) |
| 0x0066ef10 | 0x0077fae0 | ProtectedString::getSalt() const | ios: | `mov eax, '10:46:55'; ret` (STUDIO '19:11:34'), a `__TIME__` salt |
| 0x00703070 | 0x0070c5d0 | Variant::convert<G3D::Vector2int16> | ios: | `jmp genericConvert<Vector2int16>`, shaped like convert<UDim2> 0x00703080 (100%); called by get<Vector2int16> |
| 0x008d3540 | 0x00846e30 | StringConverter<ContentId>::convertToValue | ios: | value = ContentId(text); return true; called by setStringValue (slot 10) |
| 0x0066f460 | 0x00780050 | StringConverter<ProtectedString>::convertToValue | ios: | fromTrustedSource 0x0066f320 + calculateHash; called by setStringValue (slot 10) |
| 0x00623bb0 | 0x006cca30 | Http::isMoneySite | str: | 'paypal.com', 'rixty.com' (STUB, 85.1) |
| 0x00624620 | 0x006cd530 | Http::trustCheckBrowser | src2016: | ends `setne al` on isMoneySite (STUB, 94.1) |

The other 12 per target cannot be named, because no 2013 body exists under that name:
- **CSHA1 (6), RakNet TCPInterface (2), PacketizedTCP (2):** neither exe has RTTI, a vftable or a string for them. Our references come only from ReliabilityLayer::GetSHA1 and PluginInterface2::AllocatePacketUnified.
- **`FLog::RegisterFlag`:** a Log.dll import, `__imp_?RegisterFlag@FLog@@YAHPBDPA_NPAPA_NW4FastVarType@@@Z`, so 2013's `FastVarType` enum is at global scope. Our FLog stand-ins in Network/Server.cpp and CrashReporter.cpp spell it `FLog::FastVarType` (`W4FastVarType@1@`).
- **`TypedPropertyDescriptor<Vector2int16>::getDataSize` / `hasStringValue`:** ICF-folded slots 7 and 8 of vftable 0x00b86658, at 0x006fd890 (`getDataSize<float>`) and 0x006be550 (`isSelectable3d`). One IDA name per address cannot carry them.

**lib-resolved** (tools/lib/lib_consistency.py, rerun): no change. I kept the 0x0044d400 conflict row. A conflict row emits no library row, so it does not affect pairing, and it records why the std::exception match was rejected.

**3. Types** (import_types.py, dumped from the scored PDBs). Each class below has a ctor or dtor at 100% in sweep19, and its PDB size equals the tree's `// SIZE`.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| LogManager | 0x28 | dtor 0x00440780 / 0x0041e120 |
| MainLogManager | 0x64 | dtor 0x00440600 / 0x0041dfa0 (ctor 87.1 / 60.2) |
| CrashReporter | 0x268 | ctor 0x00520070 / 0x005d78c0, dtor 0x005200f0 / 0x005d7940 |
| RobloxCrashReporter | 0x268 | ctor 0x0043f570 / 0x0041d0c0 |
| RBX::VideoControl | 0x120 | dtor 0x00464dd0 / 0x0048c940 |
| RBX::SoundState | 0x60 | dtor 0x00464b60 / 0x0048c6e0 |
| RBX::LeaveGameVerb (PLAYER only) | 0x10 | ctor 0x00438220 |
| RBX::RecordToggleVerb (PLAYER only) | 0x70 | dtor 0x00439230 |

- RBX::ILogProvider and Ogre::IFrameDataCallback came in as bases.
- Counts: local types PLAYER 317 (117 listed), STUDIO 301 (111); before, 307 / 293. `this` is typed on 496 / 450 functions (299 / 277 ctor/dtor); before, 460 / 424. No size mismatch and no type removed. DataModel's dump differs only in two folded-slot comments.
- **Not imported:**
  - VistaAPIs: dtor at 100%, but no size annotation.
  - Network::Players and Server: ctor/dtor at 0.51–0.91.
  - Still waiting from earlier waves: AssemblyHistory, GlobalAdvancedSettings, TaskScheduler, JointInstance, BallBall/BallBlockContact, Workspace.

**4. Unnamed functions** (ida/exports/PLAYER-app-unnamed-by-class.csv regenerated).
- PLAYER App block (0x00606a00–0x009ad3a0, 21,631 functions): ios:/mac:/match: 8,473 -> 8,488; 13,143 (60.8%) still without one.
- Network block (0x0051f6b0–0x00606a00, 5,336): 2,005 -> 2,037.

Largest App-block groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,060 (596 KB) |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 |
| EnumDesc<> | 178 |
| DataModel | 164 |
| RBX free functions | 159 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| ScriptDebugger | 86 |
| Creatable<> | 86 |
| PartInstance | 82 |
| RemoteFunction | 82 |
| SceneUpdater | 81 |

Network-block groups: Replicator 375, Players 160, ServerReplicator 118, Player 113 (was 122), Server 56.

**5. Coverage, before -> after.** I regenerated the names, functions and types exports (PLAYER 748 type declarations, STUDIO 640; before, 737 / 631). Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py or body_match.py. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 14,645 -> 14,746 | 15,274 -> 15,362 |
| match: | 6,787 -> 6,876 | 6,515 -> 6,599 |
| ios: | 6,209 -> 6,213 | 6,268 -> 6,272 |
| src2016: | 10 -> 16 | 10 -> 16 |
| str: | 4 -> 7 | 3 -> 6 |
| rtti: | 189 -> 185 (four slot names are now match:) | 185 -> 181 |
| ida: | 1,064 -> 1,067 (IDA's `j_` names on six thunks 0x0053fe50–0x0053fea0 to the newly named signal dtors) | 1,814 -> 1,809 |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64; ida/snapshots/{PLAYER,STUDIO}-21-wave12-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (14 facts), tools/ida/import_types.py (8 classes, PLAYER_ONLY)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv; ida/exports/PLAYER-app-unnamed-by-class.csv (lib-resolved rewritten, unchanged)
- ida/logs/{PLAYER,STUDIO}-21-db-updates.log/.tsv, -21-types.log, -21-export-names.log

## Unresolved
- **Integrator:** rerun tools/gen_link_stubs.py for both targets (delete the exe first). These now bind to stubs: utf8_encode/decode, G3D::Log::println, RBX::Log::setLogProvider, ProtectedString::getSalt, Variant::convert<Vector2int16>, and both StringConverter::convertToValue. Then regenerate the CSVs.
- **Network owner:** `FLog::RegisterFlag` is imported from Log.dll with a global-scope `enum FastVarType` (`...PAPA_NW4FastVarType@@@Z`). The FLog stand-ins in Network/Server.cpp and CrashReporter.cpp use `FLog::FastVarType`, so they stay unresolved. The CSHA1, TCPInterface and PacketizedTCP references (ReliabilityLayer::GetSHA1, PluginInterface2::AllocatePacketUnified) have no 2013 counterpart. They are probably harmless /FORCE:UNRESOLVED leftovers from COMDATs that /OPT:REF drops; a RakNet define could remove them.
- **App owner:**
  - Util/ProtectedString.h declares `getSalt()` static. iOS and the Lua stand-in say it is a const member, which is the decoration lauxlib.c wants.
  - `TypedPropertyDescriptor<Vector2int16>::getDataSize` / `hasStringValue` are folded in 2013 (0x006fd890 / 0x006be550). They need the template member definitions instantiated, not a stub.
- **Not named (no decoration in our build to copy):** MemberDescriptorContainer<T>::compare 0x00614cd0 (one folded copy, T unproven), and GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>::singleton 0x00407230 with its createInstance 0x00409020.
- **Carried:**
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv predate STUDIO isUpperRoot 0x00a0e170.
  - STUDIO 0x004337a0 is still unnamed.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - 0x009b7020 is still a dummy.

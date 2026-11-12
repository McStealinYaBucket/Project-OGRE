# lib-raknet (workflow phase2-skeleton, wf_e3afb0d0-6b4)

I compiled RakNet into the 2013 Network library form. PLAYER has 567 located RakNet functions and 566 are byte-identical after masking (469 distinct addresses, 105,672 bytes). STUDIO has 565 located and 565 identical. The same source and flags produce both.

**Version: 4.036, as the facts already bounded it, plus Roblox's 2013 edits.**
- **Source.** No pristine 4.035 or 4.036 PC release can be fetched. The archived 4.035 and 4.037 zips on jenkinssoftware are password-protected PS3/Vita builds, and facebookarchive/RakNet's git history starts at 4.081 (2014). So the staged base is RakNet_PC-4.034 from web.archive.org, and its SHA1 matches the archive's recorded digest. Upstream code text in the patches comes from RakNet_PC-4.067, also archive-verified, not from the 2016 tree.
- **4.035 and 4.036 changes found in the bytes:**
  - DS_Multilist.h removed (4.035): the `__LINE__` inside ReliabilityLayer.h drops by 1, seen in BPSTracker::Push1 at 0x005ca3e0 (push 0x5a instead of 0x5b).
  - `UsesReliabilityLayer` and the split plugin lists (4.036): the PluginInterface2 vftable at 0x00b27304 has 17 slots; seen in AttachPlugin 0x005fbc30 and Receive 0x005fcef0.
  - Duplicate-datagram crash fix (4.036): HandleSocketReceiveFromConnectedPlayer at 0x005cd5a0 counts the dropped bytes before freeing the packet.
  - GetLengthUTF8 / TruncateUTF8 (4.036): every later `__LINE__` in RakString.cpp moves by 64 (RakString::Free 0x005c7a80 pushes 0x4e9).
- **Upper bound.** The lower length check on ID_OUT_OF_BAND_INTERNAL that 4.067 has is absent: ProcessOfflineNetworkPacket at 0x005ffc90 only tests `length < 0x1a9`. 4.067's OnReliabilityLayerNotification rename is also absent. No change after 4.036 was found. I could not prove anything stronger than "4.036" for the upper bound, because the only 4.037 fix (GetClientPublicKeyFromSystemAddress) sits in LIBCAT_SECURITY code that is compiled out.

**Roblox 2013 edits, each confirmed by PLAYER bytes:**
- RunUpdateCycle is virtual: RakPeer's vftable at 0x00b27f54 has 83 slots against 82 for RakPeerInterface, and ProfiledRakPeer overrides it.
- The startingActiveSystemListIndex round-robin, which accounts for the +4 field shift.
- ReliabilityLayer::Update returns bool.
- FastLog RakNetDisconnect messages: "Raknet: AckTimeout", "Raknet: connection lost" and two formatted variants, "RakNet: CloseConnection", and "RakNet: Calling CloseConnectionInternal, BCS_CLOSE_CONNECTION".
- Receive skips the NTS loop with `if (packet)`.
- SeedMT's printf is removed (upstream 4.067 still has it).
- The RakNetSocket destructor sets `s=-1` under `FFlag::MarkRakNetSocketOnDelete`; that flag is gone from the 2016 tree.

**What is not in the 2013 build:** these 2016 edits are missing from PLAYER, so the 2016 tree is not a 2013 source:
- RakNetMaxSplitPacketCount
- the iOS SIGPIPE blocks (line count only)
- the boost hash_value / GetBinaryAddress additions, for which there is no byte evidence either way.

**Linked translation units (identical in PLAYER and STUDIO), 25:** BitStream, CCRakNetSlidingWindow, CheckSum, DS_HuffmanEncodingTree, FileOperations, GetTime, Itoa, LocklessTypes, PluginInterface2, RakNetSocket, RakNetStatistics, RakNetTypes, RakPeer, RakSleep, RakString, RakThread, Rand, ReliabilityLayer, SignaledEvent, SimpleMutex, SocketLayer, StringCompressor, StringTable, SuperFastHash, WSAStartupSingleton. RakMemoryOverride also has `__FILE__` refs, but none of its functions could be placed. No RakNet plugin is linked: RTTI has only RakPeer, RakPeerInterface and PluginInterface2. This set agrees with the 26 files in the prior decomp's `Network/raknet/Source`, which turn out to be byte-identical copies of the 2016 files.

**Network flags = the proven App set** (`/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0`), measured as identical-function counts in PLAYER against 566 for the App set:

| Variant | Identical |
|---|---|
| `/Oy-` | 108 |
| 2016 vcxproj (`/Oy- /arch:SSE2`) | 106 |
| `/arch:SSE2` | 496 |
| `/GS` | 424 |
| `/Ob1` | 330 |
| no `/arch` | 557 |
| `/fp:precise` | 563 |

- `/fp:fast` is proven by ReadFloat16 0x005c3910, WriteFloat16 0x005c39b0 and StatisticsToString 0x005d9de0.
- `/arch:SSE` is proven by 69 functions, for example SystemAddress::operator= 0x005710d0.

**Patches, applied to the pristine copy in order:**
- `0001`: the upstream 4.035/4.036 changes.
- `0002`: the Roblox edits; it needs FastLog.h from "2013 55% App/Log/include".
- `0003`: 13 `#line` directives, measured from PLAYER's aligned `_FILE_AND_LINE_` pushes. The 2013 files' line counts can't be rebuilt any other way without copying the 2016 text.

`raknet_patches.py` regenerates all three and records the PLAYER evidence for each edit. The 2016 tree was only probe-compiled in scratch to recognise edits; nothing from it went into the repo.

**Reproduce:**
```
python3 -P tools/lib/raknet_patches.py   # regenerate the three patches (~2 min)
python3 -P tools/lib/raknet_match.py     # compile, match PLAYER and STUDIO, write the CSV (~1 min)
```

**Unmatched rows.** One PLAYER row is not identical: NumberOfLeadingZeroes(int) at 0x005c6cc0, placed only by .obj order and almost certainly the wrong place (the target there is a `jmp [mem]`). Four functions per target sit inside RakNet spans with no compiled counterpart.

**Caveats:**
- I copied the `locate` logic from `tools/lib/lua_match.py` into `raknet_match.py` instead of importing it, because that file changed while I was working. The copy fixes a crash in its `None - int` branch, which still exists in `lua_match.py`.
- 7 identical PLAYER rows match only another object's copy of the same template COMDAT; their evidence says which one.
- 12 compiled-out RakNet functions were rejected when more than half their bytes differed; the script prints these as notes.

## Structured fields

```json
{
 "files": [
  "ida/exports/PLAYER-lib-raknet.csv",
  "tools/lib/raknet_match.py",
  "tools/lib/raknet_patches.py",
  "staging/raknet/RakNet-4.034/Source",
  "staging/raknet/patches/0001-upstream-4.036-plugin-lists.patch",
  "staging/raknet/patches/0002-roblox-2013.patch",
  "staging/raknet/patches/0003-line-numbers.patch",
  "3rdparty/_downloads/raknet-4.034-pc/RakNet_PC-4.034.zip",
  "3rdparty/_downloads/raknet-4.067-pc/RakNet_PC-4.067.zip"
 ],
 "csv": "ida/exports/PLAYER-lib-raknet.csv",
 "matched_identical": 1131,
 "located_not_identical": 9,
 "flags": "Network/RakNet: /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_MBCS (/W0 /TP /c), identical to the App set. Identical-function counts in PLAYER: final set 566; /Oy- 108; 2016 Release (/Oy- /arch:SSE2) 106; /arch:SSE2 496; /GS 424; /Ob1 330; no /arch 557; /fp:precise 563. /fp:fast is proven by RakNet::BitStream::ReadFloat16 0x005c3910, WriteFloat16 0x005c39b0 and RakNet::StatisticsToString 0x005d9de0. /arch:SSE over SSE2 is proven by 69 functions, e.g. SystemAddress::operator= 0x005710d0, SocketLayer::CreateBoundSocket_Old 0x005c6fc0, ReliabilityLayer::GetStatistics 0x005c7d00. /Oy, /GS- and /Ob2 are each confirmed by hundreds of functions (RakPeer and ReliabilityLayer bodies). STUDIO matches with the same flags and patches (565/565).",
 "unresolved": [
  "No pristine RakNet 4.035 or 4.036 PC source exists in any fetchable archive (the 4.035/4.037 zips are password-protected console builds; facebookarchive git starts at 4.081). The base is 4.034, and 0001 rebuilds the 4.035/4.036 changes from the bytes, the upstream revision log and the 4.067 text.",
  "Upper version bound is only 'consistent with 4.036': the one 4.037 fix (GetClientPublicKeyFromSystemAddress) is in LIBCAT_SECURITY code that is compiled out, so it leaves no byte trace.",
  "PLAYER 0x005c6cc0 is not identical: RakNet::BitStream::NumberOfLeadingZeroes(int) was placed only by .obj order and is probably the wrong location (the target is a jmp [mem]).",
  "4 unclaimed functions per target inside RakNet spans. PLAYER 0x005f7f30 and 0x005f7f40 (STUDIO 0x0068e510 and 0x0068e520) are probably RakPeerInterface's inline destructor and its scalar deleting destructor, emitted from another object (they store vftable 0x00b27d54). PLAYER 0x005c7690 and STUDIO 0x0066b9f0 (141 bytes, in the RakString span) are not identified. PLAYER 0x005c9170 and STUDIO 0x0069eeb0 (13 and 14 bytes) are probably funclet-like fragments.",
  "RakMemoryOverride.cpp has __FILE__ refs in PLAYER, but none of its functions were located (its TU was not detected as linked).",
  "Where the FFlag MarkRakNetSocketOnDelete and the RakNetDisconnect log channel are defined in 2013 is not settled. Their initializers are at 0x00a63c50 and 0x00a66370, outside RakNet; the patches only declare them extern.",
  "The #line directives in 0003 reproduce PLAYER's __LINE__ values. They do not recover the 2013 files' actual line layout.",
  "tools/lib/lua_match.py (another agent's file) still has the None - int crash in locate(); raknet_match.py uses its own fixed copy."
 ]
}
```


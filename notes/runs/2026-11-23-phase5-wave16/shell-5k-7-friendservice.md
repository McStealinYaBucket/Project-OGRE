# phase5-wave16 shell-5k-7 (item 1): FriendService Creator stand-in in the shell

**Landed (uncommitted): `Client/Win/placeholder_creators.cpp`.** It is a new shell TU with no marker, compiled by both shells. CMakeLists.txt does not list it yet, so I linked it privately. **Integrator:** add `Client/Win/placeholder_creators.cpp` to `OGRE_CLIENT_COMMON_SOURCES`, after `Client/Win/placeholder.cpp`. After that, network-5f-7 can include `V8DataModel/FriendService.h` in `Network/Players.cpp`.

## What the stand-in does

- **First include.** The file's first Object.h include is per target:
  - PLAYER: `#include "util/object.h"`;
  - STUDIO: `#include "Util/Object.h"`.
- **Instantiation.** Its only code is `RBX::FriendService::className()`, plus a `/include:` so the function survives the link. This instantiates FactoryProduct<FriendService>'s Creator in the /GL shell.
- **Why the spelling differs per target:**
  - **PLAYER.** The 2013 Creator copies all sit in one shell TU (0x0041e950..0x00428000):
    - create 0x0041eb70, getClassName 0x004200e0, ctor 0x00422ee0, callDoDeclare 0x004273f0, doDeclare 0x00427400 and create<FriendService> 0x00421820;
    - they assert with `...\Client\App\include\util/object.h` (string 0x00bda690);
    - 152 of that string's 187 PLAYER references sit in the shell.
  - **STUDIO.** The copies are 0x00474490 / 0x00479490, also in the shell, but they spell `Util/Object.h` (0x00d0a468). STUDIO's `util/object.h` string has no shell reference at all.

## PLAYER, measured

Four links of identical objects (`build-sh/w16/{A2,B2,C2,D2}`). Each ran the wave-11 protocol: two runs, regenerate, two runs, regenerate, then the final `-r` run. The final runs are in `build-sh/w16/runs/*-r.json`.

| Link | What it adds |
|---|---|
| A2 | the current tree |
| B2 | A2 + the stand-in |
| C2 | B2 + a private copy of Players.cpp |
| D2 | A2 + the same Players.cpp copy, no stand-in |

The Players.cpp copy (`build-sh/w16/Players_fs.cpp`) includes `V8DataModel/FriendService.h`. Its onChildRemoving calls `ServiceProvider::find<FriendService>(this)->playerRemoving(0)` so that the Creator is really instantiated there. The line count is kept, so the asserts stay on lines 182 and 189.

Match % of each 2013 row in the final run ("–" means not paired):

| 2013 | A2 | B2 | C2 | D2 |
|---|---|---|---|---|
| Creator::create 0x0041eb70 | 80.0 | **100** | **100** | 77.4 |
| Creator::getClassName 0x004200e0 | 100 | 100 | 100 | 92.6 |
| Creatable::create<FriendService> 0x00421820 | – | **100** | **100** | – |
| Creator ctor 0x00422ee0 | 99.2 | **100** | **100** | 92.9 |
| callDoDeclare 0x004273f0 | 100 | 100 | 100 | 100 |
| doDeclare<sFriendService> 0x00427400 | 46.2 | **100** | **100** | 46.2 |
| className/getClassName fold 0x006abba0 | 100 | 100 | 100 | 100 |
| sp_counted_impl_pd<FriendService*> vftable 0x00b11db0 | 20.0 | **100** | **100** | 20.0 |

- **Totals:** A2 12,576; B2 12,574; C2 12,542; D2 (the problem reproduced) loses all the Creator rows.
- **A2 → B2: +4 / −6.**
  - **Gains:** create 0x0041eb70, create<FriendService> 0x00421820, doDeclare 0x00427400, the sp_counted_impl_pd vftable.
  - **Losses, each read with `--verbose` against B2's regenerated CSVs (`csv-B2`):** all six are Network packet-cache twins:
    - create<InstancePacketCache> 0x005b21a0 and create<ClusterPacketCache> 0x005b2220 call the twin ctor (InstancePacketCache's ctor where ClusterPacketCache's is expected);
    - the ClusterPacketCache ctor 0x005d9a10 is unpaired;
    - three PacketCache Debugable vftables follow the twin.
  - No FriendService or shell row is lost.
  - A2 and B2 link identical engine objects; the stand-in adds only shell code. So these six are the twin-pairing churn CLAUDE.md describes (the identical packet-cache ctors pair with a different twin after a regeneration), not the stand-in.
- **B2 → C2: 0 FriendService losses.** The 32 losses are all caused by the private copy's path. Its markers do not pair (it is not `Client/Network/Players.cpp`), so every Players function and its callers drop. None of this applies once Players.cpp itself carries the include.
- **A2 → D2 (no stand-in):** create, getClassName, ctor, create<FriendService>, doDeclare and the vftable are lost or never paired. Players.obj's `Util/Object.h` copy wins the COMDATs. This is the problem item 1 had to fix.
- **Before the earlier links (A/B):** another agent made FriendService's ctor out of line (`FriendService.h`/`.cpp`, 04:29). Before that, the header's inline `FriendService() {}` made the stand-in pull the Described ctor and the signal ctors into the shell (−5 absent in the first B). With the current header that problem is gone (B2).

## Still open after the stand-in

**`FactoryProduct<FriendService>::static_getCreator` 0x0052c200 (90.5 in A2, B2 and C2; 100 only in D2).**
- 2013's copy is native: Network callers 0x0052f3da and 0x0053970a, plus 0x006abba0. It spells `Util/Object.h`.
- Our shell emits its own out-of-line copy, because the link uses `/OPT:NOREF` and LTCG keeps every inline body, the same cause as SoundService's className in wave 15. That copy wins the COMDAT.
- Before this change DataModel's `util/object.h` copy won it, so the row was already 90.5. Nothing is lost.

## STUDIO, measured

Links SA (the current tree) and SB (+ stand-in, `Util/Object.h`), in `build-sh/w16/SA` and `SB`, with the same protocol. Final runs: SA 11,867, SB 11,858.

**SA → SB: +4 / −13.**

| 2013 | SA | SB |
|---|---|---|
| Creator::create 0x00474490 | 80.0 | **100** |
| Creatable::create<FriendService> 0x00478250 | – | **100** |
| Creator ctor 0x00479490 | 99.2 | **100** |
| doDeclare 0x0047e9e0 | 46.2 | **100** |
| sp_counted_impl_pd<FriendService*> vftable 0x00bc9b7c | 20.0 | **100** |

Creator::getClassName 0x004767a0, static_getCreator 0x005bacb0 and the className fold 0x0080ac20 stay at 100.

**The 13 losses are no-.reloc coincidences of the new layout.** None is a FriendService or shell row:
- the Lua 0x7fc000 mask set (symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns, luaK_setoneret, luaK_nil, luaK_posfix);
- Math::isDenormal and isNanInfDenormVector3, where the original's `and eax,0x7fffff` reads as an address;
- RakNet IsOlderOrderedPacket, where the original's immediate reads as Snap's vftable;
- httpGetPostWinHttp.

The `--verbose` reads are in `vbr.sh` against `csv-SB`.

## For network-5f-7

Once the stand-in is linked:
- Players.cpp can `#include "V8DataModel/FriendService.h"`, spelled that way.
- The header comment at the top of Players.cpp that explains why it is left out can go.

The Creator copies stay the shell's in both targets.

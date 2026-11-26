# Wave 16: App headers for network-5f-7 (app-16, item 1)

The changes are in the working tree (Client/App only, not committed). NetworkSettings.cpp compiles in build-ap
(PLAYER, 2026-11-26 03:5x).

## NetworkSettings: the two MTU adjusts (`V8DataModel/NetworkSettings.h`)

| Offset | Member (private) | Ctor value | Accessor (public, inline, const) | 2013 evidence |
|---|---|---|---|---|
| +0xfc | `int physicsMtuAdjust` | -200 | `int getPhysicsMtuAdjust() const` | `cmp dword [eax+0xfc],0`; assert `settings().getPhysicsMtuAdjust() <= 0`, Replicator.cpp line 3424, in getPhysicsMtuSize 0x00581ca0 |
| +0x100 | `int replicationMtuAdjust` | -200 | `int getReplicationMtuAdjust() const` | `cmp dword [eax+0x100],0`; assert `settings().getReplicationMtuAdjust() <= 0`, Replicator.cpp line 3418, in getAdjustedMtuSize 0x00581c10 |

- **Names:** 2016 names. The assert strings name the accessors.
- **Members:** the old `m_unk0xfc` and `m_unk0x100` are renamed, and the NetworkSettings ctor initializers are renamed to match. The offsets do not change.
- **2013 body shape:** both functions re-read `settings()` (`[this+0x224c]`) after the assert. They then add the Replicator member `+0x1f20` and clamp the result at 0 with a two-slot `std::max`-style select (`lea eax,[esp]` / `lea eax,[esp+4]`).
- **Network side:** write the assert as `RBXASSERT(settings().getReplicationMtuAdjust() <= 0)` on line 3418, and the physics one on line 3424.

## Not mine (Client/Network): size annotations

- `Replicator::ReplicationData` and `PacketReceiveJob` are declared in `Client/Network/Replicator.h` and `Client/Network/Peer.h` / its .cpp, so the network agent owns them. I did not edit them.
- From network-5f.md: ReplicationData is part of the 0x34 ClusterReplicationData (ReplicationData, then a 0x18 ClusterUpdateBuffer), which puts ReplicationData at 0x1c. Confirm that against its ctor before adding `// SIZE 0x1c`.

## Measured at the end of the wave

- **Both targets:** with the network agent's working tree, `getAdjustedMtuSize` and `getPhysicsMtuSize` are at 100%.
  - PLAYER: 0x00581c10 and 0x00581ca0 (run build-ap/w/runs/Q4a.json).
  - STUDIO: 0x00623bc0 and 0x00623c50 (run R2a.json).

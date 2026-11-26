# network-5f-8: Players members for FriendService (early note for app-17)

The Players members that FriendService's replication handlers call are now declared in
`Client/Network/include/Network/Players.h` (public), with the iOS 10-29 names and signatures:

| PLAYER | STUDIO | Declaration |
|---|---|---|
| 0x00566790 | 0x005c9fb0 | `void Players::friendEventFired(int userId, int otherUserId, FriendService::FriendEventType friendEvent);` |
| 0x0055f070 | 0x005bf850 | `void Players::friendStatusChanged(int userId, int otherUserId, FriendService::FriendStatus friendStatus);` |
| 0x0055dc60 | 0x005bdce0 | `shared_ptr<Player> Players::getPlayerByID(int userId);` (helper both of them call) |

Also:

- `Network/Players.h` now includes `V8DataModel/FriendService.h` (through `Network/Player.h`), so a
  FriendService.cpp that includes `Network/Players.h` sees both enums. Include it after FriendService.h or
  rely on the include guard; nothing in FriendService.h includes a Network header.
- `Players::friendRequestEventSignal` (0x1c0) is now `signal<void(shared_ptr<Instance>, shared_ptr<Instance>,
  FriendService::FriendEventType)>` and `Player::friendStatusChangedSignal` (0x190) is
  `signal<void(shared_ptr<Instance>, FriendService::FriendStatus)>` (iOS types; were `int`).
- `Players::m_unk0x194` is renamed `players` (copy_on_write_ptr<Instances>; 2016 name, getPlayerByID walks it).
- `Player::onFriendStatusChanged(shared_ptr<Instance>, FriendService::FriendStatus)` is declared (PLAYER
  0x0053eab0, STUDIO 0x005ed850).

The call shapes the FriendService handlers need are only the two declarations above:

- 0x006b14e0 friendEventReplicationChanged: `if (Network::Players* p = ServiceProvider::find<Network::Players>(this))
  p->friendEventFired(...)` (per app-16's read).
- 0x006b2440 friendStatusReplicationChanged: calls `friendStatusChanged` (status remapped 3<->4, per app-16).

2013 bytes of the two handlers, for app-17:

- friendEventReplicationChanged PLAYER 0x006b14e0 / STUDIO 0x008101e0 is 23 bytes: `push ecx; call
  find<Network::Players>` (PLAYER 0x00547df0, cdecl on `this`), then a **tail jump** to friendEventFired with
  the three arguments untouched. So `if (Network::Players* players = ServiceProvider::find<Network::Players>(this))
  players->friendEventFired(playerId, otherPlayerId, eventType);`.
- Measured (reccmp, effective 100% in both targets): friendEventFired PLAYER 0x00566790 / STUDIO 0x005c9fb0,
  friendStatusChanged 0x0055f070 / 0x005bf850, getPlayerByID 0x0055dc60 / 0x005bdce0 and
  Player::onFriendStatusChanged 0x0053eab0 / 0x005ed850 are all `FUNCTION`. In those runs app-17's
  friendEventReplicationChanged (0x006b14e0 / 0x008101e0) and STUDIO friendStatusReplicationChanged 0x00811140
  were already at 100. Details in network-5f-8.md.

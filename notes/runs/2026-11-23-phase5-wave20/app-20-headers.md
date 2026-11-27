# Wave 20 app-20: headers for network-5f-11 and shell-5k-11

Landed in the working tree (uncommitted), 2026-11-26 21:25. ScriptContext.cpp, Game.cpp and UserController.cpp
compile in build-ap. Update 23:00: network-5f-11 had already deleted its copies, and both targets link and score
with these headers (app-20.md).

## For network-5f-11: the two FLog channels now live in App's ScriptContext.cpp

`Client/App/script/ScriptContext.cpp`, lines 79-87, right after NetworkCache's block (the file's line count
did not change, so its asserts kept their lines):

```cpp
namespace FLog {	// then ReplicationDataLifetime's and NetworkInstances's, then these two: 0x00a66150 and 0x00a66170
	// GLOBAL: PLAYER 0x00de3f85
	// GLOBAL: STUDIO 0x00f84e5e
	Channel NetworkReadItem = 0;
	int NetworkReadItemIniter = RegisterLogGroup("NetworkReadItem", &NetworkReadItem, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00d30f1e
	// GLOBAL: STUDIO 0x00ec180a
	Channel JoinSendExtraItemCount = 1;
	int JoinSendExtraItemCountIniter = RegisterLogGroup("JoinSendExtraItemCount", &JoinSendExtraItemCount, NULL, FASTVARTYPE_STATIC); }
```

**What you delete:**

- `Client/Network/Replicator.cpp` 2711-2717: the comment and the `namespace FLog { ... NetworkReadItem ... }` block,
  GLOBAL markers included.
- `Client/Network/ServerReplicator.cpp` 7-13: the comment and the `namespace FLog { ... JoinSendExtraItemCount ... }`
  block, GLOBAL markers included.

Then declare them with `LOGGROUP(NetworkReadItem)` / `LOGGROUP(JoinSendExtraItemCount)` (FastLog.h), or nothing if a
header you include already does. Keep Replicator.cpp's readItem assert lines (PLAYER 2750 / STUDIO 2769): reuse the
deleted lines as blanks.

The PLAYER initializers in 0x00a66090-0x00a66170 run Network, HttpQueue, Player, NetworkCache,
ReplicationDataLifetime (0x00de3f83), NetworkInstances (0x00de3f84), NetworkReadItem, JoinSendExtraItemCount. The
middle ones are still defined elsewhere in our tree (HttpQueue in util/AsyncHttpQueue.cpp, Player in Network). Moving
them is not part of this change.

## For shell-5k-11

- **`V8DataModel/Game.h`**: public
  `boost::shared_ptr<OverlayDataModel> getOverlayDataModel() const { return overlayDataModel; }`. It is inline and has no
  marker yet. 2013's /GL shell copy is PLAYER 0x0041ce60 / STUDIO 0x004d5770. `OverlayDataModel` is only
  forward-declared in Game.h. To convert the result to `shared_ptr<DataModel>` or a `ServiceProvider*`, include
  `V8DataModel/OverlayDataModel.h` (or the header that defines it) in your TU.
- **`V8DataModel/UserController.h`**: ControllerService +0x64 is now `UserInputBase* hardwareDevice` (it was
  `m_unk0x64`). Public `void setHardwareDevice(UserInputBase* value) { hardwareDevice = value; }` is inline.
  `UserInputBase` is forward-declared in `namespace RBX`.
- You can now delete the `#ifdef OGRE_APP_INPUT_ACCESSORS` / `#else` stand-ins in View.cpp and UserInput.cpp. Keep the
  assert lines (View.cpp 739, UserInput.cpp 148), and measure A/B as your shell-5k-10 caution says.

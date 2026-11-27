# Wave 21: headers for the shell and network agents (app-21, item 1)

Landed 2026-11-27 01:35 in the shared tree; measured at 02:20 (PLAYER W21A3, STUDIO W21S1) and re-measured at
02:50 (W21A5 / W21S3). Every function below is at effective 100 in both targets and carries `// FUNCTION:`.

1. **`DFLog::NetworkJoin`** is defined in `Client/App/script/ScriptContext.cpp`, right after
   `FLog::JoinSendExtraItemCount` (explicit block, `FASTVARTYPE_DYNAMIC`, default 0, GLOBAL PLAYER 0x00de3f8b /
   STUDIO 0x00f84e65). ScriptContext.cpp keeps its line count (3,404; asserts unmoved). network-5f-12's
   `DYNAMIC_LOGGROUP(NetworkJoin)` in ServerReplicator.cpp is already the only Network reference.
2. **`V8DataModel/NetworkSettings.h`:** +0xcc is now `PhysicsSendMethod physicsSendMethod` (private; the ctor
   initializes it to `ErrorComputation`). It has a public inline getter, `PhysicsSendMethod getPhysicsSendMethod() const`.
   ServerReplicator::onSentMarker 0x00572270 is at 100 in both targets with it.
3. **`PartInstance::fromPrimitive(Primitive*)` / `fromConstPrimitive(const Primitive*)`** are public statics in
   PartInstance.h, defined at the end of PartInstance.cpp:
   `return p ? static_cast<PartInstance*>(p->getOwner()) : NULL;`.
   - FUNCTION PLAYER 0x0071b970 / STUDIO 0x007b8f90.
   - The const one has no marker, because our link folds it into the non-const one, as 2013's did.
4. **`Mechanism::getConstRootMovingPrimitive(const Primitive*)` and `getRootMovingPrimitive(Primitive*)`** are
   public statics in V8World/Mechanism.h, defined in Mechanism.cpp.
   - FUNCTION: PLAYER 0x007f9f90 / 0x007fa010, STUDIO 0x00915520 / 0x009155a0.
   - The assert sits on 2013 line 88.
5. **`OverlayDataModel::DualLegacyLock`** (V8DataModel/OverlayDataModel.h, SIZE 0xc):
   ```cpp
   class DualLegacyLock {
       DataModel::LegacyLock legacyLock;               // 0x0
       DataModel::scoped_write_request writeRequest;   // 0x4  overlay.get() when taskType == Write
       DataModel::scoped_read_request readRequest;     // 0x8  overlay.get() when taskType == Read (0x006431d0)
   public:
       DualLegacyLock(shared_ptr<OverlayDataModel> overlay, shared_ptr<DataModel> dataModel, DataModelJob::TaskType taskType);
   };
   ```
   - The ctor is in OverlayDataModel.cpp: FUNCTION PLAYER 0x0068c1f0 / STUDIO 0x0084b050.
   - The dtor is implicit, so a shell TU that uses the class gets its own copy. 2013's copy is 0x0041d550: it runs
     ~read 0x0062db70, then ~write 0x0062daf0, then ~LegacyLock.
   - The argument order is (overlay, dataModel, type), as the 2013 stack shows.
6. **`reflection/Event.h`** now carries `// FUNCTION:` header markers:
   - `Event::getInstance`: PLAYER 0x00572d90 / STUDIO 0x00654d90.
   - `EventInvocation::replicateEvent`: 0x00572dc0 / 0x00654dc0.

   Both are at 100 in both targets. The line count is unchanged, and the asserts at 295..413 have not moved.

// PV.cpp — no implementation exists for this module in the October 2013 binary.
//
// Evidence: the only out-of-line symbol for RBX::PV in the symbolled Mach-O Studio
// (F:\2013Decompile\exports\studio_function_list.txt) is
//   mac: 0x555CD2 __ZNK3RBX2PVeqERKS0_  RBX::PV::operator==(RBX::PV const&)const
//   mac: 0x9666B8 j___ZNK3RBX2PVeqERKS0_ (thunk)
// which Hex-Rays decompiles to a comparison of the CoordinateFrame translation floats
// ([9],[10],[11]), G3D::Matrix3::operator==, then RBX::Velocity::operator== — exactly the
// inline bool operator== declared in Util/PV.h (kept inline there; on itanium ABI a key
// function was not emitted so it stays header-only).
//
// The decompilation export file "Util_PV.cpp.txt" was assembled from matches on the
// type name "PVInstance" and contains no RBX::PV members; everything in it belongs to
// other modules (PVInstance, PVAdornment, FilteredSelection<PVInstance>, Mouse,
// RefPropDescriptor<*, PVInstance>, boost shared_ptr instantiations, std::vector<PVInstance*>,
// MegaDragger, Body::setPv, Assembly::setPhysics, Primitive::setPV).
//
// TODO (other modules, not written here):
//  - PVInstance members (0x437434 isTopLevelPVInstance, 0x4686A4 setPVGridOffsetLegacy,
//    mac: 0x4687C8 ctor, 0x468894/0x4688D8/0x46891C moveToPoint*, 0x468960 clearLegacyOffset,
//    mac: 0x4689D0 readProperty, 0x468B10+ dtors, 0x468BBC hasLegacyOffset, 0x468BEA
//    renderCoordinateFrame, 0x468DD0 initOnDemand, 0x468CD6/0x469314 PropDescriptor
//    <PVInstance, CoordinateFrame>, 0x46D4E0 topHashCode, 0x5188F0 getLegacyOffset,
//    mac: 0x519920 childHashCode, 0x687120 getPersistentDataCost) -> v8datamodel/PVInstance
//  - PVAdornment (0x2601FA setAdornee, 0x2602BA ctor, 0x260428 getAdorneeDangerous,
//    mac: 0x26056A+ dtors, 0x260460+ RefPropDescriptor<PVAdornment, PVInstance>) -> PVAdornment module
//  - FilteredSelection<PVInstance> (0x1B93B8.., 0x2BF8FA getSelection, 0x2BFA5A ctor,
//    mac: 0x2BFBD0 onAncestorChanged, 0x2BFD0A onSelectionChanged, 0x2BFEF0 dtor, Service
//    /ServiceProvider/Creatable instantiations 0x1C0178.., 0x2BBEFC..) -> V8DataModel/Selection
//  - Mouse target filter (0x43A52C, 0x43A544, 0x6A6F22) + RefPropDescriptor<Mouse, PVInstance>
//    (0x43AE4C..) -> Mouse module
//  - Reflection templates: RefType<PVInstance*> (0x43C6D0 singleton, 0x43CC20 Type ctor,
//    mac: 0x43CCF0/0x468E64 dtors), Type<PVInstance*> (0x43CC20), PropDescriptor GetSetImpl
//    (0x260DE4.., 0x43CB9A..), SetImpl<void (PVInstance::*)(CoordinateFrame const&)>
//    (0x46944A..) -> reflection headers
//  - Body::setPv (0x524B7E) -> v8kernel/Body; Primitive::setPV (0x591B48) -> v8world/Primitive;
//    Assembly::setPhysics (0x555084) -> v8world/Assembly
//  - MegaDragger ctors (0x1D0B6C/0x1D0B76), DragUtilities::pvsToParts (0x1C753E),
//    RootInstance::moveToPoint (0x46B712), RotateAxisCommand::rotateAboutAxis (0x2BA74C)
//    -> drag/rotate command modules
//  - ManualJointHelper::setPVInstanceToJoin (0x6C07A2) / setPVInstanceTarget (0x6C085E)
//    -> ManualJointHelper module
//  - std::vector<RBX::PVInstance*>::push_back (0x46D3C4), _M_insert_aux (0x6C3938),
//    _M_allocate (0x6C3A5C), std::__find (0x2BFE38) -> STL, not emitted
//  - boost shared_ptr/weak_ptr/dynamic_pointer_cast instantiations over PVInstance
//    (0x1D4932, 0x1D5174, 0x4193E2, 0x419681, 0x95xxxx/0x96xxxx thunks) -> boost, vendored

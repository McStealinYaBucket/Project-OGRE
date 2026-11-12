/* Copyright 2003-2013 ROBLOX Corporation, All Rights Reserved */

// SurfaceType.cpp
//
// This module is intentionally header-only. Util/SurfaceType.h defines only the
// RBX::SurfaceType and RBX::Legacy::SurfaceConstraint enums plus two inline
// helpers (IsNoSurface, IsRotate); the 2013 binary contains no out-of-line
// symbols for them.
//
// All SurfaceType-related code in the binary belongs to other modules:
//
//  - RBX::Reflection::EnumDesc<RBX::SurfaceType> (EnumDesc ctor 0x57CBDE,
//    addPair 0x57CF36, lookup 0x3580B2, convertToString 0x358100/0x57D2B4,
//    convertToValue 0x4B15F8, convertToItem 0x4B1288, convertToIndex 0x4B1376,
//    dtor 0x57D2A0/0x57D2B4/0x358082) - Reflection/EnumConverter.h template +
//    the explicit instantiation driven by SurfaceEnums / EnumRegistrar.
//    The ctor evidence confirms the enum ordering documented in SurfaceType.h:
//    0 Smooth, 1 Glue, 2 Weld, 3 Studs, 4 Inlet, 5 Universal, 6 Hinge,
//    7 Motor, 8 SteppingMotor, 9 Unjoinable, 10 SmoothNoOutlines, with
//    "Spawn" and "Bumps" added to the name alias map.
//
//  - rbx::implementation::typed_holder<RBX::SurfaceType> (0x3581CC,
//    mac: 0x35822C, 0x4B1490) and rbx::placement_any<RBX::Region3>::operator=
//    <RBX::SurfaceType> (0x57D3F0) - Util/any.h / placement_any templates.
//
//  - rbx::any_cast<RBX::SurfaceType, RBX::Region3> (0x4B7A7D) and
//    any_cast<RBX::SurfaceType const&, RBX::Region3> (0x4B1410) -
//    Util/any.h template.
//
//  - RBX::Reflection::Variant::genericConvert<RBX::SurfaceType> (0x4AF762) -
//    Reflection/Variant.h template.
//
//  - RBX::Reflection::Type::getSingleton<RBX::SurfaceType> (0x347460) and
//    Singleton<EnumDesc<SurfaceType> const> (0x358070, 0x4B1544) -
//    Reflection/Type.h + Reflection/Singleton.h templates.
//
//  - RBX::Surface::* (ctor 0x4AF214, setSurfaceType 0x4AF22C,
//    setSurfaceInput 0x4AF252, setParamA 0x4AF278, setParamB 0x4AF2A2,
//    flat 0x4AF2CC, getSurfaceTypeStatic 0x4AF44A,
//    registerSurfaceDescriptors 0x4AF3E4, isSurfaceDescriptor 0x4AF3EA) -
//    v8datamodel/Surface (owned by the Surface module).
//
//  - Tool doAction implementations (FlatTool 0x4E0E0E, GlueTool 0x4E0E3C,
//    WeldTool 0x4E0EF8, StudsTool 0x4E0F56, InletTool 0x4E0FB4,
//    UniversalTool 0x4E1012, HingeTool 0x4E107A, RightMotorTool 0x4E10E2,
//    LeftMotorTool 0x4E114A, OscillateMotorTool 0x4E11B2,
//    SmoothNoOutlinesTool 0x4E107A-range, DecalTool 0x51B87E) -
//    ToolsSurface module.
//
//  - std::vector<RBX::SurfaceType>::* / std::vector<RBX::Legacy::SurfaceConstraint>::*
//    (0x57D87E, 0x57D8D2, 0x57D914, 0x57DA42, 0x57DA68, 0x57DAB6, 0x42A5C4...)
//    - libstdc++ template instantiations, not reconstructed.

#include "Util/SurfaceType.h"

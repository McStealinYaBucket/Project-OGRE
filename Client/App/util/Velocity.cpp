/********************************************************************************
 * Copyright 2013 ROBLOX Corporation, All Rights Reserved
 *
 * Reconstructed from the October 2013 binary (Mach-O studio 0.126.0.39503).
 ********************************************************************************/

// RBX::Velocity is a header-only value type: Velocity.h contains the entire class.
//
// Only three members are emitted out-of-line in the binary, and all three are
// defined inline in Velocity.h (class-body definitions the compiler chose not to
// fold away at every use site):
//   mac: 0x3DFBA0 / 0x961B46  RBX::Velocity::zero()      -- guarded function-local static
//   mac: 0x556EC6 / 0x9666BE  RBX::Velocity::operator==  -- 6 sequential float compares
//   mac: 0x86A36C / 0x96E4CA  RBX::Velocity::lerp        -- component-wise lerp
//
// Therefore this translation unit is intentionally empty.
//
// NOTE: the functions named RBX::VelocityMotor::* in the exported decompilation
// (0x36CEDA..0x3760C8, plus the Reflection::PropDescriptor< VelocityMotor, ... >
// machinery) belong to the VelocityMotor Instance class, not to Util/Velocity.
// They are owned by the VelocityMotor module (reflection registration at
// mac: 0x960586/0x3759FE mirrors the standard PropDescriptor pattern used by every
// Described class). They are NOT reconstructed here.
//
// Likewise, the free functions below touch Velocity but belong to other modules:
//   mac: 0x524CCC RBX::Body::setVelocity(Velocity const&, BodyPvSetter const&)
//   mac: 0x591D58 RBX::Primitive::setVelocity(Velocity const&)
//   mac: 0x877946 RBX::Network::PhysicsReceiver::readVelocity(RakNet::BitStream&, Velocity&)
//   mac: 0x879DA6 RBX::Network::PhysicsSender::writeVelocity(RakNet::BitStream&, Velocity const&)
// They confirm the Velocity memory layout: two G3D::Vector3 (24 bytes),
// linear at +0, rotational at +12.

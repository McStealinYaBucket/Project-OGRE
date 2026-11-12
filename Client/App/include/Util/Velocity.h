// Reconstructed from October 2013 binary (Mach-O studio 0.126.0.39503).
// RBX::Velocity is header-only in the binary; only three members appear out-of-line:
//   mac: 0x3DFBA0 (thunk 0x961B46) RBX::Velocity::zero()  -- __cxa_guard + function-local static, 24 bytes
//   mac: 0x556EC6 (thunk 0x9666BE) RBX::Velocity::operator== -- sequential float compare of all 6 components
//   mac: 0x86A36C (thunk 0x96E4CA) RBX::Velocity::lerp -- component-wise lerp of both Vector3s
// All other members (ctors, arithmetic ops, rotateBy, toObjectSpace/toWorldSpace,
// linearVelocityAtOffset, velocityAtOffset) are fully inlined at use sites
// (e.g. Body::setVelocity 0x524CCC, Primitive::setVelocity 0x591D58), consistent
// with a header-only class. Structure matches the March 2016 reference source.

#pragma once

#include "Util/G3DCore.h"

namespace RBX {

	class Velocity
	{
	public:
		Vector3				linear;
		Vector3				rotational;

		inline Velocity() : 
			linear(Vector3::zero()), rotational(Vector3::zero()) {}

		Velocity(const Vector3& _linear) :
			linear(_linear), rotational(Vector3::zero()) {}

		Velocity(const Vector3& _linear, const Vector3& _rotational) :
			linear(_linear), rotational(_rotational) {}

		Velocity(const Velocity &other) :
			linear(other.linear), rotational(other.rotational) {}

		inline Velocity operator+ (const Velocity& rhs) const {
		    return Velocity(linear + rhs.linear, rotational + rhs.rotational);
		}

		inline Velocity operator- (const Velocity& rhs) const {
		    return Velocity(linear - rhs.linear, rotational - rhs.rotational);
		}

		inline Velocity operator*(float f) const {
			return Velocity(linear * f, rotational * f);
		}

		inline Velocity operator- () const {
			return Velocity(-linear, -rotational);
		}

		Velocity rotateBy(const Matrix3& m) const {
			return Velocity(m * linear, m * rotational);
		}

		static Velocity toObjectSpace(const Velocity& vWorld, const CoordinateFrame& c) {
			return vWorld.rotateBy(c.rotation.transpose());
	    }

		static Velocity toWorldSpace(const Velocity& vInObject, const CoordinateFrame& c) {
			return vInObject.rotateBy(c.rotation);
		}

		Vector3 linearVelocityAtOffset(const Vector3& offset) const {
			return linear + rotational.cross(offset);
		}

		Velocity velocityAtOffset(const Vector3& offset) const {
			return Velocity(linearVelocityAtOffset(offset), rotational);
		}

		// mac: 0x3DFBA0 (thunk 0x961B46) static const Velocity& zero()
		// Binary: guarded function-local static, 24-byte object = two Vector3::zero()
		// FUNCTION: PLAYER 0x005ea950
		// FUNCTION: STUDIO 0x00682a50
		static const Velocity& zero() {
			static Velocity v; return v;
		}

		// mac: 0x86A36C (thunk 0x96E4CA) RBX::Velocity::lerp(RBX::Velocity const&, float)const
		Velocity lerp(const Velocity& other, float alpha) const {
			return Velocity(	linear.lerp(other.linear, alpha),
								rotational.lerp(other.rotational, alpha)	);
		}

		// mac: 0x556EC6 (thunk 0x9666BE) RBX::Velocity::operator==(RBX::Velocity const&)const
		// Binary compares all 6 floats sequentially (see also PV::operator== 0x555CD2)
		bool operator==(const Velocity& other) const {
			return linear == other.linear && rotational == other.rotational;
		}

		bool operator!=(const Velocity& other) const {
			return !(*this == other);
		}
	};


} // namespace RBX

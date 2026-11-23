/* Util/Vector3int32.h -- October 2013 reconstruction.
   Integer 3-vector (x, y, z as int, 12 bytes) for cell and grid coordinates.
   2013 evidence: PLAYER keeps Vector3int32::floor(G3D::Vector3 const&) out of
   line at 0x0066d6e0 (three Math::iFloor calls); iOS 10-29 uses the type in
   LightGrid and SpatialHash signatures. All other members are inline and
   keep the arithmetic callers compile (including the int-to-float result of
   squaredMagnitude and the function-local statics named v, whose decorated
   names the reccmp data can carry). */
#pragma once

#include "G3D/Vector3.h"
#include "G3D/Vector3int16.h"
#include "Util/Math.h"
#include "rbx/Debug.h"
#include <limits.h>

namespace RBX {

class Vector3int32
{
public:
	int x;
	int y;
	int z;

	Vector3int32() : x(0), y(0), z(0) {}
	Vector3int32(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
	explicit Vector3int32(const Vector3int16& v) : x(v.x), y(v.y), z(v.z) {}

	static Vector3int32 floor(const G3D::Vector3& v)
	{
		return Vector3int32(Math::iFloor(v.x), Math::iFloor(v.y), Math::iFloor(v.z));
	}

	// shared constants
	inline static const Vector3int32& zero()   { static Vector3int32 v(0, 0, 0); return v; }
	inline static const Vector3int32& one()    { static Vector3int32 v(1, 1, 1); return v; }
	inline static const Vector3int32& maxInt() { static Vector3int32 v(INT_MAX, INT_MAX, INT_MAX); return v; }
	inline static const Vector3int32& minInt() { static Vector3int32 v(INT_MIN, INT_MIN, INT_MIN); return v; }

	const int& operator[](int i) const { return ((int*)this)[i]; }
	int& operator[](int i) { return ((int*)this)[i]; }

	// component-wise arithmetic
	Vector3int32 operator-() const { return Vector3int32(-x, -y, -z); }
	Vector3int32 operator+(const Vector3int32& o) const { return Vector3int32(x + o.x, y + o.y, z + o.z); }
	Vector3int32 operator-(const Vector3int32& o) const { return Vector3int32(x - o.x, y - o.y, z - o.z); }
	Vector3int32 operator*(const int k) const { return Vector3int32(x * k, y * k, z * k); }
	Vector3int32 operator*(const Vector3int16& o) const { return Vector3int32(x * o.x, y * o.y, z * o.z); }
	Vector3int32 operator*(const Vector3int32& o) const { return Vector3int32(x * o.x, y * o.y, z * o.z); }
	Vector3int32 operator&(const Vector3int32& o) const { return Vector3int32(x & o.x, y & o.y, z & o.z); }
	Vector3int32 operator%(const Vector3int32& o) const { return Vector3int32(x % o.x, y % o.y, z % o.z); }

	// shifts: per-axis amounts, or one amount for all three
	Vector3int32 operator>>(const Vector3int32& s) const { return Vector3int32(x >> s.x, y >> s.y, z >> s.z); }
	Vector3int32 operator>>(const Vector3int16& s) const { return Vector3int32(x >> s.x, y >> s.y, z >> s.z); }
	Vector3int32 operator>>(unsigned int s) const
	{
		RBXASSERT_SLOW(s < 32);
		return Vector3int32(x >> s, y >> s, z >> s);
	}
	Vector3int32 operator<<(const Vector3int32& s) const { return Vector3int32(x << s.x, y << s.y, z << s.z); }
	Vector3int32 operator<<(const Vector3int16& s) const { return Vector3int32(x << s.x, y << s.y, z << s.z); }
	Vector3int32 operator<<(unsigned int s) const
	{
		RBXASSERT_SLOW(s < 32);
		return Vector3int32(x << s, y << s, z << s);
	}

	void shiftRight(int s)
	{
		RBXASSERT_SLOW(s >= 0);
		RBXASSERT_SLOW(s < 32);
		x >>= s;
		y >>= s;
		z >>= s;
	}

	bool operator==(const Vector3int32& o) const { return x == o.x && y == o.y && z == o.z; }
	bool operator!=(const Vector3int32& o) const { return x != o.x || y != o.y || z != o.z; }

	// lexicographic: x, then y, then z
	bool operator<(const Vector3int32& o) const
	{
		return x < o.x || (x == o.x && y < o.y) || (x == o.x && y == o.y && z < o.z);
	}

	inline float squaredMagnitude() const { return x*x + y*y + z*z; }
	int sum() const { return x + y + z; }

	Vector3int32 min(const Vector3int32& o) const
	{
		return Vector3int32(std::min(o.x, x), std::min(o.y, y), std::min(o.z, z));
	}
	Vector3int32 max(const Vector3int32& o) const
	{
		return Vector3int32(std::max(o.x, x), std::max(o.y, y), std::max(o.z, z));
	}

	G3D::Vector3 toVector3() const
	{
		return Vector3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	}

	// truncating narrow to 16 bits per axis
	G3D::Vector3int16 toVector3int16() const
	{
		return Vector3int16(static_cast<G3D::int16>(x), static_cast<G3D::int16>(y), static_cast<G3D::int16>(z));
	}
};

std::ostream& operator<<(std::ostream& os, const Vector3int32& v);
::std::size_t hash_value(const RBX::Vector3int32& v);

inline RBX::Vector3int32 fastFloorInt32(const RBX::Vector3& v)
{
	return RBX::Vector3int32(fastFloorInt(v.x), fastFloorInt(v.y), fastFloorInt(v.z));
}

} // namespace RBX

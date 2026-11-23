/* Util/Quaternion.h -- October 2013 reconstruction.
   Rotation quaternion stored x, y, z (imaginary part) then w, so the first
   twelve bytes read as a G3D::Vector3 (imag()). Out of line, per iOS 10-29:
   Quaternion(G3D::Matrix3 const&), operator=(Quaternion const&) and
   toRotationMatrix(G3D::Matrix3&) const. Everything else is inline; the
   bodies keep the arithmetic as the serializer and physics callers compile
   it (note getAxis compares against a double literal, an x87 compare under
   /arch:SSE). The += and *= bodies are in Quaternion.inl. */
#pragma once

#include "Util/G3DCore.h"

namespace RBX {

class Quaternion
{
public:
	float x, y, z, w;

	Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {} // identity
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Quaternion(const G3D::Vector3& v, float _w = 0) : x((float)v.x), y((float)v.y), z((float)v.z), w(_w) {}
	Quaternion(const G3D::Matrix3& rot);

	Quaternion& operator=(const Quaternion& other);

	void toRotationMatrix(Matrix3& rot) const;

	// (x, y, z) viewed in place as a vector
	const G3D::Vector3& imag() const { return *reinterpret_cast<const G3D::Vector3*>(this); }
	G3D::Vector3& imag() { return *reinterpret_cast<G3D::Vector3*>(this); }

	// raw component access, index 0..3 = x, y, z, w
	float& operator[](int i) const { return ((float*)this)[i]; }
	operator float*() { return (float*)this; }
	operator const float*() const { return (float*)this; }

	float dot(const Quaternion& q) const { return (x * q.x) + (y * q.y) + (z * q.z) + (w * q.w); }

	// squared length (not its root)
	float magnitude() const { return x*x + y*y + z*z + w*w; }

	float maxComponent() const
	{
		return std::max(std::max(fabs(x), fabs(y)), std::max(fabs(z), fabs(w)));
	}

	// axis-angle view
	float getAngle() const { return 2.0f * acos(w); }

	Vector3 getAxis() const
	{
		float s2 = 1.f - w * w;
		if (s2 < 1e-6)
			return Vector3(1.0, 0.0, 0.0); // no rotation: any axis will do
		float inv = 1.f / sqrtf(s2);
		return Vector3(x * inv, y * inv, z * inv);
	}

	Quaternion conjugate() const { return Quaternion(-x, -y, -z, w); }

	// Hamilton product in vector form: (s1 v2 + s2 v1 + v1 x v2, s1 s2 - v1 . v2)
	Quaternion operator*(const Quaternion& q) const
	{
		const Vector3& v1 = imag();
		const Vector3& v2 = q.imag();
		float s1 = w;
		float s2 = q.w;
		return Quaternion(s1*v2 + s2*v1 + v1.cross(v2), s1*s2 - v1.dot(v2));
	}

	Quaternion operator+(const Quaternion& q) const { return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w); }
	Quaternion operator-(const Quaternion& q) const { return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w); }
	Quaternion operator*(float s) const { return Quaternion(s*x, s*y, s*z, s*w); }

	Quaternion& operator*=(float s);
	Quaternion& operator+=(const Quaternion& q);

	void normalize() { *this *= 1.0f / sqrtf(magnitude()); }
};

} // namespace RBX

#include "Quaternion.inl"

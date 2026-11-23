/* Util/Quaternion.inl -- October 2013 reconstruction.
   The two compound operators Quaternion.h declares without a body.
   Component order x, y, z, w (Quaternion's member order). */

namespace RBX {

inline Quaternion& Quaternion::operator+=(const Quaternion& q)
{
	x += q.x; y += q.y; z += q.z; w += q.w;
	return *this;
}

inline Quaternion& Quaternion::operator*=(float k)
{
	x *= k; y *= k; z *= k; w *= k;
	return *this;
}

} // namespace RBX

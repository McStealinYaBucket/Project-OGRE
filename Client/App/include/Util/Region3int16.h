/* Util/Region3int16.h -- October 2013 reconstruction.
   An inclusive box of 16-bit integer cell coordinates (terrain regions).
   iOS 10-29 names the default and (min, max) constructors, getMinPos and
   getMaxPos (both const); members are the two Vector3int16 corners, min
   first. contains() tests each axis with one unsigned compare. */
#pragma once

#include "G3D/Vector3int16.h"
#include "G3DCore.h"

namespace RBX {

class Region3int16
{
	Vector3int16 lo;
	Vector3int16 hi;

public:
	Region3int16() {}
	Region3int16(const Vector3int16& min, const Vector3int16& max) : lo(min), hi(max) {}

	const Vector3int16& getMinPos() const { return lo; }
	const Vector3int16& getMaxPos() const { return hi; }

	bool operator==(const Region3int16& rhs) const { return lo == rhs.lo && hi == rhs.hi; }
	bool operator!=(const Region3int16& rhs) const { return !operator==(rhs); }

	// p - min <= max - min, unsigned, per axis: below min wraps to a huge value
	bool contains(const Vector3int16& p) const
	{
		typedef unsigned int U;
		return U(p.x - lo.x) <= U(hi.x - lo.x)
			&& U(p.y - lo.y) <= U(hi.y - lo.y)
			&& U(p.z - lo.z) <= U(hi.z - lo.z);
	}

	bool empty() const { return lo.x > hi.x || lo.y > hi.y || lo.z > hi.z; }
};

} // namespace RBX

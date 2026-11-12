#pragma once

#include "Util/Vector3int32.h"
#include "Util/Extents.h"
#include "rbx/Debug.h"

namespace RBX {

	// Reconstructed from Oct 2013 Studio binary (0.126.0.39503) + 2016 reference header.
	// Functions observed out-of-line in the binary are defined in Util/ExtentsInt32.cpp:
	//   ctor (0x56EFA4), contains (0x569688), overlapsOrTouches (0x56AC76), empty (0x56ACB8)
	class ExtentsInt32 {
	public:
		Vector3int32	low;
		Vector3int32	high;

		ExtentsInt32(); // 0x56EFA4: low = maxInt(), high = minInt()

		ExtentsInt32(const Vector3int32& _min, const Vector3int32& _max)
			: low(_min)
			, high(_max)
		{
			RBXASSERT_SLOW(low == low.min(high));
			RBXASSERT_SLOW(high == high.max(low));
		}

		bool operator==(const ExtentsInt32& other) const {
			return ((low == other.low) && (high == other.high));
		}

		bool operator!=(const ExtentsInt32& other) const {
			return !(*this == other);
		}

		ExtentsInt32 shiftRight(int shift) const {
			RBXASSERT_SLOW(shift >= 0);
			RBXASSERT_SLOW(shift <= 32);
			return ExtentsInt32(low >> shift, high >> shift);
		}

		ExtentsInt32 shiftRight(const Vector3int32& shift) const {
			return ExtentsInt32(low >> shift, high >> shift);
		}

		ExtentsInt32 shiftLeft(int shift) const {
			RBXASSERT_SLOW(shift >= 0);
			RBXASSERT_SLOW(shift <= 32);
			return ExtentsInt32(low << shift, high << shift);
		}

		ExtentsInt32 shiftLeft(const Vector3int32& shift) const {
			return ExtentsInt32(low << shift, high << shift);
		}

		static ExtentsInt32 vv(const Vector3int32& v0, const Vector3int32& v1) {
			ExtentsInt32 e;
			e.low = v0.min(v1);
			e.high = v0.max(v1);
			return e;
		}

		const Vector3int32& min() const					{return low;}
		const Vector3int32& max() const					{return high;}

		Vector3int32 getCorner(int i) const; // not in binary export list; declared per 2016 source

		Vector3int32 size() const {
			return high - low;
		}

		Vector3int32 center() const {
			return ((low + high) >> 1);
		}

		Vector3int32 bottomCenter() const {
			Vector3int32 answer = center();
			answer.y = low.y;
			return answer;
		}

		Vector3int32 topCenter() const {
			Vector3int32 answer = center();
			answer.y = high.y;
			return answer;
		}

		int longestSide() const {
			Vector3int32 s = size();
			return std::max(std::max(s.x, s.y), s.z);
		}

		int volume() const {
			Vector3int32 s = size();
			long long int answer = s.x * s.y * s.z;
			RBXASSERT(answer < INT_MAX);
			return static_cast<int>(answer);
		}

		static ExtentsInt32 unionExtents(const ExtentsInt32& a, const ExtentsInt32& b) {
			return ExtentsInt32(a.low.min(b.low), a.high.max(b.high));
		}

		void shift(const Vector3int32& shiftVector) {
			low = low + shiftVector;
			high = high + shiftVector;
		}

		void expand(int x) {
			low = low - Vector3int32(x,x,x);
			high = high + Vector3int32(x,x,x);
		}

		const Vector3int32& operator[] (int i) const {
			return ((Vector3int32*)this)[i];
		}

		Vector3int32& operator[] (int i) {
			return ((Vector3int32*)this)[i];
		}

		operator Vector3int32* () {
			return (Vector3int32*)this;
		}
		operator const Vector3int32* () const {
			return (Vector3int32*)this;
		}

		bool contains(int x, int y, int z) const {
			return (	(x >= low.x)
					&&	(y >= low.y)
					&&	(z >= low.z)
					&&	(x <= high.x)
					&&	(y <= high.y)
					&&	(z <= high.z)	);
		}

		bool contains(const Vector3int32& point) const; // 0x569688

		bool overlapsOrTouches(const ExtentsInt32& other) const; // 0x56AC76: true if sides touch

		static bool overlapsOrTouches(const ExtentsInt32& e0, const ExtentsInt32& e1) {return e0.overlapsOrTouches(e1);}

		Extents toExtents() const {
			return Extents(low.toVector3(), high.toVector3());
		}

		static const ExtentsInt32& zero() {
			static ExtentsInt32 e(Vector3int32::zero(), Vector3int32::zero());
			return e;
		}

		static const ExtentsInt32& empty(); // 0x56ACB8

	};
} // namespace

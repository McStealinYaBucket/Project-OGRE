// ExtentsInt32.cpp — reconstructed Oct 2013 Roblox source.
// Only the functions that the Oct 2013 Studio binary emitted out-of-line live here
// (everything else is inline in Util/ExtentsInt32.h).
// Evidence: Mach-O itanium symbols in october2013symbolledrobloxstudio.i64.

#include "Util/ExtentsInt32.h"

namespace RBX {

// mac: 0x56EFA4 __ZN3RBX12ExtentsInt32C1Ev
// Binary: low = Vector3int32::maxInt() (0x7FFFFFFF x3), high = Vector3int32::minInt() (0x80000000 x3)
// FUNCTION: PLAYER 0x004e5df0
// FUNCTION: STUDIO 0x0056dac0
ExtentsInt32::ExtentsInt32()
	: low(Vector3int32::maxInt())
	, high(Vector3int32::minInt())
{}

// mac: 0x569688 __ZNK3RBX12ExtentsInt328containsERKNS_12Vector3int32E
bool ExtentsInt32::contains(const Vector3int32& point) const {
	return (	(point.x >= low.x)
			&&	(point.y >= low.y)
			&&	(point.z >= low.z)
			&&	(point.x <= high.x)
			&&	(point.y <= high.y)
			&&	(point.z <= high.z)	);
}

// mac: 0x56AC76 __ZNK3RBX12ExtentsInt3217overlapsOrTouchesERKS0_
// true if sides are exactly equal (touching)
bool ExtentsInt32::overlapsOrTouches(const ExtentsInt32& other) const {
	return (	(this->low.x > other.high.x)
			||	(this->low.y > other.high.y)
			||	(this->low.z > other.high.z)
			||	(this->high.y < other.low.y)
			||	(this->high.x < other.low.x)
			||	(this->high.z < other.low.z)) ? false : true;
}

// mac: 0x56ACB8 __ZN3RBX12ExtentsInt325emptyEv
// Binary shows an itanium __cxa_guard-protected function-local static -> plain static local under MSVC.
const ExtentsInt32& ExtentsInt32::empty() {
	static ExtentsInt32 e;	// constructor sets low = maxInt, high = minInt
	return e;
}

// TODO: RBX::SpatialHash<RBX::Primitive, RBX::Contact, RBX::ContactManager, 4>::changeMinMax (0x5692A0)
// appears in the same pseudocode dump but belongs to include/v8World/SpatialHashMultiRes.inl
// (assert string: "destroyMe file: include/v8World/SpatialHashMultiRes.inl line: 624") — not this module.
// TODO: ExtentsInt32::getCorner(int) is declared (per 2016 header) but no out-of-line body was found
// in the binary export list; if the 2013 binary inlines it, an implementation will be needed here.

} // namespace RBX

/* Util/Faces.h -- October 2013 reconstruction.
   A six-bit set of box faces, one bit per NormalId. iOS 10-29 has the
   out-of-line members Faces(int), setNormalId and getNormalId (const). */
#pragma once

#include "Util/NormalId.h"

namespace RBX {

class Faces
{
public:
	Faces(int normalIdMask = 0);

	void setNormalId(NormalId normalId, bool value);
	bool getNormalId(NormalId normalId) const;
	void clear() { normalIdMask = NORM_NONE_MASK; }

	bool operator==(const Faces& rhs) const { return normalIdMask == rhs.normalIdMask; }
	bool operator!=(const Faces& rhs) const { return normalIdMask != rhs.normalIdMask; }

	int normalIdMask; // bit (1 << NormalId) per face
};

} // namespace RBX

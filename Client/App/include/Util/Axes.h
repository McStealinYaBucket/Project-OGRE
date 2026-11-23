/* Util/Axes.h -- October 2013 reconstruction.
   A three-bit set of the X/Y/Z axes, addressable by axis or by a NormalId
   lying on that axis. iOS 10-29 has Axes(int), the three static
   converters (axisToMask, normalIdToAxis, axisToNormalId), getAxis and
   getAxisByNormalId; the two setters are declared for the .cpp. */
#pragma once

#include "Util/NormalId.h"

namespace RBX {

class Axes
{
public:
	// axis <-> bit / NormalId helpers
	static int axisToMask(Vector3::Axis axis);
	static Vector3::Axis normalIdToAxis(NormalId normalId);
	static NormalId axisToNormalId(Vector3::Axis axis);

	Axes(int axisMask = 0);

	void setAxisByNormalId(NormalId normalId, bool value);
	bool getAxisByNormalId(NormalId normalId) const;
	void setAxis(Vector3::Axis axis, bool value);
	bool getAxis(Vector3::Axis axis) const;
	void clear() { axisMask = 0; }

	bool operator==(const Axes& rhs) const { return axisMask == rhs.axisMask; }
	bool operator!=(const Axes& rhs) const { return axisMask != rhs.axisMask; }

	int axisMask; // bit (1 << axis)
};

} // namespace RBX

/* App/util/OrientId.cpp — October 2013 (axis-alignment/orientation helpers).
   MOVED VERBATIM from App/util/Math.cpp (2026-09-26; same Math:: members,
   same bodies, same 2013 EAs) to break Math.obj link debts (Line::intersection,
   CoordinateFrame::lookAt, gpc_*) off the CFrame binary codec path.
   Evidence: symbolled Studio 0.126 (EAs below).
   TODO: re-merge if Math.obj debts ever resolve (Line/CoordinateFrame/gpc). */

#include "stdafx.h"

#include "Util/Math.h"
#include "Util/NormalId.h"
#include "rbx/Debug.h"
#include <cmath>
#include <limits>

namespace RBX {

bool Math::isAxisAligned(const Matrix3& matrix)
{
	int x[3] = {0,0,0};
	int y[3] = {0,0,0};

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			float t = matrix[i][j];
			RBXASSERT_VERY_FAST(0.0f == -0.0f);
			if ((t == 1.0f) || (t == -1.0f)) {
				x[i] += 1;
				y[j] += 1;
			}
			else {
				if ((t != 0.0f) && (t != -0.0f)) {
					return false;
				}
			}
		}
	}

	return (	(x[0] == 1)
			&&	(x[1] == 1)
			&&	(x[2] == 1)
			&&	(y[0] == 1)
			&&	(y[1] == 1)
			&&	(y[2] == 1)	);
}

// mac: 0x22D973
int Math::getOrientId(const Matrix3& matrix)
{
	RBXASSERT_VERY_FAST(isAxisAligned(matrix));

	int xNormal = Vector3ToNormalId(matrix.column(0));
	int yNormal = Vector3ToNormalId(matrix.column(1));
	int answer = (6 * xNormal) + yNormal;
	return answer;
}

// mac: 0x22D9D0
void Math::idToMatrix3(int orientId, Matrix3& matrix)
{
	NormalId xNormal = intToNormalId(orientId / 6);
	NormalId yNormal = intToNormalId(orientId % 6);
	Vector3 vX = normalIdToVector3(xNormal);
	Vector3 vY = normalIdToVector3(yNormal);
	Vector3 vZ = vX.cross(vY);
	matrix.setColumn(0, vX);
	matrix.setColumn(1, vY);
	matrix.setColumn(2, vZ);
}


// mac: 0x22E4C8
bool Math::isOrthonormal(const Matrix3& m)
{
	return m.isOrthonormal();
}

// mac: 0x22D648
bool Math::orthonormalizeIfNecessary(Matrix3& m)
{
	if (!isOrthonormal(m)) {
		m.orthonormalize();
		return true;
	}
	else {
		return false;
	}
}

// mac: 0x22CAF7
bool Math::isNanInf(float f)
{
	return (fabsf(f) >= std::numeric_limits<float>::infinity());
}


// mac: 0x22CBE3
bool Math::hasNanOrInf(const CoordinateFrame& c)
{
	for (int i = 0; i < 3; ++i)
		if (Math::isNanInf(c.translation[i]))
			return true;
	return hasNanOrInf(c.rotation);
}

// helper: 0x22CBE3 inlined hasNanOrInf(Matrix3) in the binary
bool Math::hasNanOrInf(const Matrix3& m)
{
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			if (Math::isNanInf(m[i][j]))
				return true;
	return false;
}

} // namespace RBX

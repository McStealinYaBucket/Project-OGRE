/* Base/rbx/MathUtil.cpp — October 2013 (isFinite homes).
   Evidence: link debts RBX::isFinite(double)/isFinite(int) from TaskScheduler
   TUs; declared in Base/include/rbx/boost.hpp. MSVC _finite semantics match
   G3D::isFinite (g3dmath.h:460).
   TODO: verify against MathUtil decompile (EA pending). */

#include <float.h>

#include "rbx/boost.hpp"

namespace RBX {

	bool isFinite(double value)
	{
		return _finite(value) != 0;
	}

	bool isFinite(int value)
	{
		(void)value;
		return true;
	}

} // namespace RBX

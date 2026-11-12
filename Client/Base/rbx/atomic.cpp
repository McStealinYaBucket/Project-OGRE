/* Base/rbx/atomic.cpp -- October 2013 (out-of-line interlocked helpers).
   Evidence: RbxInterlockedIncrement 0x153433 + RbxInterlockedDecrement
     0x153415 (batch452; iOS bodies use OSAtomicAdd32, Win32 uses the
     Interlocked* family below). Declared in Base/include/rbx/atomic.h.
   Original file: (2013 TU layout TODO). */

#include "rbx/atomic.h"

namespace RBX {

	long RbxInterlockedIncrement(long volatile* value)
	{
		return BOOST_INTERLOCKED_INCREMENT(value);
	}

	long RbxInterlockedDecrement(long volatile* value)
	{
		return BOOST_INTERLOCKED_DECREMENT(value);
	}

} // namespace RBX

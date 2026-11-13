/* Base/rbx/atomic.cpp -- October 2013 (out-of-line interlocked helpers).
   The four PLAYER bodies at 0x009b8f00-0x009b8f55 (STUDIO 0x00a70810-0x00a70865) are
   12- to 22-byte wrappers that call the kernel32 imports (InterlockedDecrement
   0x00aa1124, InterlockedIncrement 0x00aa123c, InterlockedExchange 0x00aa1104,
   InterlockedCompareExchange 0x00aa1274), not the _Interlocked* intrinsics.
   Names and order from iOS 1029 (0x002560ec..0x00256130). iOS also has
   RbxInterlockedIncrementAcquire between Increment and Exchange; Windows has no
   separate body for it (on x86 the SDK maps InterlockedIncrementAcquire to
   InterlockedIncrement), so it is left out until a caller needs it.
   Original file: no 2013 __FILE__ string; path from the 2016 tree. */

#include "rbx/atomic.h"

#include <windows.h>

namespace RBX {

// FUNCTION: PLAYER 0x009b8f00
// FUNCTION: STUDIO 0x00a70810
long RbxInterlockedDecrement(long volatile* value)
{
	return ::InterlockedDecrement(value);
}

// FUNCTION: PLAYER 0x009b8f10
// FUNCTION: STUDIO 0x00a70820
long RbxInterlockedIncrement(long volatile* value)
{
	return ::InterlockedIncrement(value);
}

// FUNCTION: PLAYER 0x009b8f20
// FUNCTION: STUDIO 0x00a70830
long RbxInterlockedExchange(long volatile* value, long exchange)
{
	return ::InterlockedExchange(value, exchange);
}

// FUNCTION: PLAYER 0x009b8f40
// FUNCTION: STUDIO 0x00a70850
long RbxInterlockedCompareExchange(long volatile* value, long exchange, long comparand)
{
	return ::InterlockedCompareExchange(value, exchange, comparand);
}

} // namespace RBX

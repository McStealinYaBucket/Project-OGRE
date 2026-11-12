/* <stdint.h> for VC9, which has none (VS2010 added it). The prior App decomp was written against
 * VS2012, whose CRT has it; what the 2013 build used in its place is not known.
 * ponytail: exact-width typedefs only (the tree uses uint32_t, uint64_t, uintptr_t); add the
 * limit macros if code ever needs them. */
#ifndef OGRE_VC9COMPAT_STDINT_H
#define OGRE_VC9COMPAT_STDINT_H
#include <crtdefs.h> /* intptr_t, uintptr_t */
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#endif

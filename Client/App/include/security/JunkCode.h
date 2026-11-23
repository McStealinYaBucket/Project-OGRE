/* security/JunkCode.h -- October 2013 reconstruction.
   RBX_JUNK drops a short, harmless multi-byte NOP sequence into the caller,
   chosen at compile time from the build seed and __LINE__, so identical
   source lines do not produce identical bytes from build to build.
   No 2013 trace located and no TU in this tree includes the header: kept so
   the path exists. Off (RBX_JUNK expands to nothing) outside Windows player
   builds. The 17 variants are the seven NOP encodings below, alone or in
   pairs; the selector (seed % 15 + 1) * line + seed, taken mod 17, uses a
   multiplier in 1..16, coprime to 17, so every variant is reachable. */
#pragma once

#if defined(_WIN32) && !defined(RBX_STUDIO_BUILD) && !defined(RBX_PLATFORM_DURANGO)

#include "Security/RandomConstant.h"

// Encodings, 3 to 6 bytes each, none with side effects:
#define RBX_NOP0 __asm _emit 0x0f __asm _emit 0x1f __asm _emit 0x00                              /* nop [eax]          */
#define RBX_NOP1 __asm _emit 0x8d __asm _emit 0x76 __asm _emit 0x00                              /* lea esi,[esi]      */
#define RBX_NOP2 __asm _emit 0x0f __asm _emit 0x1f __asm _emit 0x40 __asm _emit 0x00             /* nop [eax+0]        */
#define RBX_NOP3 __asm _emit 0x8d __asm _emit 0x74 __asm _emit 0x26 __asm _emit 0x00             /* lea esi,[esi]      */
#define RBX_NOP4 __asm _emit 0x90 __asm _emit 0x8d __asm _emit 0x74 __asm _emit 0x26 __asm _emit 0x00 /* nop; lea esi,[esi] */
#define RBX_NOP5 __asm _emit 0x66 __asm _emit 0x0f __asm _emit 0x1f __asm _emit 0x44 __asm _emit 0x00 __asm _emit 0x00 /* nop [eax+eax+0] */
#define RBX_NOP6 __asm _emit 0x8d __asm _emit 0xb6 __asm _emit 0x00 __asm _emit 0x00 __asm _emit 0x00 __asm _emit 0x00 /* lea esi,[esi+0] */

template <int N> inline void junk() {}

// One specialisation per variant; each __asm _emit is its own statement.
#define RBX_JUNK_VARIANT(n, seq) template <> inline void junk<n>() { seq }

RBX_JUNK_VARIANT(0,  RBX_NOP0)
RBX_JUNK_VARIANT(1,  RBX_NOP1)
RBX_JUNK_VARIANT(2,  RBX_NOP2)
RBX_JUNK_VARIANT(3,  RBX_NOP3)
RBX_JUNK_VARIANT(4,  RBX_NOP4)
RBX_JUNK_VARIANT(5,  RBX_NOP5)
RBX_JUNK_VARIANT(6,  RBX_NOP6)
RBX_JUNK_VARIANT(7,  RBX_NOP0 RBX_NOP1)
RBX_JUNK_VARIANT(8,  RBX_NOP1 RBX_NOP2)
RBX_JUNK_VARIANT(9,  RBX_NOP1 RBX_NOP3)
RBX_JUNK_VARIANT(10, RBX_NOP4 RBX_NOP0)
RBX_JUNK_VARIANT(11, RBX_NOP3 RBX_NOP5)
RBX_JUNK_VARIANT(12, RBX_NOP2 RBX_NOP5)
RBX_JUNK_VARIANT(13, RBX_NOP6 RBX_NOP0)
RBX_JUNK_VARIANT(14, RBX_NOP2 RBX_NOP1)
RBX_JUNK_VARIANT(15, RBX_NOP4 RBX_NOP5)
RBX_JUNK_VARIANT(16, RBX_NOP3 RBX_NOP2)

#undef RBX_JUNK_VARIANT

#define RBX_JUNK (junk<((RBX_BUILDSEED % 15 + 1) * __LINE__ + RBX_BUILDSEED) % 17>())

#else

#define RBX_JUNK

#endif

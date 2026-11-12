// Compiler compatibility. The tree targets VC9 SP1 (cl 15.00.30729.01) and C++03: no C++11, no
// static_assert, no nullptr. Include this before anything that depends on the compiler.
#ifndef COMPAT_H
#define COMPAT_H

// MSVC_FOR_DECOMP is 1 only under the exact compiler the 2013 binaries were built with (Rich header,
// CLAUDE.md facts table). Anything else can build the tree but will not match.
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER == 150030729
#define MSVC_FOR_DECOMP 1
#else
#define MSVC_FOR_DECOMP 0
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1500
// 4481: "nonstandard extension used: override specifier 'override'". VC9 accepts the keyword.
#pragma warning(disable : 4481)
#endif

#endif // COMPAT_H

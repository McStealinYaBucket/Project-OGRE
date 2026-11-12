// Decompilation helpers shared by every library in the tree.
// Written for Project OGRE; same contract as the isledecomp/OpenRBX helpers of the same names.
#ifndef DECOMP_H
#define DECOMP_H

// Placeholder types for members, parameters and globals whose real type is not proven yet
// (CLAUDE.md, Types and Names). Replace them only when a 100% function proves the type.
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef unsigned int undefined4;

// DECOMP_SIZE_ASSERT(T, S) fails the build when sizeof(T) != S. It only fires under the original
// compiler (VC9 SP1, _MSC_VER 1500): other compilers lay classes out differently, so the check
// would be meaningless there. Put it in the .cpp, not the header.
#define DECOMP_CAT_(a, b) a##b
#define DECOMP_CAT(a, b) DECOMP_CAT_(a, b)
#if defined(_MSC_VER) && _MSC_VER == 1500
#define DECOMP_STATIC_ASSERT(V) typedef char DECOMP_CAT(decomp_static_assert_, __LINE__)[(V) ? 1 : -1]
#else
#define DECOMP_STATIC_ASSERT(V)
#endif
#define DECOMP_SIZE_ASSERT(T, S) DECOMP_STATIC_ASSERT(sizeof(T) == (S))

#ifndef sizeOfArray
#define sizeOfArray(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// STUB(0xADDRESS) goes in the body of every function that is not written yet. Each stub stores a
// different constant to a global, so no two stub bodies are identical and /OPT:ICF cannot fold
// them onto one address (which would make reccmp pair the wrong functions).
#ifdef __cplusplus
extern "C" {
#endif
extern undefined4 g_foldingDummyVariable;
#ifdef __cplusplus
}
#endif

#define STUB(addr)                                                                                                     \
	do {                                                                                                               \
		g_foldingDummyVariable = (addr);                                                                               \
	} while (0)

#endif // DECOMP_H

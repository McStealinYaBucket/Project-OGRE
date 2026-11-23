// Base/include/rbx/BaldPtr.h: October 2013 reconstruction (Project OGRE).
// A raw pointer wrapper that, in builds with very-fast asserts on, checks the pointer against the
// fill patterns of freed or uninitialized memory each time it is used. It costs nothing otherwise:
// one pointer, all inline. No 2013 binary names it (as a header-only template it leaves no symbol
// of its own) and nothing in this tree includes it; it is kept, rewritten, for later imports.
// The 2016 file's commented-out test program is not carried over.
#pragma once

#include "rbx/Debug.h"

namespace RBX {

template<class T>
class BaldPtr
{
	T* ptr;

	// Patterns the MSVC debug CRT and Ogre's allocator write into memory that is not a live object.
	static bool looksValid(T* p)
	{
		const unsigned v = (unsigned)p;
		return v != 0xCCCCCCCC   // CRT: uninitialized stack
			&& v != 0xCDCDCDCD   // CRT: uninitialized heap
			&& v != 0xFDFDFDFD   // CRT: guard bytes around a heap block
			&& v != 0xDDDDDDDD   // CRT: freed heap
			&& v != 0xFEEEFEEE   // HeapFree fill
			&& v != 0xBAADF00D   // Ogre: guard before the block
			&& v != 0xDEADC0DE   // Ogre: guard after the block
			&& v != 0xFEEDFACE   // Ogre: uninitialized
			&& v != 0xDEADBEEF;  // Ogre: freed
	}

public:
	BaldPtr() : ptr(NULL) {}
	BaldPtr(T* p) : ptr(p) { validate(); }

	T*& operator=(T* p)
	{
		ptr = p;
		validate();
		return ptr;
	}

	// Dereferencing also rejects NULL.
	T& operator*() const { RBXASSERT_VERY_FAST(ptr); validate(); return *ptr; }
	T* operator->() const { RBXASSERT_VERY_FAST(ptr); validate(); return ptr; }
	operator T*() const { validate(); return ptr; }

	// unchecked
	T* get() const { return ptr; }

	// NULL passes; a fill pattern fails the assert.
	void validate() const
	{
#if defined(__RBX_VERY_FAST_ASSERT)
		if (!ptr)
			return;
#endif
		RBXASSERT_VERY_FAST(looksValid(ptr));
	}
};

}

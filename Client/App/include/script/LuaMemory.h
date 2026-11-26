/* App/script/LuaMemory.h -- October 2013 (Lua pool allocator).
   Evidence: C2 0x17C950 (210B, batch317: vptr + 7 zero dwords; 16x
   boost::pool(28 + i*4) on the bool, pushed to the +20 vector).
   First-party 2016 LuaMemory.h/ThreadRef carry the same shape. */

#pragma once

#include <vector>
#include "boost/pool/pool.hpp"

namespace RBX {

	// VTABLE: PLAYER 0x00b7f5f8
	// VTABLE: STUDIO 0x00c52fa8
	// SIZE 0x24
	class LuaAllocator
	{
	public:
		// mac: 0x17C950: vptr + heapSize/heapCount/maxHeapSize/maxHeapCount
		// + 2 more zero dwords; 16 pools (28,32,...,88) iff usePool.
		explicit LuaAllocator(bool usePool);
		~LuaAllocator();

		static void* alloc(void* ud, void* ptr, size_t osize, size_t nsize);
		// The only virtual (vftable PLAYER 0x00b7f5f8: one slot, 0x0086b400); the ctor 0x0086b300
		// stores the vptr and openState allocates 0x24 bytes (PLAYER 0x006883fd).
		virtual void* alloc(void* ptr, size_t osize, size_t nsize);	// vtable+0x00
		bool hasSpace(long diff);
		// mac: 0x17CAAE (40B, batch590: reads +4/+8/+12/+16). Const per
		// the `__ZNK` mangling.
		void getHeapStats(size_t& a, size_t& b, size_t& c, size_t& d) const;
		// mac: 0x17CA98 (22B, batch590: zeroes +12/+16).
		void clearHeapMax();

		static size_t heapLimit;

	private:
		size_t heapSize;      // +4.
		size_t heapCount;     // +8.
		size_t maxHeapSize;   // +12.
		size_t maxHeapCount;  // +16.
		// +0x14: pool vector (binary boost::pool<new_delete>; begin/end/capacity at 0x18/0x1c/0x20).
		std::vector<boost::pool<>*> memPools;
	};

} // namespace RBX

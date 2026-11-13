/* App/script/LuaMemory.cpp -- October 2013 (Lua pool allocator).
   See script/LuaMemory.h for evidence. */

#include "stdafx.h"

#include "script/LuaMemory.h"
#include "security/SecurityContext.h"
#include "FastLog.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

LOGGROUP(LuaMemoryPool);

namespace RBX {

	size_t LuaAllocator::heapLimit = 0;

	// mac: 0x17C950 (210B, batch317).
	LuaAllocator::LuaAllocator(bool usePool)
		: heapSize(0)
		, heapCount(0)
		, maxHeapSize(0)
		, maxHeapCount(0)
	{
		if (usePool)
		{
			for (int i = 0; i < 16; ++i)
				memPools.push_back(new boost::pool<>(28 + i * 4));
		}
	}

	LuaAllocator::~LuaAllocator()
	{
		for (size_t i = 0; i < memPools.size(); ++i)
			delete memPools[i];
	}

	// Pool index (binary lane: (size&3)==0 && (size-24)>>2 < 17, then -1).
	static int poolIndex(size_t size)
	{
		if ((size & 3) != 0)
			return -1;
		if (((int)(size - 24) >> 2) >= 17)
			return -1;
		return ((int)(size - 24) >> 2) - 1;
	}

	bool LuaAllocator::hasSpace(long diff)
	{
		if (heapLimit > 0 && diff > 0 && (size_t)(diff + heapSize) > heapLimit
			&& (RBX::Security::Context::current().identity == RBX::Security::GameScript_
				|| RBX::Security::Context::current().identity == RBX::Security::RobloxGameScript_))
			return false;
		return true;
	}

	// Static entry (0x17CAD6, 13B: forwards to the member).
	void* LuaAllocator::alloc(void* ud, void* ptr, size_t osize, size_t nsize)
	{
		LuaAllocator* allocator = reinterpret_cast<LuaAllocator*>(ud);
		return allocator->alloc(ptr, osize, nsize);
	}



































	// mac: 0x17CB2C (990B, batch318).
	// STUB: PLAYER 0x0086b400
	// STUB: STUDIO 0x009ad420
	void* LuaAllocator::alloc(void* ptr, size_t osize, size_t nsize)
	{
		if (!hasSpace((long)(nsize - osize)))
		{
			// Arity DELTA: binary calls the 4-arg FastLog (channel, msg,
			// NULL, extra) which our Log lib does not export; 3-arg here.
			if (FLog::LuaMemoryPool)
				FLog::FastLog(FLog::LuaMemoryPool, "lua alloc has no space", NULL);
			return NULL;
		}
		void* result;
		if (!memPools.empty())
		{
			if (nsize != 0)
			{
				int index1 = poolIndex(nsize);
				if (index1 >= 0)
				{
					RBXASSERT(nsize == memPools[index1]->get_requested_size());  // :121
					result = memPools[index1]->malloc();
					memcpy(result, ptr, std::min(nsize, osize));
					int index2 = poolIndex(osize);
					if (index2 >= 0)
					{




						RBXASSERT(osize == memPools[index2]->get_requested_size());  // :131
						memPools[index2]->free(ptr);
					}
				}
				else
				{
					int index2 = poolIndex(osize);
					if (index2 >= 0)
					{

						RBXASSERT(osize == memPools[index2]->get_requested_size());  // :141
						result = malloc(nsize);
						memcpy(result, ptr, std::min(osize, nsize));
						memPools[index2]->free(ptr);
					}
					else
					{
						result = realloc(ptr, nsize);
					}
				}
			}
			else
			{
				int index = poolIndex(osize);
				if (index >= 0)
				{
					RBXASSERT(osize == memPools[index]->get_requested_size());  // :108
					memPools[index]->free(ptr);
				}
				else
				{
					free(ptr);
				}
				result = NULL;
			}
		}
		else
		{
			if (nsize == 0)
			{
				free(ptr);
				result = NULL;
			}
			else
			{
				// FInt::LuaMemoryBonus defaults 0 (binary adds it) -- plain.
				result = realloc(ptr, nsize);
			}
		}
		heapSize = heapSize + nsize - osize;
		if (osize == 0)
			++heapCount;
		size_t count = heapCount;
		if (nsize == 0)
			heapCount = --count;
		if (maxHeapSize < heapSize)
			maxHeapSize = heapSize;
		if (maxHeapCount < count)
			maxHeapCount = count;
		// DELTA: RBXPROFILER_COUNTER_ADD("memory/lua/heap") unmodeled.
		return result;
	}

	// mac: 0x17CAAE (40B, batch590).
	void LuaAllocator::getHeapStats(size_t& a, size_t& b, size_t& c, size_t& d) const
	{
		a = heapSize;
		b = heapCount;
		c = maxHeapSize;
		d = maxHeapCount;
	}

	// mac: 0x17CA98 (22B, batch590).
	void LuaAllocator::clearHeapMax()
	{
		maxHeapSize = 0;
		maxHeapCount = 0;
	}

} // namespace RBX

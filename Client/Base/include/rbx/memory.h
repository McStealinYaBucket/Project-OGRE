// Base/include/rbx/memory.h: October 2013 reconstruction (Project OGRE).
// Per-class pooled allocation for the engine.
// Evidence: asserts at Base\include\rbx/memory.h lines 231 (AutoMemPool::Object::operator new)
// and 264 (AutoMemPool::free) in PLAYER and STUDIO, kept on those lines; iOS 1029 symbols
// RBX::Allocator<T>::Allocator(), ::operator new(unsigned long), ::operator delete(void*),
// ::releaseMemory(), ::initialized, ::availableSize, RBX::poolAvailabilityList,
// RBX::poolReleaseMemoryFuncList, RBX::poolAllocationList, RBX::roblox_allocator::malloc/free
// and ::crashOnAllocationFailure, and the RTTI of RBX::AutoMemPool::Object. The Allocator
// constructor and operator new instantiated for FWInstance are placed below.
// Release builds (as shipped) take the singleton-pool path; _DEBUG adds counters and statistics.
#ifndef RBX_MEMORY_H
#define RBX_MEMORY_H

#include "boost/pool/singleton_pool.hpp"
#include "boost/scoped_ptr.hpp"
#include <assert.h>
#include "rbx/debug.h"
#include "rbx/atomic.h"
#include <vector>

#if defined(_DEBUG)
// Per-class instance counts (not interlocked) and pool byte statistics.
#define RBX_ALLOCATOR_COUNTS
#define RBX_POOL_ALLOCATION_STATS
#else
// Each Allocator<T> class gets its own boost::singleton_pool. Faster, but freed memory stays in
// the pool until releaseMemory, which hides leaks.
#define RBX_ALLOCATOR_SINGLETON_POOL
#endif

namespace RBX {

// Registries every Allocator<T> joins on first construction, so memory pressure handling can
// walk all pools: their free-byte counters and their release functions.
#if defined(RBX_POOL_ALLOCATION_STATS)
extern std::vector<size_t*> poolAllocationList;
#endif
typedef bool (*releaseFunc)();
extern std::vector<size_t*> poolAvailabilityList;
extern std::vector<releaseFunc> poolReleaseMemoryFuncList;

// Book-keeping for one allocation of `size` bytes: it is either new memory (counted only with
// statistics on) or taken from what the pool had free.
inline void addToPool(size_t* allocatedSize, size_t* availableSize, size_t size)
{
	if (size > *availableSize)
	{
#if defined(RBX_POOL_ALLOCATION_STATS)
		(*allocatedSize) += (size);
#endif
	}
	else
		(*availableSize) -= (size);
}

inline void removeFromPool(size_t* availableSize, size_t size) { (*availableSize) += (size); }

// A UserAllocator for std and boost containers (and boost pools) over the engine's heap.
class roblox_allocator
{
public:
	static bool crashOnAllocationFailure; // crash instead of throwing bad_alloc
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	static char* malloc(const size_type bytes);
	static void free(char* const block);
	static char* realloc(char* ptr, size_t nsize);
};

// Base class that routes T's operator new/delete to a pool of sizeof(T) blocks. T derives from
// Allocator<T> (CRTP), so T is incomplete while this class is instantiated: nothing at class scope
// may depend on sizeof(T); only the member function bodies do. The static counters are per T.
// 2013 users seen in the symbols and in this tree: FWInstance, FWPartInstance, OnDemandInstance,
// XmlElement, XmlAttribute, the physics kernel's Body, SimBody and Cofm, and SpatialHash's nodes.
// The pool registries above let a low-memory handler call every releaseMemory (iOS 1029 has 39
// Allocator<T>::releaseMemory instantiations).
template<class T>
class Allocator
{
#if defined(RBX_ALLOCATOR_COUNTS)
	static rbx::atomic<int> count;
#endif
public:
	static size_t allocatedSize;
	static size_t availableSize;
	static bool initialized;

	// Registers this class's pool the first time any T is built.
	// TEMPLATE: PLAYER 0x0061c800
	// TEMPLATE: STUDIO 0x006f1310
	// ??0?$Allocator@VFWInstance@RBX@@@RBX@@QAE@XZ
	Allocator()
	{
		if (!initialized)
		{
#if defined(RBX_POOL_ALLOCATION_STATS)
			poolAllocationList.push_back(&allocatedSize);
#endif
			poolAvailabilityList.push_back(&availableSize);
			bool (*release)() = releaseMemory;
			poolReleaseMemoryFuncList.push_back(release);
			initialized = true;
		}
	}

#if defined(RBX_ALLOCATOR_SINGLETON_POOL)
	// (a macro, not a typedef: T is still incomplete while Allocator<T> is instantiated)
#define RBX_ALLOCATOR_POOL boost::singleton_pool<T, sizeof(T), boost::default_user_allocator_malloc_free>

	// TEMPLATE: PLAYER 0x0061bbd0
	// ??2?$Allocator@VFWInstance@RBX@@@RBX@@SAPAXI@Z
	void* operator new(size_t nSize)
	{
		assert(nSize==sizeof(T));
		void* block = RBX_ALLOCATOR_POOL::malloc();
		if (!block)
		{
			// a hard crash gets the process logged; the exception is for when crashing is off
			if (roblox_allocator::crashOnAllocationFailure)
				RBXCRASH();
			throw std::bad_alloc();
		}
#if defined(RBX_ALLOCATOR_COUNTS)
		count++;
#endif
		addToPool(&allocatedSize, &availableSize, nSize);
		return block;
	}

	void* operator new(size_t size, void* p)
	{
		addToPool(&allocatedSize, &availableSize, size);
		return p;
	}
	void operator delete(void*, void*) { removeFromPool(&availableSize, sizeof(T)); }

	// Gives the pool's unused blocks back to the heap.
	static bool releaseMemory()
	{
#if defined(RBX_POOL_ALLOCATION_STATS)
		allocatedSize -= availableSize;
#endif
		availableSize = 0;
		return RBX_ALLOCATOR_POOL::release_memory();
	}

	// Frees every block of the pool, live objects included: only safe when no T exists.
	static bool purgeMemory()
	{
#if defined(RBX_POOL_ALLOCATION_STATS)
		allocatedSize = 0;
#endif
		availableSize = 0;
		return RBX_ALLOCATOR_POOL::purge_memory();
	}

	void operator delete(void* p)
	{
		RBX_ALLOCATOR_POOL::free(p);
#if defined(RBX_ALLOCATOR_COUNTS)
		count--;
#endif
		removeFromPool(&availableSize, sizeof(T));
	}

#undef RBX_ALLOCATOR_POOL
#else
	// Debug builds: straight to the heap, so leak checkers see every object.
	void* operator new(size_t nSize)
	{
		assert(nSize==sizeof(T));
		void* block = (void*)roblox_allocator::malloc(nSize);
		if (!block)
		{
			if (roblox_allocator::crashOnAllocationFailure)
				RBXCRASH();
			throw std::bad_alloc();
		}
#if defined(RBX_ALLOCATOR_COUNTS)
		count++;
#endif
		return block;
	}
	void operator delete(void* p)
	{
		roblox_allocator::free((char*)p);
#if defined(RBX_ALLOCATOR_COUNTS)
		count--;
#endif
	}
	void* operator new(size_t size, void* p) { return p; }
	void operator delete(void*, void*) {}
	static bool releaseMemory() { return true; }
	static bool purgeMemory() { return true; }
#endif

#if defined(RBX_ALLOCATOR_COUNTS)
	static long getCount() { return count; }
	static long getHeapSize() { return sizeof(T) * count; }
#endif
};

template<class T> size_t Allocator<T>::allocatedSize = 0;
template<class T> size_t Allocator<T>::availableSize = 0;
template<class T> bool Allocator<T>::initialized = false;
#if defined(RBX_ALLOCATOR_COUNTS)
template<class T> rbx::atomic<int> Allocator<T>::count;
#endif

// A boost::pool<> whose blocks carry a hidden pointer back to the pool, so an Object allocated
// from it can be deleted with a plain delete. Each block is requested_size + one pointer; the
// object starts after the pointer. Typical use:
//     AutoMemPool pool(sizeof(Thing));
//     Thing* t = new (&pool) Thing(...);   // Thing derives from AutoMemPool::Object
//     delete t;                            // finds &pool in the hidden header
// The pool must outlive its objects: its destructor releases the memory without running any
// destructors.
class AutoMemPool
{
	boost::scoped_ptr< boost::pool<> > pool;


public:
	// Base for objects created with new (pool) T; their delete finds the pool by itself.
	class Object
	{
	public:
		// Block layout: [AutoMemPool*][object]. The pool's block size must leave room for it.
		void* operator new(size_t size, AutoMemPool* pool)
		{
			RBXASSERT(((size_t)pool->getRequestedSize()) == size + sizeof(AutoMemPool*));
			void* block = pool->malloc();
			*(AutoMemPool**)block = &(*pool);
			return (char*)block + sizeof(AutoMemPool*);
		}
		// matches the placement form, for a constructor that throws
		void operator delete(void* p, AutoMemPool* pool) { pool->free(p); }
		void operator delete(void* p)
		{
			p = (char*)p - sizeof(AutoMemPool*);
			AutoMemPool* owner = *(AutoMemPool**)p;
			owner->free(p);
		}
	};

	// requested_size: the object size; the hidden pointer is added on top.
	AutoMemPool(int requested_size) { pool.reset(new boost::pool<>(requested_size + sizeof(this))); }

	// (spacing: keeps the asserts below on their 2013 line numbers)











	inline void* malloc() { return pool->malloc(); }
	inline void free(void* p)
	{
		RBXASSERT(pool->is_from(p));
		pool->free(p);
	}
	inline int getRequestedSize() { return int(pool->get_requested_size()); }
};

}

#endif

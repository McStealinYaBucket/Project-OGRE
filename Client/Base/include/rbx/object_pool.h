// Base/include/rbx/object_pool.h: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbols RBX::object_pool<SpatialHash<...>::SpatialNode/TreeNode,
// RBX::roblox_allocator>::for_each<CallDestructor> and for_each<FastClearSpatialNode/TreeNode>:
// a boost::object_pool (1.53) that can visit every live element. clear() destroys them all and
// frees the blocks without boost's own destructor pass. T's destructor must not throw.
#pragma once

#include "boost/pool/object_pool.hpp"

namespace RBX {

template <typename T, typename UserAllocator>
class object_pool : public boost::object_pool<T, UserAllocator>
{
	typedef boost::object_pool<T, UserAllocator> base;

protected:
	struct CallDestructor
	{
		void operator()(T* item) { item->~T(); }
	} callDestructor;

public:
#if !defined(_WIN32)
	// gcc does not see the base class's typedef through a dependent base
	typedef typename boost::pool<UserAllocator>::size_type size_type;
#endif

	// next_size: elements in the first block (boost's own default is 32)
	explicit object_pool<T, UserAllocator>(const size_type next_size = 32) : base(next_size) {}

	// Calls f(T*) for each allocated element. Walks every block chunk by chunk; the free list is
	// sorted (ordered_malloc/ordered_free), so one cursor along it tells free chunks apart.
	template<class F>
	void for_each(F& f)
	{
		if (!this->list.valid())
			return;

		boost::details::PODptr<size_type> block = this->list;
		boost::details::PODptr<size_type> following = block;
		void* nextFree = this->first;
		const size_type step = this->alloc_size();

		do
		{
			following = following.next();
			for (char* chunk = block.begin(); chunk != block.end(); chunk += step)
			{
				if (chunk == nextFree)
				{
					nextFree = boost::simple_segregated_storage<size_type>::nextof(nextFree);
					continue;
				}
				f(static_cast<T*>(static_cast<void*>(chunk)));
			}
			block = following;
		} while (block.valid());
	}

	// Destroys every element, then releases the memory with purge_memory, which runs no destructors.
	void clear()
	{
		for_each(callDestructor);
		boost::pool<UserAllocator>::purge_memory();
	}
};

}

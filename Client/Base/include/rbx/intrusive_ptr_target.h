// Base/include/rbx/intrusive_ptr_target.h: October 2013 reconstruction (Project OGRE).
// Mix-ins that give a class the hooks boost::intrusive_ptr (and rbx::intrusive_weak_ptr) call.
// Evidence: seven asserts at Base\include\rbx/intrusive_ptr_target.h in PLAYER and STUDIO, at
// lines 133, 134 (operator delete), 163 (quick release), 184 (strong add), 214 and 224 (weak add)
// and 259 (weak release); they sit 8 lines lower than in the tree this replaces, and are placed on
// their 2013 lines here. RTTI: intrusive_ptr_target<rbx::signals::connection::islot, int, 0, 0>
// and quick_intrusive_ptr_target<RBX::Lua::WeakThreadRef, int, 0>; iOS 1029 names
// intrusive_ptr_target<islot,...>::counts::counts() and its operator delete(void*).
//
// quick_intrusive_ptr_target<T, Count, maxRefs>: one atomic count inside the object.
// intrusive_ptr_target<T, Count, maxStrong, maxWeak>: strong and weak counts kept in a header
//   that operator new allocates in front of the object, so the memory outlives the object while
//   weak references remain. weak counts the weak references plus one for "any strong left".
// A non-zero max* throws too_many_refs past that many references; it matters only for narrow
// Count types and is checked loosely (racing threads can overshoot a little).
#pragma once

#include "rbx/debug.h"
#include "rbx/atomic.h"
#include "rbx/declarations.h"
#include "boost/cast.hpp"

namespace rbx {
template<class T, typename Count, Count maxRefs>
class quick_intrusive_ptr_target;
template<class T, typename Count, Count maxStrong, Count maxWeak>
class intrusive_ptr_target;
}

// The parameter list and type every intrusive_ptr_target hook below is written against.
#define RBX_IPT_PARAMS template<class T, typename Count, Count maxStrong, Count maxWeak>
#define RBX_IPT rbx::intrusive_ptr_target<T, Count, maxStrong, maxWeak>
#define RBX_QIPT_PARAMS template<class T, typename Count, Count maxRefs>
#define RBX_QIPT rbx::quick_intrusive_ptr_target<T, Count, maxRefs>

// Declared first so the classes can befriend them.
namespace boost {
RBX_QIPT_PARAMS void intrusive_ptr_add_ref(const RBX_QIPT* p);
RBX_QIPT_PARAMS void intrusive_ptr_release(const RBX_QIPT* p);

RBX_IPT_PARAMS void intrusive_ptr_add_ref(const RBX_IPT* p);
RBX_IPT_PARAMS void intrusive_ptr_release(const RBX_IPT* p);
RBX_IPT_PARAMS bool intrusive_ptr_expired(const RBX_IPT* p);
RBX_IPT_PARAMS bool intrusive_ptr_try_lock(const RBX_IPT* p);
RBX_IPT_PARAMS void intrusive_ptr_add_weak_ref(const RBX_IPT* p);
RBX_IPT_PARAMS void intrusive_ptr_weak_release(const RBX_IPT* p);
}

namespace rbx {

class too_many_refs : public std::exception
{
public:
	virtual const char* what() const throw() { return "too many refs"; }
};

// pack(8) only matters when Count is narrower than int.
#pragma pack(push)
#pragma pack(8)
template<class T, typename Count = int, Count maxRefs = 0>
class RBXBaseClass quick_intrusive_ptr_target
{
	rbx::atomic<Count> refs;
public:
	inline quick_intrusive_ptr_target() { refs = 0; }
	friend void boost::intrusive_ptr_add_ref<>(const quick_intrusive_ptr_target<T, Count, maxRefs>* p);
	friend void boost::intrusive_ptr_release<>(const quick_intrusive_ptr_target<T, Count, maxRefs>* p);
};
#pragma pack(pop)

template<class T, typename Count = int, Count maxStrong = 0, Count maxWeak = maxStrong>
class RBXBaseClass intrusive_ptr_target
{
#pragma pack(push)
#pragma pack(8)
	// Lives immediately before the object.
	struct counts
	{
		rbx::atomic<Count> strong; // strong references
		rbx::atomic<Count> weak;   // weak references, plus 1 while strong > 0
		counts()
		{
			strong = 0;
			weak = 1;
		}
	};
#pragma pack(pop)

	static inline counts* fetch(const T* t) { return reinterpret_cast<counts*>((char*)t - sizeof(counts)); }

public:
	// One block: counts header, then the object.
	void* operator new(std::size_t t)
	{
		void* block = ::malloc(sizeof(counts) + t);
		new (block) counts();
		return (char*)block + sizeof(counts);
	}

	// (spacing: keeps the asserts below on their 2013 line numbers)



























	// Reached only for an object no intrusive_ptr ever held (a constructor threw, or it was
	// deleted directly); otherwise the release hooks free the block.
	void operator delete(void* p)
	{
		counts* c = fetch(reinterpret_cast<T*>(p));
		RBXASSERT(c->strong == 0);
		RBXASSERT(c->weak == 1);
		::free(c);
	}

	friend void boost::intrusive_ptr_add_ref<>(const intrusive_ptr_target<T, Count, maxStrong, maxWeak>* p);
	friend void boost::intrusive_ptr_release<>(const intrusive_ptr_target<T, Count, maxStrong, maxWeak>* p);
	friend void boost::intrusive_ptr_add_weak_ref<>(const intrusive_ptr_target<T, Count, maxStrong, maxWeak>* p);
	friend bool boost::intrusive_ptr_expired<>(const intrusive_ptr_target<T, Count, maxStrong, maxWeak>* p);
	friend bool boost::intrusive_ptr_try_lock<>(const intrusive_ptr_target<T, Count, maxStrong, maxWeak>* p);
	friend void boost::intrusive_ptr_weak_release<>(const intrusive_ptr_target<T, Count, maxStrong, maxWeak>* p);
};

}

namespace boost {

// quick_intrusive_ptr_target

RBX_QIPT_PARAMS
void intrusive_ptr_add_ref(const RBX_QIPT* p)
{
	if (maxRefs > 0 && p->refs >= maxRefs)
		throw rbx::too_many_refs();
	const_cast<RBX_QIPT*>(p)->refs++;
}
// The last release deletes the object.
RBX_QIPT_PARAMS
void intrusive_ptr_release(const RBX_QIPT* p)
{
	RBXASSERT(p->refs > 0);
	if (--(const_cast<RBX_QIPT*>(p)->refs) == 0)
		delete static_cast<const T*>(p);
}

// intrusive_ptr_target



RBX_IPT_PARAMS
void intrusive_ptr_add_ref(const RBX_IPT* p)
{
	typename RBX_IPT::counts* c = RBX_IPT::fetch(static_cast<const T*>(p));
	if (maxStrong > 0)
	{
		if (++(c->strong) > maxStrong)
			throw rbx::too_many_refs();
	}
	else
	{
		c->strong++;
		RBXASSERT(c->strong < std::numeric_limits<Count>::max() - 10);
	}
}

// The last strong release destroys the object; the block goes once no weak reference is left.
RBX_IPT_PARAMS
void intrusive_ptr_release(const RBX_IPT* p)
{
	const T* object = static_cast<const T*>(p);
	typedef typename RBX_IPT::counts Counts;
	Counts* c = RBX_IPT::fetch(object);
	if (--(c->strong) == 0)
	{
		object->~T();
		if (--(c->weak) == 0)
		{
			c->Counts::~counts();
			::free((void*)c);
		}
	}
}




// Only valid while a strong reference exists (weak_ptr copies go through raw()).
RBX_IPT_PARAMS
void intrusive_ptr_add_weak_ref(const RBX_IPT* p)
{
	typename RBX_IPT::counts* c = RBX_IPT::fetch(static_cast<const T*>(p));
	RBXASSERT(c->strong > 0);
	if (maxWeak > 0)
	{
		if (++(c->weak) > maxWeak + 1) // the +1 is the strong side's share
			throw rbx::too_many_refs();
	}
	else
	{
		++(c->weak);

		RBXASSERT(c->weak < std::numeric_limits<Count>::max() - 10);
	}
}

RBX_IPT_PARAMS
bool intrusive_ptr_expired(const RBX_IPT* p)
{
	typename RBX_IPT::counts* c = RBX_IPT::fetch(static_cast<const T*>(p));
	return c->strong == 0;
}

// Adds a strong reference unless the count has reached zero, by compare-and-swap.
RBX_IPT_PARAMS
bool intrusive_ptr_try_lock(const RBX_IPT* p)
{
	typename RBX_IPT::counts* c = RBX_IPT::fetch(static_cast<const T*>(p));
	while (true)
	{
		Count seen = c->strong;
		if (seen == 0)
			return false;
		if (c->strong.compare_and_swap(seen + 1, seen) == seen)
			return true;
	}
}


RBX_IPT_PARAMS
void intrusive_ptr_weak_release(const RBX_IPT* p)
{
	const T* object = static_cast<const T*>(p);
	typedef typename RBX_IPT::counts Counts;
	Counts* c = RBX_IPT::fetch(object);
	if (--(c->weak) == 0)
	{
		RBXASSERT(c->strong == 0);
		c->Counts::~counts();
		::free((void*)c);
	}
}

}

#undef RBX_IPT_PARAMS
#undef RBX_IPT
#undef RBX_QIPT_PARAMS
#undef RBX_QIPT

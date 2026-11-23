// Base/include/rbx/intrusive_weak_ptr.h: October 2013 reconstruction (Project OGRE).
// A weak counterpart to boost::intrusive_ptr. Its only user is rbx/signal.h, whose 2013 asserts
// (signal.h lines 261-310, PLAYER) and the intrusive_ptr_target<connection::islot> RTTI show the
// same weak-referenced slot design. The pointee type provides, beside intrusive_ptr's two hooks,
//   intrusive_ptr_add_weak_ref, intrusive_ptr_weak_release, intrusive_ptr_try_lock and
//   intrusive_ptr_expired (rbx/intrusive_ptr_target.h implements all of them),
// found through boost:: as the calls below spell them.
#pragma once

#include "boost/intrusive_ptr.hpp"

namespace rbx {

template<class T>
class intrusive_weak_ptr
{
	T* p_;

public:
	intrusive_weak_ptr() : p_(0) {}
	intrusive_weak_ptr(T* p) : p_(p)
	{
		if (p_ != 0)
			boost::intrusive_ptr_add_weak_ref(p_);
	}

	// Copies of a dead pointer start out empty.
	intrusive_weak_ptr(const intrusive_weak_ptr& other) : p_(0)
	{
		if (!other.expired())
		{
			p_ = other.raw();
			boost::intrusive_ptr_add_weak_ref(p_);
		}
	}
	template<class U>
	intrusive_weak_ptr(const intrusive_weak_ptr<U>& other) : p_(0)
	{
		if (!other.expired())
		{
			p_ = other.raw();
			boost::intrusive_ptr_add_weak_ref(p_);
		}
	}
	template<class U>
	intrusive_weak_ptr(const boost::intrusive_ptr<U>& strong) : p_(strong.get())
	{
		if (p_ != 0)
			boost::intrusive_ptr_add_weak_ref(p_);
	}

	~intrusive_weak_ptr()
	{
		if (p_ != 0)
			boost::intrusive_ptr_weak_release(p_);
	}

	template<class U>
	intrusive_weak_ptr& operator=(T* p) { reset(p); return *this; }
	template<class U>
	intrusive_weak_ptr& operator=(const boost::intrusive_ptr<U>& strong) { reset(strong.get()); return *this; }
	intrusive_weak_ptr& operator=(const rbx::intrusive_weak_ptr<T>& other) { reset(other.raw()); return *this; }
	template<class U>
	intrusive_weak_ptr& operator=(const rbx::intrusive_weak_ptr<U>& other) { reset(other.raw()); return *this; }

	void reset()
	{
		if (p_ != 0)
		{
			boost::intrusive_ptr_weak_release(p_);
			p_ = 0;
		}
	}
	// Drops the old target first, so resetting to the same object needs it kept alive elsewhere.
	void reset(T* p)
	{
		if (p_ != 0)
			boost::intrusive_ptr_weak_release(p_);
		p_ = p;
		if (p_ != 0)
			boost::intrusive_ptr_add_weak_ref(p_);
	}

	// A strong pointer, or null once the target is gone.
	boost::intrusive_ptr<T> lock() const
	{
		if (p_ && boost::intrusive_ptr_try_lock(p_))
			return boost::intrusive_ptr<T>(p_, false); // try_lock already counted it
		else
			return boost::intrusive_ptr<T>();
	}

	bool expired() const { return !p_ || boost::intrusive_ptr_expired(p_); }

	// The stored pointer, alive or not.
	T* raw() const { return p_; }
};

}

// Base/include/rbx/make_shared.h: October 2013 reconstruction (Project OGRE).
// boost 1.53 has its own make_shared, but the 2013 engine used a private copy in namespace rbx:
// PLAYER RTTI names rbx::detail::sp_ms_deleter<T> for std::vector<Reflection::Variant>,
// Reflection::Tuple and the Variant unordered_map, each inside a
// boost::detail::sp_counted_impl_pd<T*, rbx::detail::sp_ms_deleter<T>>. So the object and its
// count block share one allocation: the deleter holds aligned raw storage plus a flag, the object
// is placement-new'ed into it, and an aliasing shared_ptr points at it.
// The deleter's two members (flag, then storage) are part of every count block's layout and are
// kept in that order. The allocator overload and the rvalue-forwarding helper are dropped:
// nothing calls them and VC9 has no rvalue references.
#pragma once
#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/type_with_alignment.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <cstddef>
#include <new>

namespace rbx {
namespace detail {

// N bytes aligned like A.
template<std::size_t N, std::size_t A>
struct sp_aligned_storage
{
	union type
	{
		char data_[N];
		typename boost::type_with_alignment<A>::type align_;
	};
};

// Deleter that also owns the object's storage; destroys the object at most once.
template<class T>
class sp_ms_deleter
{
	typedef typename sp_aligned_storage<sizeof(T), ::boost::alignment_of<T>::value>::type storage_type;

	bool initialized_;
	storage_type storage_;

	void destroy()
	{
		if (initialized_)
		{
			reinterpret_cast<T*>(storage_.data_)->~T();
			initialized_ = false;
		}
	}

public:
	sp_ms_deleter() : initialized_(false) {}
	// shared_ptr copies the deleter into the count block before anything is built in it, so a
	// copy never needs the storage's contents
	sp_ms_deleter(sp_ms_deleter const&) : initialized_(false) {}
	~sp_ms_deleter() { destroy(); }

	void operator()(T*) { destroy(); }

	void* address() { return storage_.data_; }
	void set_initialized() { initialized_ = true; }
};

}

// Body shared by every arity: an empty owner carrying the deleter, the object built in the
// deleter's storage, then an aliasing pointer that shares the owner's count. Does not hook up
// enable_shared_from_this.
#define RBX_MAKE_SHARED_BODY(CTOR_ARGS) \
	boost::shared_ptr<T> owner(static_cast<T*>(0), rbx::detail::sp_ms_deleter<T>()); \
	rbx::detail::sp_ms_deleter<T>* deleter = boost::get_deleter<rbx::detail::sp_ms_deleter<T> >(owner); \
	void* where = deleter->address(); \
	::new (where) T CTOR_ARGS; \
	deleter->set_initialized(); \
	T* object = static_cast<T*>(where); \
	return boost::shared_ptr<T>(owner, object)

template<class T>
boost::shared_ptr<T> make_shared()
{
	RBX_MAKE_SHARED_BODY(());
}

template<class T, class A1>
boost::shared_ptr<T> make_shared(A1 const& a1)
{
	RBX_MAKE_SHARED_BODY((a1));
}

template<class T, class A1, class A2>
boost::shared_ptr<T> make_shared(A1 const& a1, A2 const& a2)
{
	RBX_MAKE_SHARED_BODY((a1, a2));
}

template<class T, class A1, class A2, class A3>
boost::shared_ptr<T> make_shared(A1 const& a1, A2 const& a2, A3 const& a3)
{
	RBX_MAKE_SHARED_BODY((a1, a2, a3));
}

#undef RBX_MAKE_SHARED_BODY

}

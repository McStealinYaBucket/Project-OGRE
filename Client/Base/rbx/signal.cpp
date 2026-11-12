/* Base/rbx/signal.cpp -- October 2013 (first import, section 243).
   Evidence: rbx::signals::connection::disconnect 0x14A06A + connected
     0x14A0CC + operator== 0x14A134 + operator!= 0x14A1CC + operator=
     0x14A264 (batch444). The lock-strong-operate-release shapes are the
     intrusive_weak_ptr::lock() temporaries spelled naturally (lock()
     performs the same cmpxchg acquire the binary inlines); operator=
     is the weak-ptr member assignment.
   Original file: (2013 TU layout TODO; methods declared in
   Base/include/rbx/signal.h, previously undefined in-tree). */

#include "rbx/signal.h"

namespace rbx { namespace signals {

	void connection::disconnect() const
	{
		boost::intrusive_ptr<islot> s(weak_slot.lock());
		if (s != NULL)
			s->disconnect();
	}

	bool connection::connected() const
	{
		boost::intrusive_ptr<islot> s(weak_slot.lock());
		return s != NULL && s->connected();
	}

	bool connection::operator==(const connection& other) const
	{
		boost::intrusive_ptr<islot> a(weak_slot.lock());
		boost::intrusive_ptr<islot> b(other.weak_slot.lock());
		return a == b;
	}

	bool connection::operator!=(const connection& other) const
	{
		return !(*this == other);
	}

	connection& connection::operator=(const connection& con)
	{
		weak_slot = con.weak_slot;
		return *this;
	}

} }

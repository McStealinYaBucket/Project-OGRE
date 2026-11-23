// Base/include/rbx/Thread.hpp: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbols RBX::thread_wrapper(boost::function0<void> const&, char const*),
// RBX::set_thread_name(char const*), and the RTTI of bind_t<void, void(*)(shared_ptr<
// worker_thread::data>, function0<work_result> const&), ...> (the thread entry) plus
// sp_counted_impl_p<worker_thread::data>. Bodies in Base/rbx/boost.cpp.
#pragma once

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

namespace RBX {

// Wraps threadfunc so the new thread names itself before running it.
boost::function0<void> thread_wrapper(const boost::function0<void>& threadfunc, const char* name);

void set_thread_name(const char* name);

// The name given through the functions above, if any.
const char* get_thread_name();

// Runs work_function on its own thread: once at start, again at once whenever it answers `more`,
// and again after each wake(). Destroying the object asks the thread to stop but does not interrupt
// a call in progress, so the function's data must outlive that call.
class worker_thread : public boost::noncopyable
{
	struct data
	{
		boost::mutex sync;
		boost::condition_variable_any wakeCondition;
		bool endRequest;

		data() : endRequest(false) {}
	};

	boost::shared_ptr<data> _data;
	boost::thread t;

public:
	enum work_result { done, more };

	worker_thread(const boost::function0<work_result>& work_function, const char* name);
	~worker_thread();

	void wake();
	void join(); // request the stop, then wait for the thread

private:
	static void threadProc(boost::shared_ptr<data> data, const boost::function0<work_result>& work_function);
};

}

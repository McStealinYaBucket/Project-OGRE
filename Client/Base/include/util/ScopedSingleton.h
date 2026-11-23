// Base/include/util/ScopedSingleton.h: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbols ScopedSingleton<T>::safe_static_do_get_sync() and
// safe_static_do_get_s_instance() with their guard variables (ProfanityFilter, Ogre::RootManager),
// i.e. two SAFE_STATIC members named sync and s_instance; CProcessPerfCounter derives from it.
// The instance lives while someone holds a shared_ptr to it and is recreated on the next request.
// 2016's getInstanceOptional has no caller here and no 2013 symbol; dropped.
#pragma once

#include "boost/weak_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "rbx/Debug.h"
#include "rbx/threadsafe.h"

namespace RBX {

template<class T>
class ScopedSingleton
{
	static int& initCount()
	{
		static int initcount = 0;
		return initcount;
	}

protected:
	// How many times an instance has been created (ProfanityFilter checks it).
	static int getInitCount()
	{
		return initCount();
	}

	SAFE_STATIC(rbx::spin_mutex, sync)
	SAFE_STATIC(boost::weak_ptr<T>, s_instance)

public:
	static boost::shared_ptr<T> getInstance()
	{
		rbx::spin_mutex::scoped_lock guard(sync());

		boost::shared_ptr<T> live = s_instance().lock();
		if (!live)
		{
			initCount()++;
			live = boost::shared_ptr<T>(new T());
			s_instance() = live;
		}
		return live;
	}
};

}

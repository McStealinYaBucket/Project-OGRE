// Base/include/rbx/GlobalVectorItem.h: October 2013 reconstruction (Project OGRE).
// Hands out free slots of a fixed static array, falling back to the heap when it is full. No 2013
// binary names it and nothing in this tree includes it; kept for later imports, written here.
#pragma once

#include <cstddef>

namespace RBX {

// T derives from this so a slot can be marked in use.
struct GlobalVectorItemBase
{
	bool valid;
	GlobalVectorItemBase() : valid(false) {}
};

template<class T>
class GlobalVectorItemPtr
{
	T* p;     // the slot or heap object in use
	T* newp;  // set only when p came from the heap

public:
	GlobalVectorItemPtr(T* slots, size_t count) : p(0), newp(0)
	{
		for (T* end = slots + count; slots != end; ++slots)
			if (!slots->valid)
			{
				slots->valid = true;
				p = slots;
				return;
			}
		p = newp = new T();
	}

	~GlobalVectorItemPtr()
	{
		if (newp)
			delete newp;
		else
			p->valid = false;
		p = newp = 0;
	}

	T* operator->() { return p; }
	T* get() { return p; }
};

}

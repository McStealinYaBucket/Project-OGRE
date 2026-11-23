/* Util/Handle.h -- October 2013 reconstruction.
   InstanceHandle: a shared reference to a Reflection::DescribedBase, the
   value type behind XML Ref properties (XmlElement stores one per referent).
   iOS 10-29 has the out-of-line members InstanceHandle(DescribedBase*),
   empty() const, linkTo(shared_ptr<DescribedBase>) and operatorLess; the
   comparison operators forward to the protected operatorEqual/Less/Greater
   helpers that App/util/Handle.cpp defines. */
#pragma once

#include <functional>
#include <algorithm>
#include <string>

#include "rbx/Debug.h"
#include "Util/Object.h"
#include "Util/Memory.h"

namespace RBX {

namespace Reflection { class DescribedBase; }

class InstanceHandle
{
	typedef shared_ptr<Reflection::DescribedBase> Target;
	Target target;

public:
	InstanceHandle() {}
	InstanceHandle(Reflection::DescribedBase* target);
	InstanceHandle(Target t);	// out of line in 2013 (PLAYER 0x007a8a20, called by MemoryBinder::processID)
	InstanceHandle(const InstanceHandle& rhs) : target(rhs.target) {}

	InstanceHandle& operator=(const InstanceHandle& rhs)
	{
		target = rhs.target;
		return *this;
	}
	InstanceHandle& operator=(Target t)
	{
		target = t;
		return *this;
	}

	bool empty() const;
	Target getTarget() const { return target; }
	void linkTo(Target t);

	bool operator==(const InstanceHandle& rhs) const { return operatorEqual(rhs); }
	bool operator!=(const InstanceHandle& rhs) const { return !operatorEqual(rhs); }
	bool operator<(const InstanceHandle& rhs) const { return operatorLess(rhs); }
	bool operator>(const InstanceHandle& rhs) const { return operatorGreater(rhs); }

protected:
	bool operatorEqual(const InstanceHandle& rhs) const;
	bool operatorLess(const InstanceHandle& rhs) const;
	bool operatorGreater(const InstanceHandle& rhs) const;
};

} // namespace RBX

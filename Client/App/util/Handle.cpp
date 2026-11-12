/* App/util/Handle.cpp — October 2013 (bodies for Util/Handle.h).
   InstanceHandle ctor/empty/linkTo follow March 2016 App/util/Handle.cpp verbatim.
   operatorEqual/Less/Greater compare the shared_ptr targets (needed by
   MemoryBinder's std::map<InstanceHandle> in SerializerV2.cpp; 2016 leaves them
   to the linker — ours are defined here). */

#include "stdafx.h"

#include "Util/Handle.h"
#include "Util/Utilities.h"
#include "RbxAssert.h"
#include <limits>
#include "boost/lexical_cast.hpp"
#include "Reflection/Object.h"

namespace RBX {

	InstanceHandle::InstanceHandle(Reflection::DescribedBase* target)
		:target(shared_from(target))
	{
	}

	bool InstanceHandle::operatorEqual(const InstanceHandle& other) const
	{
		return target == other.target;
	}

	bool InstanceHandle::operatorLess(const InstanceHandle& other) const
	{
		return target < other.target;
	}

	bool InstanceHandle::operatorGreater(const InstanceHandle& other) const
	{
		return other.target < target;
	}

	bool InstanceHandle::empty() const {
		return !target;
	}

	void InstanceHandle::linkTo(shared_ptr<Reflection::DescribedBase> target) {
		this->target = target;
	}



}	// end namespace RBX

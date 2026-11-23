/* Util/HeapValue.h -- October 2013 reconstruction.
   No 2013 trace (see Util/ObscureValue.h): DataModel.h records that the 2016
   HeapValue members are not in the 2013 binary, and the 2013
   ProgramMemoryChecker is an interface without them. Kept as a header only;
   nothing in the build instantiates it.

   Holds a T on the heap inside an ObscureValue and reads like a plain T. */
#pragma once

#include <boost/scoped_ptr.hpp>
#include "ObscureValue.h"
#include "FastLog.h"

namespace RBX {

template<typename T>
class HeapValue
{
	typedef ObscureValue<T> Cell;
	boost::scoped_ptr<Cell> cell;

public:
	explicit HeapValue(const T& v)
		: cell(new Cell(v))
	{
	}

	operator const T() const { return *cell; }

	HeapValue& operator=(const T& v)
	{
		*cell = v;
		return *this;
	}

private:
	HeapValue();
	HeapValue(const HeapValue&);
	HeapValue& operator=(const HeapValue&);
};

} // namespace RBX

/* V8Kernel/KernelIndex.h -- October 2013 (kernel list index base).
   2016 shape (first-party) + ONE 2013 DELTA: the 2013 hierarchy is
   polymorphic (Point C2 0x532AB8 writes a vptr at +0), so the dtor is
   virtual here (2016's is not). kernelIndex at +4 in both (C2 -1).
   Original file: Client/App/include/V8Kernel/KernelIndex.h (path from
   2016 + Point.cpp assert strings). */

#pragma once

#include "rbx/Debug.h"

namespace RBX {

	class Kernel;
	class KernelData;

	class KernelIndex {
		friend class Kernel;
		friend class KernelData;

	protected:
		int kernelIndex;

	public:
		bool indexInKernel() const {
			return (kernelIndex != -1);
		}

		KernelIndex() : kernelIndex(-1)
		{}

		virtual ~KernelIndex() {
			RBXASSERT(!indexInKernel());
		}
	};

} // namespace RBX

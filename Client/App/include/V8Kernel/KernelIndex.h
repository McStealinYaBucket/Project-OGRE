/* V8Kernel/KernelIndex.h -- October 2013 (kernel list index base). 2016 shape + one 2013 delta: polymorphic (Point C2
   0x532AB8 writes a vptr at +0), so the dtor is virtual here. kernelIndex at +4 (C2 -1). Path from Point.cpp asserts. */
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

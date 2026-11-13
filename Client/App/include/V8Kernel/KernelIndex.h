/* V8Kernel/KernelIndex.h -- October 2013 (kernel list index base). 2016 shape: not polymorphic. Point's RTTI places
   KernelIndex at +4 (Base Class Descriptor ??_R13?0A@EA@KernelIndex PLAYER 0x00c68138, "at (4,-1,0,64)") behind
   Point's own vptr, and Point's deleting destructor (PLAYER 0x00978890) stores only Point's vftable before the
   inlined ~KernelIndex assert. kernelIndex at +0 here (+4 in Point). Path from Point.cpp asserts. */
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
		~KernelIndex() {
			RBXASSERT(!indexInKernel());
		}
	};

} // namespace RBX

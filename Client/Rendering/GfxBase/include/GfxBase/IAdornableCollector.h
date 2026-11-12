/* GfxBase/IAdornableCollector.h -- October 2013 (minimal render-collector
   interface, reconstructed for the JointsService adornable pair s521+).
   Evidence: JointsService::onDescendantAdded 0x418EFE calls
     IAdornableCollector::onRenderableDescendantAdded(IAdornable*)
     0x8294AE; ::onDescendantRemoving 0x418EA6 calls
     ...::onRenderableDescendantRemoving 0x829624 (batch625).
   Only these two slots are evidenced; further vtable layout is TODO.
   Bodies live in Rendering/GfxBase (outside this build) -- pure here
   so test doubles implement them. No virtual dtor (RBXInterface rule). */

#pragma once

#include "rbx/Declarations.h"

namespace RBX {

class IAdornable;

class RBXInterface IAdornableCollector
{
public:
	virtual void onRenderableDescendantAdded(IAdornable* adornable) = 0;
	virtual void onRenderableDescendantRemoving(IAdornable* adornable) = 0;
};

} // namespace RBX

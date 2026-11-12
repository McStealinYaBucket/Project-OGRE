/* App/v8datamodel/JointsService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/JointsService.h"

namespace RBX {
	const char* const sJointsService = "JointsService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::JointsService);
}

namespace RBX {

// mac: 0x418EFE (batch625): Instance::onDescendantAdded FIRST (direct --
// skips the ServiceProvider service wiring by binary design), then
// dynamic_cast<IAdornable> the child (null-skipped) and forward hits
// to the +0x74 collector. No null checks anywhere (binary-identical:
// a hit with no collector AVs).
// STUB: PLAYER 0x007ff300
// STUB: STUDIO 0x008ee4d0
void JointsService::onDescendantAdded(Instance* descendant)
{
	Instance::onDescendantAdded(descendant);
	if (descendant != NULL)
	{
		if (IAdornable* adornable = dynamic_cast<IAdornable*>(descendant))
			adornableCollector->onRenderableDescendantAdded(adornable);
	}
}

// mac: 0x418EA6 (batch625): dynamic_cast<IAdornable> the child
// (null-skipped), forward hits to the +0x74 collector, then TAIL-JUMP
// Instance::onDescendantRemoving (skips the ServiceProvider wiring).
// STUB: PLAYER 0x007ff2c0
// STUB: STUDIO 0x008ee490
void JointsService::onDescendantRemoving(const shared_ptr<Instance>& descendant)
{
	if (descendant.get() != NULL)
	{
		if (IAdornable* adornable = dynamic_cast<IAdornable*>(descendant.get()))
			adornableCollector->onRenderableDescendantRemoving(adornable);
	}
	Instance::onDescendantRemoving(descendant);
}

} // namespace RBX

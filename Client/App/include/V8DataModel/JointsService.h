/* V8DataModel/JointsService.h -- October 2013 (adornable pair 0x418EA6/0x418EFE, s521+).
   Evidence: Added 0x418EFE = Instance::onDescendantAdded first (DIRECT,
     skips the ServiceProvider service wiring by binary design), then
     dynamic_cast<IAdornable> the child and forward hits to the +0x74
     collector's onRenderableDescendantAdded 0x8294AE; Removing 0x418EA6
     = forward onRenderableDescendantRemoving 0x829624 on hits, then
     TAIL-JUMP Instance::onDescendantRemoving (batch625).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/JointsService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "GfxBase/IAdornable.h"
#include "GfxBase/IAdornableCollector.h"

namespace RBX {

	extern const char* const sJointsService;

	class JointsService
		: public DescribedNonCreatable<JointsService, Instance, sJointsService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		JointsService() : adornableCollector(NULL) {}

		// Render-side collector for adornable descendants. Set by the
		// render engine (writer unmapped -- TODO); NULL until set. The
		// binary never null-checks the slot, neither do the overrides
		// (a hit with no collector AVs binary-identically).
		void setAdornableCollector(IAdornableCollector* collector) { adornableCollector = collector; }

		/*override*/ void onDescendantAdded(Instance* descendant);
		/*override*/ void onDescendantRemoving(const shared_ptr<Instance>& descendant);

	private:
		// Binary +0x74 role (offset model-relative here).
		IAdornableCollector* adornableCollector;
	};

} // namespace RBX

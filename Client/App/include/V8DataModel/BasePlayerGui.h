/* V8DataModel/BasePlayerGui.h -- October 2013.
   Layout from the PLAYER ctor 0x0070b480 and dtor 0x00708720 (RTTI of BasePlayerGui, vftable 0x00b47eac):
   Instance (0x60), IScriptFilter at 0x60 and GuiTarget at 0x64 (each a vptr), a bool at 0x68 that
   GuiTarget::process 0x0070a090 clears and sets (2016 name mouseWasOverGui), then the
   shared_ptr<IAdornableCollector> at 0x6c (the ctor's operator new(0x30)). Size 0x74: CoreGuiService's
   Service base sits at 0x74 (RTTI mdisp 116).
   Primary vftable: 29 slots, Instance's 28 with askAddChild (vtable+0x3c, folded `mov al,1; ret 4` at
   0x006ceaa0) and onDescendantAdded/Removing overridden, plus the new virtual append3dSortedAdorn at +0x70.
   IAdornableCollector is GfxBase's (Client/Rendering) and is only forward-declared there, so the bodies
   that call into it (ctor, onDescendantAdded/Removing, append3dSortedAdorn) stay STUB.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BasePlayerGui.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8DataModel/IScriptFilter.h"
#include "Gui/GuiEvent.h"
#include "GfxBase/IAdornable.h"

#include <vector>

namespace RBX {

	extern const char* const sBasePlayerGui;

	// VTABLE: PLAYER 0x00b47eac RBX::Debugable
	// VTABLE: STUDIO 0x00bfd1bc RBX::Debugable
	// SIZE 0x74
	class BasePlayerGui
		: public DescribedNonCreatable<BasePlayerGui, Instance, sBasePlayerGui>
		, public IScriptFilter
		, public GuiTarget
	{
	private:
		typedef DescribedNonCreatable<BasePlayerGui, Instance, sBasePlayerGui> Super;

		bool mouseWasOverGui;	// 0x68

	public:
		BasePlayerGui();
		~BasePlayerGui();

		virtual void append3dSortedAdorn(std::vector<IAdornable*>& sortedAdorn, const Camera* camera) const;	// vtable+0x70

		/*override*/ bool askAddChild(const Instance* instance) const { return true; }
		/*override*/ bool scriptShouldRun(BaseScript* script);
		/*override*/ void onDescendantAdded(Instance* instance);
		/*override*/ void onDescendantRemoving(const shared_ptr<Instance>& instance);

	protected:
		shared_ptr<IAdornableCollector> adornableCollector;	// 0x6c
	};

} // namespace RBX

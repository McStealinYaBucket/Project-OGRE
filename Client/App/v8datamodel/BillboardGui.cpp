/* App/v8datamodel/BillboardGui.cpp -- October 2013 STUB -> adornee chain landed.
   Source of truth: symbolled Mach-O Studio binary (0.126.0.39503), EAs noted per function.
   Self-audit: setAdornee 0x283354 (700B incl. the unwinder coda) and
   getAdorneeDangerous 0x284D16 (56B) landed from raw asm.  DELTAS:
   (1) the raisePropertyDescriptor object in the binary sits at 0xFAFD38 whose
   storage reads as a relocated (unloaded) block in dbq24 -- the landed registers
   the named "Adornee" ref property with the landed RefPropDescriptor machinery
   (method pair per the setAdornee/getAdorneeDangerous mangles; category_Data
   carried from the landed corpus convention, category unread); (2) the
   setAdornee tail call rcx=this+0x60 -> IAdornable::shouldRenderSetDirty
   (0x828E16: *collector(+16) -> recomputeShouldRender) is PARKED -- the
   IAdornable subobject is unported and the call is render-side only;
   (3) C2 0x283B18 member defaults unpoured (render arcs era). */

#include "stdafx.h"

#include "V8DataModel/BillboardGui.h"
#include "V8DataModel/ModelInstance.h"
#include "V8DataModel/PartInstance.h"
#include "Util/Object.h"
namespace RBX {
	const char* const sAdornmentGui = "BillboardGui";
}

using namespace RBX;

// "Adornee" ref property: the binary descriptor sits at 0xFAFD38 (raised from
// setAdornee 0x283559); its storage is a relocated block in dbq24 -- landed
// registers the named pair per the mangled method set.  DELTAs: category and
// attribute bits unread.
static Reflection::RefPropDescriptor<BillboardGui, Instance> prop_Adornee(
	"Adornee", category_Data, &BillboardGui::getAdorneeDangerous, &BillboardGui::setAdornee);

// BillboardGui::setRenderFunction 0x283DA0 (34B, mangle
// __ZN3RBX12BillboardGui17setRenderFunctionEN5boost8functionIFvPS0_PNS_5AdornEEEE):
// the by-value boost function body op= into the +0xC4 member (the
// binary's two-step = the GCC sret of an op= job -- the landed direct
// member assign bakes the same observable; the caller's temp-function
// clear() after the call = the GCC temp dtor tail, replaced by the
// natural C++ scope end here).  MSVC spelling note: the out-of-class
// definition sits at file scope, where the global-namespace ::Adorn
// (GfxBase/IAdornable.h:26 tree-convention fwd decl) and RBX::Adorn are
// both visible -- the parameter type is spelled RBX::Adorn to pin the
// class-space board (the mangle's PNS_5Adorn = RBX::Adorn).
void BillboardGui::setRenderFunction(boost::function<void (BillboardGui*, RBX::Adorn*)> renderFunction)
{
	this->renderFunction = renderFunction;
}

// BillboardGui::setAdornee 0x283354 (700B incl. the coda): mangle
// __ZN3RBX12BillboardGui10setAdorneeEPNS_8InstanceE.
//   mac: 0x283369  shared_ptr<Instance>(weak @+240, sp_nothrow) = the OLD adornee
//             lock probe; 0x283393 `cmp old.px, inst; jz ret` -- equal (incl.
//             both-null) returns with NO property raise.
//   mac: 0x28339E-3A3  the null-inst path jumps straight to the generic clear
//             (0x283517): *+240 = NULL + the weak count emptied; the model
//             and part slots are LEFT AS-IS on this path.
//   mac: 0x2833AC-C1  non-null inst: desc = *(shadow +12) = the cached Instance
//             descriptor read (landed getDescriptor()); isA(ModelInstance
//             desc) -> 0x2833D9 arm: shared_from<ModelInstance>(inst) ->
//             *+224 = px, weak_count(+228) op=(shared) -- landed weak
//             operator=(shared_ptr); then +232 px = NULL (0x283426) and the
//             +236 count emptied (0x283440 weak_count op=(empty)) landed
//             adorneePart.reset() (the inlined last-ref destroy dance at
//             mac: 0x283445-14 covers the old part ref).
//   mac: 0x28345F-F7  the Part arm: isA(PartInstance desc) -> shared_from
//             <PartInstance> -> *+232 = px, weak_count(+236) op=(shared);
//             then the +224 px = NULL (0x2834DA) and the +228 count emptied
//             (0x2834F4) -- landed adorneeModel.reset().
//   mac: 0x283517-4D  the generic arm ALWAYS runs (even the null-inst path):
//             shared_from<Instance>(inst) -> *+240 = px, weak_count(+244)
//             op=(shared); release of the arm temporaries is the landed
//             local shared dtor (none of these touch the member pair).
//   mac: 0x283559  raisePropertyChanged(the Adornee PropertyDescriptor);
//   mac: 0x28356E  (this+0x60) -> IAdornable::shouldRenderSetDirty() -- PARKED
//             (see the header DELTAs).
void BillboardGui::setAdornee(Instance* inst)
{
	shared_ptr<Instance> old = adornee.lock();

	if (old.get() == inst)
		return;

	if (inst != NULL)
	{
		if (inst->getDescriptor().isA(ModelInstance::classDescriptor()))
		{
			adorneeModel = shared_from(static_cast<ModelInstance*>(inst));
			adorneePart.reset();
		}
		else if (inst->getDescriptor().isA(PartInstance::classDescriptor()))
		{
			adorneePart = shared_from(static_cast<PartInstance*>(inst));
			adorneeModel.reset();
		}
	}
	// the null-inst path (0x28339E/0x2833A3 jz) skips BOTH probes: only the
	// generic arm below runs; the model and part slots are left as-is.

	adornee = shared_from(inst);

	raisePropertyChanged(prop_Adornee);
}

// BillboardGui::getAdorneeDangerous 0x284D16 (56B): landed inline in the
// header (the shared(weak +240, sp_nothrow) + px-return shape = lock() + .get()).

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::BillboardGui);
}

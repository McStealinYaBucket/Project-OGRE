/* V8DataModel/BillboardGui.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BillboardGui, GuiLayerCollector, sAdornmentGui>; s-var VALUE BillboardGui PE-proven (AdornmentGui x00 absent);
     sBillboardGui = "BillboardGui".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BillboardGui.h */

#pragma once

#include "V8DataModel/GuiLayerCollector.h"
#include "V8Tree/Instance.h"
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>

namespace RBX {

	extern const char* const sAdornmentGui;

	class Adorn;
	class ViewportBillboarder;

	class BillboardGui
		: public DescribedCreatable<BillboardGui, GuiLayerCollector, sAdornmentGui>
	{
	public:
		// C2 0x283B18 (647B; rarc15): member defaults pinned across the
		// +0xA8..+0xF4 span -- viewportBillboarder = NULL then operator
		// new(0x94) ViewportBillboarder stored via the dword-44 slot
		// (283C6F..283CA2; ours STAYS NULL until ViewportBillboarder
		// lands -- DELTA); windowRect = all-zero (283BD4..283C06);
		// renderFunction.px = 0 (the empty boost::function -- its
		// clear() in the C2's own unwind path re-pins the +0xC4 type);
		// word +0xD4 = 1 (283C10); the adornee weak trio = empty weak
		// pairs (the ctor's weak-zero path = the landed default ctors
		// -- behavior identical); the +0xD8..+0xF4 shared_ptr region
		// zero (released again in the same C2's unwind block).
		// DELTAs: the word +0xD4 (Active-style), the +0xD8..+0xF4 sps
		// and the size/extents/pixels-per-stud defaults land with the
		// render arcs.
		BillboardGui()
			: viewportBillboarder(NULL)
			, windowRect(G3D::Rect2D::xyxy(0.0f, 0.0f, 0.0f, 0.0f))
		{
		}

		// +0xB0 (rarc15 C2): the owned viewport billboarder --
		// operator new(0x94) + ViewportBillboarder C1, stored by the
		// dword-44 slot with the old value deleted when replaced.
		// Forward-declared; its creation lands when the class does.
		ViewportBillboarder* viewportBillboarder;  // +0xB0

		// +0xB4..+0xC0 (rarc15 C2 zero-inits 283BD4..283C06): the
		// billboard's own window rect -- GuiObject::getWindowRect
		// mac: 0x3ABA9A copies these four dwords VERBATIM into its Rect2D
		// sret on the billboard branch (raw 3ABB4F..3ABB7D).  All-zero
		// default; the billboard sizes itself elsewhere.
		G3D::Rect2D windowRect;  // +0xB4..+0xC0

		// +0xC4 (per the setRenderFunction 0x283DA0 mangle
		// ...5AdornEEEE and the render2d_bubbleStyle arc): the render
		// function slot, member BEFORE the adornee trio (binary order;
		// MSVC DELTA -- order the invariant).
		boost::function<void (BillboardGui*, Adorn*)> renderFunction;  // +0xC4

		// mac: 0x283DA0 (34B; mangle
		// ...17setRenderFunctionEN5boost8functionIFvPS0_PNS_5AdornEEEE =
		// the by-value parameter): the op= into the +0xC4 member (the
		// binary's two-step = the GCC sret of an op= job -- same member
		// assign observable; the empty-boost-function visits from the
		// ChatOutput pass-top are the no-render reset).
		void setRenderFunction(boost::function<void (BillboardGui*, Adorn*)> renderFunction);

		// setAdornee 0x283354 (700B) decodes THREE 8-byte weak pairs
		// (px/count each).  +224 = the ModelInstance adornee, +232 = the
		// PartInstance adornee, +240 = the generic Adornee property pair
		// (getAdorneeDangerous 0x284D16 locks it; both property accessors
		// route through +240 only).  Landed member names DELTA
		// placeholders (behavior-pinned); types are value-identical
		// wide-weak pairs (the binary stores shared_from<T> px + a
		// weak_count bump -- landed reads the same 8B weak pair).
		weak_ptr<Instance> adorneeModel;   // +224 (count @+228)
		weak_ptr<Instance> adorneePart;    // +232 (count @+236)
		weak_ptr<Instance> adornee;        // +240 (count @+244)

		// mac: 0x283354 (700B; mangle ...10setAdorneeEPNS_8InstanceE): the
		// old +240 lock != inst; Model arm -> +224 set & +232 reset;
		// Part arm -> +232 set & +224 reset; the +240 weak always
		// assigned; raisePropertyChanged(Adornee) every non-early-out.
		void setAdornee(Instance* inst);

		// mac: 0x284D16 (56B): shared(weak +240, sp_nothrow) -> return px
		// (landed lock()); "Dangerous" = the unlocked getter.
		Instance* getAdorneeDangerous() const { return adornee.lock().get(); }
	};

} // namespace RBX

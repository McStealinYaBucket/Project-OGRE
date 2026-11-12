/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

#pragma once

#include "G3D/Rect2D.h"
#include "G3D/Vector3.h"
#include "SelectState.h"
#include "rbx/Declarations.h"

// Carried from March 2016 Rendering/GfxBase/include/GfxBase/IAdornable.h
// (first-party; member layout + all 12 virtual defaults match the 0xF4EB00
// vtable: shouldRender*2d/3d, isVisible, render*Background2d(+Context),
// render*2d(+Context), render*3dAdorn/Sorted/Select, render3dSortedPosition).
// DELTAS vs 2016 (all marked): no Selectable base (module unmapped — layout
// note: binary stores IAdornable vtable at PartInstance+108 with -1/-1/-1/0
// after, matching our ctor below); Instance/Adorn/Camera forward-declared
// (no V8Tree include — keeps GfxBase decoupled); shouldRenderSetDirty is
// NULL-guarded inline (collector absent from tree; binary calls
// collector->recomputeShouldRender only when bucket != NULL — identical
// observable while no collector exists); calculateDepth is a TODO stub
// (body unmapped, uncalled).
// Evidence: IAdornable D2 0x828C5E (:19-:22 asserts) / shouldRenderSetDirty
// mac: 0x828E16 (33B, non-virtual) / C2 stores (via PartInstance C2 0x4438BB-DC).
//
// ZTV @0xF4EB00 slot pins (rarc13 slot-mangle census + rarc14 default-body
// raws, October-2013 DB): slot 0 shouldRender2d 0x46D530 (7B xor-eax/ret);
// slot 1 shouldRender3dAdorn 0x4DF570 (7B ret-0); slot 2
// shouldRender3dSortedAdorn 0x382780 (7B ret-0); slot 3 isVisible 0x4DF580
// (10B al=1, rect unused); slot 4 renderBackground2d 0x44CA70 (5B);
// slot 5 renderBackground2dContext 0x4DF590 (14B tail-jmp [vptr+0x10] =
// slot 4, context dropped); slot 6 render2d 0x686DC0 (5B); slot 7
// render2dContext 0x4DDB90 (14B tail-jmp [vptr+0x18] = slot 6); slot 8
// render3dAdorn 0x4D85C0 (5B); slot 9 render3dSortedAdorn 0x345C20 (5B);
// slot 10 render3dSelect 0x3F7260 (5B, SelectState by value per the
// mangle); slot 11 render3dSortedPosition 0x32C330 (30B — sret stores
// THREE ZERO DWORDS, retn 4; G3D::Vector3 carries the zero shape).
// Every default body above = byte-checked against these RARC14 raws.
// CONSUMER MERGE LANDED (rarc17): V8DataModel/GuiBase2d now derives
// this class as its SECOND BASE (the +0x60 subobject); the merged
// GuiBase2d ZTV routes GuiBase2d::shouldRender2d 0x4C6960 (return 0)
// and GuiBase2d::isVisible 0x3F5500 (getRect2D strict rect-intersection)
// through the Thn96 adjustor rows — both faces share those final
// overriders; all other slots keep the defaults above.

namespace RBX {
	// The binary mangles EVERY adorn parameter here PNS_ (the 12 slot
	// mac: mangles: renderBackground2d 0x44CA70, render2d 0x686DC0,
	// render3dAdorn 0x4D85C0, render3dSortedAdorn 0x345C20,
	// render3dSelect 0x3F7260 all EPNS_5AdornE) AND the consumer arcs
	// re-pin it (GuiObject::legacyRender2d EPNS_5AdornE -- rarc18,
	// AdornBillboarder C1, ChatOutput::render*) = RBX::Adorn = the
	// Gui/Adorn.h class tree.  The 2016 carry's global-namespace spell
	// was mangle-untrue; the explicit RBX:: qualification below is
	// include-order immune.  Unported consumers (Workspace::
	// append3dSortedAdorn 0x516AB2, BasePlayerGui 0x45D31C,
	// StarterGuiService) hold the same face.
	class Adorn;
	class IAdornableCollector;
	class Camera;
	class Instance;

	class RBXInterface IAdornable
	{
	friend class IAdornableCollector;

	private:
		int index2d;
		int index3d;
		int index3dSorted;
		int& indexFunc2d() {return index2d;}
		int& indexFunc3d() {return index3d;}
		int& indexFunc3dSorted() {return index3dSorted;}

		IAdornableCollector* bucket;

	protected:
		// STUB: PLAYER 0x00927cc0
		// STUB: STUDIO 0x008db930
		virtual bool shouldRender2d() const {return false;}
		// STUB: PLAYER 0x00927cc0
		// STUB: STUDIO 0x008db930
		virtual bool shouldRender3dAdorn() const {return false;}
		// STUB: PLAYER 0x00927cc0
		// STUB: STUDIO 0x008db930
		virtual bool shouldRender3dSortedAdorn() const {return false;}

	public:
		IAdornable() : bucket(NULL), index2d(-1), index3d(-1), index3dSorted(-1)
		{}

		// mac: 0x828E16: NULL-guarded (see header note).
		// STUB: PLAYER 0x009ca750
		// STUB: STUDIO 0x006c6c80
		void shouldRenderSetDirty()
		{
			IAdornableCollector* c = bucket;
			if (c != NULL)
			{
				// TODO: c->recomputeShouldRender(this) (collector unported).
			}
		}

		// TODO: body (needs Camera depth math; unmapped, uncalled).
		float calculateDepth(const Camera* camera) const
		{
			(void)camera;
			return 0.0f;
		}

		// STUB: PLAYER 0x006ceaa0
		// STUB: STUDIO 0x00812790
		virtual bool isVisible(const G3D::Rect2D& rect) const { (void)rect; return true; }

		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void renderBackground2d(RBX::Adorn* adorn) { (void)adorn; }
		// FUNCTION: PLAYER 0x00575d10
		// FUNCTION: STUDIO 0x006125d0
		virtual void renderBackground2dContext(RBX::Adorn* adorn, const Instance* context) { (void)context; renderBackground2d(adorn); }
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void render2d(RBX::Adorn* adorn) { (void)adorn; }
		// FUNCTION: PLAYER 0x00575d20
		// FUNCTION: STUDIO 0x006125e0
		virtual void render2dContext(RBX::Adorn* adorn, const Instance* context) { (void)context; render2d(adorn); }
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void render3dAdorn(RBX::Adorn* adorn) { (void)adorn; }
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void render3dSortedAdorn(RBX::Adorn* adorn) { (void)adorn; }
		// STUB: PLAYER 0x008c8450
		// STUB: STUDIO 0x006125f0
		virtual void render3dSelect(RBX::Adorn* adorn, SelectState selectState) { (void)adorn; (void)selectState; }

		// FUNCTION: PLAYER 0x00575d30
		// FUNCTION: STUDIO 0x00612600
		virtual G3D::Vector3 render3dSortedPosition() const { return G3D::Vector3(0,0,0); }
	};

} // namespace RBX

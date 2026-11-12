/* App/v8datamodel/GuiBase2d.cpp — October 2013 (ctor shell).
   Evidence: GuiBase2d C2 0x6E4B32.
   Now also carries the resize arc: getChildRect2D 0x3F6380,
   notifyChildHandleResize (GCC-unnamed sub_6E501D) and
   GuiBase2d::handleResize 0x6E4F28 (all rarc12/13/14-raw-verified,
   comments carry the raw lines), and the RENDER HALF over the
   already-landed GfxBase/IAdornable.h second base: GuiBase2d::
   shouldRender2d 0x4C6960 (7B return-0 final overrider), isVisible
   0x3F5500 (84B strict rect-intersection), recursiveRender2d
   0x6E4E7E (NON-virtual; the +0x60 face slot-6 vcall then the
   read-bumped Instances walk) and RecursiveRenderChildren 0x6E4E26
   (88B static; no-receiver frame) -- rarc17-raw-verified, comments
   carry the raw lines.  TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GuiBase2d.h"
#include "V8Tree/Instance.h"
#include "Util/FastFlags.h"

namespace RBX {
	const char* const sGuiBase2d = "GuiBase2d";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiBase2d);
}

namespace RBX {

	typedef DescribedNonCreatable<GuiBase2d, GuiBase, sGuiBase2d, Reflection::ClassDescriptor::PERSISTENT> GuiBase2dSuper;

	// mac: 0x6E4B32: GuiBase(name); +30/+31/+32/+33 = 0; +34 = 1; +35 = 0.
	// (+30..+33 are absolutePosition/absoluteSize, zero origin/size.)
	GuiBase2d::GuiBase2d(const char* name)
		: GuiBase2dSuper(name)
		, absolutePosition(0.0f, 0.0f)
		, absoluteSize(0.0f, 0.0f)
		, unknown34(1)
		, unknown35(0)
	{
	}

	// mac: 0x6E4D56 (98B).  NaN-aware compare (NaN always stores, like the
	// GuiObject setters); raise skipped (see header DELTA).
	bool GuiBase2d::setAbsolutePosition(const G3D::Vector2& value)
	{
		if (absolutePosition == value)
			return false;
		absolutePosition = value;
		return true;
	}

	// mac: 0x6E4DB8 (110B).
	bool GuiBase2d::setAbsoluteSize(const G3D::Vector2& value)
	{
		if (absoluteSize == value)
			return false;
		absoluteSize = value;
		return true;
	}

	// mac: 0x6E4C4C (266B).
	// STUB: PLAYER 0x007bc5d0
	// STUB: STUDIO 0x008d9000
	bool GuiBase2d::recalculateAbsoluteSizeAndPosition(const G3D::Rect2D& rect)
	{
		// DEVIATION (toolchain): the binary uses lrintf (banker's); VS2012
		// has neither lrintf nor _lrintf, so G3D::iRound (half-away) stands
		// in.  Identical except on exact .5 fractions.
		G3D::Vector2 pos;
		G3D::Vector2 size;
		if (FFlag::RoundGuiAbsoluteSizeAndPosition)
		{
			pos = G3D::Vector2((float)G3D::iRound(rect.x0y0().x),
				(float)G3D::iRound(rect.x0y0().y));
			size = G3D::Vector2(
				(float)G3D::iRound(rect.x1y1().x - rect.x0y0().x),
				(float)G3D::iRound(rect.x1y1().y - rect.x0y0().y));
		}
		else
		{
			pos = rect.x0y0();
			size = rect.x1y1() - rect.x0y0();
		}
		bool moved = setAbsolutePosition(pos);
		bool resized = setAbsoluteSize(size);
		return moved || resized;
	}

	// mac: 0x6E50D6 (101B).  minss/maxss as ternaries (NaN-equivalent, proven at
	// Primitive::clipToSafeSize).
	// STUB: PLAYER 0x007bc7e0
	// STUB: STUDIO 0x008d9210
	G3D::Rect2D GuiBase2d::getRect2D() const
	{
		float x0 = absolutePosition.x + absoluteSize.x;
		float y0 = absolutePosition.y + absoluteSize.y;
		return G3D::Rect2D::xyxy(
			x0 < absolutePosition.x ? x0 : absolutePosition.x,
			y0 < absolutePosition.y ? y0 : absolutePosition.y,
			x0 > absolutePosition.x ? x0 : absolutePosition.x,
			y0 > absolutePosition.y ? y0 : absolutePosition.y);
	}

	// mac: 0x3F6380 (31B, ZTV slot 37; raw 3F6393 call __ZNK3RBX9GuiBase2d9
	// getRect2DEv tail with __stdcall retn 4 = the sret artifact):
	// getChildRect2D forwards to getRect2D, full stop.
	// STUB: PLAYER 0x006bd920
	// STUB: STUDIO 0x0083aa70
	G3D::Rect2D GuiBase2d::getChildRect2D() const
	{
		return getRect2D();
	}

	// sub_6E501D (101B): the GCC-UNNAMED static behind handleResize's
	// children walk (bound by the binary through boost._bi
	// list3<arg<1>, value<G3D::Rect2D>, value<bool>>; our direct static
	// call stages the same evaluation).  Raw 6E502E..6E5081: child.get()
	// null-check FIRST (6E502E test/jz-out), then the isA gate --
	// [child+0x30] descriptor (our landed getDescriptor(),
	// Reflection/object.h) isA(sGuiBase2d described,
	// ClassDescriptor::isA 0x157C0E) -- then the tail: esi -= 0x24
	// (chunk-crawl undo), [child vptr+0x98] vcall = child->
	// handleResize(rect, flag) -- slot 38, ZTV-proven (rarc13), i.e.
	// the recursion re-enters the OVERRIDING handle (GuiObject's when
	// the child is a GuiObject).
	static void notifyChildHandleResize(
		const boost::shared_ptr<Instance>& child,
		const G3D::Rect2D& rect, bool flag)
	{
		if (child.get() == NULL)
			return;
		if (!child->getDescriptor().isA(GuiBase2d::classDescriptor()))
			return;
		static_cast<GuiBase2d*>(child.get())->handleResize(rect, flag);
	}

	// mac: 0x6E4F28 (245B, __ZN3RBX9GuiBase2d12handleResizeERKN3G3D6Rect2DEb:
	// raw 6E4F31 sub esp,4Ch) -- ZTV slot 38 (rarc13).  Binary order:
	// vcall slot 39 (recalculate -- our now-virtual member) with the
	// pending rect; gate (changed || flag); slot-37 sret fills the
	// binder's value<G3D::Rect2D>; THEN the children pair read-bump
	// (lock-inc [ctrl+4] = read()'s landed shape) and the walk.  The
	// child receives the PARENT'S child-rect (not the incoming rect).
	// DEVIATIONS: the walk = our getChildren2() snapshot loop (same
	// px/ctrl semantics, rarc14-proven pair); the bind-call = the
	// direct static; the GCC EH tail = compiler-managed (not spelled).
	// STUB: PLAYER 0x007bce50
	// STUB: STUDIO 0x008d9800
	void GuiBase2d::handleResize(const G3D::Rect2D& rect, bool flag)
	{
		bool changed = recalculateAbsoluteSizeAndPosition(rect);
		if (changed || flag)
		{
			G3D::Rect2D childRect = getChildRect2D();
			boost::shared_ptr<const Instances> snapshot = getChildren2();
			if (snapshot)
			{
				for (Instances::const_iterator iter = snapshot->begin();
					iter != snapshot->end(); ++iter)
					notifyChildHandleResize(*iter, childRect, flag);
			}
		}
	}

	// ZTV slot 40 (code-keyed rarc13/rarc17, vptr+0xA0; mangle
	// __ZNK3RBX9GuiBase2d14shouldRender2dEv = CONST): 0x4C6960 (7B),
	// xor eax,eax; retn.  The final overrider of the IAdornable face
	// slot 0 -- the merged ZTV's face section routes the Thn96
	// adjustor here (rarc17 row 63).
	bool GuiBase2d::shouldRender2d() const
	{
		return false;
	}

	// ZTV slot 41 (vptr+0xA4; mangle
	// __ZNK3RBX9GuiBase2d9isVisibleERKN3G3D6Rect2DE = CONST):
	// mac: 0x3F5500 (84B).  getRect2D() sret (3F550D..3F5518) then the
	// strict-inequality intersection test (raw 3F551D..3F5549:
	// ours[2] > other[0], ours[3] > other[1], other[2] > ours[0],
	// other[3] > ours[1]; ucomiss+setnbe = NaN-false = the C++ float
	// '>'; the final-overrider proof = rarc17 row 66 Thn96).  The
	// plain local = no aliasing hazard (the const& and the fresh
	// rect never overlap).
	bool GuiBase2d::isVisible(const G3D::Rect2D& rect) const
	{
		const G3D::Rect2D ours = getRect2D();
		return ours.x1y1().x > rect.x0y0().x
			&& ours.x1y1().y > rect.x0y0().y
			&& rect.x1y1().x > ours.x0y0().x
			&& rect.x1y1().y > ours.x0y0().y;
	}

	// mac: 0x6E4E7E (170B): see GuiBase2d.h (the ZTV-absent non-virtual +
	// the +0x60 face slot-6 vcall shape).
	void GuiBase2d::recursiveRender2d(RBX::Adorn* adorn)
	{
		render2d(adorn);
		boost::shared_ptr<const Instances> snapshot = getChildren2();
		if (snapshot)
		{
			for (Instances::const_iterator iter = snapshot->begin();
				iter != snapshot->end(); ++iter)
				RecursiveRenderChildren(*iter, adorn);
		}
	}

	// mac: 0x6E4E26 (88B): see GuiBase2d.h (the static + invisible-pointer
	// note -- the mangle frame (sp*, adorn) matches ours exactly).
	void GuiBase2d::RecursiveRenderChildren(boost::shared_ptr<Instance> child, RBX::Adorn* adorn)
	{
		if (child.get() != NULL
			&& child->getDescriptor().isA(GuiBase2d::classDescriptor()))
		{
			static_cast<GuiBase2d*>(child.get())->recursiveRender2d(adorn);
		}
	}

} // namespace RBX

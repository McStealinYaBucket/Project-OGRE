/* V8DataModel/GuiBase2d.h — October 2013 (2D gui root shell).
   Evidence: GuiBase2d C2 0x6E4B32 (GuiBase(name) + members +30/+31 = 0,
     +32/+33 = 0, +34 = 1, +35 = 0; Described<GuiBase2d, sGuiBase2d,
     GuiBase, 27>). sGuiBase2d 19 hits in the 0.125 PE string table.
   RARC12-PROVEN (C2 raw): dwords 30..35 = byte slots 0x78/0x7C
     (absolutePosition x/y), 0x80/0x84 (absoluteSize x/y), 0x88 = 1,
     0x8C = 0 -- the landed member identities above are the C2's own
     zero/one-init writes, byte-exact.  The x86 layout also carries
     mixin vptr slices at +0x0C/+0x20/+0x24/+0x5C/+0x60, the LAST =
     IAdornable (named by the C2's abnormal-construction ~IAdornable
     cleanup at [this+0x60]); recursiveRender2d 0x6E4E7E renders via
     that subobject's slot +0x18 and walks the children pair at
     +0x38/+0x3C -- RARC14-PROVEN: GuiBase2d's D1/D0 release nothing
     at +0x38, so the pair is NOT GuiBase2d-owned; it = Instance's
     own copy_on_write_ptr<Instances> children ({px@+0x38,
     ctrl@+0x3C}; the [ctrl+4] lock-inc = read()'s ref-bump; *px/
     px[1] = the Instances _M_start/_M_finish).
      SECOND BASE LANDED (rarc17 merge-verified): the +0x60 subobject =
      RBX::IAdornable -- ALREADY LANDED at Rendering/GfxBase/include/
      GfxBase/IAdornable.h (this banner's earlier TODO wrongly called
      the base "absent from the tree" -- corrected here).  The landed
      class = the 0xF4EB00 ZTV contract: 12 virtuals, the -1/-1/-1/0
      index/bucket quadrant (PartInstance C2 0x4438BB-DC stores; the
      PartInstance +108 face = the same class via the same D2), and
      the NON-virtual dtor (ZTV slot 0 = shouldRender2d; the D2
      0x828C5E the GuiBase2d C2 unwinder calls on [this+0x60], rarc14
      6E51A3/6E51DF).
      MERGE (the merged GuiBase2d ZTV, rarc17): GuiBase2d's own
      shouldRender2d 0x4C6960 (return 0) / isVisible 0x3F5500 (
      getRect2D rect-intersection) = the final overriders on BOTH
      faces (the Thn96 adjustor rows); render2d/render2dContext/
      renderBackground2d/renderBackground2dContext/shouldRender3dAdorn/
      shouldRender3dSortedAdorn stay the interface defaults.  The
      other x86 mixin vptr slices (+0x0C GuidItem, +0x24 EventSource,
      +0x20/+0x5C per the merged ZTV sections) remain UNPORTED
      single-base stand-ins.
      RENDER ARC LANDED: recursiveRender2d 0x6E4E7E +
      RecursiveRenderChildren 0x6E4E26 (GuiBase2d), legacyRender2d
      0x3AC2AC (GuiObject).
      (getWindowRect 0x3ABA9A and the GuiObject-own +0x208..+0x214
      cache LANDED with GuiObject::handleResize 0x3ABBE2 -- the
      ScreenGui/BillboardGui windowRect member windows pinned via
      rarc15/rarc16.)
   TODO: full decompile (member identities + 2D surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiBase2d.h */

#pragma once

#include "V8DataModel/GuiBase.h"
#include "GfxBase/IAdornable.h"
#include "G3D/Vector2.h"
#include "G3D/Rect2D.h"

namespace RBX {

	extern const char* const sGuiBase2d;

	class GuiBase2d
		: public DescribedNonCreatable<GuiBase2d, GuiBase, sGuiBase2d, Reflection::ClassDescriptor::PERSISTENT>
		, public IAdornable  // SECOND BASE (the x86 mixin at +0x60; rarc17)
	{
	public:
		GuiBase2d(const char* name = NULL);

		// Absolute layout (batch191).  The +30..+33 unknown dwords (zeroed
		// by the C2, matching the binary's +30..+33 = 0) ARE the absolute
		// position/size -- identified, not theater (inits match both sides).
		const G3D::Vector2& getAbsolutePosition() const { return absolutePosition; }
		const G3D::Vector2& getAbsoluteSize() const { return absoluteSize; }
		// mac: 0x6E4D56 (98B) / 0x6E4DB8 (110B): NaN-aware change-detect, store,
		// raise prop_AbsolutePosition/prop_AbsoluteSize, return changed.
		// The raises are DELTAs (descriptor globals 0xFAA0AC/0xFAA080, no
		// Lua reflection for them in-tree; nothing listens).
		bool setAbsolutePosition(const G3D::Vector2& value);
		bool setAbsoluteSize(const G3D::Vector2& value);
		// mac: 0x6E4C4C (266B): split a rect into pos + size (rounded iff the
		// flag), store both, return posChanged|sizeChanged.  Rounding is
		// G3D::iRound (half-away) -- the binary's lrintf (banker's) is
		// absent on VS2012; identical except on exact .5 (see cpp).
		// VIRTUAL-UPGRADE (rarc13 ZTV): the binary reaches this via
		// [vptr+0x9C] = GuiBase2d primary-table slot 39 from
		// GuiBase2d::handleResize 0x6E4F28 (raw 6E4F46) -- landed plain,
		// now virtual to keep the dispatch faithful.
		virtual bool recalculateAbsoluteSizeAndPosition(const G3D::Rect2D& rect);
		// mac: 0x6E50D6 (101B): min/max-normalized rect from absolute pos/size.
		G3D::Rect2D getRect2D() const;
		// mac: 0x3F6380 (31B, __ZNK3RBX9GuiBase2d14getChildRect2DEv, ZTV slot
		// 37 at +0x94): tail-forward to getRect2D.  The binary's
		// __stdcall retn 4 = the GCC sret artifact; ours is the plain
		// value return.  Called by handleResize's child-notify staging.
		virtual G3D::Rect2D getChildRect2D() const;
		// mac: 0x6E4F28 (245B, __ZN3RBX9GuiBase2d12handleResizeERKN3G3D6Rect2DEb,
		// ZTV slot 38 at +0x98 -- every inherited gui table exposes the
		// same slot):
		// vcall slot 39 (recalculateAbsoluteSizeAndPosition) with the pending
		// rect; gate (changed || flag); vcall slot 37 (getChildRect2D) sret;
		// then the children walk -- Instance's copy_on_write_ptr children
		// (rarc14-proven pair {px@+0x38, ctrl@+0x3C}, the lock-inc [ctrl+4]
		// = read()'s ref-bump), *px = _M_start / px[1] = _M_finish, stride 8,
		// each element through bind(sub_6E501D, _1, getChildRect2D-rect
		// BYVAL, flag) = notifyChildHandleResize(child, childRect, flag)
		// below (the child receives the PARENT'S child-rect, not the
		// incoming one).  DEVIATIONS: the binder staging = our const& pass
		// (same reads); the vcalls = our virtual dispatch; the GCC EH tail
		// and the +0x24 chunk crawl = compiler artifacts (not spelled).
		// The OVERRIDING GuiObject::handleResize 0x3ABBE2 LANDED
		// (GuiObject.h/.cpp -- getWindowRect + the +0x208 cache;
		// rarc15/rarc16 raws).
		virtual void handleResize(const G3D::Rect2D& rect, bool flag);

		// ZTV slots 40/41 (code-keyed = rarc13 indexing; vptr+0xA0/+0xA4):
		// the GuiBase2d overriders of the IAdornable face.  MERGE PROOF
		// (rarc17, the merged GuiBase2d ZTV memory rows): the +0x60
		// IAdornable face shows __ZThn96_NK...14shouldRender2dEv and
		// __ZThn96_NK...9isVisibleERKN3G3D6Rect2DE -- both faces
		// dispatch to THESE bodies; the other six IAdornable slots
		// (shouldRender3dAdorn/SortedAdorn, renderBackground2d{,Context},
		// render2d{,Context}) keep the interface defaults.  Without
		// these two decls the second base would silently promote the
		// interface defaults to final overriders.
		// mac: 0x4C6960 (7B, __ZNK3RBX9GuiBase2d14shouldRender2dEv):
		// xor eax,eax -- return 0 (same value as the interface default,
		// still the named final overrider).
		virtual bool shouldRender2d() const;
		// mac: 0x3F5500 (84B, __ZNK3RBX9GuiBase2d9isVisibleERKN3G3D6Rect2DE):
		// the strict-inequality intersection test against getRect2D()
		// (raw 3F551D..3F5549: rect.x1 > other.x0, rect.y1 > other.y0,
		// other.x1 > rect.x0, other.y1 > rect.y0 -- ucomiss/jbe/
		// setnbe = NaN-false = the C++ float '>', same semantics).
		// NOT a trivial return (the interface default = 1) -- this body
		// is part of the base add.
		virtual bool isVisible(const G3D::Rect2D& rect) const;

		// mac: 0x6E4E7E (170B, __ZN3RBX9GuiBase2d17recursiveRender2dEPNS_5AdornE):
		// NON-VIRTUAL (absent from the merged GuiBase2d ZTV -- the two
		// call sites, GuiObject::legacyRender2d 3AC2DE and this class's
		// own RecursiveRenderChildren 6E4E71, are direct __ZN calls).
		// The render-self-then-children cascade: the +0x60 IAdornable
		// subobject slot-6 vcall (raw 6E4E90..6E4EA0: eax=[esi+60h],
		// ecx=esi+60h, call [eax+18h] = render2d with the subobject as
		// receiver -- ours = the unqualified virtual call, MSVC adjusts
		// to the second base subobject, same dispatch), then the
		// Instance children read-bump (lock-inc [ctrl+4] 6E4EBD =
		// read()'s landed shape) and the _M_start/_M_finish walk (stride
		// 8, 6E4EC4..6E4EEE) calling RecursiveRenderChildren per
		// element.  DEVIATIONS: the boost-binder staging (list2/
		// list1 callables) = our direct call; the GCC EH tail (6E4F07..)
		// = compiler-managed; the +0x24 chunk crawl = artifacts.  The
		// snapshot release (6E4EF7, sp_counted_base::release) = our
		// shared_ptr dtor.
		// SPELLING (the rarc18 mangle correction): the params spell
		// RBX::Adorn explicitly -- EVERY adorn-param mangle in the arc
		// is PNS_5AdornE (the rarc13/rarc17 slot census, GuiObject::
		// legacyRender2d EPNS_5AdornE at 5C4E3A (rarc18), the whole
		// ChatOutput render bind set, AdornBillboarder C1).  The
		// earlier ::Adorn spell (the 2016-carry convention) was
		// mangle-untrue and flipped at include-order boundaries; the
		// explicit RBX:: = order immune and the same class everywhere
		// (the GfxBase carry now spells RBX::Adorn too).
		void recursiveRender2d(RBX::Adorn* adorn);
		// mac: 0x6E4E26 (88B, __ZN3RBX9GuiBase2d23RecursiveRenderChildrenE
		// N5boost10shared_ptrINS_8InstanceEEEPNS_5AdornE = the S
		// HAPE: no receiver in the frame (arg_0 = the sp, arg_4 = the
		// adorn) and the binder stores a plain void(*)(shared_ptr,
		// Adorn*) -- GCC decays statics; ours = static member.  The
		// [arg_0] deref = GCC's invisible-pointer pass of the by-value
		// non-trivial-copy shared_ptr (MSVC copies the 8-byte POD at
		// the call; identical observable).  Body: px null test,
		// [child+0x30] descriptor isA(GuiBase2d described), then the
		// direct recursiveRender2d(child, adorn).  Visibility unproven;
		// public = no harm.
		static void RecursiveRenderChildren(boost::shared_ptr<Instance> child, RBX::Adorn* adorn);

	private:
		// mac: 0x6E4B32 members (x86 dword offsets; +34/+35 identities TODO).
		G3D::Vector2 absolutePosition;  // +30/+31.
		G3D::Vector2 absoluteSize;  // +32/+33.
		int unknown34;
		int unknown35;
	};

} // namespace RBX

/* GfxBase/Typesetter.h -- October 2013 render board (NEW reconstruction
   file).  Text measurement service internals; the chat bubbles + the
   GuiTextMixin render path pull typesetters from TextService.

   Binary evidence:
     __ZTSN3RBX16TypesetterBitmapE                 (typeinfo name)
     _ZTVN3RBX16TypesetterBitmapE  @0xEF68B0 (8 fn slots visible)
     __ZN3RBX11TextService13getTypesetterENS0_4FontE   -- the 5-slot
       shared_ptr<Typesetter> table lives at TextService+0x60 (this+96)
     __ZN3RBX11TextService18registerTypesetterNS0_4FontEN5boost10
       shared_ptrINS_10TypesetterEEE
     __ZN3RBX11TextService16clearTypesettersEv

   TypesetterBitmap vtable geometry (rarc26, ZTV @0xEF68B0; vptr = ZTV+8,
   the two leading entries are the offset-to-top/typeinfo header):
     vptr[0] = sub_7C31AE  -- complete-object dtor body (resets vptr to
                 off_EF68B8, tears down the +36 subobject, frees [+32])
     vptr[1] = sub_7C3202  -- deleting-dtor pair entry (same body +
                 operator delete(this))
     vptr[2] = sub_7C2330  -- layout renderer: walks the glyph stream and
                 emits quads through sub_7C2C6E(target, rectW, uvMin,
                 uvMax, colorIdx); returns the laid-out extents
     vptr[3] = sub_7C2D74  -- wrapped-break-point query; int return
                 (this-first stack shape -- no sret -- matches the int
                 return)
     vptr[4] = sub_7C3072  -- MEASURE: reads *(face+4)*fontSize as the
                 target pixel size, scans *(face+20)[i] for the first
                 size >= target, submits sub_7C2932(face, text, NULL,
                 sizeIdx, Vector2int16(scale/...), (scale.x != 0),
                 align) and scales the returned long-word pair back out
                 to the caller-supplied extents slot.

   The chat bubbles (ChatOutput::renderBubbles 0x5C4EC0) reach the
   measurer through the Typesetter vtable slot at vptr+0x10 (slot 4;
   var_20C = [vptr+16]): the staged call = sret Vector2 extents, this,
   const std::string& text (ChatLine+0x04), float fontSize, const
   G3D::Vector2& scale (ChatOutput-staged {w-fontSize, 7.5*fontSize+0.01}),
   int trailing alignment (0 everywhere observed; the target-side
   sub_7C2D74 align tests are ==1 center / ==2 right == Left0/Center1/
   Right2 ordering).  GCC emits the sret for the 8B Vector2 like the
   getTypesetter shared_ptr return; the eax leftover (the line count
   from the layout stream) is scratch the call site ignores.

   The _ZTS/_ZTI pair for the abstract Typesetter exists (typeinfo word
   [8] = 0 => no base chain) but NO _ZTV -- the base never constructs
   out-of-line.  TypesetterBitmap (the concrete registered per-Font
   bitmap measurer) overrides the measure slot and now stands spelled
   beside this header (GfxBase/TypesetterBitmap.h; the RbxTypesetter/
   RbxFontFactory TUs carry the measure, the walker and the factory
   getter) -- its construction site is the (still unlanded) view font
   bootstrap.  DELTA: the slot-2/3 signatures below are geometry-only
   placeholders -- names/pools unobservable; only the slot count and the
   measure call-site decode are ground truth. */

#pragma once

#include "G3D/Vector2.h"
#include <string>

namespace G3D
{
	class Vector2;
}

namespace RBX
{

	class Typesetter
	{
	public:
		/* vtable geometry (TypesetterBitmap ZTV @0xEF68B0): the dtor
		   pair leads the table (vptr[0]/[1]), two geometry slots sit
		   in the middle (vptr[2]/[3] = the layout-render and
		   wrapped-break queries in the bitmap concrete), and the
		   measure lands fourth (vptr[4]; the bubble arcs call
		   vptr+0x10). */

		virtual ~Typesetter() {}

		/* DELTA placeholders: the binary slot-2/3 signatures are
		   unobservable (the ZTV lists sub_7C2330/sub_7C2D74 unnamed);
		   their bodies live with the TypesetterBitmap font bootstrap.
		   Declared to hold the slot geometry only. */
		virtual G3D::Vector2 layout(const std::string& text) = 0;

		virtual int breakLine(const std::string& text) = 0;

		// The slot-4 member the bubble arcs call through vptr+0x10
		// (0x5C4EC0 size pass; the {sret, text&, fontSize, scale&,
		//  int-align} shape; align observation: 0 == left horizontal
		// per the sub_7C2D74 center/right tests).  Returns the text
		// extents at fontSize, scaled by the caller-supplied Vector2.
		virtual G3D::Vector2 measure(const std::string& text,
			float fontSize, const G3D::Vector2& scale, int align) = 0;
	};

} // namespace RBX

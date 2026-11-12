/* Gui/Adorn.h -- October 2013 render board (NEW reconstruction file; the
   2013 tree touches Adorn only via forward decls -- the class lived in the
   render backend, and the gui layer draws through it by virtual slot).
   Evidence: batch787 vt dump __ZTVN3RBX5AdornE @ F0D9C0 (36 slots):
     [2]=__cxa_pure_virtual, [3]/[4]=D1/D0 @ 5CA848/822A74, [5]=pure,
     [6..9]=prepareRenderPass 821F8C / finishRenderPass 5CA858 /
     preSubmitPass 821F98 / postSubmitPass 286C40, [10..20]+[22..35]=pure,
     [21]=drawFont2D 822AA4.  Object vptr = ZTV+8 (verified: the
     GuiItem::label2d call [a2+0x4C] lands exactly on [21]).
   Binary object offsets (vptr-relative): +0x00 pure (a virtual ahead of
   the dtor pair -- GCC declaration order), +0x04/+0x08 D1/D0, +0x0C pure,
   +0x10..+0x1C the four pass names, +0x20/+0x30/+0x40 pure, +0x4C
   drawFont2D, +0x50.. pure x14.  The class below keeps the named-slot
   ORDER; the +0x00 pure head is not fabricatable (name unobservable).
   The three pure slots consumed by the landed renders, semantics decoded:
     +0x20 = 4-float sret extents (render2dMe 5CE34E/label2d 5CD627;
             canvas = f[2]-f[0], f[3]-f[1] => the min/max x/y form);
     +0x30 = (rect, float width, Color4&) outline -- render2dMe 5CE486
             passes rect + 3F800000 (1.0f) + (G3D::Color4)(black,1.0f);
     +0x40 = (rect, Color4&) fill -- render2dMe 5CE402 (white/menuselect).
     Names carry the G3D::Adorn heritage (drawRectangle/fillRectangle/
     drawFont2D are that API's): the two pure-rect slots keep them, the
     extents slot has no G3D counterpart (name ours).
     DELTA: +0x20/+0x30/+0x40 signatures decode from the callers only
     (pure slots share the __cxa_pure_virtual stub, no symbol names); the
     extent return is our Util/Rect (16B, struct-identical to the binary's
     sret) while drawFont2D's rect is mangle-fixed G3D::Rect2D (kept
     exactly).
   drawFont2D mangle (return omitted => encodable; 8B non-POD => hidden
   sret in GCC): the 12-slot caller 5CD717-5CD756 = (sret 8B Vector2,
   this, const string&, const G3D::Vector2&, float fontSize, const
   Color4&, const Color4&, Text::Font, Text::XAlign, Text::YAlign, const
   G3D::Vector2&, const G3D::Rect2D&); the returned Vector2 is discarded
   at every landed caller.
   RBX::Text = the render-text enums.  No EnumDesc instantiation in the
   binary (reflection covers only TextService::XAlignment); DELTA: value
   names ours, the decoded arm arithmetic is the ground truth --
   XAlign 0 -> x = rect.high.x + width*(-0.1f) [C86268 = -0.1f],
   XAlign 1 -> x = rect.low.x + width*0.1f [CA8984 = 0.1f],
   XAlign 2 -> x = the center [C1E4BC = 0.5f avg] (5CD6A3-6BD);
   ChatWidget passes 2, TextDisplay inits 1.
   Fidelity ~99% on the landed surface (slot map exact; bodies stubbed
   pending the render backend).  Original file: none recovered (the
   backend Adorn never shipped in the 2013 source dump we carry). */

#pragma once

#include <string>
#include "Util/G3DCore.h"
#include "Util/Rect.h"

namespace RBX {

	namespace Text
	{
		enum Font
		{
			font0 = 0,  // the drawFont2D call passes 0 (5CD75E); name unobservable (DELTA)
			font1,      // DELTA name (the font0 precedent); enumerating
			font2,      // the 0..4 run feeds the FromTextFont fence and
			font3,      // the view font bootstrap pair reads.
			font4,

			// 5 pinned by the RbxFontFactory.cpp:25 assert expansion
			// "font < Text::FONT_LAST" (rarc31).
			FONT_LAST = 5
		};

		enum XAlign
		{
			xaRight = 0,   // decoded arm: x = rect.high.x + width*(-0.1f)
			xaLeft = 1,    // decoded arm: x = rect.low.x  + width*(0.1f)
			xaCenter = 2,  // decoded arm: x = the center (ChatWidget); names DELTA
		};

		enum YAlign
		{
			yaInvariant = 0,
			yaCenter = 1,  // the drawFont2D call passes 1 (5CD756); name unobservable (DELTA)
		};
	}

	class Adorn
	{
	public:
		virtual ~Adorn() {}

		// The four render-pass virtuals with real bodies in the binary;
		// ours stay empty (no render backend landed).
		// STUB: PLAYER 0x009d55f0
		// STUB: STUDIO 0x00aa1ec0
		virtual void prepareRenderPass() {}      // ZTV[6] = 0x821F8C
		// STUB: PLAYER 0x009d55f0
		// STUB: STUDIO 0x00aa1ec0
		virtual void finishRenderPass() {}       // ZTV[7] = 0x5CA858
		// STUB: PLAYER 0x009d55f0
		// STUB: STUDIO 0x00aa1ec0
		virtual void preSubmitPass() {}          // ZTV[8] = 0x821F98
		// STUB: PLAYER 0x009d55f0
		// STUB: STUDIO 0x00aa1ec0
		virtual void postSubmitPass() {}         // ZTV[9] = 0x286C40

		// The slots the GuiItem drawing family consumes (binary order):
		virtual Rect get2DExtents() = 0;         // +0x20: 4-float sret (DELTA name)
		virtual void drawRectangle(const Rect& rect, float lineWidth, const G3D::Color4& color) = 0;   // +0x30
		virtual void fillRectangle(const Rect& rect, const G3D::Color4& color) = 0;                     // +0x40
		// +0x48 = ZTV[20]: NAMED in the binary by the AdornBillboarder
		// override 0x822002 __ZNK3RBX16AdornBillboarder17get2DStringBoundsE
		// RK Ss f NS_4Text4FontE RKN3G3D7Vector2E (rarc9 object-vptr table
		// off_EF7498 slot 18).  Classic-chat painter call sites
		// mac: 0x5C45A7/0x5C45E7 stage (sret Vector2 extents, this, const
		// std::string&, float fontSize = 12.0f, Text::Font = font 0,
		// const G3D::Vector2& scale = Vector2::zero()).
		// DELTA: 4th-arg semantic unobserved at the slots (scale/wrap?);
		// named scale from the Typesetter::measure twin shape.
		virtual G3D::Vector2 get2DStringBounds(const std::string& text, float size, Text::Font font,
			const G3D::Vector2& scale) const = 0;         // +0x48
		virtual G3D::Vector2 drawFont2D(const std::string& text, const G3D::Vector2& position, float fontSize,
			const G3D::Color4& color, const G3D::Color4& outlineColor,
			Text::Font font, Text::XAlign xAlign, Text::YAlign yAlign,
			const G3D::Vector2& scale, const G3D::Rect2D& clipRect) = 0;   // +0x4C (mangle-exact 822AA4)

		// renderBubbles 0x5C4EC0 calls this through vptr+0x58 (decompile
		// mac: 0x5C5095: the fn-ptr load off a2's vtable +88 = slot 3 of the
		// +0x50.. pure band per the ZTV map -- the two unnamed pures in
		// front are the class-omission DELTA documented in the head
		// note).  The bubble pass stages a default-constructed
		// CoordinateFrame (= identity) into it (0x5C50AC..0x50BE: the
		// CF C1 + the vtail call).
		virtual void setObjectToWorldMatrix(const G3D::CoordinateFrame& objectToWorld) = 0;
	};

} // namespace RBX

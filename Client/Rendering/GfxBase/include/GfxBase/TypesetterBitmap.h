/* GfxBase/TypesetterBitmap.h -- October 2013 render board (NEW
   reconstruction file).  The concrete per-Font bitmap measurer the
   font bootstraps register into TextService's 5-slot table (the
   registerTypesetter loops land every Text::Font 0..4 -- rarc30
   bootstrap 1 raw 0x7B8C43-0x7B8CB2).  Spelled alongside the abstract
   Typesetter.h so the bubble arcs pull both through one include path.

   Binary evidence (rarc26 ZTV @0xEF68B0 + rarc31 probe; the measure
   slot's own decomp = rarc26 lines 468-504, its raw = rarc31
   0x7C3072..0x7C315F):

     __ZN3RBX16TypesetterBitmapE                  (typeinfo name)
     __ZNK3G3D7Vector2dvEf                        (the const-member
       operator/(float) the measure scales the caps through --
       0x7C30C9-D3 pins sret = var_18, this = scale, s = the ratio)
     __ZN3G3D12Vector2int16C1ERKNS_7Vector2E      (the int16 caps ctor
       the walker feeds).

   vptr slots (vptr = ZTV+8 = off_EF68B8):
     [0] sub_7C31AE = dtor D1 body: resets *this to off_EF68B8,
         releases the +0x24 subobject (sub_818AF8), delete[]s +0x20.
     [1] sub_7C3202 = dtor D0: the same body + operator delete(this).
     [2] sub_7C2330 = the layout glyph-walk (12-stack-arg __userpurge
         shape per IDA; emits quads through sub_7C2C6E) -- decoded in
         rarc26 lines 21-288, PARKED (the override below holds the
         slot).
     [3] sub_7C2D74 = the wrapped-break query (int return; ==2/==1
         align arms in its x-redo) -- decoded in rarc26 lines
         290-465, PARKED (same reason).
     [4] sub_7C3072 = the MEASURE -- REAL in RbxTypesetter.cpp
         (0x7C3072..0x7C315F, 238B, retn 4 = the sret-clean).

   Face member offsets below = the C2's own stores (rarc31 raw
   0x7C1F03-0x7C203F): +4 float (arg_C), +8 = RBXF dword[0] (the
   size-table count; new[>15] guard = the "Corrupt file header" throw
   at 0x7C21CB), +0xC = RBXF dword[1] (the 2330's sizeIdx-scaled row
   stride factor), +0x10 = buf+8 (the char-class bytes), +0x14 =
   buf+0x108 (the float pixel-size table), +0x18 = buf+0x108+4*count,
   +0x1C = buf+0x108+4*count+32*count*count (the shift bytes -- the
   0x7C203F shl-5 chain), +0x20 = the arena pointer (delete[]ed by
   both dtors), +0x24 = the SECOND base (own vptr off_F96628; the C2
   zeroes +0x28/+0x2C/+0x30 first) torn by sub_818AF8.

   The C2 0x7C1EE0 = LANDED (RbxTypesetter.cpp; the loader real,
   the getSingleton/createManual tail parked behind the Ogre
   import).  DELTAs: the +0x24 second base = undeclared
   spelled raw; the slot-2/3 signatures = the Typesetter.h geometry
   placeholders (their true arg lists differ -- see the parked
   decodes).  The bootstrap construction site (0x7B8CDC's
   find<RenderHooksService>-armed factory scatter) stays parked too. */

#pragma once

#include "GfxBase/Typesetter.h"
#include "G3D/Vector2int16.h"

#include <string>
#include <vector>

namespace RBX {

	class TypesetterBitmap : public Typesetter
	{
	public:
		/* The walker's 16-byte line unit (rarc32/33 decode).  The layout
		   slot collects these through a local std::vector<Line> (the
		   12B {start, finish, end-of-storage} triple v85/v86/v87 at
		   decomp 129-131; the record fields read back as v66(+0)/v65
		   (+1)/v29+2/v30 in the layout pass).  DELTA: the type and
		   member names are unobservable (the GCC instantiations write
		   raw 16B); our ctor set keeps push_back/resize spelling
		   available.  The binary's out-of-line instantiations behind
		   the walker's emit arms:
		     0x7C357A (85B) = vector<Line>::push_back(const Line&)
		         -- 16B record copy into finish, finish += 16, the
		         capacity-hit arm = 0x7C35CE.
		     0x7C35CE   = _M_insert_aux (realloc max(2n, n+1) +
		         __throw_length_error("vector::_M_insert_aux"))
		     0x7C34FE   = the record allocator; 0x7C3524 = the move-range
		         (movmem) helper.
		     0x7C3160 (79B) = vector<Line>::resize(limit): count > limit
		         swings finish back (POD shrink), else fill-inserts
		         limit - count ZERO records through 0x7C3270 =
		         _M_fill_insert -- the walker stages the zero record in
		         the call frame itself ("0,0,0,0" = the 16B fill BYREF).
		   DELTAs spelled the corpus way: our std::vector carries MSVC's
		   grow policy (not the GCC 2x) and the semantic ops identical;
		   the addresses kept as the recorded evidence. */
		struct Line
		{
			Line()
				: data(NULL), length(0), y(0), width(0) {}

			Line(const char* data_, int length_, int y_, int width_)
				: data(data_), length(length_), y(y_), width(width_) {}

			const char* data;   // the text buffer slice (str.data() + start)
			int length;         // byte length into that slice
			int y;              // the line's top offset (v7 shadow)
			int width;          // the line's pen width (v46 shadow)
		};
	public:
		/* mac: 0x7C1EE0 = the .RBXF loader C2 (928B): (const std::string&
		   path, int textureName, float pixelBase) -- arg_4 = the
		   path string, arg_8 = the int the parked createManual tail
		   passes as its first member arg (the font's resource name
		   pointer, identity riding the Ogre arc), arg_C = the +0x04
		   _pixelBase float.  Raw order: *this = off_EF68B8; +0x04 =
		   arg_C; +0x20 = 0; +0x28/+0x2C/+0x30 = 0; +0x24 = the
		   second-base vptr off_F96628 seeded last.  Body =
		   RbxTypesetter.cpp; the texture-establishment tail parks
		   behind the Ogre import (getSingleton = the
		   @executable_path/../Frameworks/Ogre.framework thunk at
		   0x96D114, body absent in this DB; the PIC+0x359A04 group
		   literal = "General", resolved manually at __cstring
		   0xB1B8F5). */
		TypesetterBitmap(const std::string& path, int textureName,
			float pixelBase);

		/* mac: 0x7C31AE D1 / 0x7C3202 D0: resets off_EF68B8, releases the
		   +0x24 subobject (sub_818AF8) and delete[]s +0x20 -- the
		   second base stays undeclared, so the shell's dtor stands in
		   for the pair (nothing owns storage until the RBXF loader
		   arc lands). */
		virtual ~TypesetterBitmap() {}

		// vptr[2] = 0x7C2330.  Body parked (rarc26 21-288); the override
		// keeps the shell concrete.
		virtual G3D::Vector2 layout(const std::string& text);

		// vptr[3] = 0x7C2D74.  Body parked (rarc26 290-465).
		virtual int breakLine(const std::string& text);

		// mac: 0x7C3072 (238B): the real per-font bitmap measure.  The body
		// = the RbxTypesetter.cpp spell.
		virtual G3D::Vector2 measure(const std::string& text,
			float fontSize, const G3D::Vector2& scale, int align);

	private:
		// mac: 0x7C2932 (766B): the face walker -- the static worker the
		// measure submits through (lineTarget = NULL on the measure
		// route; fits-out = the trailing int the shipped call sites
		// only pass as 0).  Body = RbxTypesetter.cpp.
		static int measureWalk(const TypesetterBitmap& face,
			const std::string& text, std::vector<Line>* lineTarget,
			int sizeIdx, const G3D::Vector2int16& caps, int wrap,
			void* fitsOut);

		// +0x04: the face pixel base (the C2's arg_C store; measure's
		// target = *[this+4] * fontSize, 0x7C308D-92).
		float _pixelBase;

		// +0x08: the RBXF size-table count (the scan cap; 0x7C307E).
		int _sizeCount;

		// +0x0C: the second RBXF header dword (the walker's
		// sizeIdx-scaled stride: v35 = sizeIdx * this[3], decomp 866).
		int _rowStrideDelta;

		// +0x10: the char-class byte table (buf+8; the walker reads
		// *(u8*)(this[4] + ch) -- the 255 class = the control set).
		const unsigned char* _charClass;

		// +0x14: the float pixel-size table (buf+0x108; the scan +
		// the walker's lineH = (int)sizeTable[sizeIdx], decomp 868).
		const float* _sizeTable;

		// +0x18: buf+0x108 + 4*_sizeCount (the metric-row base; the
		// walker reads the int16 advance at +6 and the int chain at
		// +12 of each 32-byte row).
		const unsigned char* _metricRows;

		// +0x1C: the char-shift byte table (the walker reads the
		// SIGNED letter at [cls + prev], decomp 920).
		const unsigned char* _charShift;

		// +0x20: the RBXF arena pointer (delete[]ed by both dtors).
		unsigned char* _arena;

		// +0x24..+0x30: a BY-VALUE Ogre::TexturePtr (rarc37 classify:
		//   off_F96628 dynamically = _ZTIN4Ogre10TexturePtrE, the
		//   inner base copy = _ZTIN4Ogre9SharedPtrINS_7TextureEEE at
		//   off_F96608 -- the C2 zero-px/ctrl seeds the OUTer vptr
		//   and the 16B counted assign temp carries the INNER view;
		//   sub_7B6E28 = the assign via the +0xC slot with ++ctrl,
		//   sub_818AF8 = the release through sub_7C1C90's
		//   {obj+4, extra+8, policy+12} switch).  Spelled as raw
		//   words -- the counting machinery is its own GfxBase/Ogre
		// hosted arc.
		void* _secondBaseDelta[4];
	};

} // namespace RBX

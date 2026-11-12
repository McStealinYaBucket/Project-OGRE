/* Rendering/RbxOgre/RbxTypesetter.cpp -- October 2013 render board
   (NEW reconstruction TU; the seat = the sibling of the landed
   RbxFontFactory.cpp the assert strings name -- the factory getter
   carries 'Client/Rendering/RbxOgre/RbxFontFactory.cpp' verbatim).
   Bodies home the TypesetterBitmap slots:

     0x7C1EE0 TypesetterBitmap::C2        928B   REAL (loader side:
                   the magic/count/arena split chain real; the
                   getSingleton + +0x98 createManual + counted
                   assign tail = PARKED behind the Ogre import)
     0x7C3072 TypesetterBitmap::measure   238B   REAL
     0x7C2932 measureWalk (static)        766B   REAL
     0x7C357A / 0x7C3160                  RESPELLED -- the GCC
                  std::vector<Line> instantiations (push_back /
                  resize, the 7C35CE insert_aux / 7C34FE allocator /
                  7C3524 move-range / 7C3270 fill_insert family
                  behind them) -- landed as the std-library ops at
                  the walker's call sites (rarc32/33 probes; the
                  Line block in TypesetterBitmap.h holds the
                  evidence).
     0x7C2330 layout / 0x7C2D74 breakLine        PARKED -- decoded in
                  rarc26 lines 21-288 / 290-465, unadopted until the
                  render-layout arc; the overrides hold the slots.

   The walker spec (decomp rarc30 816-1047, raw 1049-1327): over the
   text bytes -- class = charClass[ch]; class 255 = the control set (the
   '\n' arm folds the line via the vector<Line> push_back and advances
   y by lineH
   = (int)sizeTable[sizeIdx]; c == 1 is the lone shift-base keeper);
   glyphs compute shift = the SIGNED letter at charShift[cls + prev]
   plus the signed-short advance at metricRows[32*(sizeIdx*+0xC + cls)
   + 6], route the space-run (c == 9 || c == 32) against the word
   cache, and when wrap && pen > caps[0] break ahead of the word (the
   split arm) or into it (the whole-word arm, which re-lines the
   overflow glyph alone).  prev = the int chain at
   metricRows[rowOff + 12].  The tail emits the last line, folds the
   widest, and when wrap && y + lineH > caps[1] clamps the line count
   to max(1, caps.y / lineH) through the 0x7C3160 resize instantiation (the count is
   staged BEFORE the fits write; the raw cmovg arm = 1), marks the
   fits byte 0 / 1, and returns the packed (clampedY << 16) |
   (u16)widest pair.

   MSVC deltas (tracked): the / 'operator/ float' const-member and the
   Vector2int16(const Vector2&) ctor land out-of-line in the G3Dcpp
   TU; the sret/retn-4 return shape and the push-stage order ride the
   GCC ABI only.  The vector respell: MSVC's push_back/resize carry
   the implementation's own grow policy (not the GCC max(2n, n+1)
   insert-aux reshuffle) per the TypesetterBitmap.h DELTA block. */

#include "stdafx.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "GfxBase/TypesetterBitmap.h"

namespace RBX {

	/* mac: 0x7C2932 (766B): the face walker.  face = the TypesetterBitmap
	   metric block itself; lineTarget = the std::vector<Line>* the
	   emit arms feed (push_back / resize; NULL on the measure route --
	   the binary's raw int* slot, rarc32/33); caps = the int16
	   {width, height} pair; wrap = the
	   governor (patrol the split/overflow breaks at 0x7C2BF9/0x7C2BFF);
	   fitsOut = the clip-verdict byte, which the shipped measure routes
	   its trailing align int into (align == 0 = the no-out fold -- the
	   only call-site value; rarc31 0x7C30EA/ED).
	   MSVC note: *_charShift/_metricRows indexes land byte/short
	   unaligned reads by construction, as in the binary. */
	int TypesetterBitmap::measureWalk(const TypesetterBitmap& face,
		const std::string& text, std::vector<Line>* lineTarget,
		int sizeIdx, const G3D::Vector2int16& caps, int wrap,
		void* fitsOut)
	{
		const int rowStrideSize = sizeIdx * face._rowStrideDelta;
		const unsigned char* metricRows = face._metricRows;
		const int lineH = static_cast<int>(face._sizeTable[sizeIdx]);
		/* v35 = a4 * a1[3] (866); v36 = a1[6] (867); v37 = the size
		   table entry trunc-forwarded (868). */

		const char* data = text.c_str();
		const unsigned int len = static_cast<unsigned int>(text.size());

		unsigned int i = 0;          // v9
		int widest = 0;              // v44
		int y = 0;                   // v7
		int prev = 0;                // v39
		int lineStart = 0;           // v43
		unsigned int startX = 0;     // v42
		int spaceRun = 0;            // v41
		int runCache = 0;            // v8
		int pen = 0;                 // v46
		/* the init block (872-880; the v50 spill rides the folds). */

		if (len != 0)
		{
			do
			{
				const unsigned int off = i;                                      // v47
				const unsigned char ch = static_cast<unsigned char>(data[i]);    // v11
				const unsigned char cls = face._charClass[ch];                   // v12

				if (cls == 255)
				{
					// The control class: c == 1 keeps the previous shift
					// base; anything else zeroes it (890-893).
					if (ch != 1)
						prev = 0;

					// The newline arm: emit the line, fold the widest,
					// advance to the next row (895-914).
					if (ch == 10)
					{
						if (lineTarget != 0)
							lineTarget->push_back(Line(data + lineStart,
								static_cast<int>(i - lineStart), y, pen));
						widest = widest < pen ? pen : widest;
						y += lineH;
						startX = i + 1;
						spaceRun = 0;
						runCache = 0;
						pen = 0;
						lineStart = i + 1;
					}
				}
				else
				{
					const int lineY = y;                     // v45 (919)
					const int shift =
						static_cast<signed char>(face._charShift[cls + prev]);   // v15 (920)
					const int rowOff = 32 * (rowStrideSize + cls);              // v16 (921)
					const int runW =
						shift + *(const signed short*)(metricRows + rowOff + 6); // v17 (922)

					int wordCarry;                           // v18
					if (ch == 9 || ch == 32)
					{
						wordCarry = 0;                       // 925
						spaceRun =
							runW + (runCache != 0 ? 0 : spaceRun);    // 926-929
						startX = off;                        // 930
					}
					else
					{
						if (runCache == 0)                   // 934-935
							startX = off;                    // 937
						wordCarry = runW + runCache;         // 938
					}

					const int penNext = runW + pen;          // v21 (940)
					if (wrap != 0 && penNext > static_cast<int>(caps.x))   // 941
					{
						int lineCarry;
						int yCarry = lineY;                  // v25 (960/980)

						if (wordCarry <= static_cast<int>(caps.x))   // 944: v18 <= v22
						{
							// Split after the space-run: the break carries
							// the whole word to the fresh row.
							const int splitW = penNext - wordCarry - spaceRun;   // v26 (946)
							if (lineTarget != 0)
								lineTarget->push_back(Line(
									data + lineStart,
									static_cast<int>(startX - lineStart),
									lineY, splitW));             // 949-952
							widest = widest < splitW ? splitW : widest;   // 955-958
							lineCarry = startX;              // v24 = v42 (959)
						}
						else
						{
							// The whole word overflows: the break re-lines
							// the overflow glyph alone (977 carries only its
							// runW forward).
							if (lineTarget != 0)
								lineTarget->push_back(Line(
									data + lineStart,
									static_cast<int>(off - lineStart),
									lineY, pen));                // 967-970
							widest = widest < pen ? pen : widest;         // 973-976
							wordCarry = runW;                // 977
							lineCarry = off;                 // 978
						}
						lineStart = lineCarry;               // 982
						y = lineH + yCarry;                  // 985
						spaceRun = 0;                        // 986
						pen = wordCarry;                     // 987: v46 = v18
						runCache = wordCarry;                // 988: v8 = v18
					}
					else
					{
						pen = penNext;                       // 993: v46 += v17
						runCache = wordCarry;                // 994: v8 = v18
						y = lineY;                           // 995 (the no-op spill)
					}
					prev = *(const int*)(metricRows + rowOff + 12);   // 998
				}   // the glyph arm

				++i;                                          // 1000
			} while (i < len);                                // 1002-1004
		}
		else
		{
			/* The empty-text arm only zero-spills (1008-1012); every
			   scalar above is already at its zero init. */
		}

		// The tail: the final line emit (1016-1023), the widest fold
		// (1025-1027), the height clamp with the fits verdict
		// (1030-1044), the packed return (1046).
		if (lineTarget != 0)
			lineTarget->push_back(Line(data + lineStart,
				static_cast<int>(len - lineStart), y, pen));

		const int widestFinal = widest < pen ? pen : widest;
		int totalY = lineH + y;

		if (wrap != 0 && totalY > static_cast<int>(caps.y))
		{
			const int over = static_cast<int>(caps.y) / lineH;   // v32 (1032)
			const int lineCount = over > 0 ? over : 1;           // v33 (1033-1035)
			if (lineTarget != 0)
				lineTarget->resize(static_cast<size_t>(lineCount)); // 1036-1037
			if (fitsOut != 0)
				*static_cast<unsigned char*>(fitsOut) = 0;       // 1038-1039
			totalY = lineH * lineCount;                          // 1040
		}
		else if (fitsOut != 0)
		{
			*static_cast<unsigned char*>(fitsOut) = 1;           // 1042-1044
		}

		return (totalY << 16) | static_cast<unsigned short>(widestFinal);
	}

	/* mac: 0x7C2330: the layout glyph-walk that emits the quad records
	   through sub_7C2C6E and returns the laid-out extents.  The
	   Typesetter.h signature = the geometry placeholder (the binary
	   slot = a 12-stack-arg __userpurge shape); the parked decode =
	   rarc26 lines 21-288 + the tail read 150-288:
	     -- the walker feeds a LOCAL std::vector<Line> (the 12B
	        {start, finish, end-of-storage} triple v85/v86/v87 at
	        decomp 129-131; fits-out 0 = the no-out arm) -- the
	        vector respell covers the layout's own collection too.
	     -- the pass count = 5 when *(a8 + 12) <= 0.05f, else 1
	        (the shadow stack: dword_FBCF98[2*pass + 0/1] carries the
	        per-pass offsets; one full vector re-walk per pass).
	     -- per line: the 4 record fields (data / length / y / width
	        = v66 / v65 / +2 / +3); x-align = a9 (0 left, 1 center =
	        width/2, 2 right -- decomp 188-200), y-offset from a10
	        (== 2 full, == 1 half -- 150-157) with floorf(x + 0.5f)
	        rounding (decomp 133-137).
	     -- per glyph byte: the class-255 arm resets the run and
	        emits the control quad through sub_7C2C6E (a12 = the
	        target; the a3+0x14 flag toggles around it); glyphs take
	        the metric-row uv rects (v80/v79 vs the shl5 rows).
	     -- the tail = the sret Vector2: (short)v53 * v70 and
	        (u16)v52 * v70 -- the unpacked measure halves times the
	        target/found ratio; the vector start is operator
	        delete'd on scope death.
	   Still unadopted: the remaining stack-arg identities behind
	   a2/a4/a5/a6/a9/a10 -- the render-layout arc owns those. */
	// STUB: PLAYER 0x0051e890
	// STUB: STUDIO 0x005b6b90
	G3D::Vector2 TypesetterBitmap::layout(const std::string& /*text*/)
	{
		return G3D::Vector2(0.0f, 0.0f);
	}

	/* mac: 0x7C2D74: the wrapped-break-point query (int return; the ==2 right
	   / ==1 center arms live in its x-redo at rarc26 367-383).  Shape =
	   an 8-arg __userpurge (a1 float*, a2 = the face block, a3 float*,
	   a4 float, a5/a6 ints, a7 unsigned int*, a8 float*) -- the same
	   family shape as layout, hence parked with it (rarc26 290-465). */
	// STUB: PLAYER 0x0051ee70
	// STUB: STUDIO 0x005b7170
	int TypesetterBitmap::breakLine(const std::string& /*text*/)
	{
		return 0;
	}

	/* mac: 0x7C3072 (238B, retn 4 = the sret-clean): TypesetterBitmap::measure.
	     0x7C308D-92: target = [this+4] * fontSize (the xmm1 mulss).
	     0x7C3099-A6: the scan -- the cap test `cmp ecx, ebx; jge` leads,
	       then entry load, pre-increment, and `ucomiss/jb` = the
	       "entry < target keeps walking" (the NaN entry arm rides CF too,
	       matching the C++ ordering); the fall-out arm 0x7C30B0-B9 reads
	       _sizeTable[_sizeCount-1] (even at count 0, the [-1] shape) and
	       settles idx = count-1.
	     0x7C30BA: ratio = target / found (the divss).
	     0x7C30C9-D3: caps = scale / ratio -- the push order (sret var_18,
	       this = scale, s = var_20) pins the const-member
	       __ZNK3G3D7Vector2dvEf; converting the request-scale INTO the
	       found-bitmap pixel units.
	     0x7C30DB-E5: Vector2int16(caps).
	     0x7C30F4-102: wrap = (scale.x != 0) (the cmpneqss against 0.0,
	       &1).
	     0x7C3111-23: the walker stages (face, text&, NULL lineTarget,
	       foundIdx, &caps, wrap, align) -- the trailing align int lands
	       the walker's fits-out slot (align == 0 folds to the no-out
	       arm; the only shipped call-site value).
	     0x7C3128-50: the packed (w | h << 16) re-expands the SIGNED
	       halves (movsx ax / sar 16) times the ratio into the extents. */
	// STUB: PLAYER 0x0051f180
	// STUB: STUDIO 0x005b7480
	G3D::Vector2 TypesetterBitmap::measure(const std::string& text,
		float fontSize, const G3D::Vector2& scale, int align)
	{
		const float target = _pixelBase * fontSize;

		int foundIdx = _sizeCount - 1;
		float foundSize = _sizeTable[_sizeCount - 1];
		for (int i = 0; i < _sizeCount; ++i)
		{
			const float entry = _sizeTable[i];
			if (entry >= target)
			{
				foundIdx = i;
				foundSize = entry;
				break;
			}
		}

		const float ratio = target / foundSize;
		G3D::Vector2 caps = scale / ratio;
		G3D::Vector2int16 limit(caps);

		const int packed = measureWalk(*this, text, NULL, foundIdx, limit,
			scale.x != 0.0f ? 1 : 0, reinterpret_cast<void*>(align));

		G3D::Vector2 extents;
		extents.x = static_cast<float>(static_cast<short>(packed & 0xFFFF)) * ratio;
		extents.y = static_cast<float>(packed >> 16) * ratio;
		return extents;
	}

	/* mac: 0x7C1EE0 (928B): TypesetterBitmap's .RBXF loader C2 --
	   (const std::string& path, int textureName, float pixelBase),
	   __ZN3RBX16TypesetterBitmapC2[...]ERKS[0?]...i...f shape; arg_C
	   = the +0x04 pixelBase float.  Raw order: *this = off_EF68B8;
	   +0x04 = arg_C; +0x20 = 0; +0x28/+0x2C/+0x30 = 0; +0x24 = the
	   second-base vptr off_F96628 seeded last.

	   The path rides RBX::utf8_decode (0x723949 = the 32B
	   std::string copy-ctor wrapper, retn 4 = the sret-clean) and
	   drives std::ifstream(decoded, 0x0C = ios::in | ios::ate)
	   (raw 0x7C1F56-79).

	   The reader = 0x7C22AA (63B): std::istream::read(out, count)
	   then the [stream + 4]-vs-count test -- GCC's basic_istream =
	   vptr at +0 with the gcount member at +0x04 (the virtual-base
	   layout puts basic_ios last), so the mismatch = the throw.
	   The throw = 0x7C22DB-0x7C2319: __cxa_allocate_exception(8) +
	   RBX::runtime_error(ex, fmt, ...) + __cxa_throw under the
	   std::runtime_error typeinfo; the builder = 0x144EA7
	   (__ZN3RBX13runtime_errorEPKcz = vformat then the
	   std::runtime_error init) and 0x144F4E
	   (__ZN3RBX7vformatEPKcPc = NULL fmt -> "", vsnprintf 0xA1
	   into a 161B stack buffer, ret > 160 = n = min(ret, 1000000)
	   with the new[] cmov clamp re-format, buf[n] = NUL).  The
	   what-strings spell identically below (the ostringstream =
	   our compiler's formatter; DELTA = the binary's va list was
	   the plain x86 char* argv).

	   "RBXF" = 0x46584252 ('RBXF' little-endian) and the count
	   > 15 guard (cmp edi, 0Fh; jle) = both arms feeding the
	   "Corrupt file header" throw at 0x7C21CB.

	   The arena reads = the exact splits in the member stores
	   below.  The tail PARKS behind the Ogre import: getSingleton
	   = the @executable_path/../Frameworks/Ogre.framework thunk
	   (jmp ds:__ZN4Ogre14TextureManager12getSingletonEv_ptr,
	   0x96D114; body absent in this DB); the +0x98 vfunc slot call
	   (raw 0x7C209B-0x7C20E3) = the createManual family with the
	   ByVal TexturePtr out at var_2E0, tm = the receiver slot,
	   textureName as the first member arg, the resource-group
	   string from the PIC+0x359A04 literal = "General"
	   (0xB1B8F5 in __cstring), tex-kind 2, 0, 0, gamma 1.0f, 0, 0,
	   0; then sub_7B6E28(this+0x24, &result) = the counted assign
	   (the +0xC slot write-through with ++ctrl count and the
	   16B handle temp {off_F96608, px, ctrl, x}; 0x7B6E28 raw
	   0x7B6E44-71) and sub_818AF8(&result) = the temp release.
	   DELTA: our second base = the by-value Ogre::TexturePtr
	   (rarc37 classify: off_F96628 dynamic = _ZTIN4Ogre10TexturePtrE;
	   the inner SharedPtr<Texture> base view = off_F96608), spelled
	   as the undeclared raw words -- the counted machinery is the
	   GfxBase counting arc when it lands. */
	// STUB: PLAYER 0x0051dcd0
	// STUB: STUDIO 0x005b4bb0
	TypesetterBitmap::TypesetterBitmap(const std::string& path,
		int textureName, float pixelBase)
	{
		(void)textureName;                         // the parked tail's slot (raw 0x7C209B)

		_pixelBase = pixelBase;                    // +0x04 (0x7C1F0E-13)
		_arena = NULL;                             // +0x20 = 0 (0x7C1F18)
		_secondBaseDelta[1] = NULL;                // +0x28 = 0 (0x7C1F1F)
		_secondBaseDelta[2] = NULL;                // +0x2C = 0 (0x7C1F26)
		_secondBaseDelta[3] = NULL;                // +0x30 = 0 (0x7C1F2D)
		_secondBaseDelta[0] =
			reinterpret_cast<void*>(0x00EF96628);  // +0x24 = off_F96628 (0x7C1F34-3A)

		std::string decoded(path);                 // utf8_decode = the copy (0x7C1F56)
		std::ifstream stream(decoded.c_str(),
			std::ios::in | std::ios::ate);         // openmode 0x0C (0x7C1F71-79)

		int magic = 0;                             // var_2CC
		stream.read(reinterpret_cast<char*>(&magic), 4);
		if (static_cast<int>(stream.gcount()) != 4)
		{
			// mac: 0x7C22DB-0x7C2319 through RBX::runtime_error and
			// RBX::vformat (see the note above).
			std::ostringstream what;
			what << "Unexpected end of file while reading " << 4;
			throw std::runtime_error(what.str());
		}
		if (magic != 0x46584252)                   // 'RBXF' (0x7C1FD3-1DD)
			throw std::runtime_error("Corrupt file header");   // 0x7C21CB

		int count = 0;                             // var_2D0
		stream.read(reinterpret_cast<char*>(&count), 4);
		if (static_cast<int>(stream.gcount()) != 4)
		{
			std::ostringstream what;
			what << "Unexpected end of file while reading " << 4;
			throw std::runtime_error(what.str());
		}
		if (count <= 15)                           // cmp edi, 0Fh; jle (0x7C1FE9-FEC)
			throw std::runtime_error("Corrupt file header");   // 0x7C21CB

		unsigned char* fresh = new unsigned char[count];   // 0x7C1FF2-5
		unsigned char* old = _arena;
		_arena = fresh;                            // +0x20 store (0x7C1FFD)
		if (old != NULL)
			delete[] old;                          // 0x7C2007 (the exchange)

		stream.read(reinterpret_cast<char*>(fresh), count);   // 0x7C2015-20
		if (static_cast<int>(stream.gcount()) != count)
		{
			std::ostringstream what;
			what << "Unexpected end of file while reading " << count;
			throw std::runtime_error(what.str());
		}

		_sizeCount = *reinterpret_cast<int*>(fresh + 0);       // +0x08 = dword[0] (0x7C2028-30)
		_rowStrideDelta =
			*reinterpret_cast<int*>(fresh + 4);                // +0x0C = dword[1] (0x7C202A-2D)
		_charClass = fresh + 8;                                // +0x10 (0x7C2036)
		_sizeTable = reinterpret_cast<const float*>(fresh + 0x108);   // +0x14 (0x7C203F)
		_metricRows = fresh + 0x108 + 4 * _sizeCount;          // +0x18 (0x7C2042-9)
		_charShift = fresh + 0x108 + 4 * _sizeCount
			+ 32 * _rowStrideDelta * _sizeCount;               // +0x1C = the imul/shl-5 chain (0x7C204C-5B)

		/* mac: 0x7C205E-0x7C2101: the parked texture tail -- getSingleton,
		   the +0x98 createManual-family slot, sub_7B6E28's counted
		   assign into this+0x24, sub_818AF8's temp release (the
		   detail lives in the note above).  The loader's own
		   exception unwinding also delete[]s +0x20 and releases the
		   +0x24 subobject through sub_818AF8 (raw 0x7C227D-0x7C229A);
		   C++ semantics handle our version. */
	}

} // namespace RBX

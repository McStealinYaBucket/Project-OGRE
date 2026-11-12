/* Build shim, not 2013 code. PLAYER and STUDIO link no libpng (no png_* strings, no libpng version
** string; /OPT:REF dropped GImage_png and everything it used), so the libpng that G3D 8.00 bundled is
** unknown. GImage.cpp only needs this declaration (the same one tools/lib/g3d_match.py compiles with). */
#pragma once
typedef unsigned char *png_bytep;
typedef unsigned int png_size_t;
extern "C" int png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num_to_check);

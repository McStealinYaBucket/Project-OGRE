/* Build shim. Of libpng, PLAYER and STUDIO link only png_sig_cmp (PLAYER 0x009e96c0, STUDIO 0x006bdbf0,
** called by GImage::resolveFormat; no other png_* code or strings, no version string: /OPT:REF dropped
** GImage_png and everything it used). These are the declarations GImage.cpp needs. */
#pragma once
typedef unsigned char *png_bytep;
typedef unsigned int png_size_t;
extern "C" int png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num_to_check);

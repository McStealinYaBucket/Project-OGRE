// Build shim: the definitions GImage.cpp references from GImage_png.cpp and libpng. GImage_png.cpp
// is not linked in PLAYER or STUDIO; of libpng only png_sig_cmp is (see png.h here). Our exes link
// with /OPT:NOREF, so GImage.cpp's references must resolve.
#include <string.h>
#include "png.h"
#include "G3D/GImage.h"

// png_sig_cmp, libpng 1.x png.c, written from the PLAYER bytes at 0x009e96c0 (STUDIO 0x006bdbf0):
// the signature built on the stack byte by byte, then an /Oi-inlined memcmp. GImage::resolveFormat
// (0x009df070) calls it. LIBRARY: third party, paired by a CSV row, not a marker.
extern "C" int png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num_to_check)
{
	unsigned char png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

	if (num_to_check > 8)
		num_to_check = 8;
	else if (num_to_check < 1)
		return (-1);

	if (start > 7)
		return (-1);

	if (start + num_to_check > 8)
		num_to_check = 8 - start;

	return ((int)(memcmp(&sig[start], &png_signature[start], num_to_check)));
}

namespace G3D {

void GImage::encodePNG(BinaryOutput&) const
{
}

void GImage::decodePNG(BinaryInput&)
{
}

} // namespace G3D

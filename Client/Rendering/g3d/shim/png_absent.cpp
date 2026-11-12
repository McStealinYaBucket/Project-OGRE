// Build shim, not 2013 code: the definitions GImage.cpp references from GImage_png.cpp and libpng,
// neither of which is linked in PLAYER or STUDIO (see png.h here). Our exes link with /OPT:NOREF, so
// GImage.cpp's references must resolve. Nothing here has an original counterpart or a marker.
#include "png.h"
#include "G3D/GImage.h"

extern "C" int png_sig_cmp(png_bytep, png_size_t, png_size_t)
{
	return 1;
}

namespace G3D {

void GImage::encodePNG(BinaryOutput&) const
{
}

void GImage::decodePNG(BinaryInput&)
{
}

} // namespace G3D

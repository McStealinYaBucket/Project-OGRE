/* App/gui/AdornBillboarder.cpp -- October 2013 (billboard arc).
   Bodies: the ctor 0x81FF2A (205B, the fully evidenced C2; C1
   0x81FF20 = a 9B thunk to it), isVisibleAndValid 0x820864 (15B),
   getViewport 0x820874 (55B, the extents-slot override), getCamera
   0x821F12 (7B null return), line2d 0x821FF6 (5B empty; inline in the
   header), drawFont2D 0x822044 (145B, the vp->isValid gate), the
   176B prim throwers (rarc9/box 0x8222E6 = the uniform
   runtime_error("Invalid operation") body).
   Parked sig-stubs (the census bodies ride the render arcs; rarc2
   signatures): rect2d x4 (0x8208AC 87B / 0x820904 942B / 0x820CB2 84B
   / 0x820D06 54B), outlineRect2d x2 (0x820E0E 954B / 0x821FFC 5B
   empty), convexPolygon2d 0x820D3C (209B), get2DStringBounds
   0x822002 (66B), and the texture trio -- createTextureProxy
   0x821F54 / setTexture 0x821FA4 / getTextureSize 0x821FCC delegate
   into the adorn member's vtable slots our Adorn.h does not declare
   (+0x0C/+0x24/+0x28).
   Evidence: rarc2 (the census mangles), rarc7 (the C2 decomp), rarc8
   (the C2 full raw + the small bodies), rarc9 (the object-vptr table
   off_EF7498 slots 0..34 + the box thrower).
   Self-audit DELTAS: (1) the binary vptr store off_EF7498 = the
   implicit MSVC ABI here (the class layout is DELTA'd; the pass
   virtuals stay Adorn's); (2) the extents passthrough = a blind 16B
   memcpy of the struct-identical Rect into G3D::Rect2D -- the binary
   vcalls the adorn vtable+0x20 into the very block it passes to
   update (0x81FF5C..0x81FF6B) and our Rect2D public ctors all sort;
   (3) the `frame` copy = the memberwise CoordinateFrame copy == the
   binary's Matrix3 copy-ctor + translation memberwise (0x81FF85..9D)
   in float values; (4) drawFont2D's delegate routes through the AB's
   own drawFont2DImpl virtual (the binary delegates the adorn member's
   slot 20; our Adorn declares no such slot); (5) getTextureSize /
   createTextureProxy / drawFont2DImpl return types are unobservable
   at the slots -- landed shared_ptr / Vector2 / Vector2 shape
   guesses (drawFont2DImpl's delegate result flows back through
   drawFont2D in the binary, hence the Vector2). */

#include "stdafx.h"

#include "Gui/AdornBillboarder.h"
#include "Gui/ViewportBillboarder.h"
#include "V8DataModel/Camera.h"

#include <string.h>

using namespace G3D;

namespace RBX
{

AdornBillboarder::AdornBillboarder(Adorn* adornArg, const Camera& camera,
	G3D::Vector3 size, G3D::CoordinateFrame frame, ViewportBillboarder& vpArg)
	: Adorn(), adorn(adornArg), vp(&vpArg), flag(false)
{
	// The tail: vp.update(rect, camera, size, frameCopy) --
	// mac: 0x81FFCE update(this = the arg's vp reference, &var_20, arg_8,
	// the ebp-0x30 size byval, &the ebp-0x60 CF copy).
	RBX::Rect ext = adorn->get2DExtents();
	G3D::Rect2D rectTemp;
	memcpy(&rectTemp, &ext, sizeof(G3D::Rect2D));

	G3D::CoordinateFrame frameCopy = frame;

	vp->update(rectTemp, camera, size, frameCopy);
}

// STUB: PLAYER 0x0096baf0
// STUB: STUDIO 0x00a1a5c0
G3D::Vector2 AdornBillboarder::drawFont2D(const std::string& text,
	const G3D::Vector2& position, float fontSize,
	const G3D::Color4& color, const G3D::Color4& outlineColor,
	Text::Font font, Text::XAlign xAlign, Text::YAlign yAlign,
	const G3D::Vector2& scale, const G3D::Rect2D& clipRect)
{
	// mac: 0x822044 (145B): vp->isValid gates the delegate.
	if (vp->isValid) {
		return drawFont2DImpl(adorn, text, position, fontSize,
			color, outlineColor, font, xAlign, yAlign, scale, clipRect);
	}
	return G3D::Vector2(0.0f, 0.0f);
}

Rect AdornBillboarder::get2DExtents()
{
	// The slot-8 twin (the binary's own-named getViewport 0x820874;
	// const there, our Adorn decl is non-const): the memberwise
	// vp->rect copy riding our Rect container.
	return Rect(vp->rect);
}

// STUB: PLAYER 0x009d3450
// STUB: STUDIO 0x00a9fd20
bool AdornBillboarder::isVisibleAndValid() const
{
	// mac: 0x820864 (15B): movzx of the +0x40 isValid byte through the
	// stored vp; the method body reads only it.
	return vp->isValid;
}

// STUB: PLAYER 0x009d3470
// STUB: STUDIO 0x00a9fd40
G3D::Rect2D AdornBillboarder::getViewport() const
{
	// mac: 0x820874 (55B): the four movss of the vp->rect homes
	// (+0x30..+0x3C) into the sret destination = the memberwise
	// Rect2D copy.
	return vp->rect;
}

// STUB: PLAYER 0x009d5600
// STUB: STUDIO 0x00aa1ed0
boost::shared_ptr<TextureProxyBase> AdornBillboarder::createTextureProxy(
	const ContentId& contentId, bool& flag, bool b)
{
	// mac: 0x821F54 (56B): delegates the adorn member's vtable+0x0C; our
	// Adorn declares no such slot -- sig-correct stub (DELTAs (2)/(5)).
	(void)contentId; (void)flag; (void)b;
	return boost::shared_ptr<TextureProxyBase>();
}

// STUB: PLAYER 0x009d5650
// STUB: STUDIO 0x00aa1f20
Camera* AdornBillboarder::getCamera() const
{
	// mac: 0x821F12 (7B): xor eax, eax -- the head-slot override is always
	// null; the camera flows through the VP's update instead.
	return NULL;
}

// STUB: PLAYER 0x009d5660
// STUB: STUDIO 0x00aa1f30
void AdornBillboarder::setTexture(int index,
	const boost::shared_ptr<TextureProxyBase>& textures)
{
	// mac: 0x821FA4 (39B): delegates the adorn member's vtable+0x24; our
	// Adorn declares no such slot -- sig-correct stub (DELTAs (2)).
	(void)index; (void)textures;
}

// STUB: PLAYER 0x009d5690
// STUB: STUDIO 0x00aa1f60
G3D::Vector2 AdornBillboarder::getTextureSize(
	const boost::shared_ptr<TextureProxyBase>& textures) const
{
	// mac: 0x821FCC (41B): delegates the adorn member's vtable+0x28; our
	// Adorn declares no such slot -- sig-correct stub (DELTAs (2)/(5)).
	(void)textures;
	return G3D::Vector2(0.0f, 0.0f);
}

} // namespace RBX

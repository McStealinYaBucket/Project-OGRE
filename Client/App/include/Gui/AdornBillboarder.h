/* Gui/AdornBillboarder.h -- October 2013 (billboard arc; the chat-bubble
   render board).  Evidence: the AB method census (rarc2 names
   0x81FF20..0x8229C6), the C2 0x81FF2A (205B; rarc7 + rarc8 full raw),
   the object-vptr table off_EF7498 (rarc9; 35 virtual slots 0..34 --
   slot 35 onward is the RTTI machinery), the small bodies (rarc8:
   isVisibleAndValid 15B, getViewport 55B, getCamera 7B null-return,
   line2d 5B empty, D0 48B) and the uniform 176B throwers (rarc9 box
   0x8222E6: __cxa_throw std::runtime_error("Invalid operation")).
   Binary vptr map (vptr-relative): +0x00 getCamera override (the Adorn
   +0x00 head), +0x04/+0x08 D1/D0, +0x0C createTextureProxy, +0x10..+0x1C
   the four pass virtuals (Adorn bodies, inherited NOT overridden),
   +0x20 getViewport (the Adorn 4-float-sret extents position: memberwise
   return of vp->rect), +0x24 setTexture, +0x28 getTextureSize, +0x2C
   line2d, +0x30/+0x34 outlineRect2d x2 (+0x34 empty body), +0x38..+0x44
   rect2d x4, +0x48 get2DStringBounds, +0x4C drawFont2D (the Adorn
   drawFont2D position: vp->isValid gate + the +0x50 delegate),
   +0x50 drawFont2DImpl, +0x54..+0x84 the 3D prim family
   (line3d, setObjectToWorldMatrix, box, sphere, explosion,
   cylinderAlongX, ray, lineSegment, axes, quad, convexPolygon,
   convexPolygon2d, extrusion -- every one a runtime_error thrower),
   +0x88 getRenderCaps (delegates the adorn member's vtable+0x88).
   Object layout: the Adorn base ends at +0x1C (base = 28B), then
   +0x1C Adorn* adorn (the ctor's first explicit param), +0x20
   ViewportBillboarder* vp (stored from the RNS_& param), +0x24 flag
   byte (zeroed by the C2; nothing else touched).
   DELTAS: (1) our landed Adorn.h names the prim slots for its render
   arcs (get2DExtents/drawRectangle/fillRectangle/drawFont2D); the
   binary AB fills those vtable positions with its own-named
   getViewport (8), outlineRect2d (12), rect2d (16), drawFont2D (19) --
   MSVC ordering cannot reproduce the GCC slot map, so the overrides
   below keep the landed Adorn names and the own-named AB methods are
   declared alongside (slot identities pinned in comments); (2) the
   delegates whose callee rides an Adorn slot our Adorn does not declare
   (createTextureProxy -> adorn+0x0C, setTexture/getTextureSize ->
   adorn+0x24/+0x28, getRenderCaps -> adorn+0x88, drawFont2D ->
   the adorn-slot-20 drawFont2DImpl) land as sig-correct stubs or route
   through the AB's own drawFont2DImpl virtual; (3) getTextureSize's
   return type is unobservable at the slot (41B delegation) -- landed
   G3D::Vector2 as the w/h reading, DELTA; (4) C1 0x81FF20 = a 9B thunk
   to the C2. */

#ifndef RBX_ADORNBILLBOARDER_H
#define RBX_ADORNBILLBOARDER_H

#include "Gui/Adorn.h"
#include "G3D/AABox.h"
#include "G3D/LineSegment.h"
#include "G3D/Sphere.h"
#include "RbxG3D/RbxRay.h"
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <string>

namespace RBX
{

	class Camera;
	class ContentId;
	class I3DLinearFunc;
	class TextureProxyBase;
	class ViewportBillboarder;

	class AdornBillboarder : public Adorn
	{
	public:
		// mac: 0x81FF2A (205B): the Adorn base C2, then adorn @+0x1C, the
		// vp reference @+0x20, flag = 0, the adorn extents-slot vcall
		// (the rect here), a memberwise CF copy of `frame`, and the
		// tail vp.update(rect, camera, size, frameCopy).
		AdornBillboarder(Adorn* adorn, const Camera& camera,
			G3D::Vector3 size, G3D::CoordinateFrame frame,
			ViewportBillboarder& vp);

		// mac: 0x820864 (15B, non-virtual): reads the +0x40 isValid byte
		// through the stored vp reference.
		bool isVisibleAndValid() const;

		// mac: 0x820874 (55B, virtual slot 8 = the Adorn extents position):
		// the memberwise copy of the vp->rect (+0x30..+0x3C) homes
		// into the sret destination.
		virtual G3D::Rect2D getViewport() const;

		// mac: 0x821F12 (7B, slot 0 = the Adorn +0x00 head override): xor
		// eax, eax -- a constant null; the camera flows through the
		// VP's update instead.
		virtual Camera* getCamera() const;

		// --- the Adorn prim-slot overrides (naming DELTA (1): the
		// binary's own-named twins are below) ---
		virtual Rect get2DExtents();
		virtual void drawRectangle(const Rect& rect, float lineWidth,
			const G3D::Color4& color)
		{
			// slot 12 twin (AB::outlineRect2d 0x820E0E, 954B): the real
			// body is parked with the render arcs.
			(void)rect; (void)lineWidth; (void)color;
		}
		virtual void fillRectangle(const Rect& rect, const G3D::Color4& color)
		{
			// slot 16 twin (AB::rect2d(rect,color) 0x820CB2, 84B): parked.
			(void)rect; (void)color;
		}
		// mac: 0x822044 (145B): vp->isValid gates the drawFont2DImpl delegate;
		// the binary delegates on the adorn member's slot 20, our Adorn
		// declares no such slot, so the call routes through the AB's own
		// drawFont2DImpl (DELTAs (2)).
		virtual G3D::Vector2 drawFont2D(const std::string& text,
			const G3D::Vector2& position, float fontSize,
			const G3D::Color4& color, const G3D::Color4& outlineColor,
			Text::Font font, Text::XAlign xAlign, Text::YAlign yAlign,
			const G3D::Vector2& scale, const G3D::Rect2D& clipRect);

		// --- the AB's own-named set (census order; the meat behind the
		// rampaging prim slots, per the banner slot identities) ---
		virtual boost::shared_ptr<TextureProxyBase> createTextureProxy(
			const ContentId& contentId, bool& flag, bool b);
		virtual void setTexture(int index,
			const boost::shared_ptr<TextureProxyBase>& textures);
		virtual G3D::Vector2 getTextureSize(
			const boost::shared_ptr<TextureProxyBase>& textures) const;

		// STUB: PLAYER 0x009d56c0
		// STUB: STUDIO 0x00aa1f90
		virtual void line2d(const G3D::Vector2& p0, const G3D::Vector2& p1,
			const G3D::Color4& color) const
		{
			(void)p0; (void)p1; (void)color;
		}

		virtual void outlineRect2d(const G3D::Rect2D& rect, float lineWidth,
			const G3D::Color4& color) const
		{
			// mac: 0x820E0E (954B): parked with the render arcs.
			(void)rect; (void)lineWidth; (void)color;
		}
		virtual void outlineRect2d(const G3D::Rect2D& rect, float lineWidth,
			const G3D::Color4& color, const G3D::Rect2D& clipRect) const
		{
			(void)rect; (void)lineWidth; (void)color; (void)clipRect;
		}

		virtual void rect2d(const G3D::Rect2D& rect, const G3D::Rect2D& clipRect,
			const G3D::Color4& color) const
		{
			// mac: 0x8208AC (87B): parked with the render arcs.
			(void)rect; (void)clipRect; (void)color;
		}
		virtual void rect2d(const G3D::Rect2D& rect, const G3D::Rect2D& clipRect,
			const G3D::Vector2& p0, const G3D::Vector2& p1,
			const G3D::Color4& color) const
		{
			// mac: 0x820904 (942B): parked with the render arcs.
			(void)rect; (void)clipRect; (void)p0; (void)p1; (void)color;
		}
		virtual void rect2d(const G3D::Rect2D& rect, const G3D::Color4& color) const
		{
			// mac: 0x820CB2 (84B): parked with the render arcs.
			(void)rect; (void)color;
		}
		virtual void rect2d(const G3D::Rect2D& rect, const G3D::Vector2& p0,
			const G3D::Vector2& p1, const G3D::Color4& color) const
		{
			// mac: 0x820D06 (54B): parked with the render arcs.
			(void)rect; (void)p0; (void)p1; (void)color;
		}

		// STUB: PLAYER 0x009d56e0
		// STUB: STUDIO 0x00aa1fb0
		virtual G3D::Vector2 get2DStringBounds(const std::string& text,
			float fontSize, Text::Font font, const G3D::Vector2& scale) const
		{
			// mac: 0x822002 (66B): parked with the render arcs.
			(void)text; (void)fontSize; (void)font; (void)scale;
			return G3D::Vector2(0.0f, 0.0f);
		}

		// STUB: PLAYER 0x009d5720
		// STUB: STUDIO 0x00aa1ff0
		virtual G3D::Vector2 drawFont2DImpl(RBX::Adorn* adorn, const std::string& text,
			const G3D::Vector2& position, float fontSize,
			const G3D::Color4& color, const G3D::Color4& outlineColor,
			Text::Font font, Text::XAlign xAlign, Text::YAlign yAlign,
			const G3D::Vector2& scale, const G3D::Rect2D& clipRect)
		{
			// mac: 0x8220D6 (176B, slot 20): the uniform thrower body -- the
			// 176B prim family's own callee on the AB side; the Vector2
			// return rides DELTAs (5) (the 8B eax:edx pair in GCC).
			(void)adorn; (void)text; (void)position; (void)fontSize;
			(void)color; (void)outlineColor; (void)font; (void)xAlign;
			(void)yAlign; (void)scale; (void)clipRect;
			throw std::runtime_error("Invalid operation");
		}

		// --- the 3D prim family (slots 21..33; every 176B body = the
		// runtime_error("Invalid operation") thrower, rarc9/box) ---
		virtual void line3d(const G3D::Vector3& p0, const G3D::Vector3& p1,
			const G3D::Color4& color)
		{
			(void)p0; (void)p1; (void)color;
			throw std::runtime_error("Invalid operation");
		}
		virtual void setObjectToWorldMatrix(const G3D::CoordinateFrame& objectToWorld)
		{
			(void)objectToWorld;
			throw std::runtime_error("Invalid operation");
		}
		// STUB: PLAYER 0x009d5840
		// STUB: STUDIO 0x00aa2110
		virtual void box(const G3D::AABox& box, const G3D::Color4& color)
		{
			(void)box; (void)color;
			throw std::runtime_error("Invalid operation");
		}
		virtual void sphere(const G3D::Sphere& sphere, const G3D::Color4& color)
		{
			(void)sphere; (void)color;
			throw std::runtime_error("Invalid operation");
		}
		virtual void explosion(const G3D::Sphere& sphere)
		{
			(void)sphere;
			throw std::runtime_error("Invalid operation");
		}
		virtual void cylinderAlongX(float a, float b, const G3D::Color4& color,
			bool flag)
		{
			(void)a; (void)b; (void)color; (void)flag;
			throw std::runtime_error("Invalid operation");
		}
		virtual void ray(const RbxRay& ray, const G3D::Color4& color)
		{
			(void)ray; (void)color;
			throw std::runtime_error("Invalid operation");
		}
		virtual void lineSegment(const G3D::LineSegment& lineSegment,
			const G3D::Color4& color, float lineWidth)
		{
			(void)lineSegment; (void)color; (void)lineWidth;
			throw std::runtime_error("Invalid operation");
		}
		virtual void axes(const G3D::Color4& c0, const G3D::Color4& c1,
			const G3D::Color4& c2, float lineWidth)
		{
			(void)c0; (void)c1; (void)c2; (void)lineWidth;
			throw std::runtime_error("Invalid operation");
		}
		virtual void quad(const G3D::Vector3& p0, const G3D::Vector3& p1,
			const G3D::Vector3& p2, const G3D::Vector3& p3,
			const G3D::Color4& color, const G3D::Vector2& uv0,
			const G3D::Vector2& uv1, int i, bool b)
		{
			(void)p0; (void)p1; (void)p2; (void)p3; (void)color;
			(void)uv0; (void)uv1; (void)i; (void)b;
			throw std::runtime_error("Invalid operation");
		}
		virtual void convexPolygon(const G3D::Vector3* points, int count,
			const G3D::Color4& color, int b)
		{
			(void)points; (void)count; (void)color; (void)b;
			throw std::runtime_error("Invalid operation");
		}
		// STUB: PLAYER 0x009d3b20
		// STUB: STUDIO 0x00aa03f0
		virtual void convexPolygon2d(const G3D::Vector2* points, int count,
			const G3D::Color4& color, int b) const
		{
			// mac: 0x820D3C (209B): parked with the render arcs.
			(void)points; (void)count; (void)color; (void)b;
		}
		virtual void extrusion(I3DLinearFunc* f0, int i0, I3DLinearFunc* f1,
			int i1, const G3D::Color4& color, bool b0, bool b1)
		{
			(void)f0; (void)i0; (void)f1; (void)i1; (void)color;
			(void)b0; (void)b1;
			throw std::runtime_error("Invalid operation");
		}

		// mac: 0x8229C6 (28B, slot 34): delegates the adorn member's
		// vtable+0x88; our Adorn declares no such slot -- stub (DELTAs (2)).
		// STUB: PLAYER 0x009d5b10
		// STUB: STUDIO 0x00aa23e0
		virtual unsigned int getRenderCaps() const
		{
			return 0;
		}

		// object members (binary order; the Adorn base ends at +0x1C):
		Adorn* adorn;                     // +0x1C
		ViewportBillboarder* vp;          // +0x20 (the ctor's RNS_& pinned as a pointer)
		bool flag;                        // +0x24 (zeroed by the C2)
	};

} // namespace RBX

#endif

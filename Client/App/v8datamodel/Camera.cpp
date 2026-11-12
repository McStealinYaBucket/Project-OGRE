/* App/v8datamodel/Camera.cpp -- October 2013 (camera render-shape arc).
   Bodies: C2 0x289114 (647B) / frustum 0x2898F0 (103B; 2-arg entry) /
   frustum 0x28E146 (398B, 3-arg build) / getImagePlaneDepth 0x28DD88
   (41B; rarc5) / project 0x28DDB2 (342B; rarc4 raw+decomp, rarc5 pins).
   Self-audit: the member map + the C2 explicit writes landed from the
   bubsurf4/bubsurf5 raw paso (see Camera.h for the map).  DELTAS:
   (1) the C2's +0x64/+0x68 signal connect sites (the call_once dance)
   are UNPORTED (the signal ctor path runs from the base chain); (2) the
   rotations/negates are carried in source float ops (the binary stages
   sign-masked xmss negates -- same IEEE result); (3) the fov = the raw
   dword 0xFFFFFFFF via a bit-union (quiet_NaN would bake the wrong
   bits; bubsurf5 pin); (4) the +0x1AC..+0x1B4 member gap = unmodeled
   (nothing writes or reads it in this decode -- queued with the
   viewports); (5) the project reject gate = the binary's literal
   `0 >= depth` (ucomiss 0,depth + jnb): NaN depth falls through to the
   scale path -- do NOT rewrite as !(0 < depth), which rejects NaN. */

#include "stdafx.h"

#include "V8DataModel/Camera.h"
#include "RbxG3D/Frustum.h"
#include "Util/Math.h"
#include <math.h>

namespace RBX {
	const char* const sCamera = "Camera";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Camera);
}

using namespace G3D;

// The raw fov dword (0x1A8) per bubsurf5: 0xFFFFFFFF = a -NaN pattern
// (quiet_NaN would bake 0xFFC00000); the C2 loads dword_FB009C into
// +0x1A8 directly, so landed bit-casts the same 4 bytes.
static float fovRawBits()
{
	union { unsigned int u; float f; } bits;
	bits.u = 0xFFFFFFFFu;
	return bits.f;
}

// C2 0x289114 (647B): the DescribedCreatable chain runs first; the
// binary's explicit member writes land below in Camera.h member order
// (the +0x64/+0x68 signals + the 4-slot front run from the base chain --
// DELTA at the header).
Camera::Camera()
{
	// +0x70: looks at the origin, then steps the frame to (0,20,20).
	cf.lookAt(Vector3::zero());
	cf.translation = Vector3(0.0f, 20.0f, 20.0f);
	// +0xA0: identity.
	unknownA0 = Matrix3::identity();
	// +0xC4/+0xC8/+0xCC.
	unknownC4 = 0;
	unknownC8 = 0;
	unknownCC = -5.0f;
	// +0x190..+0x1A0 (preRenderTmpCFs[4] default-constructs).
	unknown190 = 0;
	unknown194 = -1.0f;
	unknown198 = 0;
	unknown19C = 0;
	unknown1A0 = 0;
	// +0x1A4.
	farPlane = 5000.0f;
	// +0x1A8.
	fov = fovRawBits();
	// +0x1B8.
	cameraPanMode = 0;
	// +0x1BC: 1/(2*tan(fov*0.5)) -- the NaN propagates through tanf.
	zoom = 1.0f / (2.0f * tanf(fov * 0.5f));
}

// mac: 0x2898F0 (103B): the negated far plane feeds the 3-arg build; the out
// slot returns by value (the binary sign-masks the negate -- the source
// unary minus bakes the same IEEE result).
// STUB: PLAYER 0x006b6e60
// STUB: STUDIO 0x006fa5b0
Frustum Camera::frustum(const G3D::Rect2D& rect) const
{
	Frustum out;
	frustum(rect, -farPlane, out);
	return out;
}

// mac: 0x28E146 (398B, const): the frustum build.  Sequence per the bubsurf6
// 403-445 + prologue decode: resize(0,false) FIRST; hasNanOrInf(cf)
// early-out (leaves the caller's out.faceArray empty); width/height
// from the rect panes; k = 0.5f/zoom (dword_C1E4BC = 0.5f);
// horizontalFov = ((width*k)/height*fov)/k left-to-right; the local
// Frustum(translation, -column(2), column(1), 0.5f, -negatedFar,
// horizontalFov, fov) via the RbxG3D ctor; the face array copy out.
// STUB: PLAYER 0x006b6cd0
// STUB: STUDIO 0x006fa420
void Camera::frustum(const G3D::Rect2D& rect, float negatedFarPlane,
	Frustum& out) const
{
	out.faceArray.resize(0, false);
	if (RBX::Math::hasNanOrInf(cf))
		return;
	float width = rect.x1() - rect.x0();
	float height = rect.y1() - rect.y0();
	float k = 0.5f / zoom;
	float horizontalFov = ((width * k) / height * fov) / k;
	Frustum local(
		cf.translation,
		-cf.rotation.column(2),
		cf.rotation.column(1),
		0.5f,
		-negatedFarPlane,
		horizontalFov,
		fov);
	out.faceArray = local.faceArray;
}

// mac: 0x28DD88 (41B, const): the image-plane depth the imposter chain uses =
// rect height * zoom (the binary returns the float through the x87 st0
// GCC return-ABI noise; the MSVC value return is the same float).
float Camera::getImagePlaneDepth(const G3D::Rect2D& rect) const
{
	return (rect.y1() - rect.y0()) * zoom;
}

// The reject-corner +/- infinities carry as exact-bit unions (the binary
// loads dword_C3AF10 = 0x7F800000 +inf / dword_C3AF14 = 0xFF800000 -inf;
// see fovRawBits above for the bit-union convention).
static float pinfRaw()
{
	union { unsigned int u; float f; } bits;
	bits.u = 0x7F800000u;
	return bits.f;
}

static float ninfRaw()
{
	union { unsigned int u; float f; } bits;
	bits.u = 0xFF800000u;
	return bits.f;
}

// mac: 0x28DDB2 (342B, const): world -> screen.  Camera-space terms are
// rotation-COLUMN dots of the world-relative point (binary build order:
// camY col1 first, then camX col0, then depth = the sign-mask negate of
// the col2 dot, dword_C1E570 carried as a source unary minus -- same
// IEEE result per the Camera.cpp DELTA (2) convention).
//
// Gate is the literal `0 >= depth` (ucomiss 0.0,depth + jnb: taken iff
// ordered depth <= 0).  NaN-depth does NOT reject -- jnb is not taken on
// unordered, so NaN depth falls through to the scale path.  Do NOT
// rewrite as !(0 < depth), which would also reject NaN (different truth
// table).
//
// Reject corner (dword_C3AF10/C3AF14 pins): out.x = camX < 0 ? -inf : +inf
// (NaN -> +inf via the ja = 0 > camX polarity); out.y = camY > 0 ? -inf :
// +inf (NaN -> +inf); out.z = +inf (raw 7F800000 literal).
//
// Scale path: halfW = (float)(int)rect.width() * 0.5f (dword_C1E4BC);
// scale = (rect.height() * zoom) / depth; out.x = ((camX * scale) * 1.0f
// literal mul, dword_C1C918 kept) + halfW; out.y = (float)(int)
// rect.height() * 0.5f - (camY * scale); out.z = scale.
G3D::Vector3 Camera::project(const G3D::Vector3& worldPoint,
	const G3D::Rect2D& rect) const
{
	float rx = worldPoint.x - cf.translation.x;
	float ry = worldPoint.y - cf.translation.y;
	float rz = worldPoint.z - cf.translation.z;
	const G3D::Vector3 rel(rx, ry, rz);

	float camY = cf.rotation.column(1).dot(rel);
	float camX = cf.rotation.column(0).dot(rel);
	float depth = -(cf.rotation.column(2).dot(rel));

	G3D::Vector3 out;
	if (0.0f >= depth) {
		out.x = camX < 0.0f ? ninfRaw() : pinfRaw();
		out.y = camY > 0.0f ? ninfRaw() : pinfRaw();
		out.z = pinfRaw();
	} else {
		float rectH = rect.y1() - rect.y0();
		float rectW = rect.x1() - rect.x0();
		int wInt = (int)rectW;
		float halfW = (float)wInt * 0.5f;
		int hInt = (int)rectH;
		float scale = rectH * zoom / depth;
		out.x = ((camX * scale) * 1.0f) + halfW;
		out.y = (float)hInt * 0.5f - (camY * scale);
		out.z = scale;
	}
	return out;
}

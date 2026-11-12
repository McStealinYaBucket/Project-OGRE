/* App/gui/ViewportBillboarder.cpp -- October 2013 (billboard arc).
   Bodies: C2(void) 0x8211D2 (239B) / C2(args) 0x8212CC / update
   0x81FFF8 (2155B) / projectOntoScreen 0x8213B2 (814B).
   Evidence: rarc3 (update decomp+raw, full), rarc4 (projectOntoScreen
   raw+decomp, full; Camera::project), rarc5 (rodata pins:
   C1E4B8=-0.5f, C1E4BC=0.5f, C1C918=1.0f, C1C91C=1000.0f, C1E570 sign
   mask, C3AF10/C3AF14 =+/-inf, CA2390 qword = the double 1.01),
   rarc7 (ctor symbols + C2 decomp+raw).
   Self-audit DELTAS: (1) isValid (+0x40) is written by update only --
   neither ctor touches it (decode); landed uninitialized; (2) member
   names low/high/param + the +0x90 flag = behavioral; (3) update
   returns nothing (the decomp "int return" = GCC eax result noise;
   landed void); (4) the update +0x90 gate = the binary's jnz on the
   byte; sign-flips ride source unary minus = the same IEEE result as
   the binary's sign-mask xorps; (5) the minss/maxss
   "second-operand-wins" polarity carries via the indexed ternaries;
   (6) the rect member writes land via the public Rect2D()
   zero-ctor and the unsorted Rect2D(Vector2) 0,0-wh ctor -- equals the
   binary's raw dword stores (the xyxy factory would sort, wrong). */

#include "stdafx.h"

#include "Gui/ViewportBillboarder.h"
#include "V8DataModel/Camera.h"
#include "Util/UDim.h"

using namespace G3D;

namespace RBX
{

ViewportBillboarder::ViewportBillboarder()
{
	cframe = CoordinateFrame();
	rect = Rect2D();
	low = Vector3::maxFinite();
	high = -Vector3::maxFinite();
	offset.x = 0.0f; offset.y = 0.0f; offset.z = 0.0f;
	scale.x = 0.0f; scale.y = 0.0f; scale.z = 0.0f;
	param.x = 0.0f; param.y = 0.0f;
	size.x = UDim(); size.y = UDim();
	pivot = NULL;
	projectOntoScreenEnabled = false;
}

ViewportBillboarder::ViewportBillboarder(const G3D::Vector3& v1,
	const G3D::Vector3& v2, const G3D::Vector2& p, const UDim2& s,
	const G3D::Vector2* pv)
{
	cframe = CoordinateFrame();
	rect = Rect2D();
	low = Vector3::maxFinite();
	high = -Vector3::maxFinite();
	offset.x = v1.x; offset.y = v1.y; offset.z = v1.z;
	scale.x = v2.x; scale.y = v2.y; scale.z = v2.z;
	param.x = p.x; param.y = p.y;
	size = s;
	pivot = pv;
	projectOntoScreenEnabled = false;
}

// mac: 0x81FFF8 (2155B): the per-frame billboard fold.  Prelude halves ride
// the pinned rodata (-0.5f dword_C1E4B8 / +0.5f dword_C1E4BC); the loop
// selects them per corner bit and folds each camera-space column dot
// into the low/high accumulators; the center = (offset + Vector3::one())
// * 0.5 span + low + scale staged axis-wise, world center =
// camCF.pointToWorldSpace(center), projected = camera.project(world,
// rect) gated on (0, 1000) (dword_C1C91C = 1000.0f), then the screen
// CF stages from pivot or the proj.z round-trip (the round-trip kept --
// the binary's own identity) and lands at +0x00 via the +0x90 branch.
void ViewportBillboarder::update(const G3D::Rect2D& screenRect,
	const Camera& camera, G3D::Vector3 boxSize, G3D::CoordinateFrame frame)
{
	low = Vector3::maxFinite();

	float negHalfZ = boxSize.z * -0.5f;
	float negHalfY = boxSize.y * -0.5f;
	float negHalfX = -0.5f * boxSize.x;
	float halfZ = boxSize.z * 0.5f;
	float halfY = boxSize.y * 0.5f;
	float halfX = boxSize.x * 0.5f;

	high = -Vector3::maxFinite();

	for (int i = 0; i < 8; ++i) {
		float cx = (i & 4) ? halfX : negHalfX;
		float cy = (i & 2) ? halfY : negHalfY;
		float cz = (i & 1) ? halfZ : negHalfZ;
		const G3D::CoordinateFrame& cam = camera.coordinateFrame();

		float relY = frame.rotation[1][0] * cx + frame.rotation[1][1] * cy
			+ frame.rotation[1][2] * cz + frame.translation.y
			- cam.translation.y;
		float relX = frame.rotation[0][0] * cx + frame.rotation[0][1] * cy
			+ frame.rotation[0][2] * cz + frame.translation.x
			- cam.translation.x;
		float relZ = frame.rotation[2][0] * cx + frame.rotation[2][1] * cy
			+ frame.rotation[2][2] * cz + frame.translation.z
			- cam.translation.z;

		const G3D::Vector3 rel(relX, relY, relZ);

		float camX = cam.rotation.column(0).dot(rel);
		float camY = cam.rotation.column(1).dot(rel);
		float camZ = cam.rotation.column(2).dot(rel);

		low.x = low.x < camX ? low.x : camX;
		low.y = low.y < camY ? low.y : camY;
		low.z = low.z < camZ ? low.z : camZ;
		high.x = high.x < camX ? camX : high.x;
		high.y = high.y < camY ? camY : high.y;
		high.z = high.z < camZ ? camZ : high.z;
	}

	const G3D::Vector3& one = Vector3::one();
	float oneX = offset.x + one.x;
	float oneY = offset.y + one.y;
	float oneZ = offset.z + one.z;

	float spanY = high.y - low.y;
	float spanX = high.x - low.x;
	float spanZ = high.z - low.z;
	const G3D::CoordinateFrame& cam = camera.coordinateFrame();

	float cy = (((oneY * 0.5f) * spanY) + low.y) + scale.y;
	float cx = (((oneX * 0.5f) * spanX) + low.x) + scale.x;
	float cz = (((oneZ * 0.5f) * spanZ) + low.z) + scale.z;

	G3D::Vector3 worldCenter = cam.pointToWorldSpace(Vector3(cx, cy, cz));

	G3D::Vector3 projected = camera.project(worldCenter, screenRect);

	// Gate = the binary's jbe pair (ucomiss proj.z,0 + ucomiss
	// 1000.0f,proj.z): proj.z is finite or +/-inf in the arcs so the
	// canonical <=/>= spell carries the same truth table here.
	if (projected.z <= 0.0f || projected.z >= 1000.0f) {
		isValid = false;
		return;
	}

	isValid = true;

	float projZ = projected.z;

	G3D::Vector2 oneTimesZ = Vector2::one() * projZ;
	G3D::Vector2 scaledSize = size * oneTimesZ;

	float invZ = 1.0f / projZ;
	float fy = scaledSize.y * invZ;
	float fx = invZ * scaledSize.x;

	if (pivot != NULL) {
		// {0, 0, pivot.x, pivot.y}
		rect = Rect2D(G3D::Vector2(pivot->x, pivot->y));
	} else {
		// Round-trip: proj.z * (invZ * scaled); the binary's own
		// multiply order (projz*fx then projz*fy), bit-kept.
		rect = Rect2D(G3D::Vector2(projZ * fx, projZ * fy));
	}

	G3D::Vector3 scaledOffset = Vector3(Vector2(param.x * fx,
		param.y * fy), 0.0f);

	cz = cz + scaledOffset.z;
	cy = cy + scaledOffset.y;
	cx = (cx + scaledOffset.x) - (fx * 0.5f);

	G3D::CoordinateFrame screen;
	screen.translation.x = cx;
	screen.translation.y = (0.5f * fy) + cy;
	screen.translation.z = cz;
	float x1 = rect.x1();
	float y1 = rect.y1();
	screen.rotation.set(fx / x1, 0.0f, 0.0f,
		0.0f, fy / y1, 0.0f,
		0.0f, 0.0f, 1.0f);

	if (projectOntoScreenEnabled) {
		cframe = projectOntoScreen(screenRect, camera, screen);
	} else {
		cframe = camera.coordinateFrame() * screen;
	}
}

// mac: 0x8213B2 (814B): the near-plane projection midpoint.  nearFront =
// (float)(nearPlaneZ() * 1.01) (the qword_CA2390 pin); p0 = the scaled
// frame's translation projected, p1 = the camCF-forward point projected;
// scaleRatio = p0.z / p1.z; the offset CF carries the rect-anchored
// translation terms over camera.getImagePlaneDepth(rect) and the
// diagonal rotation = (scaleRatio, scaleRatio, 1) * frame.rotation; the
// return = camCF * offsetCF.
G3D::CoordinateFrame ViewportBillboarder::projectOntoScreen(
	const G3D::Rect2D& rect, const Camera& camera,
	const G3D::CoordinateFrame& frame)
{
	const G3D::CoordinateFrame& camCF = camera.coordinateFrame();
	double nearZ = camera.nearPlaneZ();

	G3D::CoordinateFrame scaled = camCF * frame;
	G3D::Vector3 p0 = camera.project(scaled.translation, rect);

	float nearFront = (float)(nearZ * 1.01);
	G3D::Vector3 frontPos = camCF.pointToWorldSpace(Vector3(0.0f, 0.0f, nearFront));
	G3D::Vector3 p1 = camera.project(frontPos, rect);

	float scaleRatio = p0.z / p1.z;
	float imgDepth = camera.getImagePlaneDepth(rect);

	float rectH = rect.y1() - rect.y0();
	float rectW = rect.x1() - rect.x0();

	G3D::CoordinateFrame offsetCF;
	offsetCF.translation.x = -((p0.x - (rectW * 0.5f)) * nearFront) / imgDepth;
	offsetCF.translation.y = ((p0.y - (rectH * 0.5f)) * nearFront) / imgDepth;
	offsetCF.translation.z = nearFront;

	G3D::Matrix3 diag(scaleRatio, 0.0f, 0.0f,
		0.0f, scaleRatio, 0.0f,
		0.0f, 0.0f, 1.0f);
	G3D::Matrix3 prod = diag * frame.rotation;
	offsetCF.rotation = prod;

	return camCF * offsetCF;
}

}  // namespace RBX

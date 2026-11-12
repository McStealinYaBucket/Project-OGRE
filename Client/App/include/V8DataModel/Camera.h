/* V8DataModel/Camera.h -- October 2013 (camera render-shape arc).
   Evidence: C2 0x289114 (647B) member writes; coordinateFrame 0x28E09A
   (11B; header-inline -- returns the +0x70 CF by reference); frustum
   0x2898F0 (103B; 2-arg entry = the 3-arg body with -farPlane) +
   frustum 0x28E146 (398B, 3-arg); nearPlaneZ 0x28DD74 (landed; const
   -0.5); setCameraPanMode 0x288360 (landed; +0x1B8 store);
   getImagePlaneDepth 0x28DD88 (41B, decl; body in Camera.cpp);
   project 0x28DDB2 (342B, decl; body in Camera.cpp).
   Member map (GCC offsets; MSVC layout DELTA -- member ORDER is the
   invariant):
     +0x64  rbx::signals::signal<void()>        (C2 ctor; name DELTA)
     +0x68  rbx::signals::signal<void(bool)>    (C2 ctor; name DELTA)
     +0x70  G3D::CoordinateFrame cf             (C2: lookAt(Vector3::zero())
            then translation (0,20,20))
     +0xA0  G3D::Matrix3                        (C2: Matrix3::identity(); name DELTA)
     +0xC4  int (0)
     +0xC8  int (0)
     +0xCC  float (-5.0f)
     +0xD0  G3D::CoordinateFrame preRenderTmpCFs[4]  (DELTA name; through +0x18C)
     +0x190 int (0)
     +0x194 float (-1.0f)
     +0x198 int (0)
     +0x19C int (0)
     +0x1A0 int (0)
     +0x1A4 float farPlane  (C2: 5000.0f)
     +0x1A8 float fov       (C2: 0xFFFFFFFF bit pattern -- bubsurf5
            dword_FB009C "f nan"; numeric_limits<float>::quiet_NaN would
            bake 0xFFC00000, so the C2 init bit-casts the exact dword)
     +0x1AC..+0x1B4  3-slot gap unmodeled in the C2 decode (queued DELTA)
     +0x1B8 int cameraPanMode (0; landed member)
     +0x1BC float zoom      (C2: 1.0f / (2.0f * tanf(fov * 0.5f)) -- NaN
            via the fov)
   DELTAs queued: the +0x64/+0x68 signal connect sites (the C2 call_once
   dance) park with the camera-pan arc; the preRenderTmpCFs consumers
   park with the viewports; the 4-slot top gap re-decodes then.

   Method bodies live in Camera.cpp:
     coordinateFrame      0x28E09A (11B) -> header-inline here (bare
                          this+0x70 reference return in the binary).
     frustum(rect)        0x2898F0 (103B) -> frustum(rect, -farPlane, out)
                          with the out slot copied back (the binary
                          negates farPlane through a sign-masked float).
     frustum(rect, far, out) 0x28E146 (398B, const): resize(0,false)
                          first; hasNanOrInf(cf) early-out; width/height
                          from the rect; k = 0.5f/zoom; horizontalFov =
                          ((width*k)/height*fov)/k (left-to-right);
                          local Frustum(cf.translation, -column(2),
                          column(1), 0.5f, -negatedFar, horizontalFov,
                          fov); the copy into out.faceArray.

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Camera.h */

#pragma once

#include "V8Tree/Instance.h"
#include "rbx/signal.h"
#include "G3D/CoordinateFrame.h"
#include "G3D/Matrix3.h"
#include "G3D/Rect2D.h"

namespace RBX {

	extern const char* const sCamera;

	class Frustum;

	class Camera
		: public DescribedCreatable<Camera, Instance, sCamera>
	{
	public:
		// C2 0x289114 (647B) -- member map above; the explicit writes live
		// in Camera.cpp (the DescribedCreatable chain runs first; the
		// front signal/cf slots default-construct from the base path).
		Camera();

		// mac: 0x28DD74: near plane Z (constant -0.5).
		double nearPlaneZ() const { return -0.5; }
		// mac: 0x288360: set camera pan mode.
		void setCameraPanMode(int mode) { cameraPanMode = mode; }

		// mac: 0x28E09A (11B): the +0x70 frame, by reference -- the binary body
		// is a bare this+0x70 return, so 2013 spells it header-inline.
		// STUB: PLAYER 0x006b4820
		// STUB: STUDIO 0x006f7fd0
		const G3D::CoordinateFrame& coordinateFrame() const { return cf; }

		// mac: 0x28DD88 (41B, const): the image-plane depth = rect height * zoom
		// (the projection-scale denominator the billboard chain uses).
		float getImagePlaneDepth(const G3D::Rect2D& rect) const;

		// mac: 0x28DDB2 (342B): world -> screen.  Returns {halfW + camX*scale,
		// halfH - camY*scale, scale} with scale = rect.height()*zoom/depth
		// (depth = -(col2 . rel); cam-space via the rotation-column dots).
		// Behind-plane projects to the +/- infinity corner keyed on the
		// camera-space x/y signs.
		G3D::Vector3 project(const G3D::Vector3& worldPoint,
			const G3D::Rect2D& rect) const;

		// mac: 0x2898F0 (103B): the 2-arg entry -- builds the 3-arg with
		// -farPlane and returns the out slot by value.
		RBX::Frustum frustum(const G3D::Rect2D& rect) const;
		// mac: 0x28E146 (398B, const): the frustum build body (see Camera.cpp).
		void frustum(const G3D::Rect2D& rect, float negatedFarPlane,
			RBX::Frustum& out) const;

	private:
		rbx::signals::signal<void()> signalVoid;      // +0x64 (name DELTA)
		rbx::signals::signal<void(bool)> signalBool;  // +0x68 (name DELTA)
		G3D::CoordinateFrame cf;                      // +0x70
		G3D::Matrix3 unknownA0;                       // +0xA0 (name DELTA)
		int unknownC4;                                // +0xC4
		int unknownC8;                                // +0xC8
		float unknownCC;                              // +0xCC
		G3D::CoordinateFrame preRenderTmpCFs[4];      // +0xD0..+0x18C
		int unknown190;                               // +0x190
		float unknown194;                             // +0x194
		int unknown198;                               // +0x198
		int unknown19C;                               // +0x19C
		int unknown1A0;                               // +0x1A0
		float farPlane;                               // +0x1A4
		float fov;                                    // +0x1A8
		// +0x1AC..+0x1B4 = 3 slots unmodeled in the C2 decode (queued).
		int cameraPanMode;                            // +0x1B8
		float zoom;                                   // +0x1BC
	};

} // namespace RBX

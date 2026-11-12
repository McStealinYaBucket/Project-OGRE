/* Gui/ViewportBillboarder.h -- October 2013 (billboard arc).
   Evidence: C1/C2(void) 0x8211C8/0x8211D2 (239B); C1/C2(args)
   0x8212C2/0x8212CC (mangle: const Vector3&, const Vector3&, const
   Vector2&, const UDim2&, const Vector2*); update 0x81FFF8 (2155B;
   rarc3 full decomp+raw); projectOntoScreen 0x8213B2 (814B; rarc4);
   the AdornBillboarder C2 0x81FF2A exercises update in the arcs.
   sizeof = 148 (0x94); member order binary-pinned from the update raw
   (this = esi): +0x00 result CF (48B), +0x30 Rect2D, +0x40 isValid
   byte, +0x44 low / +0x50 high (min/max accumulators; Extents.h-
   precedent name DELTA), +0x5C offset V3, +0x68 scale V3, +0x74 param
   V2, +0x7C size UDim2, +0x8C const Vector2* pivot, +0x90 flag byte.
   Non-virtual (no vfptr: update writes the result CF straight at
   +0x00; the C2(void) zero-list has no vfptr store).
   DELTAS: (1) isValid is NOT zeroed by either ctor (decode; landed
   uninitialized); (2) member names low/high/param/size/pivot and the
   +0x90 flag = behavioral (binary names unobserved); (3) update
   returns nothing (the decomp "int return" = GCC eax noise; landed
   void); (4) update's CoordinateFrame param = by value in the mangle
   (GCC accesses a caller copy; the landing is the same by-value
   semantics). */

#ifndef RBX_VIEWPORTBILLBOARDER_H
#define RBX_VIEWPORTBILLBOARDER_H

#include "G3D/CoordinateFrame.h"
#include "G3D/Rect2D.h"
#include "G3D/Vector2.h"
#include "Util/UDim.h"

namespace RBX
{

	class Camera;

	class ViewportBillboarder
	{
	public:
		ViewportBillboarder();
		ViewportBillboarder(const G3D::Vector3& v1, const G3D::Vector3& v2,
			const G3D::Vector2& param, const UDim2& size,
			const G3D::Vector2* pivot);

		// mac: 0x81FFF8 (2155B): folds the 8 box corners into +0x44/+0x50,
		// camera-projects the low/high-prescribed center, and (iff
		// projected.z in (0, 1000)) stages the screen-aligned CF that
		// lands at +0x00 (projectOntoScreen when the +0x90 flag != 0,
		// else camera.coordinateFrame() * the staged CF).  Param names
		// are behavioral: the binary reads the rect PARAM for the
		// project/projectOntoScreen calls (arg_4) and writes the MEMBER
		// rect (+0x30), so screenRect deliberately does not shadow it.
		void update(const G3D::Rect2D& screenRect, const Camera& camera,
			G3D::Vector3 boxSize, G3D::CoordinateFrame frame);

		// mac: 0x8213B2 (814B): the near-plane projection helper the +0x90
		// branch calls (returns camCF * the offset/diag-scaled frame).
		G3D::CoordinateFrame projectOntoScreen(const G3D::Rect2D& rect,
			const Camera& camera, const G3D::CoordinateFrame& frame);

		G3D::CoordinateFrame cframe;     // +0x00 (the result CF)
		G3D::Rect2D rect;                // +0x30
		bool isValid;                    // +0x40 (uninitialized; DELTA (1))
		G3D::Vector3 low;                // +0x44 (min accumulator; init maxFinite)
		G3D::Vector3 high;               // +0x50 (max accumulator; init -maxFinite)
		G3D::Vector3 offset;             // +0x5C
		G3D::Vector3 scale;              // +0x68
		G3D::Vector2 param;              // +0x74 (DELTA (2))
		UDim2 size;                      // +0x7C (pinned by the C2 args mangle)
		const G3D::Vector2* pivot;       // +0x8C
		bool projectOntoScreenEnabled;   // +0x90 (name DELTA (2))
	};

}  // namespace RBX

#endif

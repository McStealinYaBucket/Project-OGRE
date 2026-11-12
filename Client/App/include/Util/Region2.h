/* Util/Region2.h -- October 2013 (minimal: WeightedPoint only).
   Carried from March 2016 App/include/util/Region2.h (first-party): the
   WeightedPoint nested struct (Vector2 + radius, 12B) for SpatialFilter's
   point array. Full Region2 (owner/others/findCloserOther) queues behind
   the region system (section 123+). */

#pragma once

#include "G3D/Vector2.h"

namespace RBX {

	class Region2 {
	public:
		class WeightedPoint {
		public:
			G3D::Vector2 point;
			float radius;

			WeightedPoint()
				: point(G3D::Vector2::zero())
				, radius(0.0f)
			{}

			WeightedPoint(const G3D::Vector2& p, float r)
				: point(p)
				, radius(r)
			{}
		};
	};

} // namespace RBX

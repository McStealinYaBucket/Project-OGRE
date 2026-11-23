/* Util/G3DCore.h -- October 2013 reconstruction.
   Brings the G3D 8.00 math types into namespace RBX under their short names
   (RBX::Vector3 is G3D::Vector3, and so on; Ray is RbxG3D's RbxRay). The
   Mac/iOS symbols spell every such parameter G3D::..., which is what a
   typedef (not a wrapper class) produces. Also IntersectResult, and the
   boost::hash hooks for Vector3 and Vector3int16 that unordered containers
   of those keys find by ADL. The G3D include set is the one App needs;
   order kept as the prior tree had it. */
#ifndef RBX_UTIL_G3DCORE_H
#define RBX_UTIL_G3DCORE_H

#include "G3D/Vector2.h"
#include "G3D/Vector3.h"
#include "G3D/Vector4.h"
#include "G3D/Matrix3.h"
#include "G3D/Matrix4.h"
#include "G3D/Vector3int16.h"
#include "G3D/Vector2int16.h"
#include "G3D/Color4uint8.h"
#include "G3D/Color3uint8.h"
#include "G3D/CoordinateFrame.h"
#include "G3D/Plane.h"
#include "G3D/Line.h"
#include "G3D/LineSegment.h"

#include "G3D/AABox.h"
#include "G3D/Box.h"
#include "RbxG3D/RbxCamera.h"
#include "G3D/Color3.h"
#include "G3D/Color4.h"
#include "G3D/g3dmath.h"
#include "G3D/Rect2D.h"
#include "G3D/Sphere.h"

#include "G3D/vectorMath.h"
#include "G3D/Debug.h"

namespace RBX {

// vectors
typedef G3D::Vector2 Vector2;
typedef G3D::Vector3 Vector3;
typedef G3D::Vector4 Vector4;
typedef G3D::Vector2int16 Vector2int16;
typedef G3D::Vector3int16 Vector3int16;

// matrices and frames
typedef G3D::Matrix3 Matrix3;
typedef G3D::Matrix4 Matrix4;
typedef G3D::CoordinateFrame CoordinateFrame;

// colours
typedef G3D::Color3 Color3;
typedef G3D::Color4 Color4;
typedef G3D::Color3uint8 Color3uint8;
typedef G3D::Color4uint8 Color4uint8;

// geometry
typedef RbxRay Ray;
typedef G3D::Plane Plane;
typedef G3D::Line Line;
typedef G3D::LineSegment LineSegment;
typedef G3D::Rect2D Rect2D;
typedef G3D::Box Box;
typedef G3D::AABox AABox;
typedef G3D::Sphere Sphere;

enum IntersectResult { irNone = 0, irPartial = 1, irFull = 2 };

} // namespace RBX

namespace G3D {
	std::size_t hash_value(const Vector3& v);
	std::size_t hash_value(const Vector3int16& v);
}

#endif

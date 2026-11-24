/* V8World/Geometry.h -- October 2013 (collision geometry polys).
   Evidence: Primitive::newGeometry 0x58F616 factory map (type->class/size:
     1 Ball 0x14 / 0,2,3,def Block 0x2C (NO Cylinder class) / 4 Wedge 0x20 /
     5 Prism 0x28 / 6 Pyramid 0x28 / 7 ParallelRamp 0x20 / 8 RightAngleRamp 0x20 /
     9 CornerWedge 0x20 / 10 MegaCluster 0x28); base members +0 vptr, +4..12
     size, +16 centerToCornerDistance; getGeometryType/getCollideType per-class
     (Ball 1/1, Block 2/2, Poly 3); getFaceFromLegacyNormalId slot 17: base
     identity 0x5886C0 + CornerWedge[0,-1,1,2,3,4] + Wedge[2,1,0,3,4,1] +
     RightAngleRamp (n-1)>4?-1:T[n] T[1..5]=[1,2,3,0,5] (all 2016-cross-checked).
   DELTAS (all marked): Block pool/vertices/mesh, Prism/Pyramid numSides/mesh/
     face-index, Poly intermediate skipped (concretes derive Geometry),
     getCenterToCorner mesh variants + buildMesh slot30, FacesOverlapped paths,
     setSize propagation, bullet slots; our GeometryType numbering matches
     binary 1:1 (minus tail).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Geometry.h (path inferred) */

#pragma once

#include "G3D/Vector3.h"
#include "Util/NormalId.h"
#include "rbx/Debug.h"

namespace RBX {

	class Geometry
	{
	public:
		// Same values as Primitive's; 2013 asserts spell Geometry::GEOMETRY_MEGACLUSTER (ContactManager.cpp:534).
		typedef enum {
			GEOMETRY_UNDEFINED = 0, GEOMETRY_BALL, GEOMETRY_BLOCK, GEOMETRY_CYLINDER, GEOMETRY_WEDGE, GEOMETRY_PRISM,
			GEOMETRY_PYRAMID, GEOMETRY_PARALLELRAMP, GEOMETRY_RIGHTANGLERAMP, GEOMETRY_CORNERWEDGE, GEOMETRY_MEGACLUSTER,
			GEOMETRY_SMOOTHCLUSTER, GEOMETRY_TRI_MESH
		} GeometryType;
		Geometry()
			: size(G3D::Vector3::zero())
			, centerToCornerDistance(0.0f)
		{}
		virtual ~Geometry() {}

		// 2013 vftable (Geometry 0x00b56444, Block 0x00b57394, Ball 0x00b5670c; mac names):
		virtual int getGeometryType() const = 0;		// vtable+0x04
		virtual int getCollideType() const = 0;		// vtable+0x08
		const G3D::Vector3& getSize() const { return size; }
		// vtable+0x0c, base 0x00753630 (Block 0x00860830, Ball 0x008fb710 override).
		virtual void setSize(const G3D::Vector3& value) { size = value; }
		void setSizeNonVirtual(const G3D::Vector3& value) { size = value; }
		// vtable+0x10 setGeometryParameter, +0x14 getGeometryParameter (bases 0x00753650/0x00753690),
		// +0x18..+0x3c pure in Geometry (getRadius ... vertOverlapsFace): placeholders until modelled.
		virtual void VTable0x10() {}
		virtual void VTable0x14() {}
		virtual void VTable0x18() {}
		virtual void VTable0x1c() {}
		virtual void VTable0x20() {}
		virtual void VTable0x24() {}
		virtual void VTable0x28() {}
		virtual void VTable0x2c() {}
		virtual void VTable0x30() {}
		virtual void VTable0x34() {}
		virtual void VTable0x38() {}
		virtual void VTable0x3c() {}
		// vtable+0x40 (0x5886C0 base identity).
		// FUNCTION: PLAYER 0x004901e0
		// FUNCTION: STUDIO 0x0051fb70
		virtual int getFaceFromLegacyNormalId(NormalId nId) const { return (int)nId; }
		// vtable+0x44; base `mov al,1` (0x006be550, folded). The sloped polys answer false.
		virtual bool isGeometryOrthogonal() const { return true; }

	protected:
		G3D::Vector3 size;          // +4..12
		float centerToCornerDistance;  // +16
	};

	class BallGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 1; }  // BALL
		virtual int getCollideType() const { return 1; }
		// FacesOverlapped always false + assert (Ball.h:54) — see note below.
	};

	class BlockGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 2; }  // BLOCK (also for type-3 input)
		virtual int getCollideType() const { return 2; }
		// Pool Token + vertices + mesh TODO (Block::setSize/D2).
	};

	class WedgePolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 4; }
		virtual int getCollideType() const { return 3; }
		virtual bool isGeometryOrthogonal() const { return false; }
		virtual int getFaceFromLegacyNormalId(NormalId nId) const
		{
			static const int t[6] = { 2, 1, 0, 3, 4, 1 };
			int n = (int)nId;
			return (n >= 0 && n < 6) ? t[n] : -1;
		}
	};

	class PrismPolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 5; }
		virtual int getCollideType() const { return 3; }
		virtual bool isGeometryOrthogonal() const { return false; }
		// numSides/numFaces/mesh + face tables TODO (needs mesh module).
	};

	class PyramidPolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 6; }
		virtual int getCollideType() const { return 3; }
		virtual bool isGeometryOrthogonal() const { return false; }
		// Same TODO as Prism (no NORM_Y case).
	};

	class ParallelRampPolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 7; }
		virtual int getCollideType() const { return 3; }
		virtual bool isGeometryOrthogonal() const { return false; }
		// Inherits base identity (no override in binary).
	};

	class RightAngleRampPolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 8; }
		virtual int getCollideType() const { return 3; }
		virtual bool isGeometryOrthogonal() const { return false; }
		virtual int getFaceFromLegacyNormalId(NormalId nId) const
		{
			static const int t[6] = { -1, 1, 2, 3, 0, 5 };
			int n = (int)nId;
			if (n < 0 || n > 5)
				return -1;
			return t[n];
		}
	};

	class CornerWedgePolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 9; }
		virtual int getCollideType() const { return 3; }
		virtual bool isGeometryOrthogonal() const { return false; }
		virtual int getFaceFromLegacyNormalId(NormalId nId) const
		{
			static const int t[6] = { 0, -1, 1, 2, 3, 4 };
			int n = (int)nId;
			return (n >= 0 && n < 6) ? t[n] : -1;
		}
	};

	class MegaClusterPolyGeometry : public Geometry
	{
	public:
		virtual int getGeometryType() const { return 10; }
		virtual int getCollideType() const { return 3; }
		virtual int getFaceFromLegacyNormalId(NormalId) const { return -1; }  // 0x696378
	};

} // namespace RBX

/* Copyright 2003-2007 ROBLOX Corporation, All Rights Reserved */

#pragma once

#include "Util/NormalId.h"
#include "Util/G3DCore.h"
#include "rbx/Debug.h"
#include "RbxG3D/RbxRay.h"
#include <limits>

namespace RBX {

	inline int fastFloorInt(float value)
	{
		return value < 0 ? static_cast<int>(value - 0.999f) : static_cast<int>(value);
	}

	inline int fastCeilInt(float value)
	{
		return value < 0 ? static_cast<int>(value) : static_cast<int>(value + 0.999f);
	}

	inline Vector3int16 fastFloorInt16(const Vector3& v)
	{
		return Vector3int16(fastFloorInt(v.x), fastFloorInt(v.y), fastFloorInt(v.z));
	}

	typedef enum  {
		AXIS_X = 0,
		AXIS_Y = 1,
		AXIS_Z = 2
	} AxisIndex;

	namespace Math
	{
		inline double pi()		{return 3.14159265358979323846;}
		inline double piHalf()	{return pi() * 0.5f;}
		inline double twoPi()	{return pi() * 2.0f;}
		inline float pif()		{return static_cast<float>(pi());}
		inline float piHalff()	{return static_cast<float>(piHalf());}
		inline float twoPif()	{return static_cast<float>(twoPi());}
		inline const float& inf() {
			static const float i = std::numeric_limits<float>::infinity();
			return i;
		}

		// Returns the 0-based most-significant bit (-1 if v is 0)
		inline size_t computeMSB(size_t v)
		{
			size_t msb = -1;
			while (v>0)
			{
				v >>= 1;
				++msb;
			}
			return msb;
		}

		inline int iRound(float value) {
			return G3D::iRound(value);
		}
		inline int iFloor(float value) {
			return G3D::iRound(::floor(value));
		}
		inline float polarity(float value) {
			return  (value >= 0.0f) ? 1.0f : -1.0f;
		}
		inline float sign(float value) {
			return  (value > 0.0f)
						? 1.0f
						: (value < 0.0f ? -1.0f : 0.0f);
		}

		////////////////////////////////////////////////////////
		//
		// Denormalized detection
		// RBX::Math::isDenormal 0x22CAD4, isNanInf 0x22CAF7,
		// isNanInfVector3 0x22CB24, isNanInfDenormVector3 0x22CB85,
		// hasNanOrInf(CoordinateFrame) 0x22CBE3, fixDenorm 0x22CC5A

		bool isDenormal(float f);
		bool isNanInf(float f);
		bool isNanInfVector3(const Vector3& v);
		bool isNanInfDenormVector3(const Vector3& v);
		bool hasNanOrInf(const CoordinateFrame& c);
		bool hasNanOrInf(const Matrix3& m);	// inlined helper in the binary (0x22CBE3 area)

		// Sets denormalized values to 0.0
		bool fixDenorm(float& f);
		bool fixDenorm(Vector3& v);

		////////////////////////////////////////////////////////
		//
		// fuzzyEq stuff
		inline float epsilonf() { return 1.0e-6f; }
		inline bool fuzzyEq(float a, float b, float epsilon) {
			float aa = fabsf(a) + 1.0f;
		    return (a == b) || (fabsf(a - b) <= (aa * epsilon));
		}

		inline bool fuzzyEq(double a, double b, double epsilon) {
			double aa = fabs(a) + 1.0f;
		    return (a == b) || (fabs(a - b) <= (aa * epsilon));
		}

		// RBX::Math::fuzzyEq 0x22E4E1 (Vector3), 0x22E545 (Matrix3),
		// mac: 0x22E5D0 (Matrix4), 0x22E65B (CoordinateFrame), fuzzyAxisAligned 0x22E381
		bool fuzzyEq(const Vector3& v0, const Vector3& v1, float epsilon = 1.0e-5f);	// Note G3D::eps == 1e-6;
		bool fuzzyEq(const Matrix3& m0, const Matrix3& m1, float epsilon = 1.0e-5f);	// Note G3D::eps == 1e-6;
		bool fuzzyEq(const Matrix4& m0, const Matrix4& m1, float epsilon = 1.0e-5f);	// Note G3D::eps == 1e-6;
		bool fuzzyEq(const CoordinateFrame& c0, const CoordinateFrame& c1, float epsT = 1.0e-5f, float epsRad = 1.0e-5f);
		bool fuzzyAxisAligned(const Matrix3& m0, const Matrix3& m1, float radTolerance);

		////////////////////////////////////////////////////////
		//
		// odd / even stuff
		inline bool isEven(int value) {
			return ((value % 2) == 0);
		}

		inline bool isOdd(int value) {
			return ((value % 2) != 0);
		}

		inline int nextEven(int value) {
			return (value + 1 + ((value + 1) % 2));
		}

		inline int nextOdd(int value) {
			return (value + 1 + (value % 2));
		}

		////////////////////////////////////////////////////////
		//
		// Vector2 stuff
		inline Vector2 expandVector2(const Vector2& v, int expand) {
			Vector2 answer(v);
			for (int i = 0; i < 2; ++i) {
				answer[i] += expand * RBX::Math::sign(v[i]);
			}
			return answer;
		}

        inline Vector2 roundVector2(const Vector2& v) {
            return Vector2(iRound(v.x), iRound(v.y));
        }

		////////////////////////////////////////////////////////
		//
		// Vector3 stuff

		// RBX::Math::iRoundVector3 0x22DF8D, angle 0x22E2AE, elevationAngle 0x22E31E,
		// vector3Abs 0x22E91B, sortVector3 0x22E8B3, safeDirection 0x22E113 (Math.cpp:811/814),
		// toGrid 0x22DF0A / 0x22E003, lessThan 0x22CAA4, sumDeltaAxis 0x22C54D,
		// closestPointOnRay 0x22E95B
		Vector3 iRoundVector3(const Vector3& point);
		float angle(const Vector3& v0, const Vector3& v1);
		float elevationAngle(const Vector3& look);
		Vector3 vector3Abs(const Vector3& v);
		Vector3 sortVector3(const Vector3& v);
		Vector3 safeDirection(const Vector3& v);	// handles case where V == vector3::zero();
		Vector3 toGrid(const Vector3& v, const Vector3& grid);
		Vector3 toGrid(const Vector3& v, float grid);
		bool lessThan(const Vector3& min, const Vector3& max);
		inline float longestVector3Component(const Vector3& v) {
			return std::max(fabs(v.x), std::max(fabs(v.y), fabs(v.z)));
		}
		inline float planarSize(const Vector3& v) {
			return (v.x < v.y)
				? ((v.x < v.z) ? v.y * v.z : v.y * v.x)
				: ((v.y < v.z) ? v.x * v.z : v.x * v.y);
		}
		inline float taxiCabMagnitude(const Vector3& v) {
			return fabs(v.x) + fabs(v.y) + fabs(v.z);
		}
		float sumDeltaAxis(const Matrix3& r0, const Matrix3& r1);
		Vector3 closestPointOnRay(const RBX::RbxRay& pointOnRay, const RBX::RbxRay& otherRay);


		////////////////////////////////////////////////////////
		//
		// Manipulate/rotate Matrix3 and CoordinateFrame

		// RBX::Math::rotateAboutYGlobal 0x22E752 (Vector3) / 0x22E6DB (CoordinateFrame),
		// snapToAxes 0x22DBC3, isOrthonormal 0x22E4C8, orthonormalizeIfNecessary 0x22D648,
		// toDiagonal 0x22D25B
		Vector3 rotateAboutYGlobal(const Vector3& v, float radians);
		Matrix3 snapToAxes(const Matrix3& matrix);
		bool isOrthonormal(const Matrix3& m);
		bool orthonormalizeIfNecessary(Matrix3& m);	// true if an orthonormalize was necessary

		Vector3 toDiagonal(const Matrix3& m);

		inline Matrix3 fromDiagonal(const Vector3& v) {
			return Matrix3(	v[0],	0.0f,	0.0f,
							0.0f,	v[1],	0.0f,
							0.0f,	0.0f,	v[2]	);
		}

		// Return the skew symmetric matrix for the the given vector.
		//	a.cross( b ) = A_* b where A_is the skew symmetric matrix for a.
		//
		inline Matrix3 toSkewSymmetric(const Vector3& v) {
			return Matrix3(	0.0f,	-v.z,	v.y,
							v.z,	0.0f,	-v.x,
							-v.y,	v.x,	0.0f	);
		}

		// RBX::Math::fromVectorToVectorRotation 0x22D284, fromRotationAxisAndAngle 0x22D45B,
		// fromDirectionCosines 0x22D674
		Matrix3 fromVectorToVectorRotation( const Vector3& fromVec, const Vector3& toVec );
		Matrix3 fromRotationAxisAndAngle( const Vector3& axis, const float& angleRads );
		Matrix3 fromDirectionCosines( const Vector3& fromX, const Vector3& fromY, const Vector3& fromZ,
									 const Vector3& toX, const Vector3& toY, const Vector3& toZ );

		inline Vector3 getColumn(const Matrix3& m, int iCol) {
			RBXASSERT_VERY_FAST((0 <= iCol) && (iCol < 3));
			return Vector3(m[0][iCol], m[1][iCol], m[2][iCol]);
		}

		// RBX::Math::mulMatrixDiagVector 0x22C5CE, mulMatrixMatrixTranspose 0x22C65E
		void mulMatrixDiagVector(const Matrix3& _mat, const Vector3& _vec, Matrix3& _answer);
		void mulMatrixMatrixTranspose(const Matrix3& _m0, const Matrix3& _m1, Matrix3& _answer);

		// Byte Angles
		// RBX::Math::rotationToByte 0x22CEBA, rotationFromByte 0x22CFB2
		unsigned char rotationToByte(float angle);
		float rotationFromByte(unsigned char byteAngle);

		// Axis Aligned Matrix / OrientId
		// RBX::Math::isAxisAligned 0x22D892, getOrientId 0x22D973, idToMatrix3 0x22D9D0
		static const int maxOrientationId = 36;
		static const int minOrientationId = 0;
		bool isAxisAligned(const Matrix3& matrix);
		int getOrientId(const Matrix3& matrix);
		void idToMatrix3(int orientId, Matrix3& matrix);

		// RBX::Math::matrixRotateX 0x22EAC3, matrixRotateY 0x22EB6A,
		// matrixTiltZ 0x22EC14, matrixTiltNegativeZ 0x22ECBB,
		// matrixTiltQuadrant 0x22ED62 (Math.cpp:1272), rotateAboutZ 0x22DAD8
		const Matrix3& matrixRotateX();
		const Matrix3& matrixRotateY();
		const Matrix3& matrixTiltZ();
		const Matrix3& matrixTiltNegativeZ();
		const Matrix3 matrixTiltQuadrant(int quadrant);
		Matrix3 rotateAboutZ(const Matrix3& matrix, float radians);
		// RBX::Math::getWellFormedRotForZVector 0x23053A,
		// momentToObjectSpace 0x22D1B7, momentToWorldSpace 0x22D209,
		// getIWorldAtPoint 0x22CFEA, getIBodyAtPoint 0x22D0D9
		Matrix3 getWellFormedRotForZVector(const Vector3& vec);
		Matrix3 momentToObjectSpace(const Matrix3& iWorld, const Matrix3& bodyRotation);
		Matrix3 momentToWorldSpace(const Matrix3& iBody, const Matrix3& bodyRotation);
		Matrix3 getIWorldAtPoint(const Vector3& cofmPos,
							   const Vector3& worldPos,
							   const Matrix3& iWorldAtCofm,
							   float mass);
		Matrix3 getIBodyAtPoint(const Vector3& pos,
							const Matrix3& iBody,
							float mass);

		// CoordinateFrame
		// RBX::Math::rotateAboutYGlobal(CoordinateFrame&) 0x22E6DB,
		// snapToGrid 0x22E03D (float) / 0x22E0B2 (Vector3)
		void rotateAboutYGlobal(CoordinateFrame& c, float radians);
		CoordinateFrame snapToGrid(const CoordinateFrame& snap, float grid);
		CoordinateFrame snapToGrid(const CoordinateFrame& snap, const Vector3& grid);
		// http://www.vlfeat.org/api/mathop_8h-source.html#l00227
		inline float atan2Fast(float y, float x) {
			float angle, r;
			float const c3 = 0.1821f;
			float const c1 = 0.9675f;
			float abs_y = fabsf(y) + 1.19209290e-07f;
			if (x >= 0) {
				r = (x - abs_y) / (x + abs_y) ;
				angle = Math::pif() / 4.0f;
			} else {
				r = (x + abs_y) / (abs_y - x) ;
				angle = 3.0f * Math::pif() / 4.0f ;
			}
			angle += (c3*r*r - c1) * r ;
			return (y < 0) ? - angle : angle ;
		}

		inline float zAxisAngle(const Matrix3& matrix) {
			Vector3 look = matrix.column(0);
			float angle = (float) atan2(look.y, look.x);
			return angle;
		}

		// Pitch, Yaw stuff - replaces Euler Angles
		// RBX::Math::radiansToQuadrant 0x22EE98 (Math.cpp:1282),
		// toYAxisQuadrant 0x22EF54, getHeadingElevation 0x22C94E, setHeadingElevation 0x22C9C6,
		// getFocusSpace 0x22C8EA
		int radiansToQuadrant(float radians);
		inline float radiansToDegrees(float radians)	{return radians * (180.0f / Math::pif());}
		inline float degreesToRadians(float degrees)	{return degrees * (Math::pif() / 180.0f);}

		/**
		 Returns the heading as an angle in radians, where
		north is 0 and west is PI/2
		North == -z

		Elevation is angle above (+) or below(-) horizon
		 */
		inline float getHeading(const Vector3& look) { return atan2( -look.x, -look.z); }
		inline float getElevation(const Vector3& look) { return asin(look.y); }

		void getHeadingElevation(const CoordinateFrame& c, float& heading, float& elevation);
		void setHeadingElevation(CoordinateFrame& c, float heading, float elevation);

		CoordinateFrame getFocusSpace(const CoordinateFrame& focus);

		int toYAxisQuadrant(const CoordinateFrame& c);		// 0..3

		// NormalId stuff
		// RBX::Math::getClosestObjectNormalId 0x22E7DE
		NormalId getClosestObjectNormalId(const Vector3& worldV, const Matrix3& objectR);

		inline Vector3 getWorldNormal(NormalId objId, const Matrix3& objectR) {
			int column = objId % 3;
			int polarity = ((objId / 3) * (-2)) + 1;
			return polarity * getColumn(objectR, column);
		}

		inline Vector3 getWorldNormal(NormalId objId, const CoordinateFrame& objectC) {
			return getWorldNormal(objId, objectC.rotation);
		}

		// wraps from -pi to pi
		// RBX::Math::deltaRotationClose 0x22C6CA, averageRotationClose 0x22C7C6
		// (asserts "include/Util/Math.h line: 346" - radWrap below)
		float deltaRotationClose(float aRot, float bRot);		// computes aRot - bRot, assuming angles are close.  Undoes wrapping
		float averageRotationClose(float aRot, float bRot);		// computes average aRot, bRot, assuming angles are close.  Undoes wrapping

		// -3pi to -pi: -1
		// -pi to pi: 0
		// pi to 3pi: 1
		inline double windingPart(double rad) {
			return ::floor((rad + pi()) / twoPi()) ;
		}

		inline float radWrap(double rad) {			// extra part
			if ((rad >= -pi()) && (rad < pi())) {
				return static_cast<float>(rad);
			}
			double answer = rad - (twoPi() * windingPart(rad));
			RBXASSERT((answer >= -pi()) && (answer <= pi()));
			return static_cast<float>(answer);
		}

	    // Matrix operations
		// RBX::Math::getAxisRotationMatrix 0x22FE40 (Math.cpp:1521)
		const Matrix3& getAxisRotationMatrix(int face);

		// Vector to Object Space
		// == mat.transpose() * vec
		inline Vector3 vectorToObjectSpace(const Vector3& vec, const Matrix3& mat);

		// Ray, Line
		// RBX::Math::intersectLinePlane 0x22FDC2, intersectRayPlane 0x22F274 (Math.cpp:1420),
		// intersectRayConvexPolygon 0x22EF9E (Math.cpp:1308),
		// lineSegmentDistanceIfCrossing 0x230024,
		// spatialPolygonIntersection 0x22F408 (Math.cpp:1343),
		// planarPolygonIntersection 0x22FC19
		bool intersectLinePlane(const Line& line, const Plane& plane, Vector3& hit);
		bool intersectRayPlane(const RbxRay& ray, const Plane& plane, Vector3& hit);
        bool intersectRayConvexPolygon(const RBX::RbxRay& ray,  const std::vector<Vector3>& poly, Vector3& hit, bool oneSided);
		bool lineSegmentDistanceIfCrossing(const Vector3& line1Pt1, const Vector3& line1Pt2, const Vector3& line2Pt1, const Vector3& line2Pt2, float& distance, float adjustEdgeTol = 0.0f);
        std::vector<Vector3> spatialPolygonIntersection(const std::vector<Vector3>& polyA,  const std::vector<Vector3>& polyB);
		std::vector<Vector2> planarPolygonIntersection(const std::vector<Vector2>& poly1, const std::vector<Vector2>& poly2);


		// Misc.
		// RBX::Math::polygonStartingPoint 0x230376, evenWholeNumberFuzzy 0x230798
		Vector2 polygonStartingPoint(int numSides, float maxWidth);
		bool evenWholeNumber( const float& rawInput );
		bool evenWholeNumberFuzzy( const float& rawInput );
	}
}	// namespace

#include "Math.inl"
